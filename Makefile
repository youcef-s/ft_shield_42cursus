NAME = ft_shield

SRC =	ft_shield.c \
		server.c \
		shell.c \
		sha256.c


FLAGS =  #-Wall -Werror -Wextra
CC = gcc

INC =	ft_shield.h


OBJ = $(SRC:.c=.o)

all : $(NAME) clean

$(NAME) : $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o : %.c $(INC)
	$(CC) $(FLAGS) -o $@ -c $<

clean :
	rm -f $(OBJ)

fclean : clean
	rm -f $(NAME)

re : fclean all
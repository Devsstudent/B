NAME=B
PARSER_GENERATOR=bison
LEXER=flex
CC=gcc


all: $(NAME)

$(NAME):
	$(PARSER_GENERATOR) $(NAME).y --defines=$(NAME).h -o $(NAME).c
	$(CC) -c -Wall -Werror -Wextra $(NAME).c
	$(LEXER) -o $(NAME)_flex.c $(NAME)_flex.l
	$(CC) -c -Wall -Werror -Wextra $(NAME)_flex.c
	$(CC) -Wall -Wextra -Werror $(NAME).o $(NAME)_flex.o -o $(NAME)

clean:
	rm -rf *.c
	rm -rf *.o
	rm -rf *.h

fclean: clean
	rm $(NAME)

re: fclean all


.PHONY:
	all re clean flcean

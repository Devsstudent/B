NAME=B_2
PARSER_GENERATOR=bison
LEXER=flex
CC=gcc
Y_FILE=$(NAME).y
L_FILE=$(NAME)_flex.l
Y_C=$(NAME).c
Y_H=$(NAME).h
L_C=$(NAME)_flex.c
OBJ= $(addprefix ./obj/, scope.o \
 statement_utils.o \
 tsearch.o \
 llvm_utils.o \
 bison_utils.o \
 building.o \
 new.o \
 assignation.o)
D_LST = $(addprefix obj/, scope.d \
 statement_utils.d \
 tsearch.d \
 llvm_utils.d \
 bison_utils.d \
 building.d \
 new.d \
 assignation.d \
 $(NAME).d \
 $(NAME)_flex.d)
HEADER=-I $(PWD) -I ./include -I/opt/homebrew/opt/llvm/include
LIB=-L /opt/homebrew/opt/llvm/lib -lLLVM-C
FLAG=-Wall -Werror -Wextra -g



all: $(NAME)

$(NAME): $(NAME).o $(NAME)_flex.o $(OBJ)
	$(CC) $(FLAG) -fsanitize=address $(LIB) $(NAME).o $(NAME)_flex.o $(OBJ) -o $(NAME)

# Parser generation
$(NAME).c $(NAME).h: $(NAME).y
	$(PARSER_GENERATOR) $(NAME).y --defines=$(NAME).h -o $(NAME).c

$(NAME).o: $(NAME).c $(NAME).h
	$(CC) -c $(FLAG) $(HEADER) $(NAME).c

# Lexer generation
$(NAME)_flex.c: $(NAME)_flex.l
	$(LEXER) -o $(NAME)_flex.c $(NAME)_flex.l

$(NAME)_flex.o: $(NAME)_flex.c
	$(CC) -c $(FLAG) $(HEADER) $(NAME)_flex.c

obj/%.o: src/%.c | object
	$(CC) -c $(FLAG) $(HEADER) $< -o $@

object:
	@mkdir -p obj

clean:
	rm -rf *.o
	rm -rf $(NAME).h $(NAME).c $(NAME)_flex.c
	rm -rf $(NAME).output $(NAME).tab.c $(NAME).tab.h $(NAME)_flex.o $(NAME).o

fclean: clean
	rm $(NAME)
	rm -rf obj

re: fclean all



.PHONY:
	all re clean fclean parser lexer

-include $(D_LST)

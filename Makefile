CC     = cc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -Iinclude
SRC    = ast.c    \
         gen.c    \
         lexer.c  \
         main.c   \
         parser.c \

.PHONY: all clean

all: void

void: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o void

clean:
	rm -f void
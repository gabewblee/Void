CC     = cc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -Ibackend -Ilexicon -Isemantic -Isyntax
SRC    = backend/gen.c       \
         lexicon/lexer.c     \
         semantic/resolver.c \
         semantic/symtbl.c   \
         semantic/type.c     \
         syntax/ast.c        \
         syntax/parser.c     \
         main.c

.PHONY: all clean

all: void

void: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o void

clean:
	rm -f void

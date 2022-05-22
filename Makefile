.prevent_execution:
	exit 0

CC = gcc
FLEX = flex
BISON = bison

PROGRAMS = mysh

all: $(PROGRAMS)

grammar.tab.c grammar.tab.h: grammar.y ast.c ast.h ast.c
	$(BISON) -t -v -d grammar.y

lexer.yy.c: lexer.l grammar.tab.h
	$(FLEX) -o lexer.yy.c lexer.l

# TODO - separate compile and link steps
mysh: lexer.yy.c grammar.tab.c grammar.tab.h launcher.c parser.c reader.c mysh.c ast.c ast.h
	$(CC) -Wall -Wextra -o mysh grammar.tab.c lexer.yy.c launcher.c parser.c reader.c mysh.c -lreadline

clean:
	rm -rf *.output *.tab.c *.tab.h *.yy.c mysh
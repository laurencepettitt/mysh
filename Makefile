.prevent_execution:
	exit 0

CC = gcc
FLEX = flex
BISON = bison

PROGRAMS = mysh

all: $(PROGRAMS)

parser.tab.c parser.tab.h:	parser.y
	$(BISON) -t -v -d parser.y

lexer.yy.c: lexer.l parser.tab.h
	$(FLEX) -o lexer.yy.c lexer.l

# TODO - separate compile and link steps
mysh: lexer.yy.c parser.tab.c parser.tab.h launcher.c parser.c reader.c mysh.c
	$(CC) -Wall -Wextra -o mysh parser.tab.c lexer.yy.c launcher.c parser.c reader.c mysh.c -lreadline

clean:
	rm -rf *.output *.tab.c *.tab.h *.yy.c mysh
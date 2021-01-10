all: mysh

bison_parser.tab.c bison_parser.tab.h:	bison_parser.y
	bison -t -v -d bison_parser.y

lex.yy.c: flex_lexer.l bison_parser.tab.h
	flex flex_lexer.l

mysh: lex.yy.c bison_parser.tab.c bison_parser.tab.h libmyshlauncher.c libmyshparser.c libmyshreader.c mysh.c
	gcc -Wall -Wextra -o mysh bison_parser.tab.c lex.yy.c libmyshlauncher.c libmyshparser.c libmyshreader.c mysh.c -lreadline

clean:
	rm bison_parser.output bison_parser.tab.c bison_parser.tab.h lex.yy.c mysh
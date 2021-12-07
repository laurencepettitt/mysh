%option noyywrap nodefault nounput noinput

%{
    # define YYSTYPE char*
    #include "parser.tab.h"
%}

%%

#.*                 { }

[ \t]+              { }

";"                 { return ';'; }

[^;\r\n# \t]+       { yylval = strdup(yytext); return STRING; }

[\r\n]+             { }

%%

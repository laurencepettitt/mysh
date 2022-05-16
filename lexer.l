%{
    #include "grammar.tab.h"
%}
/* do not echo unmatched scanner input to stdout */
%option nodefault
/* do not call yywrap() upon end-of-file (assume that there are no more files to scan) */
%option noyywrap
/* do not generate several unnecesary functions that causes compilation warnings */
%option nounput noinput
/* turn on line numbers */
%option yylineno

%%

#.*                 { }

[ \t]+              { }

";"                 { return ';'; }

"|"                 { return '|'; }

">"                 { return '>'; }

"<"                 { return '<'; }

[^<>|;\r\n# \t]+    { yylval.str = strdup(yytext); return STRING; }

[\r\n]+             { }

%%

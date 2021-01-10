%define parse.trace
%define parse.error custom

%{
    # define YYSTYPE char*
    #include <stdio.h>
    #include <stdlib.h>

void add_arg(char *arg);
void end_expr();
%}

%token <YYSTYPE> STRING
%type <YYSTYPE> arg

%%

line: %empty { end_expr(); }
    | expr_set
    | expr_set ';'
    ;

expr_set:
    expr { end_expr(); }
    | expr_set ';' expr { end_expr(); }
    ;

expr: arg
    | expr arg
    ;

arg: STRING { add_arg(yylval); }
    ;

%%

extern int line_number;

static int yyreport_syntax_error(const yypcontext_t *ctx)
{
    int res = 0;
    fprintf(stderr, "error:%d: syntax error", line_number);


    // Report the unexpected token.
    {
        yysymbol_kind_t lookahead = yypcontext_token(ctx);
        if (lookahead != YYSYMBOL_YYEMPTY)
            fprintf (stderr, " near unexpected token %s", yysymbol_name (lookahead));
    }
    fprintf(stderr, "\n");
    return res;
}

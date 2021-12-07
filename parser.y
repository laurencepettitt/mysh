%define parse.error custom

%{
    # define YYSTYPE char*
    #include <stdio.h>
    #include <stdlib.h>

void add_arg(char *arg);
void end_expr();
int yylex();
int yyerror(const char * format);
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

    // Report the tokens expected at this point.
    {
        enum { TOKENMAX = 5 };
        yysymbol_kind_t expected[TOKENMAX];
        int n = yypcontext_expected_tokens (ctx, expected, TOKENMAX);
        if (n < 0)
        // Forward errors to yyparse.
        res = n;
        else
        for (int i = 0; i < n; ++i)
            fprintf (stderr, "%s %s",
                    i == 0 ? ": expected" : " or", yysymbol_name (expected[i]));
    }

    // Report the unexpected token.
    {
        yysymbol_kind_t lookahead = yypcontext_token(ctx);
        if (lookahead != YYSYMBOL_YYEMPTY)
            fprintf (stderr, " near unexpected token %s", yysymbol_name (lookahead));
    }
    fprintf(stderr, "\n");
    return res;
}

%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.c"

// remove implicit declaration warnings
void yyerror(struct list_list_t **parse_result, char *);
int yylex();
%}
%debug
%define parse.error custom
%locations
%parse-param {struct list_list_t **parse_result}
%code requires {
#include "ast.h"
}

%union {
    char *str;
    struct arg_list_t arg_list_data;
    Redir redir_data;
    struct cmd_list_t pipeline_data;
    struct list_list_t list_data;
}
%type <str> arg file
%type <redir_data> redirs in_redir out_redir redir out_redir_append
%type <arg_list_data> redirs_args cmd
%type <pipeline_data> pipeline
%type <list_data> list
%token STRING
%token EOL
%%

// line is a list optionally followed with a semicolon
line:
    %empty { *parse_result = NULL; }
    | list { **parse_result = $1; }
    | list ';' { **parse_result = $1; }
    ;

// list is a list of pipeline, separated by semicolons
list:
    pipeline { $$ = init_list_list($1); }
    | list ';' pipeline { $$ = insert_list_list_elem($1, $3); }
    ;

// pipeline is a list of commands, separated by pipes
pipeline:
    cmd { $$ = init_cmd_list($1); }
    | pipeline '|' cmd { $$ = insert_cmd_list_elem($1, $3); }
    ;

// cmd is a list of args (with optional redirection(s) for input or output)
cmd:
    redirs_args { $$ = $1; }
    | redirs_args redirs { $$ = set_arg_list_redir($1, $2); }
    ;

redirs_args:
    arg { $$ = init_arg_list($1); }
    | redirs arg { $$ = set_arg_list_redir(init_arg_list($2), $1); }
    | redirs_args arg { $$ = add_arg($1, $2); }
    | redirs_args redir arg { $$ = set_arg_list_redir(add_arg($1, $3), $2); }
    ;

redirs:
    redir { $$ = $1; }
    | redirs redir { $$ = merge_redirs($1, $2); }
    ;

redir:
    in_redir { $$ = $1; }
    | out_redir_append { $$ = $1; }
    | out_redir { $$ = $1; }
    ;

in_redir:
    '<' file { $$ = init_redir($2, NULL, -1); }
    ;

out_redir:
    '>' file { $$ = init_redir(NULL, $2, 0); }
    ;

out_redir_append:
    '>' '>' file { $$ = init_redir(NULL, $3, 1); }
    ;

file:
    STRING { $$ = yylval.str; }
    ;

arg:
    STRING { $$ = yylval.str; }
    ;

%%

extern int line_number;

int yyreport_syntax_error(const yypcontext_t *ctx, struct list_list_t **parse_result)
{
    int res = 0;
    *parse_result = NULL;
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

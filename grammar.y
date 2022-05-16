%{
#include <stdlib.h>
#include <stdio.h>
#include "arg_list.c"


// remove implicit declaration warnings
void yyerror(struct listq_head **parse_result, char *);
int yylex();
//int yyerror(const char * format);
%}

%define parse.error custom
%locations
%parse-param {struct listq_head **parse_result}
%code requires {
#include "arg_list.h"
}

%union {
    char *str;
    struct arg_list_t *arg_list_p;
    struct Redirs redir_data;
    struct cmdq_head *pipeline_data;
    struct listq_head *list_data;
}
%type <str> arg file
%type <redir_data> redirs in_redir out_redir redir
%type <arg_list_p> args cmd
%type <pipeline_data> pipeline
%type <list_data> list
%token STRING
%token EOL
%%

// line is a list optionally followed with a semicolon
line:
    %empty { printf("line: empty\n"); *parse_result = NULL; }
    | list { printf("line: list\n"); *parse_result = $1; }
    | list ';' { printf("line: list ';'\n"); *parse_result = $1; }
    ;

// list is a list of pipeline, separated by semicolons
list:
    pipeline { printf("list: pipeline\n"); $$ = init_listq(alloc_listq_elem($1)); }
    | list ';' pipeline { printf("list: list ';' pipeline\n"); $$ = insert_listq_elem($1, alloc_listq_elem($3)); }
    ;

// pipeline is a list of commands, separated by pipes
pipeline:
    cmd {printf("pipeline: cmd\n");  $$ = init_cmdq(alloc_cmdq_elem($1)); }
    | pipeline '|' cmd { printf("pipeline: pipeline '|' cmd\n"); $$ = insert_cmdq_elem($1, alloc_cmdq_elem($3)); }
    ;

// cmd is a list of args (with optional redirection(s) for input or output)
cmd:
    args { $$ = $1; }
    | redirs args { $$ = set_arg_list_redir($2, $1); }
    | args redirs { $$ = set_arg_list_redir($1, $2); }
    | redirs args redirs { { $$ = set_arg_list_redir($2, merge_redirs($1, $3)); }}
    ;

redirs:
    redir { $$ = $1; }
    | redirs redir { $$ = merge_redirs($1, $2); }
    ;

redir:
    in_redir { $$ = $1; }
    | out_redir { $$ = $1; }
    ;

in_redir:
    '<' file { $$ = init_redir($2, NULL); }
    ;

out_redir:
    '>' file { $$ = init_redir(NULL, $2); }
    ;

args:
    arg { $$ = add_arg(init_arg_list(), $1); }
    | args arg { $$ = add_arg($1, $2); }
    ;

file:
    STRING { $$ = yylval.str; }
    ;

arg:
    STRING { $$ = yylval.str; }
    ;

%%

extern int line_number;

int yyreport_syntax_error(const yypcontext_t *ctx, struct listq_head **parse_result)
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

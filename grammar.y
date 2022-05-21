%{
#include <stdlib.h>
#include <stdio.h>
#include "arg_list.c"


// remove implicit declaration warnings
void yyerror(struct listq_head **parse_result, char *);
int yylex();
//int yyerror(const char * format);
%}
%debug
%define parse.error custom
%locations
%parse-param {struct listq_head **parse_result}
%code requires {
#include "arg_list.h"
}

%union {
    char *str;
    struct arg_list_t *arg_list_p;
    Redir redir_data;
    struct cmdq_head *pipeline_data;
    struct listq_head *list_data;
}
%type <str> arg file
%type <redir_data> redirs in_redir out_redir redir out_redir_append
%type <arg_list_p> args cmd
%type <pipeline_data> pipeline
%type <list_data> list
%token STRING
%token EOL
%%

// line is a list optionally followed with a semicolon
line:
    %empty { printf("line: empty\n"); }
    | list { printf("line: list\n"); *parse_result = $1; }
    | list ';' { printf("line: list ';'\n"); *parse_result = $1; }
    ;

// list is a list of pipeline, separated by semicolons
list:
    pipeline { printf("list: pipeline\n"); print_cmdq($1); struct listq_elem *e = alloc_listq_elem($1); print_listqelem(e); $$ = init_listq(e); print_listqelem(e);}
    | list ';' pipeline { printf("list: list ';' pipeline\n"); struct listq_elem *e = alloc_listq_elem($3); print_listqelem(e); $$ = insert_listq_elem($1, e); print_listqelem(e); }
    ;

// pipeline is a list of commands, separated by pipes
pipeline:
    cmd { printf("pipeline: cmd\n");  $$ = init_cmdq(alloc_cmdq_elem($1)); }
    | pipeline '|' cmd { printf("pipeline: pipeline '|' cmd\n"); struct cmdq_elem * e = alloc_cmdq_elem($3); print_cmdqelem(e); $$ = insert_cmdq_elem($1, e); print_cmdqelem(e); }
    ;

// cmd is a list of args (with optional redirection(s) for input or output)
cmd:
    args { printf("cmd: args\n"); $$ = $1; }
    | redirs args { printf("cmd: redirs args\n"); $$ = set_arg_list_redir($2, $1); }
    | args redirs { printf("cmd: args redirs\n"); $$ = set_arg_list_redir($1, $2); }
    | redirs args redirs { printf("cmd: redirs args redirs\n"); { $$ = set_arg_list_redir($2, merge_redirs($1, $3)); }}
    ;

redirs:
    redir { printf("redirs: redir\n"); $$ = $1; }
    | redirs redir { printf("redirs: redirs redir\n"); $$ = merge_redirs($1, $2); }
    ;

redir:
    in_redir { $$ = $1; }
    | out_redir { $$ = $1; }
    | out_redir_append { $$ = $1; }
    ;

in_redir:
    '<' file { printf("in_redir: '<' file\n"); $$ = init_redir($2, NULL, -1); }
    ;

out_redir:
    '>' file { printf("out_redir: '>' file\n"); $$ = init_redir(NULL, $2, 0); }
    ;

out_redir_append:
    ">>" file { printf("out_redir_append: \">>\" file\n"); $$ = init_redir(NULL, $2, 1); }
    ;

args:
    arg { printf("args: arg\n"); $$ = add_arg(init_arg_list(), $1); }
    | args arg { printf("args: args arg\n"); $$ = add_arg($1, $2); }
    ;

file:
    STRING { printf("file: STRING (%s)\n", yylval.str); $$ = yylval.str; }
    ;

arg:
    STRING { printf("arg: STRING (%s)\n", yylval.str); $$ = yylval.str; }
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

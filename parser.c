#define YYSTYPE char*

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exitcodes.h"
#include "parser.h"
#include "launcher.h"

typedef char *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer (YY_BUFFER_STATE new_buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yyparse();

typedef char *arg_t;

arg_t *arg_list = NULL;
size_t arg_list_len = 0;
size_t arg_list_cap = 0;

void init_arg_list() {
    if (arg_list != NULL)
        return;
    size_t cap = 1;
    arg_list = malloc(cap * sizeof(arg_t));
    arg_list_len = 0;
    arg_list_cap = cap;
    *arg_list = NULL;
}

void reset_arg_list() {
    arg_list = NULL;
    arg_list_len = 0;
    arg_list_cap = 0;
}

void free_args() {
    for (size_t i = 0; i < arg_list_len; ++i) {
        free(arg_list[i]);
        arg_list[i] = NULL;
    }
}

void free_arg_list() {
    free_args();
    free(arg_list);
    reset_arg_list();
}

void add_arg(arg_t arg) {
    init_arg_list();
    if (arg_list_len + 1 > arg_list_cap) {
        arg_list_cap *= 2;
        arg_list = realloc(arg_list, arg_list_cap * sizeof(arg_t));
    }
    arg_list[arg_list_len] = arg;
    arg_list_len++;
}

void end_expr() {
    add_arg(NULL);
}

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}


int parse_line_internal(const char *line) {
    reset_arg_list();
    char *bp;
    bp = yy_scan_string(line);
    yy_switch_to_buffer(bp);
    int res = yyparse();
    yy_delete_buffer(bp); // yy_scan_string creates a duplicate of line.
    if (res != 0)
        return EXIT_SYNTAX_ERROR;
    return EXIT_SUCCESS;
}

// TODO: custom yyparse to catch sigint flag
// TODO: use left recursion for semicolon delimited lists
int parse_line(const char *line)
{
    if (line == NULL)
        return launch(0, NULL);
    int res = parse_line_internal(line);
    if (res == EXIT_SUCCESS) {
        for (size_t i = 0; i < arg_list_len;) {
            size_t j;
            for (j = i; j < arg_list_len; j++) {
                if (arg_list[j] == NULL)
                    break;
            }
            int nres = launch(j - i, arg_list + i);
            if (nres == EXIT_EOF) {
                res += EXIT_EOF;
                if (i > 0)
                    res++;
                break;
            }
            res = nres;
            i = j + 1;
        }
    }
    free_arg_list();
    return res;
}

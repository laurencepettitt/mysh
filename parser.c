#define YYSTYPE char*

#include <stdio.h>
#include "arg_list.h"
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



void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}


int parse_line_internal(const char *line, struct listq_head **parse_result) {
    char *bp;
    bp = yy_scan_string(line);
    yy_switch_to_buffer(bp);
    int res = yyparse(parse_result);
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

    struct listq_head *listq_head_res = NULL;
    struct listq_head **parse_result = &listq_head_res;

    int res = parse_line_internal(line, parse_result);

    if (res != EXIT_SUCCESS)
        return res;

    struct listq_elem *l;
    struct cmdq_elem *c;
    TAILQ_FOREACH(l, *parse_result, tailq) {
        TAILQ_FOREACH(c, l->datum, tailq)
        {
            struct arg_list_t *arg_list = c->datum;
            int nres = launch(arg_list->length, arg_list->head);
            if (nres == EXIT_EOF) {
                res += EXIT_EOF;
//                if (i > 0)
//                    res++;
                break;
            }
            res = nres;
        }
    }
    // TODO: free arg list
    return res;
}

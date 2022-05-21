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
extern int yyparse(struct listq_head **parse_result);

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

int parse_line_internal(const char *line, struct listq_head **parse_result) {
    char *bp;
    bp = yy_scan_string(line);
    yy_switch_to_buffer(bp);
    extern int yydebug;
    yydebug = 0;
    int res = yyparse(parse_result);
//    yy_delete_buffer(bp); // yy_scan_string creates a duplicate of line.
    if (res != 0)
        return EXIT_SYNTAX_ERROR;
    return EXIT_SUCCESS;
}

// TODO: custom yyparse to catch sigint flag
// TODO: use left recursion for semicolon delimited lists
int parse_line(const char *line)
{
    if (line == NULL) {
        return EXIT_EOF;
    }

    struct listq_head *listq_head_res = NULL;
    struct listq_head **parse_tree = &listq_head_res;
    int res = parse_line_internal(line, parse_tree);

    struct listq_elem *eleml;
    STAILQ_FOREACH(eleml, listq_head_res, listq_entries) {
        struct cmdq_elem *elemc;
        STAILQ_FOREACH(elemc, eleml->cmds, cmdq_entries) {
            printf("pipeline elem\n");
            printf("&elem = %p\n", elemc->arg_list);
            printf("head = %s\n", elemc->arg_list->head[0]);
            printf("&out = %p\n", elemc->arg_list->redir.out_redir);
            printf("out = %s\n", elemc->arg_list->redir.out_redir ? elemc->arg_list->redir.out_redir : "NULL");
            printf("in = %s\n", elemc->arg_list->redir.in_redir ? elemc->arg_list->redir.in_redir : "NULL");
            printf("\n");
        }
    }

    if (res != EXIT_SUCCESS)
        return res;

    res = launch_list(*parse_tree);

    // TODO: free arg_list, cmdq, listq
    return res;
}

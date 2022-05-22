#define YYSTYPE char*

#include <stdio.h>
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include "exitcodes.h"
#include "parser.h"
#include "launcher.h"

typedef char *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_switch_to_buffer (YY_BUFFER_STATE new_buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yyparse(struct list_list_t **parse_result);

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

int parse_line_internal(const char *line, struct list_list_t **parse_result) {
    char *bp;
    bp = yy_scan_string(line);
    yy_switch_to_buffer(bp);
    extern int yydebug;
    yydebug = 0;
    int res = yyparse(parse_result);
    yy_delete_buffer(bp); // yy_scan_string creates a duplicate of line.
    if (res != 0)
        return EXIT_SYNTAX_ERROR;
    return EXIT_SUCCESS;
}

// TODO: custom yyparse to catch sigint flag
int parse_line(const char *line) {
    if (line == NULL)
        return EXIT_EOF;

    // Check if line is empty (size zero)
    if (line[0] == '\0')
        return EXIT_EMPTY;

    struct list_list_t _parse_tree;
    struct list_list_t *parse_tree = &_parse_tree;
    int res = parse_line_internal(line, &parse_tree);

    if (res != EXIT_SUCCESS)
        return res;

    if (parse_tree == NULL)
        return EXIT_BLANK;

    res = launch_list(*parse_tree);

    free_list_list(_parse_tree);
    return res;
}

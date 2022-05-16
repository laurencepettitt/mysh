#include "arg_list.h"

Redir init_redir(char *in_redir, char *out_redir) {
    Redir new_redir;
    new_redir.in_redir = in_redir;
    new_redir.out_redir = out_redir;
    return new_redir;
}

Redir merge_redirs(Redir prev_redir, Redir next_redir) {
    Redir new_redir = prev_redir;
    if (next_redir.in_redir != NULL)
        new_redir.in_redir = next_redir.in_redir;
    if (next_redir.out_redir != NULL)
        new_redir.out_redir = next_redir.out_redir;
    return new_redir;
}

struct arg_list_t *init_arg_list() {
    struct arg_list_t *arg_list = (struct arg_list_t*)malloc(sizeof(struct arg_list_t));
    arg_list->length = 0;
    arg_list->capacity = 1;
    arg_list->head = malloc(sizeof(arg_t));
    *(arg_list->head) = NULL;
    arg_list->redir = (struct Redirs) {NULL, NULL};
    return arg_list;
}

struct arg_list_t *add_arg(struct arg_list_t *arg_list, arg_t arg) {
    if (arg_list->length + 1 > arg_list->capacity) {
        arg_list->capacity *= 2;
        arg_list = realloc(arg_list, arg_list->capacity * sizeof(arg_t));
    }
    arg_list->head[arg_list->length] = arg;
    arg_list->length++;
    arg_list->head[arg_list->length] = NULL;
    return arg_list;
}

struct arg_list_t *set_arg_list_redir(struct arg_list_t *arg_list, Redir redir) {
    arg_list->redir = redir;
    return arg_list;
}

struct cmdq_elem mk_cmdq_elem(struct arg_list_t *datum) {
    struct cmdq_elem elem = {{0, 0}, datum};
    return elem;
}

struct cmdq_elem *alloc_cmdq_elem(struct arg_list_t *elem) {
    struct cmdq_elem *cont = (struct cmdq_elem*)malloc(sizeof(struct cmdq_elem));
    *cont = mk_cmdq_elem(elem);
    return cont;
}

struct cmdq_head *init_cmdq(struct cmdq_elem *elem) {
    struct cmdq_head *h = (struct cmdq_head*)malloc(sizeof(struct cmdq_head));
    TAILQ_INIT(h);
    TAILQ_INSERT_HEAD(h, elem, tailq);
    return h;
}

struct cmdq_head * insert_cmdq_elem(struct cmdq_head *h, struct cmdq_elem *elem) {
    TAILQ_INSERT_TAIL(h, elem, tailq);
    return h;
}

struct listq_elem mk_listq_elem(struct cmdq_head *datum) {
    struct listq_elem elem = {{0, 0}, datum};
    return elem;
}

struct listq_elem *alloc_listq_elem(struct cmdq_head *elem) {
    struct listq_elem *cont = (struct listq_elem*)malloc(sizeof(struct listq_elem));
    *cont = mk_listq_elem(elem);
    return cont;
}

struct listq_head *init_listq(struct listq_elem *elem) {
    struct listq_head *h = (struct listq_head*)malloc(sizeof(struct listq_head));
    TAILQ_INIT(h);
    TAILQ_INSERT_HEAD(h, elem, tailq);
    return h;
}

struct listq_head *insert_listq_elem(struct listq_head *h, struct listq_elem *elem) {
    TAILQ_INSERT_TAIL(h, elem, tailq);
    return h;
}
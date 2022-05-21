#include "arg_list.h"

Redir init_redir(char *in_redir, char *out_redir, int out_redir_append) {
    Redir new_redir;
    new_redir.in_redir = in_redir;
    new_redir.out_redir = out_redir;
    new_redir.out_redir_append = out_redir_append;
    printf("init_redir(\n");
    printf(" in::%s\n", in_redir ? in_redir : "NULL");
    printf(" &out::%p\n", out_redir);
    printf(" app::%i\n", out_redir_append);
    printf(" &new_redir.out::%p\n", new_redir.out_redir);
    printf(" &new_redir.in::%p\n", new_redir.in_redir);
    printf(")\n");
    return new_redir;
}

Redir merge_redirs(Redir prev_redir, Redir next_redir) {
    printf("merge_redirs\n");
    Redir new_redir = prev_redir;
    if (next_redir.in_redir != NULL)
        new_redir.in_redir = next_redir.in_redir;
    if (next_redir.out_redir != NULL) {
        new_redir.out_redir = next_redir.out_redir;
        new_redir.out_redir_append = next_redir.out_redir_append;
    }
    return new_redir;
}

struct arg_list_t *init_arg_list() {
    printf("init_arg_list\n");
    struct arg_list_t *arg_list = (struct arg_list_t*)malloc(sizeof(struct arg_list_t));
    arg_list->length = 0;
    arg_list->capacity = 1;
    arg_list->head = malloc(sizeof(arg_t));
    *(arg_list->head) = NULL;
    arg_list->redir = init_redir(NULL, NULL, -1);
    return arg_list;
}

struct arg_list_t *add_arg(struct arg_list_t *arg_list, arg_t arg) {
    printf("add_arg\n");
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
    printf("set_arg_list_redir(\n");
    printf(" in::%s\n", redir.in_redir ? redir.in_redir : "NULL");
    printf(" &out::%p\n", redir.out_redir);
    printf(" app::%i\n", redir.out_redir_append);
    printf(" &res out::%p\n", arg_list->redir.out_redir);
    printf(" &res in::%p\n", arg_list->redir.in_redir);
    printf(")\n");
    return arg_list;
}

//struct cmdq_elem mk_cmdq_elem(struct arg_list_t *arg_list) {
//    struct cmdq_elem elem = {{0, 0}, arg_list};
//    return elem;
//}

struct cmdq_elem *alloc_cmdq_elem(struct arg_list_t *elem) {
    printf("SHEN BEFORE: %p\n", elem->redir.out_redir);
    printf("SHEN BEFORE elem: %p\n", elem);
    struct cmdq_elem *cont = (struct cmdq_elem*)malloc(sizeof(struct cmdq_elem));
//    cont->tailq.tqe_prev = 0;
    cont->arg_list = elem;
//    cont->tailq.tqe_next = 0;
//    cont->arg_list->redir = elem->redir;
//    cont->arg_list->redir.out_redir = elem->redir.out_redir;
//    *cont = mk_cmdq_elem(elem);
    printf("SHEN AFTER (SAME): %p\n", elem->redir.out_redir);
    printf("SHEN AFTER: %p\n", cont->arg_list->redir.out_redir);
    printf("SHEN AFTER cont->arg_list: %p\n", cont->arg_list);
    return cont;
}

struct cmdq_head *init_cmdq(struct cmdq_elem *elem) {
    struct cmdq_head *h = (struct cmdq_head*)malloc(sizeof(struct cmdq_head));
    STAILQ_INIT(h);
    STAILQ_INSERT_TAIL(h, elem, cmdq_entries);
    return h;
}

struct cmdq_head * insert_cmdq_elem(struct cmdq_head *h, struct cmdq_elem *elem) {
    STAILQ_INSERT_TAIL(h, elem, cmdq_entries);
//    (elem)->cmdq_entries.tqe_next = NULL;
//	(elem)->cmdq_entries.tqe_prev = (h)->tqh_last;
//	*(h)->tqh_last = (elem);
//	(h)->tqh_last = &(elem)->cmdq_entries.tqe_next;
    return h;
}

//struct listq_elem mk_listq_elem(struct cmdq_head *arg_list) {
//    struct listq_elem elem = {{0, 0}, arg_list};
//    return elem;
//}

struct listq_elem *alloc_listq_elem(struct cmdq_head *elem) {
    struct listq_elem *cont = (struct listq_elem*)malloc(sizeof(struct listq_elem));
//    *cont = mk_listq_elem(elem);
    cont->cmds = elem;
    return cont;
}

struct listq_head *init_listq(struct listq_elem *elem) {
    struct listq_head *h = (struct listq_head*)malloc(sizeof(struct listq_head));
    STAILQ_INIT(h);
    STAILQ_INSERT_TAIL(h, elem, listq_entries);
    return h;
}

struct listq_head *insert_listq_elem(struct listq_head *h, struct listq_elem *elem) {
    STAILQ_INSERT_TAIL(h, elem, listq_entries);
    return h;
}

void print_listq(struct listq_head *h) {
    printf("SHEN listq: %p\n", h->stqh_first->cmds->stqh_first->cmdq_entries.stqe_next->arg_list->redir.out_redir);
}

void print_cmdq(struct cmdq_head *h) {
    printf("SHEN cmdq: %p\n", h->stqh_first->cmdq_entries.stqe_next->arg_list->redir.out_redir);
}

void print_listqelem(struct listq_elem *e) {
    printf("SHEN listqelem %p\n", e->cmds->stqh_first->cmdq_entries.stqe_next->arg_list->redir.out_redir);
}

void print_cmdqelem(struct cmdq_elem *e) {
    printf("SHEN cmdqelem %p\n", e->arg_list->redir.out_redir);
}

void print_arglist(struct arg_list_t *s) {
    printf("SHEN arglist: %p\n", s->redir.out_redir);
}
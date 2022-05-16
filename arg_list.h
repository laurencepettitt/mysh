#ifndef LIBMYSHARGLIST_H
#define LIBMYSHARGLIST_H

#include <stdlib.h>
#include <sys/queue.h>

typedef char *arg_t;

struct Redirs {
    char *in_redir;
    char *out_redir;
};
typedef struct Redirs Redir;

struct arg_list_t {
    arg_t *head;
    size_t length;
    size_t capacity;
    Redir redir;
};

struct cmdq_elem {
    TAILQ_ENTRY(cmdq_elem) tailq;
    struct arg_list_t *datum;
    /* ... */
};

TAILQ_HEAD(cmdq_head, cmdq_elem);

struct listq_elem {
    TAILQ_ENTRY(listq_elem) tailq;
    struct cmdq_head *datum;
    /* ... */
};

TAILQ_HEAD(listq_head, listq_elem);

//Redir init_redir(char *in_redir, char *out_redir);
//
//Redir merge_redirs(Redir prev_redir, Redir next_redir);
//
//struct arg_list_t *init_arg_list();
//
//struct arg_list_t *add_arg(struct arg_list_t *arg_list, arg_t arg);


//void free_args() {
//    for (size_t i = 0; i < arg_list_len; ++i) {
//        free(arg_list[i]);
//        arg_list[i] = NULL;
//    }
//}

//void free_arg_list(struct arg_list_t *arg_list) {
//    if (arg_list == NULL)
//        return;
//    free_args();
//    free(arg_list);
//    reset_arg_list();
//}
#endif
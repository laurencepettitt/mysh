#ifndef LIBMYSHARGLIST_H
#define LIBMYSHARGLIST_H

#include <stdlib.h>
#include <sys/queue.h>

typedef char *arg_t;

struct Redirs {
    char *in_redir;
    char *out_redir;
    int out_redir_append;
};
typedef struct Redirs Redir;

struct arg_list_t {
    arg_t *head;
    size_t length;
    size_t capacity;
    Redir redir;
};


struct cmd_list_t {
    struct arg_list_t *head;
    size_t length;
    size_t capacity;
};

struct list_list_t {
    struct cmd_list_t *head;
    size_t length;
    size_t capacity;
};

struct cmdq_elem {
    struct arg_list_t *arg_list;
    STAILQ_ENTRY(cmdq_elem) cmdq_entries;
};

STAILQ_HEAD(cmdq_head, cmdq_elem);

struct listq_elem {
    struct cmdq_head *cmds;
    STAILQ_ENTRY(listq_elem) listq_entries;
};

STAILQ_HEAD(listq_head, listq_elem);

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
#ifndef LIBMYSHARGLIST_H
#define LIBMYSHARGLIST_H

#include <stdlib.h>

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

struct list_list_t free_list_list(struct list_list_t l);

#endif

#include "ast.h"
#include "stdio.h"

Redir init_redir(char *in_redir, char *out_redir, int out_redir_append) {
    Redir new_redir;
    new_redir.in_redir = in_redir;
    new_redir.out_redir = out_redir;
    new_redir.out_redir_append = out_redir_append;
    return new_redir;
}

Redir merge_redirs(Redir prev_redir, Redir next_redir) {
    Redir new_redir = prev_redir;
    if (next_redir.in_redir != NULL)
        new_redir.in_redir = next_redir.in_redir;
    if (next_redir.out_redir != NULL) {
        new_redir.out_redir = next_redir.out_redir;
        new_redir.out_redir_append = next_redir.out_redir_append;
    }
    return new_redir;
}

struct arg_list_t add_arg(struct arg_list_t l, arg_t elem) {
    if (l.length + 1 >= l.capacity) {
        l.capacity *= 2;
        l.head = (arg_t *)realloc(l.head, l.capacity * sizeof(arg_t));
        if (!l.head) {
            fprintf(stderr, "mysh: realloc: failed to reallocate memory.");
            exit(EXIT_FAILURE);
        }
    }
    l.head[l.length] = elem;
    l.length++;
    l.head[l.length] = NULL;
    return l;
}

struct arg_list_t init_arg_list(arg_t elem) {
    struct arg_list_t h;
    h.length = 0;
    h.capacity = 1;
    h.head = (arg_t *)malloc(sizeof(arg_t));
    if (!h.head) {
        fprintf(stderr, "mysh: malloc: failed to allocate memory.");
        exit(EXIT_FAILURE);
    }
    h.redir = init_redir(NULL, NULL, -1);
    return add_arg(h, elem);
}

struct arg_list_t set_arg_list_redir(struct arg_list_t l, Redir redir) {
    Redir new_redir = merge_redirs(l.redir, redir);
    l.redir = new_redir;
    return l;
}

struct cmd_list_t insert_cmd_list_elem(struct cmd_list_t l, struct arg_list_t elem) {
    if (l.length >= l.capacity) {
        l.capacity *= 2;
        l.head = (struct arg_list_t *)realloc(l.head, l.capacity * sizeof(struct arg_list_t));
        if (!l.head) {
            fprintf(stderr, "mysh: realloc: failed to reallocate memory.");
            exit(EXIT_FAILURE);
        }
    }
    l.head[l.length] = elem;
    l.length++;
    return l;
}

struct cmd_list_t init_cmd_list(struct arg_list_t elem) {
    struct cmd_list_t h;
    h.head = (struct arg_list_t *) malloc(sizeof(struct arg_list_t));
    if (!h.head) {
        fprintf(stderr, "mysh: malloc: failed to allocate memory.");
        exit(EXIT_FAILURE);
    }
    h.length = 0;
    h.capacity = 1;
    return insert_cmd_list_elem(h, elem);
}

struct list_list_t insert_list_list_elem(struct list_list_t l, struct cmd_list_t elem) {
    if (l.length >= l.capacity) {
        l.capacity *= 2;
        l.head = (struct cmd_list_t *)realloc(l.head, l.capacity * sizeof(struct cmd_list_t));
        if (!l.head) {
            fprintf(stderr, "mysh: realloc: failed to reallocate memory.");
            exit(EXIT_FAILURE);
        }
    }
    l.head[l.length] = elem;
    l.length++;
    return l;
}

struct list_list_t init_list_list(struct cmd_list_t elem) {
    struct list_list_t h;
    h.head =  (struct cmd_list_t *)malloc(sizeof(struct cmd_list_t));
    if (!h.head) {
        fprintf(stderr, "mysh: malloc: failed to allocate memory.");
        exit(EXIT_FAILURE);
    }
    h.length = 0;
    h.capacity = 1;
    return insert_list_list_elem(h, elem);
}

struct list_list_t free_list_list(struct list_list_t l) {
    for (size_t i = 0; i < l.length; ++i) {
        for (size_t j = 0; j < l.head[i].length; ++j) {
            for (size_t k = 0; k < l.head[i].head[j].length; ++k) {
                // free each arg
                free(l.head[i].head[j].head[k]);
            }
            // free each cmd (args list)
            free(l.head[i].head[j].head);
        }
        // free each pipeline
        free(l.head[i].head);
    }
    // free list of pipelines
    free(l.head);
    l.head = NULL;
    l.length = 0;
    l.capacity = 0;
    return l;
}

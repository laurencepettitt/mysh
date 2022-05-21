#ifndef LIBMYSHLAUNCHER_H
#define LIBMYSHLAUNCHER_H
#include "arg_list.h"

int launch(size_t argc, char **args);
int launch_list(struct listq_head *parse_tree);

#endif
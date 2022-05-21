#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <wait.h>
#include <limits.h> /* PATH_MAX */
#include "exitcodes.h"
#include "launcher.h"
#include "arg_list.h"
#include <sys/queue.h>
#include <fcntl.h>

extern int sigint_received;

// Present Working Directory
char *PWD = NULL;
// OLDPWD
char *OWD = NULL;

/* Changes directory to nwd.
 * Also keeps environment up to date and handles errors.
 */
int mysh_chdir(char *nwd) {
    if (nwd == NULL)
        return EXIT_FAILURE;
    char *abs_nwd = realpath(nwd, NULL);
    if (abs_nwd == NULL) {
        perror("mysh: cd");
        return EXIT_FAILURE;
    }
    free(OWD);
    OWD = PWD;
    PWD = abs_nwd;
    int res = chdir(PWD);
    if (res != EXIT_SUCCESS) {
        perror("mysh: cd");
    }
    return res;
}

/* Launches the builtin "cd" command with an optional argument.
 * First element of args must be "cd"
 * Second element of args can be an optional argument.
 * */
int launch_builtin_cd(size_t argc, char **args) {
    // Make sure PWD is consistent
    if (PWD == NULL) {
        char *pwd = getenv("PWD");
        if (pwd == NULL)
            pwd = getenv("HOME");
        if (pwd != NULL)
            mysh_chdir(pwd);
    }
    // First element of args list must be "cd"
    if (argc == 0 || args[0] == NULL || strcmp(args[0], "cd") != 0)
        return -1;
    // Too many args
    if (argc > 2) {
        fprintf(stderr, "%s", "mysh: cd: too many arguments.");
        return EXIT_FAILURE;
    }
    // Go home
    if (argc == 1) {
        return mysh_chdir(getenv("HOME"));
    }
    // Go to previous dir
    if (strcmp(args[1], "-") == 0) {
        int res = mysh_chdir(OWD);
        printf("%s\n", PWD);
        return res;
    }
    // Go to dir
    return mysh_chdir(args[1]);
}

int is_builtin(char **args) {
    return (args == NULL) || (args[0] == NULL) || (strcmp(args[0], "exit") == 0) || (strcmp(args[0], "cd") == 0);
}

/* Launches a builtin command (with optional arguments).
 * First element of args list is name of command.
 * Following elements are arguments to the command.
 */
int launch_builtin(size_t argc, char **args) {
    // EOF
    if (args == NULL) {
        printf("args == NULL\n");
        return EXIT_EOF;
    }
    // Empty line, not EOF.
    if (args[0] == NULL) {
        printf("args[0] == NULL\n");
        return EXIT_SUCCESS;
    }
    // Exit command
    if (strcmp(args[0], "exit") == 0)
        return EXIT_EOF;
    // Change directory command
    if (strcmp(args[0], "cd") == 0)
        return launch_builtin_cd(argc, args);
    // Not a builtin
    return -1;
}

/* Launches an external (non-builtin) command.
 * The process forks and execs the command in a child process.
 * First element of args must be the actual command.
 * Following elements of args can be arguments to the command.
 */
int launch_exec(char **args) {
    pid_t pid;
    int status;
    // FORK
    if ((pid = fork()) < 0) {
        // Couldn't fork
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        // Problem with args, exit child process
        exit(EXIT_UNKNOWN_COMMAND);
    }

    // Parent process
    do {
        // TODO: Not sure if to use WUNTRACED here
        waitpid(pid, &status, WUNTRACED);
    } while (!WIFSIGNALED(status) && !WIFEXITED(status));

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return EXIT_SIGNAL_OFFSET + WTERMSIG(status);
//    if (WIFSTOPPED(status))
//        return EXIT_SIGNAL_OFFSET + WSTOPSIG(status);
    return EXIT_FAILURE;
}

/* Launches a command (builtin or external).
 */
//int launch(size_t argc, char **args) {
//}

int launch_pipeline(struct cmdq_head *h) {
//    int res = EXIT_SUCCESS;
    struct cmdq_elem *elem;
    size_t len_pipe = 0;
    STAILQ_FOREACH(elem, h, cmdq_entries) {
        len_pipe++;
    }

    pid_t pids[len_pipe];
    int pds[len_pipe - 1][2];
    size_t i = 0;
    STAILQ_FOREACH(elem, h, cmdq_entries) {
        struct arg_list_t *arg_list = elem->arg_list;
        size_t argc = arg_list->length;
        char **args = arg_list->head;
        Redir redir = arg_list->redir;

        if (i == 0 && len_pipe == 1 && is_builtin(args)) {
            return launch_builtin(argc, args);
        }
        if (i < len_pipe - 1) {
            pipe(pds[i]);
        }

        // Fork
        if ((pids[i] = fork()) < 0) {
            // Couldn't fork
            return EXIT_FAILURE;
        } else if (pids[i] == 0) {

            // Child process
            if (i < len_pipe - 1) {
                // replace stdout with write end of pds[i]
                // TODO: check use of  dup2
                dup2(pds[i][1], 1);
                close(pds[i][0]);
                close(pds[i][1]);
            }
            if (i > 0) {
                // replace stdin with read end of pds[i-1]
                dup2(pds[i - 1][0], 0);
                close(pds[i - 1][0]);
                close(pds[i - 1][1]);
            }
            printf("CHILD in pipe\n");
            printf("%s\n", redir.out_redir);
            if (redir.out_redir) {
                int flags = O_CREAT | O_WRONLY | (redir.out_redir_append ? O_APPEND : O_TRUNC);
                int outfd = open(redir.out_redir, flags);
                if (outfd == -1) {
                    // TODO: error
                }
                dup2(outfd, 1);
                close(outfd);
            }
            if (redir.in_redir) {
                int flags = O_CREAT | O_RDONLY;
                int infd = open(redir.out_redir, flags);
                if (infd == -1) {
                    // TODO: error
                }
                dup2(infd, 0);
                close(infd);
            }
            // TODO: launch builtin
            execvp(args[0], args);
            // Problem with args, exit child process
            exit(EXIT_UNKNOWN_COMMAND);
        }
        // Parent process
        close(pds[i - 1][0]);
        close(pds[i - 1][1]);
        i++;
    }

    int res[len_pipe];
    i = 0;
    STAILQ_FOREACH(elem, h, cmdq_entries) {
        // Parent process
        int status;
        do {
            // TODO: Not sure if to use WUNTRACED here
            waitpid(pids[i], &status, WUNTRACED);
        } while (!WIFSIGNALED(status) && !WIFEXITED(status));

        if (WIFEXITED(status))
            res[i] = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            res[i] = EXIT_SIGNAL_OFFSET + WTERMSIG(status);
        else
            res[i] = EXIT_FAILURE;
//    if (WIFSTOPPED(status))
//        return EXIT_SIGNAL_OFFSET + WSTOPSIG(status);
        i++;
    }
    return res[i - 1];
}

int launch_list(struct listq_head *h) {
    int res = EXIT_SUCCESS;
    struct listq_elem *elem;
    int i = 0;
    if (h == NULL) {
        // empty line
        return EXIT_EOF;
    }
    STAILQ_FOREACH(elem, h, listq_entries) {
        int nres = launch_pipeline(elem->cmds);
        printf("res %i\n", nres);
        if (nres == EXIT_EOF) {
            res += EXIT_EOF;
            if (i > 0)
                res++;
            break;
        }
        res = nres;
        i++;
    }
    printf("res %i\n", res);
    return res;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <wait.h>
#include "exitcodes.h"
#include "launcher.h"
#include "ast.h"
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
        if (OWD == NULL)
            return EXIT_FAILURE;
        // print the previous dir
        printf("%s\n", OWD);
        int res = mysh_chdir(OWD);
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
//        printf("args == NULL\n");
        return EXIT_EOF;
    }
    // Empty line, not EOF.
    if (args[0] == NULL) {
//        printf("args[0] == NULL\n");
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

/* Launches a command pipeline (or just one command).
 * The process forks and execs the commands, each in a child process.
 */
int launch_pipeline(struct cmd_list_t l) {
    pid_t pids[l.length];
    int pds[l.length - 1][2];
    for (size_t i = 0; i < l.length; i++) {
        struct arg_list_t arg_list = l.head[i];
        size_t argc = arg_list.length;
        char **args = arg_list.head;
        Redir redir = arg_list.redir;

        if (i == 0 && l.length == 1 && is_builtin(args)) {
            return launch_builtin(argc, args);
        }
        if (i < l.length - 1) {
            pipe(pds[i]);
        }

        // Fork
        pids[i] = fork();
        if (pids[i] < 0) {
            // Couldn't fork
            return EXIT_FAILURE;
        } else if (pids[i] == 0) {
            // Child process
            if (i < l.length - 1) {
                // replace stdout with write end of pds[i]
                if (!redir.out_redir)
                    dup2(pds[i][1], fileno(stdout));
                close(pds[i][0]);
                close(pds[i][1]);
            }
            if (i > 0) {
                // replace stdin with read end of pds[i-1]
                if (!redir.in_redir)
                    dup2(pds[i - 1][0], fileno(stdin));
                close(pds[i - 1][0]);
                close(pds[i - 1][1]);
            }
            if (redir.out_redir) {
                int flags = O_CREAT | O_WRONLY | (redir.out_redir_append ? O_APPEND : O_TRUNC);
                mode_t mode = 0664;
                int outfd = open(redir.out_redir, flags, mode);
                if (outfd == -1) {
                    char *fmt = "mysh: %s";
                    char pref[strlen(fmt) + strlen(redir.out_redir)];
                    sprintf(pref, fmt, redir.out_redir);
                    perror(pref);
                    exit(EXIT_FAILURE);
                } else {
                    dup2(outfd, fileno(stdout));
                    close(outfd);
                }
            }
            if (redir.in_redir) {
                int flags = O_RDONLY;
                int infd = open(redir.in_redir, flags);
                if (infd == -1) {
                    char *fmt = "mysh: %s";
                    char pref[strlen(fmt) + strlen(redir.in_redir)];
                    sprintf(pref, fmt, redir.in_redir);
                    perror(pref);
                    exit(EXIT_FAILURE);
                } else {
                    dup2(infd, fileno(stdin));
                    close(infd);
                }
            }
            if (is_builtin(args)) {
                exit(launch_builtin(argc, args));
            }
            execvp(args[0], args);
            // Problem with args, exit child process
            char *fmt = "mysh: %s";
            char pref[strlen(fmt) + strlen(args[0])];
            sprintf(pref, fmt, args[0]);
            perror(pref);
            exit(EXIT_UNKNOWN_COMMAND);
        }

        // Parent process
        if (i > 0) {
            close(pds[i - 1][0]);
            close(pds[i - 1][1]);
        }
    }

    // Parent process
    int res[l.length];
    for (size_t i = 0; i < l.length; i++) {
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
    }
    return res[l.length - 1];
}

int launch_list(struct list_list_t l) {
    int res = EXIT_SUCCESS;
    for (size_t i = 0; i < l.length; i++) {
        int nres = launch_pipeline(l.head[i]);
        if (nres == EXIT_EOF) {
            res += EXIT_EOF;
            if (i > 0)
                res++;
            break;
        }
        res = nres;
    }
    return res;
}

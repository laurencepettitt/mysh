#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <wait.h>
#include <limits.h> /* PATH_MAX */
#include "libmyshexitcodes.h"
#include "libmyshlauncher.h"

extern int sigint_received;

// Present Working Directory
char* PWD = NULL;
// OLDPWD
char* OWD = NULL;

char *abs_path(char *p) {
    char *buf = malloc(sizeof(char)*(PATH_MAX+1));
    return realpath(p, buf);
}

int mysh_chdir(char *nwd) {
    if (nwd == NULL)
        return EXIT_FAILURE;
    char *abs_nwd = abs_path(nwd);
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

int launch_builtin_cd(char **args) {
    if (PWD == NULL)
        PWD = strdup(getenv("PWD")); // TODO: strdup
    if (OWD == NULL)
        OWD = strdup(PWD);
    if (args[0] == NULL || strcmp(args[0], "cd") != 0)
        return -1;
    // Go home
    if (args[1] == NULL) {
        return mysh_chdir(getenv("HOME"));
    }
    // Go to previous dir
    if (strcmp(args[1], "-") == 0) {
        return mysh_chdir(OWD);
    }
    // Go to dir
    return mysh_chdir(args[1]);
}

int launch_builtin(char **args) {
    // EOF
    if (args == NULL)
        return EXIT_EOF;
    // Empty line, not EOF.
    if (args[0] == NULL)
        return EXIT_SUCCESS;
    // Exit command
    if(strcmp(args[0], "exit") == 0)
        return EXIT_EOF;
    // Change directory command
    if (strcmp(args[0], "cd") == 0)
        return launch_builtin_cd(args);
    // Not a builtin
    return -1;
}

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

int launch(char **args) {
    int res = launch_builtin(args);
    if (res >= EXIT_SUCCESS)
        return res;
    return launch_exec(args);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include <wait.h>
#include <limits.h> /* PATH_MAX */
#include "exitcodes.h"
#include "launcher.h"

extern int sigint_received;

// Present Working Directory
char* PWD = NULL;
// OLDPWD
char* OWD = NULL;

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

/* Launches a builtin command (with optional arguments).
 * First element of args list is name of command.
 * Following elements are arguments to the command.
 */
int launch_builtin(size_t argc, char **args) {
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
int launch(size_t argc, char **args) {
    // First try to launch the command as a builtin (will not do anything if not)
    int res = launch_builtin(argc, args);
    // If it was not a builtin command, launch with exec
    if (res == -1)
        res = launch_exec(args);
    return res;
}

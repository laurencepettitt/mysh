#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "libmyshreader.h"
#include <stdlib.h>

int sigint_received;

static void sighandler (int sig)
{
    if (sig == SIGINT)
        sigint_received = 1;
}

int main(int argc, char *argv[]) {
    sigint_received = 0;
    signal(SIGINT, sighandler);

    // Interactive mode
    if (argc == 1)
        return repl_interactive();

    // String mode
    if (strcmp(argv[1], "-c") == 0) {
        if (argc != 3)
            return EXIT_FAILURE;
        FILE *in = fmemopen(argv[2], strlen(argv[2]), "r");
        if (in == NULL) {
            perror("mysh: fmemopen");
            return EXIT_FAILURE;
        }
        int res = repl_script(in);
        fclose(in);
        return res;
    }

    // File mode
    if (argc == 2) {
        FILE *in = fopen(argv[1], "r");
        int res = repl_script(in);
        fclose(in);
        return res;
    }

    fprintf(stderr, "Unsupported arguments.");
    return EXIT_FAILURE;
}

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "reader.h"
#include <stdlib.h>

volatile sig_atomic_t sigint_received;

static void sighandler(int sig) {
    if (sig == SIGINT)
        sigint_received = 1;
}

int main(int argc, char *argv[]) {
    sigint_received = 0;
    struct sigaction act = { 0 };
    act.sa_handler = sighandler;
    sigaction(SIGINT, &act, NULL);

    // Interactive mode
    if (argc == 1)
        return repl_interactive();

    // String mode
    if (strcmp(argv[1], "-c") == 0) {
        if (argc != 3)
            return EXIT_FAILURE;
        int res = repl_string(argv[2]);
        return res;
    }

    // File mode
    if (argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror("mysh: open");
            return EXIT_FAILURE;
        }
        int res = repl_file(fd);
        close(fd);
        return res;
    }

    fprintf(stderr, "Unsupported arguments.");
    return EXIT_FAILURE;
}

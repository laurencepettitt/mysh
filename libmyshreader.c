#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include <locale.h>

#include <readline/readline.h>
#include <readline/history.h>

/* Used for select(2) */
#include <sys/select.h>

#include <asm/errno.h>
#include <errno.h>

#include <signal.h>

#include "libmyshexitcodes.h"
#include "libmyshreader.h"
#include "libmyshparser.h"

extern volatile sig_atomic_t sigint_received;

int line_number = 1;

int status = EXIT_SUCCESS;
int running = 1;

void receive_signal(int print_message) {
    sigint_received = 0;
    status = EXIT_SIGNAL_OFFSET + SIGINT;
    if (print_message)
        fprintf(stderr, "Killed by signal %d\n", SIGINT);
}

int linehandler(const char *line) {
    int next_status = parse_line(line);
    // No EOF received
    if (next_status < EXIT_EOF) {
        status = next_status;
        if (status >= EXIT_SIGNAL_OFFSET)
            receive_signal(1);
        return EXIT_SUCCESS;
    }
    // If next_status is strictly greater than EXIT_EOF, a command was executed on this line before we got EXIT_EOF.
    // so the exit status of the previous command was added to EXIT_EOF + 1 before being returned as next_status.
    if (next_status > EXIT_EOF)
        status = next_status - EXIT_EOF - 1;
    // If next_status is equal to EXIT_EOF, then we received EOF, so keep status from previous command and stop running.
    running = 0;
    return EXIT_EOF;
}

static void rl_callback_handler(char *line) {
    if (line != NULL)
        add_history(line);
    if (linehandler(line) == EXIT_SUCCESS)
        return;
    free(line);
    // Else we got EXIT_EOF.
    // Readline's callback handler is removed, so it won't renew the prompt once this callback returns.
    rl_callback_handler_remove();
}

int repl_string(char *in) {
    if (in == NULL)
        return EXIT_FAILURE;
    const char nl[2] = "\n";
    while (running) {
        char *line = strtok(in, nl);
        if (sigint_received) {
            receive_signal(1);
            break;
        }
        if (line == NULL)
            break;
        in = NULL;
        linehandler(line);
        line_number++;
        if (sigint_received) {
            receive_signal(1);
            break;
        }
    }
    return status;
}

int repl_file(int fd) {
    char *lbuf = NULL;
    size_t lbuf_len = 0;
    size_t lbuf_cap = 0;

    while (running) {
        char c;
        int s = read(fd, &c, 1);
        if (s == -1) {
            perror("mysh: read");
            status = EXIT_FAILURE;
            break;
        }
        
        if (s == 0) {
            break;
        }

        if (sigint_received) {
            receive_signal(1);
            break;
        }

        if (c == '\n')
            c = '\0';

        if (lbuf == NULL) {
            size_t cap = 1;
            lbuf = malloc(cap * sizeof(char));
            lbuf_len = 0;
            lbuf_cap = cap;
        }

        if (lbuf_len + 1 > lbuf_cap) {
            lbuf_cap *= 2;
            lbuf = realloc(lbuf, lbuf_cap * sizeof(char));
        }

        lbuf[lbuf_len] = c;
        lbuf_len++;

        if (c == '\0') {
            linehandler(lbuf);
            free(lbuf);
            lbuf = NULL;
            lbuf_len = 0;
            lbuf_cap = 0;
            line_number++;
        }

        if (sigint_received) {
            receive_signal(1);
            break;
        }
    }
    free(lbuf);
    return status;
}

int repl_interactive()
{
    const char *prompt = "mysh4$ ";

    fd_set fds;
    int r;

    rl_callback_handler_install(prompt, (rl_vcpfunc_t*) &rl_callback_handler);

    status = EXIT_SUCCESS;
    running = 1;
    while (running) {
//        rl_set_prompt(prompt);
        if (sigint_received) {
            receive_signal(0);
            rl_free_line_state();
            rl_callback_sigcleanup();
            rl_crlf(); // Move input to a new line
            rl_replace_line("", 0); // Clear the line buffer
            rl_on_new_line(); // Prompt will regenerate on the new line
            rl_redisplay(); // Regenerate prompt
        }
        FD_ZERO(&fds);
        FD_SET(fileno(rl_instream), &fds);
        r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
        if (r < 0 && errno != EINTR)
        {
            perror("mysh: select"); // TODO: this?
            status = EXIT_FAILURE;
            running = 0;
            break;
        }
        if (r < 0 && errno == EINTR) {
            sigint_received = 1;
            continue;
        }
        if (FD_ISSET(fileno(rl_instream), &fds))
            rl_callback_read_char();
    }

    rl_callback_handler_remove();
    return status;
}


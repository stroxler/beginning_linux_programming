#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* The functions dup and dup2 let you copy file descriptors.
 *
 * you can read the docs in more detail if you need them, but in this case
 * what we are doing is:
 *   - making a pipe, which creates two file descriptors (all of them > 2,
 *     b/c stdout, stdin, and stderr are taken)
 *   - calling fork
 *   - in the parent, we write pretty much as before
 *   - in he child, we use dup() to remap the file descriptor and then
 *     close the ones opened by pipe() first, then we call exec
 *
 *     -> this leads to the child process (which has file descriptor 0
 *        free, and dup() always maps to the smallest free file descriptor)
 *        getting the read end of the pipe as stdin.
 *
 *        This means we can use ordinary programs!
 *        We could just do this with popen much more easily, of course, but
 *          that creates an extra shell as we saw, so in a situation where we
 *          need low overhead it can be an issue.
 */


int main() {
    int data_processed;
    int file_pipes[2];
    const char some_data[] = "123 456 789";
    pid_t fork_result;

    if (pipe(file_pipes) == 0) {
        fork_result = fork();

        // handle errors
        if (fork_result == (pid_t)-1) {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }

        // child process:
        //   - call dup, which due to the fork - which leads to
        //     a process lacking stdin - causes the file descriptor to be
        //     mapped to 0, which is stdin
        //   - close both of the file descriptors opened via the pipe
        //   - then pass control off using execlp: at this point, the read
        //     end of the pipe is stdin, so we don't need a custom process
        //     the way we did in the pipe_exec_child.c program!
        if (fork_result == (pid_t)0) {
            close(0);
            dup(file_pipes[0]);
            close(file_pipes[0]);
            close(file_pipes[1]);

            execlp("awk", "{print $2}", NULL);
            exit(EXIT_FAILURE);
        }

        // parent process: just write some data to the write end of the pipe.
        //                 close both ends (we've been sloppy about this till
        //                 now) of the pipe, also.
        else {
            close(file_pipes[0]);
            data_processed = write(file_pipes[1], some_data,
                                   strlen(some_data));
            close(file_pipes[1]);
            printf("%d - wrote %d bytes\n", (int)getpid(), data_processed);
        }
    }
    exit(EXIT_SUCCESS);
}


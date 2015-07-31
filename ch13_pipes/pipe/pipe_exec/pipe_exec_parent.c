#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* this is the parent of an example that uses fork and exec to communicate
 * over a pipe using two different programs
 *
 * The parent program forks, then in the parent we write to the pipe and
 * exit. The child calls execl to change the program to the pipe_exec_child,
 * which will in turn read the data.
 */

int main() {
    int data_processed;
    int file_pipes[2];
    const char some_data[] = "123";
    char buffer[BUFSIZ + 1];
    pid_t fork_result;

    memset(buffer, '\0', sizeof(buffer));

    if (pipe(file_pipes) == 0) {
        fork_result = fork();

        // handle failure
        if (fork_result == (pid_t)-1) {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }

        // in child, print the file descriptor number to a string because
        // when we call exec the file descriptors are preserved, but we're
        // going to need to know which one to use. To find out, we pass it as
        // a command line option.
        //
        // Then call execl. If execl fails, exit as a failure.
        if (fork_result == 0) {
            sprintf(buffer, "%d", file_pipes[0]);
            execl("./pipe_exec_child", "pipe_exec_child",
                  buffer, (char *)0);
            exit(EXIT_FAILURE);
        }

        // in the parent, write some stuff to the pipe and print status. Then
        // exit.
        else {
            data_processed = write(file_pipes[1], some_data,
                                   strlen(some_data));
            printf("%d - wrote %d bytes\n", getpid(), data_processed);
        }
    }
    exit(EXIT_SUCCESS);
}


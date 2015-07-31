#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Here we make a simple demo of combining fork() with pipe() to actually
 * communicate between processes. The content of the example is identical
 * to our single-process pipe demo
 *
 * First we make a pipe, then we call fork() and in the parent we write
 * to the pipe, in the child we read from it.
 *
 * It's still not a super-interesting example yet, but when we go one further
 * and use exec(), we get minimal-overhead pipes with no shell invocation.
 * See later examples...
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

        // handle errors 
        if (fork_result == -1) {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }

        // in the child: read from the 0th (read side of the pipe) descriptor
        if (fork_result == 0) {
            data_processed = read(file_pipes[0], buffer, BUFSIZ);
            printf("Read %d bytes: %s\n", data_processed, buffer);
            exit(EXIT_SUCCESS);
        }

        // in the parent: write to the 1st (write side of the pipe) descriptor
        else {
            data_processed = write(file_pipes[1], some_data,
                                   strlen(some_data));
            printf("Wrote %d bytes\n", data_processed);
        }
    }
    exit(EXIT_SUCCESS);
}


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* In contrast to popen(), which accepts a command and mode much like
 * fopen() does, and invokes a shell, pipe() provides low-level shell-free
 * pipes using raw file descriptors.
 *
 * In this example we demo the api in just one process (which is obviously
 * pointless except as a demo). The pipe() command takes an array
 *     int fds[2]
 * and it sets fd[0] to be the file descriptor of the read end of a pipe,
 * while fd[1] is the write end of the pipe (this seems backward :(  )
 *   ... it returns 0 if pipe opened properly, and -1 (plus sets errno) o/w.
 *
 * Here we open the pipe and talk with both ends from one process using the
 * low level read and write functions.
 */


int main() {
    int data_processed;
    int file_pipes[2];
    const char some_data[] = "123";
    char buffer[BUFSIZ + 1];

    memset(buffer, '\0', sizeof(buffer));

    if (pipe(file_pipes) == 0) {
        data_processed = write(file_pipes[1], some_data, strlen(some_data));
        printf("Wrote %d bytes\n", data_processed);
        data_processed = read(file_pipes[0], buffer, BUFSIZ);
        printf("Read %d bytes: %s\n", data_processed, buffer);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

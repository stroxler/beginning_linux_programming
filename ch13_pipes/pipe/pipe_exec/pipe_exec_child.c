#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* This is the consumer program that gets started via an execl() call in
 * the `pipe_exec_parent.c` code.
 *
 * It accepts a single command line argument which should be a string
 * representation of the file descriptor to use as the read end of the pipe
 * (the actual file descriptor is preserved by exec()).
 *
 * Once it has scanned the file descriptor, and also zeroed out the buffer,
 * it just reads in the data and prints a message.
 */

int main(int argc, char *argv[]) {
    int data_processed;
    char buffer[BUFSIZ + 1];
    int file_descriptor;

    memset(buffer, '\0', sizeof(buffer));
    sscanf(argv[1], "%d", &file_descriptor);
    data_processed = read(file_descriptor, buffer, BUFSIZ);

    printf("%d - read %d bytes: %s\n", getpid(), data_processed, buffer);
    exit(EXIT_SUCCESS);
}

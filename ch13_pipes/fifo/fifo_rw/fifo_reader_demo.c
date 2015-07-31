#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_NAME "/tmp/my_fifo"
#define BUFFER_SIZE PIPE_BUF


/* This is the fifo reader demo program.
 *    compare to fifo_writer_demo.c
 *
 * The writere writes 10 meggabytes of junk to the fifo.
 * Here we just read until a read() returns 0 (which happens
 * when the writer exits or closes the fifo; in this case
 * we close explicitly in the writer function - remember that
 * if there's no data but the writer didn't close, we would block
 * on read rather than returning 0). Once read returns 0, we are
 * finished so we close the pipe and exit.
 */
int main() {
    int pipe_fd;
    int res;
    int open_mode = O_RDONLY;
    char buffer[BUFFER_SIZE + 1];
    int bytes_read = 0;

    memset(buffer, '\0', sizeof(buffer));
    
    printf("Process %d opening FIFO O_RDONLY\n", getpid());
    pipe_fd = open(FIFO_NAME, open_mode);
    printf("Process %d result %d\n", getpid(), pipe_fd);

    if (pipe_fd != -1) {
        do {
            res = read(pipe_fd, buffer, BUFFER_SIZE);
            bytes_read += res;
        } while (res > 0);
        (void)close(pipe_fd);
    }
    else {
        fprintf(stderr, "Failed to open fifo!\n");
        exit(EXIT_FAILURE);
    }

    printf("Process %d finished, %d bytes read\n", getpid(), bytes_read);
    exit(EXIT_SUCCESS);
}


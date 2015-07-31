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
#define TEN_MEG (1024 * 1024 * 10)

/* This is the fifo writer demo program.
 *    compare to fifo_reader_demo.c
 *
 * It writes 10 megabytes of data to the pipe, in chuncks
 * of size BUFFER_SIZE == PIPE_BUF, and prints a message
 * when done if it finishes normally.
 */

int main() {
    int pipe_fd;
    int res;
    int open_mode = O_WRONLY;
    int bytes_sent = 0;
    char buffer[BUFFER_SIZE + 1];

    // make the fifo if needed.
    if (access(FIFO_NAME, F_OK) == -1) {
        res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
            exit(EXIT_FAILURE);
        }
    }

    // open the fifo
    printf("Process %d opening FIFO O_WRONLY\n", getpid());
    pipe_fd = open(FIFO_NAME, open_mode);
    printf("Process %d result %d\n", getpid(), pipe_fd);

    // assuming no error, loop and write 10 mb to fifo, in chuncks
    // of BUFFER_SIZE.
    if (pipe_fd != -1) {
        while(bytes_sent < TEN_MEG) {
            res = write(pipe_fd, buffer, BUFFER_SIZE);
            if (res == -1) {
                fprintf(stderr, "Write error on pipe\n");
                exit(EXIT_FAILURE);
            }
            bytes_sent += res;
        }
        (void)close(pipe_fd); 
    } else {
        fprintf(stderr, "Failed to open fifo!\n");
        exit(EXIT_FAILURE);        
    }

    // all done, print a msg and quit. This closes the write
    // side of the fifo.
    printf("Process %d finished\n", getpid());
    exit(EXIT_SUCCESS);
}

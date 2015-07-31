// Let's start with the header files, a #define and the check that the correct number
// of command-line arguments have been supplied.

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* This demo shows using various flags when opening named pipes.
 *
 * Pipes can only be opened in RDONLY or WRONLY mode.
 * The other option which can be bitwise-or'ed when opening is O_NONBLOCK.
 * 
 * If you set O_NONBLOCK (assuming you start at 0), then open calls
 * will execute and the program will continue on its way until the first
 * read...
 *  If you don't set O_NONBLOCK, then the open call will block until a matching
 * open happens: a writer will block on open until someone also opens in
 * read mode, and a reader will block on open until someone also opens in
 * write mode.
 *
 * This program demos just the opening logic by letting you pass the flags
 * in as strings from the command line. To try it out, try opening with
 * combinations of O_RDONLY, O_WRONLY, O_NONBLOCK in two different terminals,
 * and see when a process waits at open or just runs to completion.
 *    ... interestingly, you can have multiple readers and writers, it won't
 *        raise errors. Not sure what the behavior would be when you started
 *        passing data around. Probably multiple writers is simple, it's just
 *        a byte queue, whereas multiple readers might cause issues unless
 *        you read in well-controlled meaningful chunk sizes.
 *
 * We demo actually reading/writing data later.
 */

#define FIFO_NAME "/tmp/my_fifo"

int main(int argc, char *argv[]) {
    int res;
    int open_mode = 0;
    int i;

    // check the arg count
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <some combination of\
               O_RDONLY O_WRONLY O_NONBLOCK>\n", *argv);
        exit(EXIT_FAILURE);
    }

    // parse the args and set flags based on them
    for(i = 1; i < argc; i++) {
        if (strncmp(*++argv, "O_RDONLY", 8) == 0)
             open_mode |= O_RDONLY;
        if (strncmp(*argv, "O_WRONLY", 8) == 0)
             open_mode |= O_WRONLY;
        if (strncmp(*argv, "O_NONBLOCK", 10) == 0)
             open_mode |= O_NONBLOCK;
     }

    // open the fifo if it doesn't exist. The mkfifo() function works mostly
    // like the command-line version, except you also pass the mode to it.
    if (access(FIFO_NAME, F_OK) == -1) {
        res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);
            exit(EXIT_FAILURE);
        }
    }

    // open the fifo with the flags we set
    printf("Process %d opening FIFO\n", getpid());
    res = open(FIFO_NAME, open_mode);
    // say when we finished opening: if O_NONBLOCK isn't set this will wait
    // until there's a matching process on the other end of the pipe
    printf("Process %d result %d\n", getpid(), res);
    sleep(5);
    if (res != -1) close(res);
    printf("Process %d finished\n", getpid());
    exit(EXIT_SUCCESS);
}

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/**
 * Demo of simple file locking (lock based on existance of the lock file)
 *
 * To run it, do something like
 *   ./a.out &
 *   ./a.out
 * and you will see both processes printing to stdout.
 */
const char *LOCK_FILE = "/tmp/LCK.test";

int main() {
    int fdesc;
    int tries = 10;
    while (tries-- > 0) {
        // the flags used here are what make it work
        fdesc = open(LOCK_FILE, O_RDWR | O_CREAT | O_EXCL, 0444);
        if (fdesc == -1) {
            printf("%d: - Lock already present, waiting...\n", getpid());
            sleep(2);
        } else {
            printf("%d: - I have exclusive access\n", getpid());
            sleep(4);
            printf("%d: - closing file now\n", getpid());
            close(fdesc);
            unlink(LOCK_FILE);
            break;
        }
    }

    exit(EXIT_SUCCESS);
}

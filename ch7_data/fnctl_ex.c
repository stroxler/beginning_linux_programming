#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

const char *TEST_FILE = "/tmp/test_lock";

/* File locking example
 *
 * The book authors present several more examples so that you can run tests
 * in several processes and see how stuff works. But given that I'm unlikely
 * to need a lot of file locking, I'm just doing this and writing some
 * detailed-ish notes on how it works. Feel free to reference the book for
 * the other two examples, which cover
 *   - using F_GETLK when a program is holding a lock
 *   - using F_SETLK in two processes (competing locks)
 */

int main() {
    int fdesc;
    int i_byte;
    char *byte = "A";   // I think this is just the authors being lazy... we
                        // never use the second '\0' byte lol. Could have
                        // just had char byte = 'A' and then used &byte in
                        // the `write` calls.
    struct flock region_1;
    struct flock region_2;
    int res;

    // open file
    fdesc = open(TEST_FILE, O_RDWR | O_CREAT, 0666);
    if (!fdesc) {
        fprintf(stderr, "Unable to open %s for read/write\n", TEST_FILE);
        exit(EXIT_FAILURE);
    }

    // write some data to the file
    for (i_byte = 0; i_byte < 100; i_byte++) {
        write(fdesc, byte, 1);
    };

    // set up a shared (read) lock on bytes 10 to 30. Any number of processes
    // may use shared locks, but requests for exclusive (write) locks on the
    // same block will fail if read locks are open.
    // 
    // The parameter l_type can bye F_RDLCK, F_WRLCK (to get a shared or
    //     exclusive lock, resp), or F_UNLOCK (to release a lock)
    // The l_whence parameter determines the semantic meaning of l_start.
    //     The value here, SEEK_SET, is by far the most common... it
    //     causes l_start to be distance from the beginning of the file. It's
    //     possible to use SEEK_CUR or SEEK_END, in which case l_start is
    //     relative to current position or end of file.
    region_1.l_type = F_RDLCK;
    region_1.l_whence = SEEK_SET;
    region_1.l_start = 10;
    region_1.l_len = 20;
    // a call to fnctl F_GETLK will just read any lock information on a file.
    //   The usual case is to find out if a region is locked without trying to
    //   lock it ourselves. The semantic way of using is is generally to set
    //   the flock.l_pid to -1. Then, when we call F_GETLK, if any lock is
    //   in place the l_pid entry of our `flock` struct will get set, which
    //   tells us that there is a lock in place.
    // a call to fnctl F_SETLK 
    //   returns -1 if the lock failed. Importantly, fnctl
    //   does *not* prevent you from reading or modifying data. It's up to your
    //   programs to use locks properly. It *does* unlock for you whenever
    //   you close a file descriptor or your program, however if you keep the
    //   file descriptor open you need to unlock yourself.
    // a call to fnctl F_SETLKW is the same semantically, except that instead
    //   of returning -1 when it cannot lock, it will block until it can, or
    //   until a signal occurs.
    printf("Process %d locking file\n", getpid());
    res = fcntl(fdesc, F_SETLK, &region_1);
    if (res == -1) fprintf(stderr, "Failed to lock region_1\n");

    // set up an exclusive (write) lock on bytes 40 to 50.
    //   any attempt to lock this region (read or write) in another
    //   process will return -1 (if F_SETLK) or block (if F_SETLKW).
    region_2.l_type = F_WRLCK;
    region_2.l_whence = SEEK_SET;
    region_2.l_start = 40;
    region_2.l_len = 10;
    printf("Process %d locking file\n", getpid());
    res = fcntl(fdesc, F_SETLK, &region_2);
    if (res == -1) fprintf(stderr, "Failed to lock region_2\n");






    exit(EXIT_SUCCESS);
}

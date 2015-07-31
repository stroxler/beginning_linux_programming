/* Here's the receiver program. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

/* Simple consumer that receives messages of type
 * my_msg_st. over a message queue.
 *
 * Note that unlike with shared memory and semaphores, we don't need a
 * msgctl call to initialize the global resource - the message queue is ready
 * to use as soon as we've called msgget, neither the consumer nor producer
 * programs initialize it.
 *
 * Note that all the messages we send involve a struct that starts with a long.
 * This is a part of the messaging api: that long is used as a key in the
 * msgrcv function, which allows a single queue to be used for different
 * purposes and/or by different clients.
 */

struct my_msg_st {
    long int my_msg_type;
    char some_text[BUFSIZ];
};

int main()
{
    int running = 1;
    int msgid;
    struct my_msg_st some_data;
    long int msg_to_receive = 0;

    // set up access to the message queue
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (msgid == -1) {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // loop over reading a `my_msg_st` struct from the queue.
    //
    // Note that unlike the shared memory example, we are doing i/o type
    // ops and have to copy data here, just like with files and pipes. So
    // although it's much simpler and more robust than shared memory, this
    // isn't as efficient in some settings. (and of course unlike shared
    // memory, we can't do random access here)
    //
    // whenever we see end, we exit.
    while(running) {
        if (msgrcv(msgid, (void *)&some_data, BUFSIZ,
                   msg_to_receive, 0) == -1) {
            fprintf(stderr, "msgrcv failed with error: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        printf("You wrote: %s", some_data.some_text);
        if (strncmp(some_data.some_text, "end", 3) == 0) {
            running = 0;
        }
    }

    // delete the message queue. This is important not to forget.
    if (msgctl(msgid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


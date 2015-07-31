/* The second program is the producer and allows us to enter data for consumers.
 It's very similar to shm1.c and looks like this. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#include "common.h"

/* The consumer_creator.c program sets up shared memory and reads data, setting
 * a byte in the data so that this program can write more.
 *
 * In this program, we just loop over stdin and then write out to that shared
 * memory whenever it is free.
 *
 * The consumer_creator.c program is in charge of deleting the shared memory
 * by id; this program only calls smdt, which detaches and frees local
 * resources.
 */

int main()
{
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    char buffer[BUFSIZ];
    int shmid;

    // we actually create the shared memory in both processes, because
    // using the IPC_CREAT flag twice doesn't cause issues and we get rid of
    // a race condition this way. But only the consumer is responsible for 
    // deleting - notice we call shmdt but we *dont* call smctl with
    // IPC_RMID at the end of the function, the way we do in consumer_creator.c
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Memory attached at %p\n", shared_memory);

    // in the loop, we only let the user enter text when `written_by_you`
    // is 0, which means the consumer_creator is ready for more input.
    //
    // When this is the case, we then write the user input into the
    // `shared_stuff` struct's `some_text`, and we also set `shared_stuff`s
    // `written_by_you` back to 1. Then the consumer_creator can read it.
    //
    // when the user types end, we exit.
    shared_stuff = (struct shared_use_st *)shared_memory;
    while(running) {
        while(shared_stuff->written_by_you == 1) {
            sleep(1);            
            printf("waiting for consumer...\n");
        }
        printf("Enter some text: ");
        fgets(buffer, BUFSIZ, stdin);
        
        strncpy(shared_stuff->some_text, buffer, TEXT_SZ);
        shared_stuff->written_by_you = 1;

        if (strncmp(buffer, "end", 3) == 0) {
                running = 0;
        }
    }

    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

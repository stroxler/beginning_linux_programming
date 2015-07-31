#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#include "common.h"

/* This program creates a chunk of shared memory, of size determined by the
 * `shared_use_st` struct defined in "common.h".
 *
 * At the end of the program, in which it loops over reading the shared
 * memory, which is set in the producer.c program based on user input, it
 * deletes the shared memory in a shmctl call. */

int main()
{
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;

    srand((unsigned int)getpid());    

    // create shared mem block. Handle errors
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    // attach to the chared memory (the call to shmget does *not* do this!
    // you need to call shmat - and you should generally pass it a null pointer
    // as the second arg - in order to get access to the shared memory inside
    // the memory space of this process.
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %p\n", shared_memory);

    // loop with some sleeping, and assign the shared memory to the
    // shared_use_st struct `shared_stuff` in this process. (Note we are not
    // reading out the data to `shared_stuff` - that's what we would do with
    // files or pipes, but here we just directly assign the pointer!)
    //
    // Whenever it's byte `written_by_you` indicates that we should, we print
    // out its `some_text` C-string to stdout.
    //
    // we then sleep for a bit before setting `written_by_you` back to 0.
    // we are using this byte to communicate when the other process is allowed
    // to write.
    //
    // The behavior if they read while we are writing is undefined, but that's
    // okay... they'll just poll again if they see 1, or charge ahead if they
    // see 0, and neither causes a problem in this simple example. In more
    // complex cases we might need to use semaphores to control access.
    shared_stuff = (struct shared_use_st *)shared_memory;
    shared_stuff->written_by_you = 0;
    while(running) {
        if (shared_stuff->written_by_you) {
            printf("You wrote: %s", shared_stuff->some_text);
            sleep( rand() % 4 ); /* make the other process wait for us ! */
            shared_stuff->written_by_you = 0;
            if (strncmp(shared_stuff->some_text, "end", 3) == 0) {
                running = 0;
            }
        }
    }

    // detach - this frees process-local resources
    if (shmdt(shared_memory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    // as with semaphores, it's very important to delete the global shared
    // memory resource. Note that shmdt doesn't do that - it detatches, which
    // frees the process-specific resources, but doesn't delete the global
    // resource.
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}



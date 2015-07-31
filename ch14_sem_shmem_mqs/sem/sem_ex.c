/* After the #includes, the function prototypes and the global variable, we come to the
 main function. There the semaphore is created with a call to semget, which returns the
 semaphore ID. If the program is the first to be called (i.e. it's called with a parameter
 and argc > 1), a call is made to set_semvalue to initialize the semaphore and op_char is
 set to X. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/sem.h>

#include "semun.h"


/* Simple demo of using System V IPC semaphores.
 *
 * In principle, these semaphores are just like those from Ch 12: they provide
 * always-positive counters, with atomic access for incrementing and
 * decrementing, and the guarantee that a decrement, when you hit zero,
 * always blocks.
 *
 * As in Chapter 12, we will only think about the most common use case, which
 * is a binary semaphore used for locking.
 *
 * The api of System V semaphores is more complicated: you create a semaphore
 * and give it a length, so that a single semaphore struct can contain many
 * sempahores. This is useful because you don't want lots of global keys
 * floating around, but sometimes programs need to lock many different
 * resources, so you could create a semaphore of some larger size (maybe 4,
 * for 4 different files you want to lock).
 *
 * In this example though, we provide a simple 4 function wrapper around using
 * just a single semaphore for binary locking:
 *    set_semvalue
 *    del_semvalue
 *    semaphore_p
 *    semaphore_v
 *
 * To demo the program, start up two or more instances. Give the very first
 * one you start any cmdline argument. The main() function will interpret that
 * argument as an indication that this process should call set_semvalue(),
 * which initializes the semaphore with a value of 1 (unlocked).
 *
 * Then the processes will take turns running locked code, printing X in the
 * process that created the semaphore and O in the others.
 *
 * The process that created the sempahore deletes it before exiting. It's a
 * global resource in your computer, so this is very important!
 */

static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);

static int sem_id;


int main(int argc, char *argv[]) {
    int i;
    int pause_time;
    char op_char = 'O';

    srand((unsigned int)getpid());
    
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);

    // if there's an arg, set op_char to X and create the semaphore
    if (argc > 1) {
        if (!set_semvalue()) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
        op_char = 'X';
        sleep(2);
    }

    // take turns locking, printing to stdout and flushing,
    // and then unlocking and waiting.
    for(i = 0; i < 10; i++) {        

        // lock around the printing and a short pause
        if (!semaphore_p()) exit(EXIT_FAILURE);
        printf("%c", op_char);fflush(stdout);
        pause_time = rand() % 3;
        sleep(pause_time);
        printf("%c", op_char);fflush(stdout);
        if (!semaphore_v()) exit(EXIT_FAILURE);
        
        // then do a random pause while unlocked
        pause_time = rand() % 2;
        sleep(pause_time);
    }    

    printf("\n%d - finished\n", getpid());

    // it's very important to delete the semaphore you created!
    if (argc > 1) {    
        sleep(10);
        del_semvalue();
    }
        
    exit(EXIT_SUCCESS);
}

/* The function set_semvalue initializes the semaphore using the SETVAL command in a
 semctl call. We need to do this before we can use the semaphore. */

static int set_semvalue(void)
{
    union semun sem_union;

    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

/* The del_semvalue function has almost the same form, except the call to semctl uses
 the command IPC_RMID to remove the semaphore's ID. */

static void del_semvalue(void)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

/* semaphore_p changes the semaphore by -1 (waiting). */

static int semaphore_p(void)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_v is similar except for setting the sem_op part of the sembuf structure to 1,
 so that the semaphore becomes available. */

static int semaphore_v(void)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}



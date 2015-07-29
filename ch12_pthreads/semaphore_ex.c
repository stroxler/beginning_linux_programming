#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

void *thread_function(void *arg);
sem_t bin_sem;

#define WORK_SIZE 1024
char work_area[WORK_SIZE];

/* Demonstration of basic semaphore functionality, including a prgramming
 * error that's easy to make where only one thread blocks (enter FAST at the
 * console to see it).
 *
 * This could be fixed by adding another semaphore, or by waiting and
 * posting in both threads. But we'll see a more standard solution - mutexes -
 * in another example.
 */

int main() {
    int res;
    pthread_t a_thread;
    void *thread_result;

    // init the semaphore.
    //
    //   the second input is always 0, which says use this for communicating
    //   between threads within a process (1 is supposed to allow between-
    //   process semaphores, but isn't supported for this semaphore library
    //   on most systems; it will crash.
    //
    //   The third input is the initial value of the underlying int. In general
    //   a semaphore can handle incr / decr operations on an arbitrary
    //   nonnegative int, but for most purposes - and all examples we will
    //   go through - the semaphore is binary, and functions as a lock for
    //   operations that aren't thread-safe.
    res = sem_init(&bin_sem, 0, 0);
    if (res != 0) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }

    // create. See simple_example.c for discusion
    res = pthread_create(&a_thread, NULL, thread_function, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // the main thread gets text and puts it in `work_area`
    // this operation is not thread safe, so it's controlled via a semaphore
    //
    // what actually happens? Well, sem_post atomically increases the semaphore
    // value. Any sem_wait calls will block if the value is 0, and if/when the
    // value is nonzero they decrement the semaphore. As a result, calling
    // sem_post will cause the sem_wait to unblock, and also decrement the
    // semaphore.
    //
    // the 'normal' behavior is when you don't enter "FAST". Note that this
    // thread never actually waits... the only reason it's "safe" is because
    // with console input, we can assume input time is sufficient for the
    // thread to finish processing before the next call to fgets unblocks.
    //
    // but by inputting "FAST", you can see that the write is not actually
    // protected... it says there are 8 charaters 3 times, and never shows 4 at
    // all!
    //
    // Why? Well, the initial fgets sets work_area to FAST, and then we
    // incr the semaphore with sem_post. But before the thread can respond,
    // this thread comes back, and instead of pausing at fgets again, it
    // goes into the `if` block, where it posts and extra time, then copies
    // "Wheeee..." to the work_area, and then calls sem_post yet again.
    //
    // By the time the first thread blocks and the second thread starts
    // (well, depending on your computer - it's possible but unlikely the
    // second thread starts earlier) you've already rewritten work_area, and
    // incremented the semaphore 3 times. Hence the thread reads prints 3
    // different messages and decrements the semaphore for each one.
    printf("Input some text. Enter 'end' to finish, FAST to demo a 'bug'\n");
    while(strncmp("end", work_area, 3) != 0) {
      if (strncmp(work_area, "FAST", 4) == 0) {
        sem_post(&bin_sem);
        strcpy(work_area, "Wheeee...");
      } else {
        fgets(work_area, WORK_SIZE, stdin);
      }
      sem_post(&bin_sem);
    }

    // cleanup. Use sem_destroy to free semaphore resources.
    sem_destroy(&bin_sem);
    exit(EXIT_SUCCESS);
}

/* Function to run in thread. Ignores argument and returns null.
 *
 * Waits until the semaphore unlocks this thread, and then it prints
 * out the length of the `work_area` string. If it sees "end" as the
 * `work_area` string, it stops. */
void *thread_function(void *arg) {
    sem_wait(&bin_sem); // this block until the semaphore is nonzero
    while(strncmp("end", work_area, 3) != 0) {
        printf("You input %d characters\n", (int) strlen(work_area) - 1);
        sem_wait(&bin_sem);
    }
    pthread_exit(NULL);
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *thread_function(void *arg);
pthread_mutex_t work_mutex;  // somewhat misnamed, protect both work_area and
int time_to_exit = 0;        //   time_to_exit

#define WORK_SIZE 1024
char work_area[WORK_SIZE];

/* A mostly direct translation of the semaphore example (other than we also
 * fix the bug related to the main thread not waiting) into mutex locking.
 *
 * We add a global variable to handle exit, and move the fgets around
 * a bit so that we can properly lock and such.
 *
 * Every lock call blocks unless / until the lock is free.
 */

int main() {
    int res;
    pthread_t a_thread;
    void *thread_result;

    // init the mutex
    res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    // create the thread
    res = pthread_create(&a_thread, NULL, thread_function, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // loop and read input. We get the lock first here b/c of the sleep at the
    // top of thread_function... we always own the lock when we hit the top
    // of the while loop, which is important.
    pthread_mutex_lock(&work_mutex);
    printf("Input some text. Enter 'end' to finish\n");
    while(!time_to_exit) {
        fgets(work_area, WORK_SIZE, stdin);
        pthread_mutex_unlock(&work_mutex);
        while(1) {
            // this loop checks for whether thread_function has read the
            // data yet. When it has, we go to the top of the outer loop and
            // read the next line.
            pthread_mutex_lock(&work_mutex);
            if (work_area[0] != '\0') {
                pthread_mutex_unlock(&work_mutex);
                sleep(1);
            }
            else {
                break;
            }
        }
    }

    // wait for the thread.. I think that under normal circumstances, we
    // actually don't wait, b/c the thread already released its lock and
    // exited by the last time we acquired the lock. But I'm not sure. In
    // general you want to unlock and then join to avoid deadlocks.
    pthread_mutex_unlock(&work_mutex);
    printf("\nWaiting for thread to finish...\n");
    res = pthread_join(a_thread, &thread_result);
    if (res != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }
    printf("Thread joined\n");

    // cleanup. Note that pthread_join took care of the thread cleanup.
    pthread_mutex_destroy(&work_mutex);
    exit(EXIT_SUCCESS);
}


/* Demo thread function. It sleeps first (which ensures that the main thread
 * acquires the first lock), and from then on every time the mutex lock is
 * available, it will look at the current value of work_area, count the number
 * of characters, reset work_area[0] to '\0' (which is how the main thread
 * knows it's time to read more input), and unlock.
 *
 * After all but the first lock, it also checks that work_area[0] isn't
 * already '\0', which is necessary b/c we might still be waiting for input,
 * and the main thread unlocks while it's waiting for input.
 */
void *thread_function(void *arg) {
    sleep(1);
    pthread_mutex_lock(&work_mutex);
    while(strncmp("end", work_area, 3) != 0) {
        printf("You input %d characters\n", (int) strlen(work_area) -1);
        work_area[0] = '\0';
        pthread_mutex_unlock(&work_mutex);
        sleep(1);
        // until the main thread resets work_area, just keep waiting.
        pthread_mutex_lock(&work_mutex);
        while (work_area[0] == '\0' ) {
            pthread_mutex_unlock(&work_mutex);
            sleep(1);
            pthread_mutex_lock(&work_mutex);
        }
        // note that at the top of the loop, the lock is always *on*.
    }
    // once we see "end", set time_to_exit (note that the lock is *on* here!)
    // so the main thread knows we can quit.
    time_to_exit = 1;
    work_area[0] = '\0';
    pthread_mutex_unlock(&work_mutex);
    pthread_exit(0);
}

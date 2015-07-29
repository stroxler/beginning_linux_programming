#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *thread_function(void *arg);

/* Simple demo of cancelling a thread.
 *
 * by setting cancelstate to PTHREAD_CANCEL_ENABLE you allow cancels
 *   in the thread (if you use DISABLE, it ignores them)
 * by setting canceltype to PTHREAD_CANCEL_DEFERRED you make it so the
 *   cancel doesn't terminate the thread until the next call to a mutex
 *   operation or a sleep or some other blocking point. The exact set of
 *   functions that act as cancellation points is implementation-dependent;
 *   we use sleep here but it isn't guaranteed, so if you rely on this,
 *   you should call pthread_testcancel() in the thread
 *   If you use PTHREAD_CANCEL_ASYNCHRONOUS, cancels happen immediately instead
 *
 * We actually don't need to setcancelstate or setcanceltype here, because
 * PTHREAD_CANCEL_ENABLE and PTHREAD_CANCEL_DEFERRED are the defaults. But
 * we do it explicilty to demo the api.
 */

int main() {
    int res;
    pthread_t a_thread;
    void *thread_result;

    // make the thread
    res = pthread_create(&a_thread, NULL, thread_function, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // give it some time to run, then cancel it
    sleep(3);
    printf("Canceling thread...\n");
    res = pthread_cancel(a_thread);
    if (res != 0) {
        perror("Thread cancelation failed");
        exit(EXIT_FAILURE);
    }

    // join the thread, which will happen as soon as it picks up the cancel
    printf("Waiting for thread to finish...\n");
    res = pthread_join(a_thread, &thread_result);
    if (res != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void *thread_function(void *arg) {
    int i, res, j;
    /*
    res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (res != 0) {
        perror("Thread pthread_setcancelstate failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    if (res != 0) {
        perror("Thread pthread_setcanceltype failed");
        exit(EXIT_FAILURE);
    }
    */
    printf("thread_function is running\n");
    for(i = 0; i < 10; i++) {
        printf("Thread is still running (%d)...\n", i);
        sleep(1);
    }
    pthread_exit(0);
}


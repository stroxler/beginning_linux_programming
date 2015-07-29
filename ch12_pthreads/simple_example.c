#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


/* Basic example of threads, without using any tools (semaphores, mutexes, etc)
 * to ease interaction; interaction is via a global var.
 *
 * Demonstrates the basic calls to the pthread functions. Note that the 
 * sleep is necessary: if two threads mutate the same global var (run_now)
 * at the same time, the results are undefined. The sleeps are our hackish
 * way of preventing this.
 */
void *thread_function(void *arg);
int run_now = 1;
char message[] = "Hello World";

int main() {
    int res;
    pthread_t a_thread;
    void *thread_result;
    int print_count1 = 0;

    // create a thread. To do this, you pass a (likely null) pthread_t, a
    // flag variable (which is NULL unless you want special behavior), a
    // function, and an argument to the function.
    //
    // The function always has to take a single void * argument, and return a
    // void *; it doesn't actually return anythong though, instead it calls
    // pthread_exit (which is maybe a macro?)
    res = pthread_create(&a_thread, NULL, thread_function, (void *)message);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    // print 1 out, 5 times, to indicate that we are in the first thread
    // inside this and the thread function, we are passing control back and
    // forth via the global run_now variable. We'll see better ways to handle
    // control later.
    while(print_count1++ < 5) {
        if (run_now == 1) {
            printf("1");
            run_now = 2;
        } else {
            sleep(1);
        }
    }

    // join the thread.
    //   ... note that thread_result comes from the call to pthread_exit inside
    //       the thread. Also note that we pass not the pointer, but the
    //       address of the pointer. The pthread_join will make it point to
    //       data created in the thread.
    printf("\nIn main, waiting for thread to finish...\n");
    res = pthread_join(a_thread, &thread_result);
    if (res != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }
    printf("Thread joined, thread_result (cast to str) is %s\n",
           (char *) thread_result);
    exit(EXIT_SUCCESS);
}

/* print 2 out, 5 times, to indicate that we are in the second thread */
void *thread_function(void *arg) {
    int print_count2 = 0;
    while(print_count2++ < 5) {
        if (run_now == 2) {
            printf("2");
            run_now = 1;
        } else {
            sleep(1);
        }
    }
    printf("\nInside thread, message (arg) was %s\n", (char *) arg);
    sleep(3);
    pthread_exit((void *) "This is what my thread 'returns' via pthread_exit");
}

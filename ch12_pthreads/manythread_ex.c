#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 6

/* Demonstration of starting many threads.
 *
 * We also show here that the void * argument to a thread function doesn't
 * actually have to be a pointer... in an initial version the authors pass
 * an int via pointer, but this actually leads to race conditions because we
 * are incrementing the lots_of_threads var in the main thread. In this
 * version, instead we pass the int by value but cast to void *, and then cast
 * back to int.
 *    -> I'm pretty sure this hack only works because int is no larger than
 *       void *, so when you cast int to void * you potentially add some bits,
 *       and then after passing by value (all passes are by value in C!) when
 *       you cast back to int you just discard those bits. If you tried to
 *       pass a big struct this way it would presumably fail. But, ints are
 *       very likely the most common use case for spnning up many threads like
 *       this.
 *    -> the compiler warns on the casts... they are probably bad practice.
 *       but I'm leaving them in the demo because they are cool examples of
 *       what's possible, and they don't interfere with showing the api.
 *
 * Because we spin up all the threads almost at once, the order in which
 * they announce themselves is close to random. In a test run, for instance,
 * the first one to say hi was thread 3.
 */

void *thread_function(void *arg);

int main() {
  
    int res;
    pthread_t a_thread[NUM_THREADS];
    void *thread_result;
    int lots_of_threads;
    

    for(lots_of_threads = 0; lots_of_threads < NUM_THREADS; lots_of_threads++) {
        // note that we don't actually pass a pointer here, but we cast
        // to a pointer... this is pretty strange! We are kind of sidestepping
        // the operating system's thread api.
        res = pthread_create(&(a_thread[lots_of_threads]), NULL,
                             thread_function, (void *)lots_of_threads);
        if (res != 0) {
          perror("Thread creation failed");
          exit(EXIT_FAILURE);
        }
    }

    printf("Waiting for threads to finish...\n");
    for(lots_of_threads = NUM_THREADS - 1; lots_of_threads >= 0; lots_of_threads--) {
      res = pthread_join(a_thread[lots_of_threads], &thread_result);
      if (res == 0) {
        printf("Picked up a thread\n");
      } else {
        perror("pthread_join failed");
      }
    }

    printf("All done\n");
    exit(EXIT_SUCCESS);
}

void *thread_function(void *arg) {
    int my_number = (int)arg;
    int rand_num;

    printf("thread_function is running. Argument was %d\n", my_number);
    rand_num=1+(int)(9.0*rand()/(RAND_MAX+1.0));
    sleep(rand_num);
    printf("Bye from %d\n", my_number);
    
    pthread_exit(NULL);
}

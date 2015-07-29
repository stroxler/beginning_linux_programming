#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

// just to be safe... most modern systems should be fine...
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
#error "Sorry, your system does not support thread priority scheduling"
#endif

void *thread_function(void *arg);

char message[] = "Hello World";
int thread_finished = 0;

/* Simple demo program of setting thread attributes, which all have similar
 * apis. The two we set are:
 *
 *    - detached: note that we don't join. The only way we know the child
 *      thread died is via a global variable, which we wouldn't need if we 
 *      weren't doing a demo. A detached thread is a 'fire-and-forget' thread.
 *
 *    - set priority: first we have to set the schedpolicy, and then we
 *      can setschedparam with a thread priority. Here we set the child thread
 *      to have minimum priority.
 *
 * There's also an api to alter thread attributes on an already created thread,
 * but here we just set them in the pthread_create function.
 */

int main() {
  int res;
  pthread_t a_thread;
  void *thread_result;
  pthread_attr_t thread_attr;
  int max_priority, min_priority;
  struct sched_param scheduling_value;

  // initialize the thread attributes struct
  res = pthread_attr_init(&thread_attr);
  if (res != 0) {
    perror("Attribute creation failed");
    exit(EXIT_FAILURE);
  }

  // set the flag that makes the thread detached.
  res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  if (res != 0) {
    perror("Setting detached attribute failed");
    exit(EXIT_FAILURE);
  }

  // set the scheduling policy... this doesn't actually change the priority,
  // but it sets the thread_attr to a state where priority can be set.
  //   (this isn't a good in-depth look, just an api demo, see docs for info)
  res = pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
  if (res != 0) {
    perror("Setting schedpolicy failed");
    exit(EXIT_FAILURE);
  }

  // query the system for the max and min piorioty (the max is only so we
  // can print interesting info, it isn't needed), and set the attributes so
  // that the thread gets min priority.
  max_priority = sched_get_priority_max(SCHED_OTHER);
  min_priority = sched_get_priority_min(SCHED_OTHER);
  scheduling_value.sched_priority = min_priority;
  res = pthread_attr_setschedparam(&thread_attr, &scheduling_value);
  if (res != 0) {
    perror("Setting schedpolicy failed");
    exit(EXIT_FAILURE);
  }
  printf("Scheduling priority set to %d, max allowed was %d\n",
         min_priority, max_priority);
  
  // now that we've set all the attributes, we can creat the thread!
  res = pthread_create(&a_thread, &thread_attr, thread_function,
                       (void *)message);
  if (res != 0) {
    perror("Thread creation failed");
    exit(EXIT_FAILURE);
  }

  // we don't need the attributes anymore, clean them up. Note that we can
  // do this as soon as the thread is created, we don't have to wait for it
  // to finish.
  (void)pthread_attr_destroy(&thread_attr);
 
  // this isn't needed in principle, but for the sake of a good demo we look
  // at a global variable to find out when the detached thread ended.
  //    ... note all the sleeps; access is racey without locking, but the
  //        sleeps work well enough for a simple demo.
  while(!thread_finished) {
    printf("Waiting for thread...\n");
    sleep(1);
  }

  printf("Thread finished, bye!\n");
  exit(EXIT_SUCCESS);
}

/* function to run in our fire-and-forget thread.
 * For the sake of the demo, we use thread_finished, a global var, to tell
 * the main thread we are done. */
void *thread_function(void *arg) {
  printf("thread_function is running. Argument was %s\n", (char *)arg);
  sleep(2);
  thread_finished = 1;
  pthread_exit(NULL);
}

/* This program is very similar to basic_signal_demo, except it uses the
 * newer sigaction api, which is supposedly to be preferred.
 *
 * To use the `sigaction` func, you map a signal to a handler via a `sigaction`
 * struct. (the name is overloaded - this is our first clear demo that C uses
 * distinct namespaces for functions and dtypes).
 *
 * before you call sigaction, you need to set the handler, mask and flags of
 * the sigaction struct. Use `sigemptyset` and flags of 0 for basic behavior;
 * read the docs if you want to know more (we are just demoing the api here).
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

struct sigaction old_act;

void ouch(int sig) {
    printf("OUCH! - I got signal %d\n", sig);
    sigaction(SIGINT, &old_act, NULL);
}

int main() {
    struct sigaction act;

    act.sa_handler = ouch;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    // the third arg, if non-NULL, is a place to save the old action. It's
    // ignored if null.
    sigaction(SIGINT, &act, &old_act);

  while(1) {
    printf("Hello World!\n");
    sleep(1);
  }
}

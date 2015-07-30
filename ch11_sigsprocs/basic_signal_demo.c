/* In this demo, we use the initial signal() call (in main) to set our
 * program up to call ouch when SIGINT is called. The signal is also passed
 * to ouch(), which isn't needed in this example because we only bound one
 * signal, but in general we could have bound ouch() to many signals.
 *
 * Inside the ouch() function, we remap the signal handler back to the
 * default handler (you can also map it to SIG_IGN, which would ignore signals)
 *
 * To use it, try pressing ctl-c once, and you'll get a message. If you press
 * it again, the program will exit as usual.
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void ouch(int sig) {
    printf("OUCH! - I got signal %d\n", sig);
    (void) signal(SIGINT, SIG_DFL);
}

/*  The main function has to intercept the SIGINT signal generated when we type Ctrl-C .
    For the rest of the time, it just sits in an infinite loop,
    printing a message once a second.  */

int main() {
    (void) signal(SIGINT, ouch);

    while(1) {
        printf("Hello World!\n");
        sleep(1);
    }
}

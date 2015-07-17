#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


/* Syslog demo.
 * This program outputs nothing. On the ubuntu machine I tested with,
 * you can see its output by running
 *   tail /var/log/syslog
 * On a lot of systems, you would instead do
 *   tail /var/log/messages
 *
 * This is also the first demo of getpid(), which is self-explanatory. There's
 * also a getppid which gets the parent's pid.
 */

int main() {
    int logmask;
    // SET UP LOGGING PARAMS
    // you can do syslog without this, but it modifies stuff: your logs will
    // say they came from the "logmask" program, and will be logged with a
    // pid. LOG_CONS says to log to console if there is an error accessing
    // the sys log.
    // The LOG_USER says to do generic user messages. See
    //   http://www.gsp.com/cgi-bin/man.cgi?section=3&topic=openlog
    openlog("logmask", LOG_PID|LOG_CONS, LOG_USER);

    // you can also get the "user-style" logging if you put
    // LOG_LEVEL|LOG_USER directly in the first arg to syslog. But it's
    // nicer to call openlog first, so that you can customize the prefixes
    // and it's easier to disentagle your program from others.
    
    // SEND SOME SIMPLE LOG MESSAGES
    // note that syslog takes printf-style formatting and varargs.
    syslog(LOG_INFO, "the pid should also be prefixed, it is %d", getpid());
    syslog(LOG_DEBUG, "This debug message will show up b/c default level");

    // RESET LOGGING LEVEL
    // (DEBUG is lower than NOTICE) Amazingly, the docs don't say what is
    // returned from this function; at any rate one calls it for side-effects.
    // the LOG_UPTO macro is named that way because the system uses small
    // numbers for high priority (much like nice).
    logmask = setlogmask(LOG_UPTO(LOG_NOTICE));
    syslog(LOG_DEBUG, "This message won't appear");

    // You can use the %m code to print the message encoded in errno
    FILE* f = fopen("file_does_not_exist.txt", "r");
    if (!f) {
        syslog(LOG_ERR, "This message should appear, an error: %m");
    }

    return 0;
}






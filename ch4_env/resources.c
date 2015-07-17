#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* This program demos getting priority and resource data.
 * NOTE: it won't compile unless you add the -lm flag on many boxes,
 *       including my ubuntu machine. math.h is added automatically,
 *       but the linker doesn't auto-link libm.
 */

void work();

int main() {

    // get this process's priority. The higher the number, the lower the
    // priority (it's basically the `nice` value, see the demo notes).
    //   the input PRIO_PROCESS says to get the priority for a process (by pid)
    //   which is usually what we want.
    int priority = getpriority(PRIO_PROCESS, getpid());
    printf("Current priority = %d\n\n", priority);

    // get resource usage date for this process, after doing some work.
    //    ru_utime is the user type, usec is microseconds
    struct rusage r_usage;
    work();
    getrusage(RUSAGE_SELF, &r_usage);
    printf("CPU usage: User = %ld.%06ld, System = %ld.%06ld\n\n",
            r_usage.ru_utime.tv_sec, r_usage.ru_utime.tv_usec,
            r_usage.ru_stime.tv_sec, r_usage.ru_stime.tv_usec);

    // get the current resource limit for file sizes.
    //    the soft limit is "an advisory limit that shouldn't be exceeded,
    //    doing so can cause library functions to return errors"
    //    the hard limit is the point at which the operating system might
    //    send a terminate signal to your process.
    struct rlimit r_limit;
    getrlimit(RLIMIT_FSIZE, &r_limit);
    printf("Current FSIZE limit: soft = %ld, hard = %ld\n\n",
           r_limit.rlim_cur, r_limit.rlim_max);

    // set a tight limit on the file size this process can create, then
    // call work() again and generate an error
    r_limit.rlim_cur = 2048;
    r_limit.rlim_max = 4096;
    printf("Setting a 2K file size limit\n");
    setrlimit(RLIMIT_FSIZE, &r_limit);
    printf("Limit set.\n");
    work();

    return 0;
}

/* does a bunch of io (write to a temp file) and cpu (stupid math) to
 * generate usage data */
void work() {
    FILE * f;
    int i;
    double x = 4.5;

    f = tmpfile();  // we ought to be doing error checking and closing the
                    // file; we don't b/c we are just demoing apis here.
    for (i = 0; i < 10000; i++) {
        fprintf(f, "Do some output\n");
        if (ferror(f)) {
            fprintf(stderr, "Error writing to temporary file.\n");
            exit(1);
        }
    }
    for (i = 0; i < 1000000; i++) {
        x = log(x * x + 3.21);
    }
}

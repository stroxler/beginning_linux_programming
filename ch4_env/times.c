/* page 150-155. The discussion of strftime / strptime on page
 * 154-156 might prove especially helpful */
#define _GNU_SOURCE // this is needed for strptime to work on linux
                    // (mac was fine without it). Without this, the compiler
                    // warns about an int to pointer issue, and the code
                    // segfaults.
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_tm(struct tm * tm_time) {
    printf("\tdate: %04d-%02d-%02d\n", 1900+tm_time->tm_year, tm_time->tm_mon+1,
           tm_time->tm_mday);
    printf("\ttime: %02d:%02d:%02d\n", tm_time->tm_hour, tm_time->tm_min,
           tm_time->tm_sec);
}

int main() {
    int i;
    time_t int_time;
    struct tm *tm_time;

    // print off raw times: you can see, time is just a timestamp in seconds.
    for (i = 1; i <= 2; i++) {
        // the time function is kind of weird. It returns, but it also modifies
        // its input, *unless* the input is null (as is `(time_t *) 0`)
        //   here we demo both approaches.
        int_time = time((time_t *) 0);
        printf("The time is %ld\n", int_time);
        sleep(1);
        time(&int_time);
        printf("The time is %ld\n", int_time);
        sleep(1);
    }

    // gmtime can help us print a useful time. It outputs a struct of
    // type `*tm_ptr`. It takes an addres to time_t, which is sort of strange.
    //
    // also, the output has years as a count since 1900. A holdout from when
    // memory was more precious, I think.
    printf("\nRaw time is %ld\n", int_time);

    tm_time = gmtime(&int_time);
    printf("gmtime gives:\n");
    print_tm(tm_time);

    tm_time = localtime(&int_time);
    printf("localtime gives:\n");
    print_tm(tm_time);

    // ctime provides more convenient times if you just want human-readable
    // strings to start with.
    printf("\nctime gives: %s\n", ctime(&int_time));

    // we can also work with the `tm` struct using
    //   strftime = string format time, for converting time to string
    //   strptime = ? string put to time ? for converting string to time

    // converting from string...
    char buf[256];
    char *remaining;
    strcpy(buf, "Thu 26 July 2007, 17:53 and then more text after");
    printf("Converting buf = \"%s\" to time...", buf);
    remaining = strptime(buf, "%a %d %b %Y, %R", tm_time);
    printf("the copied tm is:\n");
    print_tm(tm_time);
    printf("The remaining (unconsumed) string is \"%s\"\n", remaining);

    // converting back to string...
    printf("strftime gives:\n");
    strftime(buf, 256, "%A %d %B %I:%S %p", tm_time);
    printf("\t\"%s\"\n", buf);

    exit(0);
}

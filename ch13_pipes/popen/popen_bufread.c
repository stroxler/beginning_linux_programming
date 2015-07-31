#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* This is a slight extension of popen_read, where we do two things:
 *   1. we demonstrate that popen starts a shell by doing ps aux, which
 *      will show a child *and* a grandchild of this process, because popen
 *      makes a shell and then it is fork / exec'ed to create the ps. This
 *      means doing a ton of popens can cause overhead.
 *   2. We show the api for reading a buffer a bit at a time. The previous
 *      examples only read one buffer worth of input because the programs
 *      were small, but here we make no such assumption, and we keep
 *      freading till we are done.
 *
 *      ... nothing is really new here, since the fread api we are using is
 *      the same as it would be for files (there are differences, I think,
 *      e.g. I'm pretty sure you can't do arbitrary seeks, but I'm guessing
 *      the operating system handles those errors; the api allows it)
 */

int main()
{
    FILE *read_fp;
    char buffer[BUFSIZ + 1];
    int chars_read;

    memset(buffer, '\0', sizeof(buffer));
    read_fp = popen("ps aux", "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), BUFSIZ/10, read_fp);
        while (chars_read > 0) {
            // The authors used `chars_read - 1` here, but that's an error
            // and overwrites the last byte read in. How do I know? I tried
            // changing the command to "echo -n 'some_stuff'", and saw that
            // the last f was removed.
            //
            // I'm guessing the authors tested using echo without the -n, and
            // the newline deleted canceled out with the newline in their
            // printf call, which is how they messed it up.
            buffer[chars_read] = '\0';
            printf("Reading %d:-\n %s\n", BUFSIZ/10, buffer);
            chars_read = fread(buffer, sizeof(char), BUFSIZ/10, read_fp);
        }
        pclose(read_fp);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

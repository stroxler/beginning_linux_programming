#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Like read_ex0, this is a minimal example of popen, except this time we
 * popen as "w", which means the new process's stdin will come from the
 * FILE we create.
 *
 * In this case we pipe to an awk program that will print just the second
 * work ("upon")
 */

int main() {
    FILE *write_fp;
    char buffer[BUFSIZ + 1];

    sprintf(buffer, "Once upon a time, there was...\n");
    write_fp = popen("awk '{print $2}'", "w");
    if (write_fp != NULL) {
        fwrite(buffer, sizeof(char), strlen(buffer), write_fp);
        pclose(write_fp);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

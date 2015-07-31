#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Simple example of a read popen.
 * popen("command", "mode") creates a FILE similar to fopen(), except that
 * instead of a filename, you give it a command.
 *
 * It only allows "r" or "w" as modes, and automatically sets you to piping
 * that processes stdout or stdin depending on whether you have "r" or "w".
 */

int main() {
    FILE *read_fp;
    char buffer[BUFSIZ + 1]; // bufsize is a constant set in stdio
    int chars_read;
    memset(buffer, '\0', sizeof(buffer));
    read_fp = popen("uname -a", "r");
    if (read_fp != NULL) {
        chars_read = fread(buffer, sizeof(char), BUFSIZ, read_fp);
        if (chars_read > 0) {
            printf("Output of \"uname -a\" was:-\n%s\n", buffer);
        }
        pclose(read_fp);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}

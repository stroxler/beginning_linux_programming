#include <unistd.h>    // Include this first
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>


void printdir(char *dirname, int depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if ((dir = opendir(dirname)) == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dirname);
        return;
    }

    chdir(dirname);  // this is how you change working directories in C
                     // I'm not sure whether it's needed... I'm guessing that
                     // without this the way we are calling lstat, and maybe
                     // also the comparisons with '.' and '..', would fail.

    while((entry = readdir(dir)) != NULL) {  // readdir iterates through
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {
            /* found a directory. But we should skip '.' and '..' */
            if (strcmp(".", entry->d_name) == 0 ||
                strcmp("..", entry->d_name) == 0) { continue; }

            /* print at this indent level, then recurse at new indent level */
            printf("%*s%s\n", depth, "", entry->d_name);
            printdir(entry->d_name, depth+4);
        }
        else {
            printf("%*s%s\n", depth, "", entry->d_name);
        }
    }

    chdir(".."); // undo the prev chdir... it doesn't work at the top level,
                 // but we don't care there anyway (although it might cause
                 // an error if we try to scan '/')

    closedir(dir); // don't forget to clean up!
                   // the prev file copy example skipped this b/c everything
                   // was in main, but here we are redursing, so we should make
                  // sure the number of open dirs is bounded by the depth of
                  // the scan.
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <dir to scan>\n", argv[0]);
        exit(1);
    }

    char *dirname = argv[1];
    printdir(dirname, 0);
    printf("[done]\n");

    return 0;
}

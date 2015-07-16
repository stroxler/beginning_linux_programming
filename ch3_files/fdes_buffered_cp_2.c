#include <unistd.h>    // has to come first b/c it can affect the others
                       // via flags
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>    // I think this is only used for exit
#include <stdio.h>     // not used for the example, just the usage message


int main(int argc, char *argv[]) {
    char block[1024];
    int infdes, outfdes;
    int nread;

    if (argc != 3) {
        printf("Usage: %s <input file> <output file>", argv[0]);
        exit(1);
    }

    // open file in read-only mode
    infdes = open(argv[1], O_RDONLY);
    // open file in write-only mode, creating if it doesn't exist and
    // assigning the user read and write permissions if umask allows it (the
    // actual permissions are the pointwise min of umask and what's requested)
    outfdes = open(argv[2], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

    while((nread = read(infdes, block, 1)) > 0) {
        write(outfdes, block, nread);
    }

    return 0;
}

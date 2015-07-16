#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
    char c;
    FILE *inf, *outf;

    if (argc != 3) {
        printf("Usage: %s <input file> <output file>", argv[0]);
        exit(1);
    }

    // open file in read-only mode
    inf = fopen(argv[1], "r");
    // open file in write-only mode
    //    note that unlike the fdes version, we don't specify create
    //    or permissions. I'm pretty sure it's defaulting them the same as
    //    we hand-specified in our fdes examples
    outf = fopen(argv[2], "w");

    // note that there's no need for a block; the FILEs already buffer
    while((c = fgetc(inf)) != EOF) {
        fputc(c, outf); 
    }

    return 0;
}

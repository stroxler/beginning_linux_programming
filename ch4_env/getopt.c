// page 141
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* Suggested example call is ./a.out -i -lr 'hi there' -f fred.c -q */

int main(int argc, char *argv[]) {
    int opt;
    // the leading colon could be left off for raw functionality. The benefit
    // is that we can detect missing args as distinct from unknown options - if
    // you leave off the leading :, then both situations would cause the case
    // to match '?', so you can't distinguish.
    while ((opt = getopt(argc, argv, ":if:lr")) != -1) {
        switch (opt) {
            case 'i':
            case 'l':
            case 'r':
                printf("option: %c\n", opt);
                break;
            case 'f':
                printf("filename: %s\n", optarg);
                break;
            case ':':
                printf("option '%c' needs a value\n", optopt);
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
        }

    }
    // optind has the index of the last argv parsed by optarg
    // ...
    // this works even if the arg is before some options, because linux's
    // getopt rearranges for you. That's why `rm file -rf` works on
    // linux but not mac.
    for(; optind < argc; optind++) {
        printf("argument: %s\n", argv[optind]);
    }
    return 0;
}

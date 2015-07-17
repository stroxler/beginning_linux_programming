// page 143 ... the inputs are a bit strange, see p 143 144 for discussion
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define _GNU_SOURCE
#include <getopt.h>


/* Suggested example call is ./a.out -i --list 'hi there' -f fred.c -q */

int main(int argc, char *argv[]) {
    int opt;

    struct option longopts[] = {
        {"initialize", 0, NULL, 'i'},
        {"file", 1, NULL, 'f'},
        {"list", 0, NULL, 'l'},
        {"restart", 0, NULL, 'r'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, ":if:lr", longopts, NULL)) != -1) {
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

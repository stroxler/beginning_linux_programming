#include <stdio.h>

/* Simple program that uppercases stdin and writes to stdout */

int main() {
    int ch;
    while((ch = getchar()) != EOF) {
        putchar(toupper(ch));
    }
    return 0;
}

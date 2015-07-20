#include <stdio.h>
#include <stdlib.h>

int main() {
    int *some_memory, *more_memory;
    size_t SIZE1 = 50, SIZE2 = 100;
    int i;

    // malloc assigns memory, does not initialize
    //     that said, often you'll get all zeros anyway, if the ram you are
    //     given hasn't been used before
    some_memory = (int *) malloc(SIZE1 * sizeof(int));
    if (!some_memory) {
        fprintf(stderr, "Memory error.");
        free(some_memory);
        exit(1);
    }
    printf("After using malloc, some_memory isi %p, with contents:",
            some_memory);
    for (i = 0; i < SIZE1; i++) {
        printf(" %d", some_memory[i]);
    }
    printf("\n");

    // calloc has a slightly different interface, and zero-initializes.
    some_memory = (int *) calloc(sizeof(int), SIZE1);
    if (!some_memory) {
        fprintf(stderr, "Memory error.");
        free(some_memory);
        exit(1);
    }
    printf("After using calloc, some_memory isi %p, with contents:",
            some_memory);
    for (i = 0; i < SIZE1; i++) {
        printf(" %d", some_memory[i]);
    }
    printf("\n");


    // realloc tries to give you more space. If it can't, it gives a new
    // address and mem copies your data.
    //   Note that we use a different pointer for the memory - it doesn't
    // matter here, but in general realloc can return NULL on memory issues,
    // and so you want to keep your old pointer around just in case -
    // especially if you have a long-running, robust program.
    more_memory = (int *) realloc(some_memory, sizeof(int) * SIZE2);
    if (!more_memory) {
        fprintf(stderr, "Memory error.");
        free(more_memory);
        exit(1);
    }
    printf("After using realloc, more_memory isi %p, with contents:",
            more_memory);
    for (i = 0; i < SIZE2; i++) {
        printf(" %d", more_memory[i]);
    }
    printf("\n");
    exit(0);
}


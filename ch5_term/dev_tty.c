#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* This is like the basic menu, except instead of crashing when the
 * terminal is non-interactive, it directly uses /dev/tty, which
 * allows us to display information and get commands interactively while
 * still directing stdout to a file.
 *
 * Suggested use is ./a.out > junk.txt
 *   (and then, say, a d q)
 */

char *menu[] = {
    "a - add new record",
    "d - delete record",
    "q - quit",
    NULL,
};

int getchoice(char * greet, char * choices[], FILE * input, FILE * output);

int main() {

    FILE * input, * output;

    if (!isatty(fileno(stdout))) {
        fprintf(stderr, "You are not at a terminal. That's okay.");
    }

    input = fopen("/dev/tty", "r");
    output = fopen("/dev/tty", "w");
    if (!input || !output) {
        fprintf(stderr, "Unable to open /dev/tty\n");
        exit(1);
    }

    int choice = 0;
    do {
        choice = getchoice("Please select an action", menu, input, output);
        printf("You have chosen: %c\n", choice);
    } while (choice != 'q');

    return 0;
}

/* Display a menu with a heading line that says "Choice: <greet>", and
 * wait for the user to enter a valid choice.
 *
 * input and output are FILE* here.
 */
int getchoice(char * greet, char * choices[], FILE * input, FILE * output) {
    int selected;
    int chosen = 0; // bool
    char **option;

    do {
        fprintf(output, "Choice: %s\n", greet);
        option = choices;
        while (*option) {
            fprintf(output, "%s\n", *option);
            option++;
        }
        do {
            selected = fgetc(input); // note that fgetc isn't named in
                                     // parallel to getchar. C libraries have
                                     // kind of sucky naming :(
        } while (selected == '\n');

        option = choices;
        while (*option) {
            if (selected == *option[0]) {
                chosen = 1; // true
                break;
            }
            option++;
        }
        if (!chosen) {
            fprintf(output, "Unrecognized choice, select again.\n");
        }
    } while (!chosen);

    return selected;
}

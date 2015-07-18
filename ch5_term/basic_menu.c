#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* Basic menu. It has some messy newline handling because by default the tty
 * doesn't send us data until a newline is entered, and a bit of messy
 * code because getchar() picks up newlines.
 *   ... we fix this in a later program by modifying how tty works
 *
 * It also detects if we are piping output and crashes.
 *   ... we show a workaround in a later program.
 */

// note the null-termination, a common C idiom.
char *menu[] = {
    "a - add new record",
    "d - delete record",
    "q - quit",
    NULL,
};

int getchoice(char * greet, char * choices[]);

int main() {

    // this bit of logic forces the app to quit if we are piping to
    // files. A later version will have a real workaround.
    if (!isatty(fileno(stdout))) {
        fprintf(stderr, "You are not a terminal!");
        exit(1);
    }

    // this bit of logic actually drives the menu
    int choice = 0;
    do {
        choice = getchoice("Please select an action", menu);
        printf("You have chosen: %c\n", choice);
    } while (choice != 'q');

    return 0;
}

/* Display a menu with a heading line that says "Choice: <greet>", and
 * wait for the user to enter a valid choice.
 */
int getchoice(char * greet, char * choices[]) {
    int selected;
    int chosen = 0; // bool
    char **option;

    do {
        printf("Choice: %s\n", greet);
        // note the use of the null termination here
        option = choices;
        while (*option) {
            printf("%s\n", *option);
            option++;
        }

        // this is messy, but without it we will pick up the newline that
        // ended the previous menu choice in getchar()
        do {
            selected = getchar();
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
            // Notice that we get one of these after every successful choice,
            // because getchar() picks up our newlines.
            printf("Unrecognized choice, select again.\n");
        }
    } while (!chosen);

    return selected;
}

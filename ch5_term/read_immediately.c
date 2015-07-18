#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

/* This is very similar to the menu which reads from /dev/tty, but we add
 * termios controls to make characters read without requiring the user
 * to input a newline, and also without echoing (so some of the code looks
 * like password.c).
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
    struct termios initialsettings, newsettings;
    tcgetattr(fileno(stdin), &initialsettings);
    newsettings = initialsettings;
    newsettings.c_lflag &= ~ICANON;  // process input immediately
    newsettings.c_lflag &= ~ISIG;    // treat control signals as orinary
                                     // input (so, e.g., ^C won't interrupt)
    newsettings.c_lflag &= ~ECHO;   // as with the password program, no echo


    if (!isatty(fileno(stdout))) {
        fprintf(stderr, "You are not at a terminal. That's okay.");
    }

    input = fopen("/dev/tty", "r");
    output = fopen("/dev/tty", "w");
    if (!input || !output) {
        fprintf(stderr, "Unable to open /dev/tty\n");
        exit(1);
    }
    if (tcsetattr(fileno(input), TCSANOW, &newsettings) != 0) {
        fprintf(stderr, "Could not set tc attributes!\n");
    }

    int choice = 0;
    do {
        choice = getchoice("Please select an action", menu, input, output);
        printf("You have chosen: %c\n", choice);
    } while (choice != 'q');

    // always reset tc attributes!
    tcsetattr(fileno(input), TCSANOW, &initialsettings);
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
            selected = fgetc(input);
        // note that we added carriage returns to the exceptions. Since we
        // don't require a newline anymore this isn't critical, but the reason
        // we do it is we've turned off the terminal's auto-conversion of
        // \r to \n, which means depending on hardware we might see 'enter'
        // presses as '\r' rather than '\n'.
        } while (selected == '\n' || selected == '\r');

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

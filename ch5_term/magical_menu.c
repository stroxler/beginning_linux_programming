#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <term.h>
#include <curses.h>

/* Here we make our last changes to the menu program, by adding term
 * control functions.
 *
 * NOTE: This one needs to be compiled with the -lcurses flag.
 *
 * The details are a bit hard to follow and probably not worth memorizing,
 * see the docs at:
 * http://linux.die.net/man/3/tparm
 */

char *menu[] = {
    "a - add new record",
    "d - delete record",
    "q - quit",
    NULL,
};

// we have to have a global output so that `char_to_terminal` can use it
// in closure. Why? Because tputs takes a function pointer that accepts a
// char. (This makes you appreciate other languages more... a global var is
// not a nice hack).
FILE * global_output;

int getchoice(char * greet, char * choices[], FILE * input, FILE * output);

int char_to_terminal(int char_to_write);

int main() {

    FILE * input, * output;
    struct termios initialsettings, newsettings;
    tcgetattr(fileno(stdin), &initialsettings);
    newsettings = initialsettings;
    newsettings.c_lflag &= ~ICANON;
    newsettings.c_lflag &= ~ISIG;
    newsettings.c_lflag &= ~ECHO;


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

        // if you leave this off, you'll never actually see the choice!
        sleep(2);

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
    int screenrow, screencol = 10;
    char *cursor, *clear;
    char **option;

    global_output = output;

    /* we should really be checking error conditions here */
    setupterm(NULL, fileno(output), (int *)0);

    // here we are asking the terminfo code to give us the strings we can
    // use to control terminal behavior.
    cursor = tigetstr("cup"); // cursor position
    clear = tigetstr("clear"); //

    screenrow = 4;
    // always use tputs to put or putp to put terminal control strings.
    // the first entry is the string, the second is the number of lines
    // which should be affected, and the third is a callback to a function
    // which actually adds the characters.
    tputs(clear, 1, char_to_terminal);
    // the tparm call parameterizes the `cursor` string, which is sort of
    // a template, with the proper details to move the cursor.
    tputs(tparm(cursor, screenrow, screencol), 1, char_to_terminal);

    fprintf(output, "Choice: %s\n", greet);
    screenrow += 2;
    option = choices;
    while (*option) {
        tputs(tparm(cursor, screenrow, screencol), 1, char_to_terminal);
        fprintf(output, "%s\n", *option);
        screenrow++;
        option++;
    }
    fprintf(output, "\n");

    do {
        fflush(output);
        selected = fgetc(input);
        option = choices;
        while (*option) {
            if (selected == *option[0]) {
                chosen = 1; // true
                break;
            }
            option++;
        }
        if (!chosen) {
            tputs(tparm(cursor, screenrow, screencol), 1, char_to_terminal);
            fprintf(output, "Unrecognized choice, select again.\n");
        }
    } while (!chosen);

    tputs(clear, 1, char_to_terminal);
    return selected;
}

/* This puts a character to `global output`. It's used as a callback to
 * `tputs`. */
int char_to_terminal(int char_to_write) {
    if (global_output) putc(char_to_write, global_output);
    return 0;
}

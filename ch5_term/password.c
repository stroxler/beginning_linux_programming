#include <termios.h>
#include <stdio.h>
#include <stdlib.h>


/* Simple example of turning off echoing, as is generally done when
 * fetching passwords from the terminal.
 */
#define PASSWORD_LEN 8

int main() {
    // if you are going to change termios settings, *always* save the old ones
    // and reset!
    struct termios initialsettings, newsettings;
    char password[PASSWORD_LEN + 1];

    tcgetattr(fileno(stdin), &initialsettings);
    newsettings = initialsettings;
    // turn off echoing by using (~ECHO) as a bitmask
    newsettings.c_lflag &= ~ECHO;
    fprintf(stdout, "Enter password: ");
    if (tcsetattr(fileno(stdin), TCSAFLUSH, &newsettings) != 0) {
        fprintf(stderr, "Could not set attributes!\n");
    } else {
        fgets(password, PASSWORD_LEN, stdin);
        tcsetattr(fileno(stdin), TCSANOW, &initialsettings);
        fprintf(stdout, "\nYou entered \"%s\".\n", password);
    }
    return 0;
}

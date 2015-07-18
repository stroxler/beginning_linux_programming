#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <unistd.h>

/* Create a tiny library that emulates MS DOS's kbhit (keyboard hit) function,
 * which lets you busy-loop waiting for keyboard input. This is not commonly
 * done in unix, so there's no built-in tool, but this micro-library can help
 * you port programs; more importantly it's a nice example of playing with
 * the terminal settings.
 *
 * The key thing here is that
 *   1. in the init_keyboard settings, we set c_cc so that we unblock waiting
 *      for input on just 1 character with 0 delay, and set no ECHO, no
 *      ICANON, no ISIG.
 *   2. inside kbhit() we change it to unblock on 0 characters, before calling
 *      read [we call read on fd 0, which is stdin]. This allows read to
 *      return without reading anything, if no keys are hit.
 *   3. We save the actual key hit, if any, in the global peek_character, which
 *      can be accessed via readch()
 *   4. We provide a close_keyboard() method, which any program using this
 *      mini-library should call, to reset the term settings.
 *
 * remember, static variables are file-visible only. I think data is actually
 * static by default so this is redundant, but it's not bad to be explicit.
 */

static int peek_character;
static struct termios initial_settings, new_settings;

/* Set up terminal settings: no ECHO, no ICANON, no ISIG, minimum
 * characters for read at 1, no delay for read. Save old settings. */
void init_keyboard() {
    tcgetattr(0, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

/* Reset to original settings. */
void close_keyboard() {
    tcsetattr(0, TCSANOW, &initial_settings);
}

/* return 1 if any keyboard character has been entered since the character
 * obtained by the most recent call to reachch(), otherwise return 0. */
int kbhit() {
    char ch;
    int nread;
    if (peek_character != -1) {
        return 1;
    }

    // set so that read() won't block at all. Remember, read returns the number
    // of bytes read, so it will return 0 if there was nothing to read. When
    // we finish reading, reset so it blocks for a single character.
    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);

    if (nread == 1) {
        peek_character = ch;
        return 1;
    }
    return 0;
}



/* Read a character. If we've already peeked at a character, then
 * return the peeked character. If not, read one from stdin. This call
 * will in fact block if no character was peeked. */
int readch() {
    char ch;
    if (peek_character != -1) {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0, &ch, 1);
    return ch;
}

/********** sample driver program *****/

int main() {
    int ch = 0;
    init_keyboard();

    while (ch != 'q') {
        printf("looping\n");
        sleep(1);
        if (kbhit()) {
            ch = readch();
            printf("You hit '%c'\n", ch);
        }
    }
    close_keyboard();
    return 0;
}

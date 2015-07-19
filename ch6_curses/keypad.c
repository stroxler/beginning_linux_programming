#include <unistd.h>
#include <stdlib.h>
#include <curses.h>

/* Simple demo of a keypad program.
 */
#define LOCAL_ESCAPE_KEY 27  // 27 is the normal escape key


int main() {
    int key;
    initscr();
    crmode();  // this is a synonym for nocbreak(); - normal line buffering
               // mode. But I think it was actually a typo for the authors
               // to have this... what you really want is cbreak() so that
               // individual characters may be read.
    cbreak();
    keypad(stdscr, TRUE); // the true enables curses to automatically translate
                          // function keys, such as arrow keys, into logical
                          // values such as KEY_LEFT.
    noecho();
    clear();
    mvprintw(5, 5, "Key pad demonstration. Press 'q' to quit.");
    move(7, 5);
    refresh();
    key = getch();

    while (key != ERR && key != 'q') {
        move(7, 5);
        clrtoeol(); // carriage return to eol
        if ((key >= 'A' && key <= 'Z') ||
            (key >= 'a' && key <= 'z')) {
            printw("Key was '%c'", (char) key);
        } else {
            switch (key) {
                case LOCAL_ESCAPE_KEY: printw("%s", "Escape key"); break;
                case KEY_END: printw("%s", "End key key"); break;
                case KEY_BEG: printw("%s", "Beginning key"); break;
                case KEY_RIGHT: printw("%s", "Right key"); break;
                case KEY_LEFT: printw("%s", "Left key"); break;
                case KEY_UP: printw("%s", "Up key"); break;
                case KEY_DOWN: printw("%s", "Down key"); break;
            }
        }
        refresh();
        key = getch();

    }

    endwin();
    exit(EXIT_SUCCESS);
}

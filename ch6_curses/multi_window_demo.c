#include <unistd.h>
#include <stdlib.h>
#include <curses.h>

/* Simple program to demonstrate how windows work in the curses
 * environment. The program is just an animation of 1 second frames
 * that you can watch and read along with. */

int main() {
    WINDOW *new_window;
    WINDOW *popup_window;
    int x_loop, y_loop;
    char a_letter = 'a';

    initscr();

    move(5, 5);
    printw("%s", "Testing multiple windows");
    refresh();

    sleep(1);

    /* add the alphabet cycled across the whole screen */
    for(y_loop = 0; y_loop < LINES - 1; y_loop++) {
        for(x_loop = 0; x_loop < COLS - 1; x_loop++) {
            // mvaddch is mv plus addch, adds a character at a location
            //   mvwaddch is the same thing but takes a WINDOW; in this case
            //   we are passing it stdscr, which is the same as mvaddch.
            mvwaddch(stdscr, y_loop, x_loop, a_letter);
            a_letter++;
            if(a_letter > 'z') a_letter = 'a';
        }
    }
    refresh();
    sleep(1);

    // newwin(nlines, ncols, top_lineidx, left_colidx)
    new_window = newwin(10, 20, 5, 5);
    // mvwprintw, like mvwaddch, is a move plus a printw, on a given WINDOW
    mvwprintw(new_window, 2, 2, "%s", "Hello world");
    mvwprintw(new_window, 5, 2, "%s", "Note that very long lines will wrap!");
    wrefresh(new_window);
    sleep(1);
   

    /* Change the background to be 0-9 cycled. When we refresh, the window
     * becomes obscured.... */
    a_letter = '0';
    for(y_loop = 0; y_loop < LINES - 1; y_loop++) {
        for(x_loop = 0; x_loop < COLS - 1; x_loop++) {
            mvwaddch(stdscr, y_loop, x_loop, a_letter);
            a_letter++;
            if(a_letter > '9') a_letter = '0';
        }
    }
    refresh();
    sleep(1);

    /* ...and re-refreshing the new window doesn't help because we haven't
     * changed the new window... */
    wrefresh(new_window);
    sleep(1);

    /* ...but if we 'touch' the new window, we can make it jump to the top
     * again by refreshing */
    touchwin(new_window);
    wrefresh(new_window);
    sleep(1);

    /* now let's make a popup window, which is just another window with a
     * box around it */
    popup_window = newwin(10, 20, 8, 8);
    box(popup_window, '|', '-'); // the border is drawn *inside* the window
                                 // boundaries.
    mvwprintw(popup_window, 5, 2, "%s", "pop up window!");
    wrefresh(popup_window);
    sleep(1);

    /* by touching and refreshing, we can bring new_window back to the fore.
     * note that where the new window has blanks, nothing is overwritten */
    touchwin(new_window);
    wrefresh(new_window);
    sleep(1);

    /* clear the contents of new_window - make it blank again. Note that this
     * overwrites all remaining traces of popup_window inside the boundaries */
    wclear(new_window);
    wrefresh(new_window);
    sleep(1);

    /* now delete new window - always remember to delete windows other than
     * stdscr, never delete stdscr except via endwin().
     * Note that this has no effect on the display. */
    delwin(new_window);
    sleep(1);

    /* bring popup window back to the fore */
    touchwin(popup_window);
    wrefresh(popup_window);
    sleep(1);

    /* delete popup_window. Touch stdscr to bring it back to front. */
    delwin(popup_window);
    touchwin(stdscr);
    refresh();
    sleep(1);

    endwin();
    exit(EXIT_SUCCESS);
}

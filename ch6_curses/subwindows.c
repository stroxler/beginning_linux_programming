#include <unistd.h>
#include <stdlib.h>
#include <curses.h>

/* In curses, windows can have subwindows. Here's a simple demo program.
 */
int main() {
    WINDOW *sub_window;
    int x_loop, y_loop;
    int counter;
    char a_letter = '0';

    initscr();

    for(y_loop = 0; y_loop < LINES - 1; y_loop++) {
        for(x_loop = 0; x_loop < COLS - 1; x_loop++) {
            mvwaddch(stdscr, y_loop, x_loop, a_letter);
            a_letter++;
            if(a_letter > '9') a_letter = '0';
        }
    }

    sub_window = subwin(stdscr, 12, 22, 10, 10);
    scrollok(sub_window, 1); // turn on 'scrolling' in the subwindow
    touchwin(stdscr);
    refresh();
    sleep(1);

    werase(sub_window); // erase contents, that is, make it blank
    mvwprintw(sub_window, 2, 0, "%s", "This window will now scroll\n\n");
    wrefresh(sub_window);
    sleep(1);

    /* print some stuff to see the scrolling effect */
    for (counter = 1; counter < 10; counter++) {
        wprintw(sub_window, "%s", "The text is both wrapping and scrolling\n");
        wrefresh(sub_window);
        sleep(1);
    }

    delwin(sub_window);
    touchwin(stdscr);
    refresh();
    sleep(1);

    endwin();
    exit(EXIT_SUCCESS);
}

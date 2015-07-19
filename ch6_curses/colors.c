#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>

/* Simple demo of curses color capabilities. An important thing to be
 * aware of with curses colors is that curses works only with color pairs,
 * of a fg and bg color. */

int main() {
    int i;
    initscr();

    // first check that the terminal supports colors
    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Error - no color support on this terminal.\n");
        exit(EXIT_FAILURE);
    }

    // next, set up the colors (and check error code)
    if (start_color() != OK) {
        endwin();
        fprintf(stderr, "Error - failed to initialize colors.\n");
        exit(EXIT_FAILURE);
    }

    clear();
    mvprintw(5, 5, "There are %d COLORS, and %d COLOR_PAIRS available",
             COLORS, COLOR_PAIRS);
    refresh();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_GREEN);
    init_pair(3, COLOR_GREEN, COLOR_RED);
    init_pair(4, COLOR_YELLOW, COLOR_BLUE);
    init_pair(5, COLOR_BLACK, COLOR_WHITE);
    init_pair(6, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(7, COLOR_CYAN, COLOR_WHITE);

    for (i = 1; i <= 7; i++) {
        attroff(A_BOLD);
        attrset(COLOR_PAIR(i));
        mvprintw(5 + i, 5, "Color pair %d", i);
        attrset(COLOR_PAIR(i) | A_BOLD);
        mvprintw(5 + i, 25, "Bold color pair %d", i);
        refresh();
        sleep(1);
    }

    endwin();
    exit(EXIT_SUCCESS);
}

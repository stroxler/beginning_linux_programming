#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>

/* In curses, a `pad` is a logical window that is allowed to be larger than
 * the screen (normal windows may not be).
 *
 * Here we do a simple demo of a pad. */

int main() {
    WINDOW *pad;
    int x, y;
    int pad_lines, pad_cols;
    char disp_char;

    initscr();
    pad_lines = LINES + 50;
    pad_cols = COLS + 50;

    // pads are initialized using a different func, but they are still WINDOWs
    pad = newpad(pad_lines, pad_cols);

    // fill up the pad with alphabetic data
    disp_char= 'a';
    for (x = 0; x < pad_lines; x++) {
        for (y = 0; y < pad_cols; y++) {
            mvwaddch(pad, x, y, disp_char);
            disp_char = (disp_char == 'z') ? 'a' : disp_char + 1;
        }
    }

    // the inputs here are:
    //  - the (line, col) coord *within* the pad which will be at the top-left
    //    of what we will actually display
    //  - the (line, col) coords of the top-left corner inside the actual
    //    screen where we want to display
    //  - the (line, col) coords of the bottom-left corner inside the actual
    //    screen where we want to display
    prefresh(pad, 5, 7, 2, 2, 9, 9);
    sleep(1);
    prefresh(pad, LINES + 5, COLS + 7, 5, 5, 21, 19);
    sleep(1);

    delwin(pad); // they are deleted like any other window
    endwin();
    exit(EXIT_SUCCESS);
}

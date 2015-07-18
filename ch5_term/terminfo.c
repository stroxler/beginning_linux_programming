#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <term.h>

/* This is an example of working with terminal info. The routines here
 * provide some of the low-level control used by curses.
 *
 * The setupterm() call lets you set the terminal (e.g., 'xterm'), or if
 * you pass NULL as the first parameter it uses the value of '$TERM'.
 *
 * It creates a structure that the other functions can read, with information
 * that can be used by libraries like curses.
 *    see the docs: http://linux.die.net/man/3/setupterm
 * ... no need to learn this well, generally you'll use higher-level
 * curses apis to do real work.
 *
 * NOTE: this has to be compiled with -lcurses, it isn't linked by
 * default (like math)
 */
int main() {
    int nrows, ncolumns;
    // the (int *) 0 looks kind of weird, but only because we are ignoring
    // error conditions here. See the docs for how the return value and that
    // argument together provide error handling.
    setupterm(NULL, fileno(stdout), (int *)0);
    nrows = tigetnum("lines");
    ncolumns = tigetnum("cols");
    printf("This terminal has %d rows and %d columns\n", nrows, ncolumns);
    exit(0);
}

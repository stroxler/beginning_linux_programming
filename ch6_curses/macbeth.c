#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

/* This is a simple deterministic demo of curses writing, using an
 * animation of some lines from Macbeth.
 *
 * Compared to the term code from chapter 5, this is very clean indeed :)
 */

int main() {
  // the space in front serves a purpose; it isn't ever printed, but allows
  // a clean while loop. See the code at line 47.
  const char witch_one[] = " First Witch  ";
  const char witch_two[] = " Second Witch ";
  const char *scan_ptr;

  // initscr() always needs to be called at the start of a curses program.
  // It returns a WINDOW *, but the output isn't usually used.
  initscr();

  /* make a title, then wait 1 second */
  move(5, 10);
  attron(A_STANDOUT); // turn on bold for the next print
  printw("%s", "Macbeth"); // printw has similar semantics to printf
  attroff(A_STANDOUT);
  refresh(); // nothing gets writen until you call this.
  sleep(1);

  /* make a boldfaced bit for the sound effects */
  move(8, 10);
  attron(A_BOLD);
  printw("%s", "Thunder and Ligning");
  attroff(A_BOLD);

  /* put the text on the screen, then wait 1 sec */
  move(10, 10);
  printw("%s", "When shall we three meet again?");
  move(11, 23);
  printw("%s", "In thunder, ligtning, or in rain?");
  move(13, 10);
  printw("%s", "When the hurlyburly's done,");
  move(14, 23);
  printw("%s", "When the battle's lost and won");
  refresh();
  sleep(1);

  /* insert actor identifications one char at a time in reverse order */
  // (insch = insert character, also remember postfix '--' is done after use)
  attron(A_DIM);
  scan_ptr = witch_one + strlen(witch_one) - 1;
  while (scan_ptr != witch_one) {
      move(10, 10);
      insch(*scan_ptr--);
      usleep(50000); // usleep is in microseconds
      refresh();     // dont't forget this if you want to see results!
  }
  scan_ptr = witch_two + strlen(witch_one) - 1;
  while (scan_ptr != witch_two) {
      move(13, 10);
      insch(*scan_ptr--);
      usleep(50000);
      refresh();
  }

 
  attroff(A_DIM);

  /* move the cursor to the bottom-right corner and tidy up */
  move(LINES - 1, COLS - 1);
  refresh();
  sleep(1);

  // always call this at the end of a curses program
  endwin();
  exit(EXIT_SUCCESS);
}

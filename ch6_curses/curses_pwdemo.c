#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

/* A simple password / login demo using curses, to show the how to read
 * keyboard input */

#define PW_LEN 256
#define NAME_LEN 256

int main() {
  // the space in front serves a purpose; it isn't ever printed, but allows
  // a clean while loop. See the code at line 47.
  char name[NAME_LEN];
  char password[PW_LEN];
  const char *real_password = "xyzzy";

  int i = 0;
  initscr();

  move(5, 10);
  printw("%s", "Please login:");
  move(7, 10);
  printw("%s", "User name:");
  getnstr(name, NAME_LEN); // get a string with length-checking
  // normally no referesh is needed above because get functions automatically
  // refresh. In some old versions of curses this is not the case.
  //
  // note that printw leaves the curser at the end of the printed string

  move(8, 10);
  printw("%s", "Password:");
  refresh();
  /* prevent the password from being echoed to the screen */
  cbreak(); // by default curses processes input line-by-line, like the
            // terminal. Use this to enable character-by-character. You can
            // unset it by calling nocbreak()
  noecho(); // similarly, stop input from being echoed; can unset with echo()
  memset(password, '\0', sizeof(password)); // set all password contents to \0
  while (i < PW_LEN) {
    password[i] = getch(); // getch gets 1 char at a time
    if (password[i] == '\n') break;
    move(8, 20+i);
    addch('*');
    refresh();
    i++;
  }

  echo();
  nocbreak();

  move(11, 10);
  if (strncmp(real_password, password, strlen(real_password)) == 0) {
      printw("Correct");
  } else {
      printw("Incorrect");
  }
  printw(" password.");

  refresh();
  sleep(2);

  endwin();
  exit(EXIT_SUCCESS);
}

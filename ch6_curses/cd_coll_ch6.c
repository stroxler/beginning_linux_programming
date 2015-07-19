#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>

/* First iteration of the CD collection program
 *   - curses-based UI
 *   - flat-file-based database
 *   - UI code and data handling code intermixed for now
 *
 * Nice example of
 *   - basic file operations
 *   - curses UI
 *
 * Compile as `gcc -Wall cd_coll_ch6 -lcurses`.
 */

#define MAX_STRING 80     // longest allowed response
#define MAX_ENTRY 1024    // longest allowed database entry

#define MESSAGE_LINE 6    // misc messages on this line
#define ERROR_LINE 22     // line to use for errors
#define Q_LINE 20         // line for questions
#define PROMPT_LINE 18    // line for propmpting

#define BOXED_LINES 11    // size of the box used for viewing and updating
#define BOXED_COLS 60     //   (note: the authors called COLS ROWS on accident
#define BOX_LINE_POS 8    // position of the box used for viewing and
#define BOX_COL_POS 2     //   editing (the top-left corner, that is)

// config constants
const char *TITLE_FILE = "title.cdb";
const char *TRACKS_FILE = "tracks.cdb";
const char *TEMP_FILE = "cdb.tmp";

// char arrays to hold input
static char current_cd[MAX_STRING] = "\0";
static char current_cat[MAX_STRING];

void clear_all_screen();
void get_return();
int get_confirm();
int getchoice(char *greet, char *choices[]);
void draw_menu(char *options[], int highlight, int start_row, int start_col);
void insert_entry(char *cd_entry);
void get_string(char *string);
void add_record();
void count_cds();
void find_cd();
void list_tracks();
void remove_tracks();
void remove_cd();
void update_cd();

char *main_menu[] = {
    "add new CD",
    "find CD",
    "count CDs and track in the catalog",
    "quit",
    0,
};

char *extended_menu[] = {
    "add new CD",
    "find CD",
    "count CDs and track in the catalog",
    "list tracks on current CD",
    "remove current CD",
    "update track information",
    "quit",
    0,
};

int main() {
    int choice;
    initscr();

    do {
        choice = getchoice("Options",
                           current_cd[0] ? extended_menu : main_menu);
        switch (choice) {
            case 'q':
                break;
                break;
            case 'a':
                add_record();
                break;
            case 'c':
                count_cds();
                break;
            case 'f':
                find_cd();
                break;
            case 'l':
                list_tracks();
                break;
            case 'r':
                remove_cd();
                break;
            case 'u':
                update_cd();
                break;
        }
    } while (choice != 'q');

    endwin();
    exit(EXIT_SUCCESS);
}


/* Menu related routines *****************************************************/

int getchoice(char *greet, char *choices[]) {
    static int selected_row = 0; // static so we can re-highlight choice
    int max_row = 0;
    int start_screenrow = MESSAGE_LINE, start_screencol = 10;
    char **option;
    int selected;
    int key = 0;


    // determine the number of rows
    option = choices;
    while (*option) {
        max_row++;
        option++;
    }

    /* deal with the case where menu gets shorter after a cd is deleted */
    if (selected_row >= max_row) {
        selected_row = 0;
    }

    clear_all_screen();
    mvprintw(start_screenrow - 2, start_screencol, greet);

    // turn on keypad, cbreak, noecho
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    // loop over the menu redraw until the press 'q' or until they have
    // a choice highlighted and press enter.
    key = 0;
    while (key != 'q' && key != KEY_ENTER && key != '\n') {

        // move the selection up or down. Wrap across line 0.
        if (key == KEY_UP) {
            if (selected_row == 0) {
                selected_row = max_row - 1;
            } else {
                selected_row--;
            }
        } else if (key == KEY_DOWN) {
            if (selected_row == max_row - 1) {
                selected_row = 0;
            } else {
                selected_row++;
            }

        }

        // [] has higher precedence than *, so this is the same as
        // choices[selected_row][0].
        selected = *choices[selected_row];

        // draw the menu, then get the next key. Note initial case is handled.
        draw_menu(choices, selected_row, start_screenrow, start_screencol);
        key = getch();
    }

    // turn off keypad, cbreak, noecho
    keypad(stdscr, TRUE);
    nocbreak();
    echo();

    
    return selected;
}

void draw_menu(char *options[], int highlighted_row,
               int start_row, int start_col) {
    char *txt;
    char **option;
    int current_row = 0;
    option = options;
    while (*option) {
        if (current_row == highlighted_row) attron(A_STANDOUT);
        txt = options[current_row];
        // the book has txt++, but that's an error... I'm guessing
        //   in an earlier edition, the strings were prefixed with one-char
        //   codes.
        mvprintw(start_row + current_row, start_col, "%s", txt);
        if (current_row == highlighted_row) attroff(A_STANDOUT);
        current_row++;
        option++;
    }

    mvprintw(start_row + current_row + 3, start_col,
            "Move highlight, then press RETURN ");
    refresh();
}

/* adding records ************************************************************/

/* top-level function for adding a record */
void add_record() {
    char catalog_number[MAX_STRING];
    char cd_title[MAX_STRING];
    char cd_type[MAX_STRING];
    char cd_artist[MAX_STRING];
    char cd_entry[MAX_STRING * 4 + 8];

    int screenrow = MESSAGE_LINE;
    int screencol = 10;

    clear_all_screen();

    // heading
    mvprintw(screenrow, screencol, "Enter new CD details");
    screenrow += 2;

    // print prompt and get each response
    mvprintw(screenrow, screencol, "  Catalog number: ");
    get_string(catalog_number);
    screenrow++;
    mvprintw(screenrow, screencol, "        CD title: ");
    get_string(cd_title);
    screenrow++;
    mvprintw(screenrow, screencol, "        CD type: ");
    get_string(cd_type);
    screenrow++;
    mvprintw(screenrow, screencol, "        CD artist: ");
    get_string(cd_artist);
    screenrow++;

    // make a single entry string with all the data
    sprintf(cd_entry, "%s, %s, %s, %s", catalog_number, cd_title, cd_type,
            cd_artist);

    // get confirmation before saving
    mvprintw(PROMPT_LINE - 2, 5, "About to add this new entry:");
    mvprintw(PROMPT_LINE, 5, cd_entry);
    refresh();
    move(PROMPT_LINE, 0);
    if (get_confirm()) {
        // save in database file, and update global constants to reflect the
        // 'active' cd.
        insert_entry(cd_entry);
        strcpy(current_cd, cd_title);
        strcpy(current_cat, catalog_number);
    };

}

/* Add a row to the database flat file with the album data */
void insert_entry(char *cd_entry) {
    FILE *fp = fopen(TITLE_FILE, "a");
    if (!fp) {
        mvprintw(ERROR_LINE, 0, "cannot open CD database");
    } else {
        fprintf(fp, "%s\n", cd_entry);
        fclose(fp);
    }
}

/* updating / removing records ***********************************************/

/* Update the tracks in a cd. Uses a boxed scrolling window, which is worth
 * a note: if you try to box a window and then sroll, the box itself will
 * scroll b/c it's inside the bounds. To deal with this, boxed windows that
 * need to scroll should be implemented as two nested windows. */
void update_cd() {

    FILE *tracks_fp;
    char track_name[MAX_STRING];
    int len;
    int track_num = 1;
    int screen_line = 1;
    WINDOW *box_window;
    WINDOW *sub_window;

    clear_all_screen();

    // get confirmation before we delete any old tracks and add new ones.
    mvprintw(PROMPT_LINE, 0,
          "Re-entering tracks for CD.");
    if (!get_confirm()) {
         return;
    }
    move(PROMPT_LINE, 0);
    clrtoeol();

    clear_all_screen();

    // go ahead and remove tracks, print prompt, and create the FILE
    remove_tracks();
    mvprintw(MESSAGE_LINE, 0, "Enter a blank line to finish");
    tracks_fp = fopen(TRACKS_FILE, "a");

    // set up the nexted windows. Note the box has +2 on all dims
    //  also note that the subwindow isn't impmlemented as a sub window of the
    //  box_window, but rather of stdscr. Not sure if this matters.
    box_window = subwin(stdscr, BOXED_LINES + 2, BOXED_COLS + 2,
                        BOX_LINE_POS - 1, BOX_COL_POS -1);
    if (!box_window) { // "error handling" lol
        return;
    }
    sub_window = subwin(stdscr, BOXED_LINES, BOXED_COLS,
                        BOX_LINE_POS, BOX_COL_POS);
    if (!sub_window) { // more "error handling"
        return;
    }

    // initialize the windows
    box(box_window, ACS_VLINE, ACS_HLINE);
    scrollok(sub_window, TRUE);
    werase(sub_window);
    touchwin(stdscr);

    do {
        mvwprintw(sub_window, screen_line++, 3, "Track %d: ", track_num);
        clrtoeol();
        refresh();

        // get the track name and strip the '\n'. If they entered a blank line,
        // the first character will e '\0', which is key in the `ifs` to follow
        wgetnstr(sub_window, track_name, MAX_STRING);
        len = strlen(track_name);
        if (len > 0 && track_name[len - 1] == '\n') {
            track_name[len - 1] = '\0';
        }

        // if the track name doesn't start with 0, write it to file. Then incr
        // track_num
        if (*track_name) {
            fprintf(tracks_fp, "%s,%d,%s\n",
                    current_cat, track_num, track_name);
        }
        track_num++;

        // scroll down if need be (refresh in the next loop)
        if (screen_line > BOXED_LINES - 1) {
            scroll(sub_window);
            screen_line--;
        }
    } while (*track_name);

    // clean up
    delwin(sub_window);
    delwin(box_window);  // the authors seem to have forgotten this
    fclose(tracks_fp);
}

/* remove the currently selected cd */
void remove_cd() {
    FILE *titles_fp, *temp_fp;
    char entry[MAX_ENTRY];
    int cat_length;

    // skip if there's no selected cd
    if (current_cd[0] == '\0') {
        return;
    }

    clear_all_screen();

    // prompt for confirmation first
    mvprintw(PROMPT_LINE, 0, "About to remove CD %s: %s. ",
             current_cat, current_cd);
    if (!get_confirm()) {
        return;
    }

    // open files
    titles_fp = fopen(TITLE_FILE, "r");
    if (!titles_fp) return;
    temp_fp = fopen(TEMP_FILE, "w");
    if (!temp_fp) return;

    // write all entries from exiting file to new file, except the one(s?)
    // matching the current_cat.
    cat_length = strlen(current_cat);
    while (fgets(entry, MAX_ENTRY, titles_fp)) {
        if (strncmp(current_cat, entry, cat_length) != 0) {
            fputs(entry, temp_fp);
        }
    }
    
    // close files and swap the temp file for the tracks file
    fclose(titles_fp);
    fclose(temp_fp);
    unlink(TITLE_FILE);
    rename(TEMP_FILE, TITLE_FILE);

    // do pretty much the same operations on the tracks file
    remove_tracks();

    // reset current_cd.
    //    we should probably reset current_cat too to be consistent, but
    //    all the code checks current_cd, so this is ok-ish
    current_cd[0] = '\0';
}

/* querying records **********************************************************/

/* Compute a count of the total number of records and tracks in the db */
void count_cds() {
    FILE *titles_fp, *tracks_fp;
    char entry[MAX_ENTRY];
    int ntitles = 0;
    int ntracks = 0;

    // count titles
    titles_fp = fopen(TITLE_FILE, "r");
    if (titles_fp) {   // note that this is expected to fail if no writes yet
        while (fgets(entry, MAX_ENTRY, titles_fp)) {
            ntitles++;
        }
        fclose(titles_fp);
    }

    // count tracks
    tracks_fp = fopen(TRACKS_FILE, "r");
    if (tracks_fp) {
        while (fgets(entry, MAX_ENTRY, tracks_fp)) {
            ntracks++;
        }
        fclose(tracks_fp);
    }

    // print message, and wait for user to press return so they have a chance
    // to read it.
    mvprintw(ERROR_LINE, 0,
             "Database containes %d titles, and a total of %d tracks",
             ntitles, ntracks);
    get_return();
}

/* Find a cd. If it's found, we store it's info in the globals
 * `current_cd` and `current_cat`. If we find 0 or more than 1 matches,
 * we inform the user and null-out `current_cd` 
 * 
 * NOTE: the authors hae typos here, everywhere where they have `found == ...`
 *       they intended to have `found = ...` */
void find_cd() {
    char match[MAX_STRING], entry[MAX_ENTRY];
    FILE *titles_fp;
    int count = 0;
    char *found, *title, *catalog;

    mvprintw(Q_LINE, 0, "Enter a string to search for in CD titles: ");
    get_string(match);

    titles_fp = fopen(TITLE_FILE, "r");
    if (titles_fp) {
        while (fgets(entry, MAX_ENTRY, titles_fp)) {

            // skip over the catalog... the strstr function returns
            //   a pointer to the first match against s2 found in s1, or NULL
            //   if not found.
            catalog = entry;
            if ((found = strstr(catalog, ","))) {
                *found = '\0';  // make `catalog` be '\0'-terminated
                title = found + 1;

                // do another search for a ',', so that we can isolate the
                // title portion of the entry before doing our actual search.
                if ((found = strstr(title, ","))) {
                    *found = '\0'; // make `title` be '\0'-terminated

                    // now that we've isolated the title, we can do our
                    // actual search
                    //   (NOTE: the above code should really be factored out
                    //    since (a) it's general, and (b) it obscures what's
                    //    going on with low-level implementation details)
                    if ((found = strstr(title, match))) {
                        count++;
                        strcpy(current_cd, title);
                        strcpy(current_cat, catalog);
                    }
                }
            }
        }
        fclose(titles_fp);
    }

    // if we did not find exactly one match, print a message and wait for user
    // to press enter.
    if (count != 1) {
        if (count == 0) {
            mvprintw(ERROR_LINE, 0, "Sorry, no matching CD found");
        }
        if (count > 1) {
            mvprintw(ERROR_LINE, 0, "Sorry, match is ambiguous; %d CDs found",
                     count);
        }
        current_cd[0] = '\0';
        get_return();
    }
}

/* This function can only be called if a cd has already been selected and
 * stored in the globals `current_cd` and `current_cat`. */
void list_tracks() {
    FILE *tracks_fp;
    char entry[MAX_ENTRY];
    int cat_length;
    int lines_op = 0;
    WINDOW *track_pad;
    int ntracks = 0;
    int key;
    int first_line = 0;

    // handle the case of no current cd
    if (current_cd[0] == '\0') {
        mvprintw(ERROR_LINE, 0, "You must select a CD before listing tracks");
        get_return();
        return;
    }

    clear_all_screen();

    // First, count the number of tracks for the current CD. close the
    // file when done so we can reopen before listing them.
    cat_length = strlen(current_cat);
    tracks_fp = fopen(TRACKS_FILE, "r");
    if (!tracks_fp) return;
    while (fgets(entry, MAX_ENTRY, tracks_fp)) {
        if (strncmp(current_cat, entry, cat_length) == 0) {
            ntracks++;
        }
    }
    fclose(tracks_fp);

    // make a new pad. Make sure it is big enough to fill up the boxed region
    // even if there are no listings.
    //   NOTE: we don't actually make a box for this operation... the box is
    //   only used in update_cd. But we are reusing the same region.
    track_pad = newpad(ntracks + 1 + BOXED_LINES, BOXED_COLS + 1);
    if (!track_pad) return;

    // reopen the tracks file
    tracks_fp = fopen(TRACKS_FILE, "r");
    if (!tracks_fp) return;

    // print out all the tracks onto the pad.
    mvprintw(4, 0, "CD Track Listing\n");
    while (fgets(entry, MAX_ENTRY, tracks_fp)) {
        if (strncmp(current_cat, entry, cat_length) == 0) {
            // print the track info. Use `cat_length+1` bc of the comma
            //    this actually is kind of strange because we are still
            //    printing a kind of ugly comma-separated track number and
            //    name, but it works. You could make it better as an exercise.
            mvwprintw(track_pad, lines_op++, 0, "%s", entry + cat_length + 1);
        }
    }
    fclose(tracks_fp);

    // tell the user what to do
    if (lines_op > BOXED_LINES) {
        mvprintw(MESSAGE_LINE, 0, "Arrows to scroll, RETURN or q to exit.");
    } else {
        mvprintw(MESSAGE_LINE, 0, "RETURN or q to exit.");
    }

    // set up the control keys and such, refresh the main screen
    wrefresh(stdscr);
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    // start up a loop over control keys; inside the loop refresh the pad
    key = 0;
    while (key != '\n' && key != KEY_ENTER && key != 'q') {
        if (key == KEY_UP) {
            if (first_line > 0) {
                first_line--;
            }
        } else if (key == KEY_DOWN) {
            if (first_line + BOXED_LINES < ntracks - 1) {
                first_line++;
            }
        }

        prefresh(track_pad, first_line, 0,
                 BOX_LINE_POS, BOX_COL_POS,
                 BOX_LINE_POS + BOXED_LINES, BOX_COL_POS + BOXED_COLS);
        key = getch();
    }
}



/* utilities *****************************************************************/

/* delete tracks in the current cd from TRACKS_FILE. used by both
 * update_cd and remove_cd */
void remove_tracks() {
    FILE *tracks_fp, *temp_fp;
    char entry[MAX_ENTRY];
    int cat_length;

    // don't do anything if there's no current cd
    if (current_cd[0] == '\0') {
        return;
    }

    // open tracks file for reading, temp file for writing
    tracks_fp = fopen(TRACKS_FILE, "r");
    if (!tracks_fp) return;
    temp_fp = fopen(TEMP_FILE, "w");

    // read each line from tracks file, and copy it to temp file if it is
    // not from the current catalogue.
    cat_length = strlen(current_cat);
    while (fgets(entry, MAX_ENTRY, tracks_fp)) {
        if (strncmp(current_cat, entry, cat_length) != 0) {
            fputs(entry, temp_fp);
        }
    }

    // clean up: delete FILES, unlink(remove) tracks file, rename temp file
    fclose(tracks_fp);
    fclose(temp_fp);
    unlink(TRACKS_FILE);
    rename(TEMP_FILE, TRACKS_FILE);
}



/* clear the screen, and add just the application title */
void clear_all_screen() {
    clear();
    mvprintw(2, 20, "%s", "CD Database Application");
    if (current_cd[0]) {
        mvprintw(ERROR_LINE, 0, "Current CD: %s: %s\n",
                 current_cat, current_cd);
    }
    refresh();
}

/* get confirmation before doing something */
int get_confirm() {
    int confirmed = 0;
    char first_char;

    mvprintw(Q_LINE, 5, "Are you sure? (y/n)");
    clrtoeol();
    refresh();
    cbreak();

    first_char = getch();
    if (first_char == 'y' || first_char == 'Y') {
        confirmed = 1;
    }
    nocbreak();

    // if not confirmed, give the user a short "Cancelled" feedback notice
    if (!confirmed) {
        mvprintw(Q_LINE, 1, "     Cancelled");
        clrtoeol();
        refresh();
        sleep(1);
    }

    return confirmed;
}

/* gets a string from curses input, stores it in `*string`. Strips trailing
 * newline. */
void get_string(char *string) {
    int len;
    wgetnstr(stdscr, string, MAX_STRING);
    len = strlen(string);
    if (len > 0 && string[len - 1] == '\n') {
        string[len - 1] = '\0';
    }
    
}

/* read from stdin until we see a \n or an EOF. Used for error messages. */
void get_return() {
    int ch;
    mvprintw(23, 0, "%s", " Press return ");
    refresh();
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

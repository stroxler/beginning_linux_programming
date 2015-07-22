#define _XOPEN_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
// note that the local include uses quotes
#include "cd_data.h"

/* Implement a full-featured console-based (not curses) ui
 * for the dbm cd database. Data logic is factored out and
 * all data functions come from "cd_data.h"
 */

// this number must be bigger than the biggest single
// string anywhere in the schema.
#define TMP_STRING_LEN 125


// this is our first enum. It's syntactic sugar for integer
// constants - very useful because the compiler checks types
typedef enum {
    mo_invalid,
    mo_add_cat,
    mo_add_tracks,
    mo_del_cat,
    mo_find_cat,
    mo_list_cat_tracks,
    mo_del_tracks,
    mo_count_entries,
    mo_exit
} menu_options;


// prototypes
static int command_mode(int argc, char *argv[]);
static void announce(void);
static menu_options get_menu_choice(const cdc_entry *current_cdc);
static int get_confirm(const char *question);
static int enter_new_cat_entry(cdc_entry *entry_to_update);
static void enter_new_track_entries(const cdc_entry *entry_to_add_to);
static void del_cat_entry(const cdc_entry *entry_to_delete);
static void del_track_entries(const cdc_entry *entry_to_delete);
static cdc_entry find_cat(void);
static void list_tracks(const cdc_entry *entry_to_use);
static void count_all_entries(void);
static void display_cdc(const cdc_entry *cdc_to_show);
static void display_cdt(const cdt_entry *cdt_to_show);
static void strip_return(char *string_to_strip);


int main(int argc, char *argv[]) {
    menu_options current_option;
    cdc_entry current_cdc_entry;
    int command_result;

    // this block provides a command line api via `command_mode`
    if (argc > 1) {
        command_result = command_mode(argc, argv);
        exit(command_result);
    }

    // if we get here, then we are in interactive mode....
    //    start with an announcement message
    announce();

    // initialize the db. Using 0 here means open an exiting db (actually it
    // will create one, but it won't delete old data for you). To clear the
    // dbs and initialize a new one, use cmd line mode with -i.
    if (!database_initialize(0)) {
        fprintf(stderr, "Sorry, unable to initialize database\n");
        fprintf(stderr, "To create a new database use %s -i\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // loop over an interactive console menu
    //    the semantics are kind of similar to the ch6 menu, where there's
    //    an "active" cd, and the operations you can do affect that cd

    while(current_option != mo_exit) {
        current_option = get_menu_choice(&current_cdc_entry);
        switch(current_option) {
            case mo_add_cat:
                // add a new catalog item. Lots of error checking here.
                if (enter_new_cat_entry(&current_cdc_entry)) {
                    if (!add_cdc_entry(current_cdc_entry)) {
                        // print message, but also make sure to zero out data
                        fprintf(stderr, "Failed to add new entry\n");
                        memset(&current_cdc_entry, '\0',
                               sizeof(current_cdc_entry));
                    }
                }
                break;
            case mo_add_tracks:
                enter_new_track_entries(&current_cdc_entry);
                break;
            case mo_del_cat:
                del_cat_entry(&current_cdc_entry);
                break;
            case mo_find_cat:
                current_cdc_entry = find_cat();
                break;
            case mo_list_cat_tracks:
                list_tracks(&current_cdc_entry);
                break;
            case mo_del_tracks:
                del_track_entries(&current_cdc_entry);
                break;
            case mo_count_entries:
                count_all_entries();
                break;
            case mo_exit:
            case mo_invalid:
            default:
                // in all three cases, do nothing
                break;
        }
    }

    // close db and exit
    database_close();
    exit(EXIT_SUCCESS);
} // end of main


/* Note that the functions below are all static. This makes them only file-
 * visible, keeping namespace clutter somewhat under control.
 * Data is static by default and requires an 'extern' to be shared; functions
 * are extern by default and require a 'static' to be hidden.
 */


/* print greeting */
static void announce(void) {
    printf("\n\nWelcome to the demonstration CD catalog database program\n");
}


/* show the menu, get menu choice */
static menu_options get_menu_choice(const cdc_entry *cdc_selected) {
    char tmp_str[TMP_STRING_LEN + 1];
    menu_options option_chosen = mo_invalid;

    // loop til we have a valid choice
    while(option_chosen == mo_invalid) {
        if (cdc_selected->catalog[0]) {
            // print the menu for when there's a selected album
            printf("\n\nCurrent entry: ");
            printf("%s, %s, %s, %s\n",
                   cdc_selected->catalog,
                   cdc_selected->title,
                   cdc_selected->type,
                   cdc_selected->artist);
            printf("\n");
            printf("1 - add new CD\n");
            printf("2 - search for a CD\n");
            printf("3 - count the CDs and tracks in the database\n");
            printf("4 - re-enter tracks for current CD\n");
            printf("5 - delete this CD, and all its tracks\n");
            printf("6 - list tracks for this CD\n");
            printf("q - quit\n");
            printf("\nChoice: ");
            fgets(tmp_str, TMP_STRING_LEN, stdin);
            switch(tmp_str[0]) {
                case '1': option_chosen = mo_add_cat; break;
                case '2': option_chosen = mo_find_cat; break;
                case '3': option_chosen = mo_count_entries; break;
                case '4': option_chosen = mo_add_tracks; break;
                case '5': option_chosen = mo_del_cat; break;
                case '6': option_chosen = mo_list_cat_tracks; break;
                case 'q': option_chosen = mo_exit; break;
            }
        } else {
            // print the smaller menu for when there's no selected album
            printf("\n\n");
            printf("1 - add new CD\n");
            printf("2 - search for a CD\n");
            printf("3 - count the CDs and tracks in the database\n");
            printf("q - quit\n");
            printf("\nChoice: ");
            fgets(tmp_str, TMP_STRING_LEN, stdin);
            switch(tmp_str[0]) {
                case '1': option_chosen = mo_add_cat; break;
                case '2': option_chosen = mo_find_cat; break;
                case '3': option_chosen = mo_count_entries; break;
                case 'q': option_chosen = mo_exit; break;
            }
        }
    } // end of the outer while

    return(option_chosen);
}


/* Gets a new cdc_entry from the user. If all goes well, saves the
 * data to the location pointed to by `entry_to_update` 
 *   returns 1 if update occurs, 0 if the user aborts */
static int enter_new_cat_entry(cdc_entry *entry_to_update) {
    cdc_entry new_entry;
    char tmp_str[TMP_STRING_LEN + 1];

    memset(&new_entry, '\0', sizeof(new_entry)); // remember to do this!

    // the code that follows is kind of copy-pastey, probably should be
    // factored out.

    printf("Enter catalog entry: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.catalog, tmp_str, CAT_LEN - 1);

    printf("Enter title: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.title, tmp_str, TITLE_LEN - 1);
    
    printf("Enter type: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.type, tmp_str, TYPE_LEN - 1);

    printf("Enter artist: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.artist, tmp_str, ARTIST_LEN - 1);
    printf("\nNew catalog entry entry is :-\n");

    // get a user confirmation before continuint
    display_cdc(&new_entry);
    if (get_confirm("Add this entry ?")) {
        memcpy(entry_to_update, &new_entry, sizeof(new_entry));
        return(1);
    }
    return(0);
} 


/* Mildly misnamed function... it loops through existing track entries if
 * there are any and the user can either edit description or delete this and
 * all later entries. When it finishes looping over existing entries, the user
 * can also add new ones. */
static void enter_new_track_entries(const cdc_entry *entry_to_add_to) {
    cdt_entry new_track, existing_track;
    char tmp_str[TMP_STRING_LEN + 1];
    int track_no = 1;

    // if there's no current catalog, do nothing
    if (entry_to_add_to->catalog[0] == '\0') return;

    // print out instructions
    printf("Updating tracks for %s\n", entry_to_add_to->catalog);
    printf("Press return to leave existing description unchanged,\n");
    printf(" a single d to delete this and remaining tracks,\n");
    printf(" or new track description\n");

    // loop over tracks, let the user choose what to do with each one.
    //   note that dbm is helping a lot here - in our flat file curses impl
    //   from ch6, the only easy things to do were to keep all tracks or
    //   redo everything. Dbm makes single-track changes much easier.
    // We are still somewhat limited because the track ids have to be in
    //   increaseing order. So you can edit descriptions atomically, but if
    //   you delete one you have to delete everything later. We could get
    //   around this, but it would be too much work for a demo program.
    while (1) {
        memset(&new_track, '\0', sizeof(new_track));
        existing_track = get_cdt_entry(entry_to_add_to->catalog,
                                       track_no);

        // existing track.caltalog[0] is set to 0 by get_cdt_entry if there
        // is no track with track number `track_no`. Hence..
        if(existing_track.catalog[0]) {
            // if we are here, there's a track description already
            printf("\tTrack %d: %s\n", track_no, existing_track.track_txt);
            printf("\tNew text: ");
        } else {
            // if we are here, there is not
            printf("\tTrack %d description: ", track_no);
        }
        fgets(tmp_str, TMP_STRING_LEN, stdin);
        strip_return(tmp_str);

        // If tmp_str has length 0, the user didn't want to do anything for
        // this track.
        if (strlen(tmp_str) == 0) {
            if (existing_track.catalog[0] == '\0') {
                // if this is true, then we had already seen all existing
                // tracks, so we are done.
                break;
            } else {
                // if we get here, it means the user didn't want to edit this
                // track, so we go on to the next one.
                track_no++;
                continue;
            }
        }

        // Entering a single 'd' is a special case for delete this track and
        // all later tracks, as mentioned in the instructions printed above.
        // So, loop through all remaining tracks, delete, then break.
        if ((strlen(tmp_str) == 1) && tmp_str[0] == 'd') {
            // del_cdt_entry will return 0 when we try to delete a nonexistant
            // track_no, so we use that as our terminate condition.
            while (del_cdt_entry(entry_to_add_to->catalog, track_no)) {
                track_no++;
            }
            break;
        }

        // if we get here, it means the user made a new description, so we
        // edit the existing track or add a new one (the api is the same, so
        // we don't care at this point whether the track exists)
        strncpy(new_track.track_txt, tmp_str, TTEXT_LEN - 1);
        strcpy(new_track.catalog, entry_to_add_to->catalog);
        new_track.track_no = track_no;
        if (!add_cdt_entry(new_track)) {
            fprintf(stderr, "Failed to add new track\n");
            break;
        }
        track_no++;
    } // end of the big while loop over track_no's
}


/* Delete a catalog entry (and all of its tracks) */
static void del_cat_entry(const cdc_entry *entry_to_delete) {
    int track_no = 1, delete_ok;
    // don't delete without confirmation
    display_cdc(entry_to_delete);
    if (get_confirm("Delete tracks for this entry? ")) {
        do {
            // remember, del_cdt_entry returns 0 if the track didn't exist
            delete_ok = del_cdt_entry(entry_to_delete->catalog, track_no);
            track_no++;
        } while (delete_ok);
        if (!del_cdc_entry(entry_to_delete->catalog)) {
            fprintf(stderr, "Failed to delete entry\n");
        }
    }
}


/* delete all track entries for the active catalog */
static void del_track_entries(const cdc_entry *entry_to_delete) {
    int track_no = 1, delete_ok;
    // don't delete without confirmation
    display_cdc(entry_to_delete);
    if (get_confirm("Delete tracks for this entry? ")) {
        do {
            // remember, del_cdt_entry returns 0 if the track didn't exist
            delete_ok = del_cdt_entry(entry_to_delete->catalog, track_no);
            track_no++;
        } while (delete_ok);
    }
}


/* Finds a catalog entry. It handles multiple matches by letting the user pick
 * one of them. If there are no matches, search_cdc_entry will set item_found
 * to be all 0 bytes and we will wind up with no active catalog entry. */
static cdc_entry find_cat(void) {
    cdc_entry item_found;
    char tmp_str[TMP_STRING_LEN + 1];
    int first_call = 1;
    int any_entry_found = 0;
    int string_ok;
    int entry_selected = 0;

    // get a search string, checking that the string isn't too long
    do {
        string_ok = 1;
        printf("Enter string to search for in catalog entry: ");
        fgets(tmp_str, TMP_STRING_LEN, stdin);
        strip_return(tmp_str);
        if (strlen(tmp_str) > CAT_LEN) {
            fprintf(stderr, "Sorry, string too long, maximum %d characters",
                    CAT_LEN);
            string_ok = 0;
        }
    } while (!string_ok);

    // loop over all cdc entries. The `first_time` is a flag which should
    // be 1 ("first time = true") on the initial call; it gets set to 0 (note
    // it's passed via ptr), and internally the database code tracks the
    // progress of the search from that point forward
    while (!entry_selected) {
        item_found = search_cdc_entry(tmp_str, &first_call);
        if (item_found.catalog[0] != '\0') {
            any_entry_found = 1;
            printf("\n");
            display_cdc(&item_found);
            if (get_confirm("This entry? ")) {
                entry_selected = 1;
            }
        } else {
            // we can get here either because the user didn't select any
            // of the matches, or because there weren't any.
            if (any_entry_found) {
                printf("Sorry, no more matches found\n");
            } else {
                printf("Sorry, no matches found\n");
            }
            break;
        }
    }
    return item_found;
}


/* list tracks for the active catalog */
static void list_tracks(const cdc_entry *entry_to_use) {
    int track_no = 1;
    cdt_entry entry_found;

    // first display the current catalog
    display_cdc(entry_to_use);

    // loop over entries and display them
    printf("\nTracks\n");
    do {
        entry_found = get_cdt_entry(entry_to_use->catalog, track_no);
        if (entry_found.catalog[0]) {
            display_cdt(&entry_found);
            track_no++;
        }
    } while (entry_found.catalog[0]);
    // let the user look before returning to menu
    get_confirm("Press return");
}


/* Print out a summary giving the number of catalog entries
 * and total number of tracks across all of them */
static void count_all_entries(void) {
    int cd_entries_found = 0;
    int track_entries_found = 0;
    cdc_entry cdc_found; cdt_entry cdt_found;
    int track_no = 1;
    int first_time = 1;
    char *search_string = "";

    // loop over all cdc entries. The `first_time` is a flag which should
    // be 1 ("first time = true") on the initial call; it gets set to 0 (note
    // it's passed via ptr), and internally the database code tracks the
    // progress of the search from that point forward
    do {
        cdc_found = search_cdc_entry(search_string, &first_time);
        if (cdc_found.catalog[0]) {
            cd_entries_found++;
            track_no = 1;
            do {
                cdt_found = get_cdt_entry(cdc_found.catalog, track_no);
                if (cdt_found.catalog[0]) {
                    track_entries_found++;
                    track_no++;
                }
            } while (cdt_found.catalog[0]);
        }
    } while (cdc_found.catalog[0]);

    printf("Found %d CDs, with a total of %d tracks\n",
           cd_entries_found, track_entries_found);
    get_confirm("Press Enter");
}



/* Command line function. The only actual valid command line option is
 * -i, which creates a new database. If any other option is used (or if
 *  an arg is passed), we print a usage message. */
static int command_mode(int argc, char *argv[]) {
    int c;
    int result = EXIT_SUCCESS;
    char *prog_name = argv[0];

    // used by getopt
    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((c = getopt(argc, argv, ":i")) != -1) {
            switch(c) {
            case 'i':
                if (!database_initialize(1)) {
                result = EXIT_FAILURE;
                fprintf(stderr, "Failed to initialize database\n");
                break;
            case ':':
            case '?':
            default:
                fprintf(stderr, "Usage: %s [-i]\n", prog_name);
                result = EXIT_FAILURE;
                break;
            }
        }
    }
    return result;
}


/* utilities ------------------------------- */


/* pretty-print a cdc (catalog entry) */
static void display_cdc(const cdc_entry *cdc_to_show) {
    printf("Catalog: %s\n", cdc_to_show->catalog);
    printf("\ttitle: %s\n", cdc_to_show->title);
    printf("\ttype: %s\n", cdc_to_show->type);
    printf("\tartist: %s\n", cdc_to_show->artist);
}


/* pretty-print a ctd (track entry) */
static void display_cdt(const cdt_entry *cdt_to_show) {
    printf("%d: %s\n", cdt_to_show->track_no, cdt_to_show->track_txt);
}

/* strip trailing newline, if there is one, from a C-string */
static void strip_return(char *string_to_strip) {
    int len;
    len = strlen(string_to_strip);
    if (string_to_strip[len - 1] == '\n') {
        string_to_strip[len - 1] = '\0';
    }
}


/* get a user confirmation before proceeding. Return 1 if confirm, 0 else */
static int get_confirm(const char *question) {
    char tmp_str[TMP_STRING_LEN + 1];
    printf("%s", question);
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    if (tmp_str[0] == 'Y' || tmp_str[0] == 'y') {
        return 1;
    }
    return 0;
}

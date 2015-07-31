/* We now move on to the user interface. This gives us a (relatively) simple program with
    which to access our database functions, which we'll implement in a separate file.
    We start, as usual, with some header files.    */

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "cd_data.h"

#define TMP_STRING_LEN 125 
/* this number must be larger than the biggest single string in any database structure */

/* We make our menu options typedefs. This is in preference to using #defined constants,
 as it allows the compiler to check the types of the menu option variables. */

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

/* Now the prototypes for the local functions.
 The prototypes for accessing the database were included in cd_data.h. */

static int command_mode(int argc, char *argv[]);
static void announce(void);
static menu_options show_menu(const cdc_entry *current_cdc);
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

/* Finally, we get to main. This starts by ensuring the current_cdc_entry that we use
 to keep track of the currently selected CD catalog entry is initialized.
 We also parse the command line, announce what program is being run and initialize the database. */

int main(int argc, char *argv[]) {
    menu_options current_option;
    cdc_entry current_cdc_entry;
    int command_result;

    memset(&current_cdc_entry, '\0', sizeof(current_cdc_entry));

    // if there's an arg, switch to command mode and exit
    if (argc > 1) {
        command_result = command_mode(argc, argv);
        exit(command_result);
    }
       
    // print a greeting, and initialize the database (it must already exist)
    announce();
    if (!database_initialize(0)) {
        fprintf(stderr, "Sorry, unable to initialize database\n");
        fprintf(stderr, "To create a new database use %s -i\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* loop over menu choices, calling show_menu every time.
     * the behavior of show_menu depends on whether a track is currently
     * selected (which is determined by the var `current_cdc_entry`) */
    while(current_option != mo_exit) {
        current_option = show_menu(&current_cdc_entry);

        switch(current_option) {
            case mo_add_cat:
                if (enter_new_cat_entry(&current_cdc_entry)) {
                    if (!add_cdc_entry(current_cdc_entry)) {
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
                break;
            case mo_invalid:
                break;
            default:
                break;
        } /* switch */
    } /* while */


    // when the loop exits, clean up and quit
    database_close();
    exit(EXIT_SUCCESS);
}


/* show a greeting */
static void announce(void) {
    printf("\n\nWelcome to the demonstration CD catalog database \
             program\n");
}

/* Print a menu of options. The actions available vary based on whether
 * a cd is selected currently (which is determined by `cdc_selected`) */
static menu_options show_menu(const cdc_entry *cdc_selected) {
    char tmp_str[TMP_STRING_LEN + 1];
    menu_options option_chosen = mo_invalid;

    while (option_chosen == mo_invalid) {
        if (cdc_selected->catalog[0]) {
            // print the menu of actions for when a cdc is selected
            printf("\n\nCurrent entry: ");
            printf("%s, %s, %s, %s\n", cdc_selected->catalog,
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
            printf("\nOption: ");
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
            // print the menu of actions for when no cdc is selected
            printf("\n\n");
            printf("1 - add new CD\n");
            printf("2 - search for a CD\n");
            printf("3 - count the CDs and tracks in the database\n");
            printf("q - quit\n");
            printf("\nOption: ");
            fgets(tmp_str, TMP_STRING_LEN, stdin);
            switch(tmp_str[0]) {
                case '1': option_chosen = mo_add_cat; break;
                case '2': option_chosen = mo_find_cat; break;
                case '3': option_chosen = mo_count_entries; break;
                case 'q': option_chosen = mo_exit; break;
            }
        }
    } /* while */
    return(option_chosen);
}


/* Confirm something before proceeding. Return 1 if confirmed, else 0 */
static int get_confirm(const char *question) {
    char tmp_str[TMP_STRING_LEN + 1];
    
    printf("%s", question);
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    if (tmp_str[0] == 'Y' || tmp_str[0] == 'y') {
        return(1);
    }
    return(0);
}


/* Get information for a new cdc entry.
 *    (just the metadata, not track information)
 * We use strip_return to get rid of the '\n' that fgets returns.
 * We avoid using gets, because it is unsafe (no buffer overflow size checks!)
 * Return 1 on success, 0 if user doesn't confirm.
 */
static int enter_new_cat_entry(cdc_entry *entry_to_update)
{
    // impl notes: we copy all the data into a new entry first so that we
    // can display it and ask for confirmation. Only then do we copy into
    // `entry_to_update`.
    cdc_entry new_entry;
    char tmp_str[TMP_STRING_LEN + 1];

    memset(&new_entry, '\0', sizeof(new_entry));

    printf("Enter catalog entry: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.catalog, tmp_str, CAT_CAT_LEN - 1);

    printf("Enter title: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.title, tmp_str, CAT_TITLE_LEN - 1);

    printf("Enter type: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.type, tmp_str, CAT_TYPE_LEN - 1);

    printf("Enter artist: ");
    fgets(tmp_str, TMP_STRING_LEN, stdin);
    strip_return(tmp_str);
    strncpy(new_entry.artist, tmp_str, CAT_ARTIST_LEN - 1);

    printf("\nNew catalog entry entry is :-\n");
    display_cdc(&new_entry);
    if (get_confirm("Add this entry ?")) {
        memcpy(entry_to_update, &new_entry, sizeof(new_entry));
        return(1);
    }
    return(0);
}

/* Enter new track entries for a cdc entry.
 *
 * You have the option of looping through some exiting tracks and leaving
 * them unchanged, but if you do change anything, it and everything after it
 * will get deleted. Then you can add lots of tracks if you want.
 *
 * You enter a blank line to indicate you are finished.
 */
static void enter_new_track_entries(const cdc_entry *entry_to_add_to) {
    cdt_entry new_track, existing_track;
    char tmp_str[TMP_STRING_LEN + 1];
    int track_no = 1;

    if (entry_to_add_to->catalog[0] == '\0') return;
    printf("\nUpdating tracks for %s\n", entry_to_add_to->catalog);
    printf("Press return to leave existing description unchanged,\n");
    printf(" a single d to delete this and remaining tracks,\n");
    printf(" or new track description\n");
    
    while(1) {

        // set up a prompt. Either the track exists and we give the user
        // an option of changing the text, or if the track doesn't exist then
        // we ask for a description.
        memset(&new_track, '\0', sizeof(new_track));
        existing_track = get_cdt_entry(entry_to_add_to->catalog, 
                                        track_no);
        if (existing_track.catalog[0]) {
            printf("\tTrack %d: %s\n", track_no, 
                           existing_track.track_txt);
            printf("\tNew text: ");
        }
        else {
            printf("\tTrack %d description: ", track_no);
        }
        fgets(tmp_str, TMP_STRING_LEN, stdin);
        strip_return(tmp_str);

        // if they entered a blank line *and* we already went through existing
        // tracks, then we are done.
        // If we aren't finished with existing tracks and they entered a blank
        // line, just go on to the next one.
        if (strlen(tmp_str) == 0) {
            if (existing_track.catalog[0] == '\0') {
                    /* no existing entry, so finished adding */
                break;
            }
            else {
                /* leave existing entry, jump to next track */
                track_no++;
                continue;
            }
        }

        // if they entered a d, delete this track and loop through all higher-
        // numbered tracks and delete them, then exit.
        if ((strlen(tmp_str) == 1) && tmp_str[0] == 'd') {
            while (del_cdt_entry(entry_to_add_to->catalog, track_no)) {
                track_no++;
            }
            break;
        }

        // otherwise, they entered new track data. Whether or not the track
        // previously existed, we copy that data into a ctd entry and add it
        // to the db on the current track_no
        strncpy(new_track.track_txt, tmp_str, TRACK_TTEXT_LEN - 1);
        strcpy(new_track.catalog, entry_to_add_to->catalog);
        new_track.track_no = track_no;
        if (!add_cdt_entry(new_track)) {
            fprintf(stderr, "Failed to add new track\n");
            break;
        }
        track_no++;
    }
}


/* Delete a catalog entry. Delete all of its tracks. */
static void del_cat_entry(const cdc_entry *entry_to_delete)
{
    int track_no = 1;
    int delete_ok;

    display_cdc(entry_to_delete);
    if (get_confirm("Delete this entry and all it's tracks? ")) {

        // loop over all the track entries and delete (we could have called
        // del_track_entries; the reason we don't is b/c that function has
        // it's own confirmation inside of it. Better factoring would have
        // been able to follow DRY better.
        do {
            delete_ok = del_cdt_entry(entry_to_delete->catalog, 
                                      track_no);
            track_no++;
        } while(delete_ok);

        // now delete the catalog entry itself
        if (!del_cdc_entry(entry_to_delete->catalog)) {
            fprintf(stderr, "Failed to delete entry\n");
        }
    }
}


/* A utility for deleting all the tracks for a catalog. */
static void del_track_entries(const cdc_entry *entry_to_delete) {
    int track_no = 1;
    int delete_ok;
    
    display_cdc(entry_to_delete);
    if (get_confirm("Delete tracks for this entry? ")) {
        do {
            delete_ok = del_cdt_entry(entry_to_delete->catalog, track_no);
            track_no++;
        } while(delete_ok);
    }
}


/* A simple catalog search facility. We allow the user to
 * enter a string, then check for catalog entries that contain the string.
 * Since there could be multiple entries that match, we simply offer the user
 * each match in turn. */ 
static cdc_entry find_cat(void)
{
    cdc_entry item_found;
    char tmp_str[TMP_STRING_LEN + 1];
    int first_call = 1;
    int any_entry_found = 0;
    int string_ok;
    int entry_selected = 0;

    do {
        string_ok = 1;
        printf("Enter string to search for in catalog entry: ");
        fgets(tmp_str, TMP_STRING_LEN, stdin);
        strip_return(tmp_str);
        if (strlen(tmp_str) > CAT_CAT_LEN) {
            fprintf(stderr, "Sorry, string too long, maximum %d \
                             characters\n", CAT_CAT_LEN);
            string_ok = 0;
        }
    } while (!string_ok);

    while (!entry_selected) {
        // the first_call flag starts as 1, and is set to 0 inside of the
        // search_cdc_entry function, which uses module-level variables to
        // handle the "curser" over results.
        item_found = search_cdc_entry(tmp_str, &first_call);
        if (item_found.catalog[0] != '\0') {
            any_entry_found = 1;
            printf("\n");
            display_cdc(&item_found);
            if (get_confirm("This entry? ")) {
                entry_selected = 1;
            }
        } else {
            if (any_entry_found) printf("Sorry, no more matches found\n");
            else printf("Sorry, nothing found\n");
            break;
        }
    }
    return(item_found);
}


/* print out all the tracks for a given catalog entry. */
static void list_tracks(const cdc_entry *entry_to_use)
{
    int track_no = 1;
    cdt_entry entry_found;

    display_cdc(entry_to_use);
    printf("\nTracks\n");
    do {
            entry_found = get_cdt_entry(entry_to_use->catalog, 
                                        track_no);
            if (entry_found.catalog[0]) {
                display_cdt(&entry_found);
                track_no++;
            }
    } while(entry_found.catalog[0]);
    get_confirm("Press return");
} /* list_tracks */


/* The count_all_entries function counts all the tracks. */
static void count_all_entries(void)
{
    int cd_entries_found = 0;
    int track_entries_found = 0;
    cdc_entry cdc_found;
    cdt_entry cdt_found;
    int track_no = 1;
    int first_time = 1;
    char *search_string = "";

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

    printf("Found %d CDs, with a total of %d tracks\n", cd_entries_found, 
                track_entries_found);
    (void)get_confirm("Press return");
}


/* show a single catalog entry. */
static void display_cdc(const cdc_entry *cdc_to_show)
{
    printf("Catalog: %s\n", cdc_to_show->catalog);
    printf("\ttitle: %s\n", cdc_to_show->title);
    printf("\ttype: %s\n", cdc_to_show->type);
    printf("\tartist: %s\n", cdc_to_show->artist);
}


/* show a single track entry. */
static void display_cdt(const cdt_entry *cdt_to_show) {
    printf("%d: %s\n", cdt_to_show->track_no, cdt_to_show->track_txt);
}


/* The utility function strip_return removes a trailing linefeed character from
 * a string.  Remember that UNIX uses a single linefeed to indicate end of
 * line. */
static void strip_return(char *string_to_strip) {
    int len;
    len = strlen(string_to_strip);
    if (string_to_strip[len - 1] == '\n') string_to_strip[len - 1] = '\0';
}


/* command_mode is a function for parsing the command line arguments.
 * the only option offered is -i, for initializing the database. All other
 * interaction is in interactive mode, which takes no command line args. */
static int command_mode(int argc, char *argv[]) {
    int c;
    int result = EXIT_SUCCESS;
    char *prog_name = argv[0];

    /* these externals used by getopt */
    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((c = getopt(argc, argv, ":i")) != -1) {
        switch(c) {
            case 'i':
                if (!database_initialize(1)) {
                    result = EXIT_FAILURE;
                    fprintf(stderr, "Failed to initialize database\n");
                }
                break;
            case ':':
            case '?':
            default:
                fprintf(stderr, "Usage: %s [-i]\n", prog_name);
                result = EXIT_FAILURE;
                break;
        } /* switch */
    } /* while */
    return(result);
}


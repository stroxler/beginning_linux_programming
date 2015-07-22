#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

// should be ndbm.h in some distros
#include <gdbm-ndbm.h>

#include "cd_data.h"

// the BASE names ar ethe names we give dbm,
// the other two are the names of the actual files
// dbm creates, which we need when removing the old db
// and replacing it.
#define CDC_FILE_BASE "/tmp/cdc_data"
#define CDC_FILE_DIR "/tmp/cdc_data.dir"
#define CDC_FILE_PAG "/tmp/cdc_data.pag"
#define CDT_FILE_BASE "/tmp/cdt_data"
#define CDT_FILE_DIR "/tmp/cdt_data.dir"
#define CDT_FILE_PAG "/tmp/cdt_data.pag"

// globals
static DBM *cdc_dbm_ptr = NULL;
static DBM *cdt_dbm_ptr = NULL;


/* Create new DBM connections. If `new_database` is 0, then
 * open existing databases and return error if they don't exist; if
 * `new_database` is 1 then delete any existing dbs and create new ones.
 * Returns 0 on failure, 1 on success. */
int database_initialize(const int new_database) {
    // if an existing database is open, close it
    if (cdc_dbm_ptr) dbm_close(cdc_dbm_ptr);
    if (cdt_dbm_ptr) dbm_close(cdt_dbm_ptr);
    // if new_database, unlink everything
    if (new_database) {
        unlink(CDC_FILE_DIR);
        unlink(CDC_FILE_PAG);
        unlink(CDT_FILE_DIR);
        unlink(CDT_FILE_PAG);
    }
    // open new files, creating if needed.
    int open_mode = O_CREAT | O_RDWR;
    cdc_dbm_ptr = dbm_open(CDC_FILE_BASE, open_mode, 0644);
    cdt_dbm_ptr = dbm_open(CDT_FILE_BASE, open_mode, 0644);
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) {
        fprintf(stderr, "Unable to create database\n");
        cdc_dbm_ptr = cdt_dbm_ptr = NULL;
        return 0;
    }
    return 1;
}


/* Close the DBM instances */
void database_close(void) {
    if (cdc_dbm_ptr) dbm_close(cdc_dbm_ptr);
    if (cdt_dbm_ptr) dbm_close(cdt_dbm_ptr);
    cdc_dbm_ptr = cdt_dbm_ptr = NULL;
}


/* Look up a single catalog entry, given the catalog as text (the id string),
 * which is the key in dbm */
cdc_entry get_cdc_entry(const char *catalog_id) {
    cdc_entry entry_to_return;
    char text_to_find[CAT_LEN + 1];
    datum local_data_datum;
    datum local_key_datum;
    // start with a null entry. Return immediately under
    // several conditions we know cause issues.
    memset(&entry_to_return, '\0', sizeof(entry_to_return));
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return entry_to_return;
    if (strlen(catalog_id) >= CAT_LEN) return entry_to_return;
    // copy over the search text; to use as a dbm key we need it to
    // be a fixed number of bytes
    memset(&text_to_find, '\0', sizeof(text_to_find));
    strcpy(text_to_find, catalog_id);
    // initialize the key datum to look up
    local_key_datum.dptr = (void *) text_to_find;
    local_key_datum.dsize = sizeof(text_to_find);
    // initialize the result datum to all zeros (it stays that way if dbm
    // does not find the key)
    memset(&local_data_datum, '\0', sizeof(local_data_datum));
    local_data_datum = dbm_fetch(cdc_dbm_ptr, local_key_datum);
    if (local_data_datum.dptr) {
        memcpy(&entry_to_return, (char *)local_data_datum.dptr,
               local_data_datum.dsize);
    }
    return entry_to_return;
}


/* look up a particular track number on a particular catalog entry,
 * using the catalog text (catalog id string) and track number as the key */
cdt_entry get_cdt_entry(const char *catalog_id, const int track_no) {
    cdt_entry entry_to_return;
    char text_to_find[CAT_LEN + 10];
    datum local_data_datum;
    datum local_key_datum;
    // set to zero, handle edge cases
    memset(&entry_to_return, '\0', sizeof(entry_to_return));
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return entry_to_return;
    if (strlen(catalog_id) >= CAT_LEN) return entry_to_return;
    // set up the search key, which comes from both the catalog text and
    // the track_no
    memset(text_to_find, '\0', sizeof(text_to_find));
    sprintf(text_to_find, "%s %d", catalog_id, track_no);
    // set up the key datum
    local_key_datum.dptr = (void *) text_to_find;
    local_key_datum.dsize = sizeof(text_to_find);
    // initialize the result datum to all zeros (it stays that way if dbm
    // does not find the key)
    memset(&local_data_datum, '\0', sizeof(local_data_datum));
    local_data_datum = dbm_fetch(cdc_dbm_ptr, local_key_datum);
    if (local_data_datum.dptr) {
        memcpy(&entry_to_return, (char *)local_data_datum.dptr,
               local_data_datum.dsize);
    }
    return entry_to_return;
}


/* add a cdc entry. The key is the catalog id string
 * Returns 1 for success, 0 for error (this is kind of weird b/c it's
 * the opposite of dbm and most unix stuff) */
int add_cdc_entry(const cdc_entry entry_to_add) {
    char key_to_add[CAT_LEN + 1];
    datum local_key_datum;
    datum local_data_datum;
    int result;
    // handle edge cases
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
    if (strlen(entry_to_add.catalog) >= CAT_LEN) return 0;
    // set up the key. Make sure to zero things out
    memset(&key_to_add, '\0', sizeof(key_to_add));
    strcpy(key_to_add, entry_to_add.catalog);
    // set up the datums
    local_key_datum.dptr = (void *) key_to_add;
    local_key_datum.dsize = sizeof(key_to_add);
    local_data_datum.dptr = (void *) &entry_to_add;
    local_data_datum.dsize = sizeof(entry_to_add);
    // store it, and return 1 for success, 0 for failure
    result = dbm_store(cdc_dbm_ptr, local_key_datum, local_data_datum,
                       DBM_REPLACE);
    return (result == 0) ? 1 : 0;
}


/* add a cdt entry. The key is a string that includes the catalog id of
 * the album and the track number */
int add_cdt_entry(const cdt_entry entry_to_add) {
    char key_to_add[CAT_LEN + 10];
    datum local_key_datum;
    datum local_data_datum;
    int result;
    // handle edge cases
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
    if (strlen(entry_to_add.catalog) >= CAT_LEN) return 0;
    // set up the key. Make sure to zero things out
    memset(&key_to_add, '\0', sizeof(key_to_add));
    sprintf(key_to_add, "%s %d", entry_to_add.catalog, entry_to_add.track_no);
    // set up the datums
    local_key_datum.dptr = (void *) key_to_add;
    local_key_datum.dsize = sizeof(key_to_add);
    local_data_datum.dptr = (void *) &entry_to_add;
    local_data_datum.dsize = sizeof(entry_to_add);
    // store it, and return 1 for success, 0 for failure
    result = dbm_store(cdt_dbm_ptr, local_key_datum, local_data_datum,
                       DBM_REPLACE);
    return (result == 0) ? 1 : 0;
}


/* delete a cdc entry, using the catalog id string.
 * return 1 for success, 0 for failure. */
int del_cdc_entry(const char *catalog_id) {
    char key_to_del[CAT_LEN + 1];
    datum local_key_datum;
    // handle edge cases
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
    if (strlen(catalog_id) >= CAT_LEN) return 0;
    // set up key
    memset(key_to_del, '\0', sizeof(key_to_del));
    strcpy(key_to_del, catalog_id);
    local_key_datum.dptr = (void *) key_to_del;
    local_key_datum.dsize = sizeof(key_to_del);
    // do the deletiion, return error code
    int result = dbm_delete(cdc_dbm_ptr, local_key_datum);
    return (result == 0) ? 1 : 0;
}

/* delete a cdc entry, using the catalog id string and the track number
 * return 1 for success, 0 for failure. */
int del_cdt_entry(const char *catalog_id, const int track_no) {
    char key_to_del[CAT_LEN + 10];
    datum local_key_datum;
    // handle edge cases
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return 0;
    if (strlen(catalog_id) >= CAT_LEN) return 0;
    // set up key
    memset(key_to_del, '\0', sizeof(key_to_del));
    sprintf(key_to_del, "%s %d", catalog_id, track_no);
    local_key_datum.dptr = (void *) key_to_del;
    local_key_datum.dsize = sizeof(key_to_del);
    // do the deletiion, return error code
    int result = dbm_delete(cdt_dbm_ptr, local_key_datum);
    return (result == 0) ? 1 : 0;
}


/* */
cdc_entry search_cdc_entry(const char *catalog_id, int *first_call_ptr) {
    // tracking whether to restart our "cursor". Note that this is static!
    static int local_first_call = 1;
    // note that the key datum is also static!
    static datum local_key_datum;
    // other vars do not need to be static
    cdc_entry entry_to_return;
    datum local_data_datum;

    // set bytes to 0, handle edge cases
    memset(&entry_to_return, '\0', sizeof(entry_to_return));
    if (!cdc_dbm_ptr || !cdt_dbm_ptr) return entry_to_return;
    if (!catalog_id || !first_call_ptr) return (entry_to_return);
    if (strlen(catalog_id) >= CAT_LEN) return (entry_to_return);

    // protect against never passing *first_call_ptr = true,
    //   then handle the usual logic: set *first_call_ptr = false and
    //   initialize the search using dbm_firstkey if *first_call_ptr,
    //   and otherwise continue a search using dbm_nextkey.
    if (local_first_call) {
        local_first_call = 0;
        *first_call_ptr = 1;
    }
    if (*first_call_ptr) {
        *first_call_ptr = 0;
        local_key_datum = dbm_firstkey(cdc_dbm_ptr);
    } else {
        local_key_datum = dbm_nextkey(cdc_dbm_ptr);
    }

    // loop over the entries (until we get out a datum with null pointers,
    //   which is how C tells us we are done)
    int found = 0;
    do {
        if (local_key_datum.dptr != NULL) {
            // read the data datum
            local_data_datum = dbm_fetch(cdc_dbm_ptr, local_key_datum);
            // if the catalog_id is a partial or full match, we are done
            // with (this) search (we can search again by passing
            // *first_call_ptr as 0)
            // otherwise, we continue looping.
            if (strstr(entry_to_return.catalog, catalog_id)) {
                found = 1;
            } else {
                local_key_datum = dbm_nextkey(cdc_dbm_ptr);
            }
        }
    } while (local_key_datum.dptr && local_data_datum.dptr && !found);
    return entry_to_return;
}

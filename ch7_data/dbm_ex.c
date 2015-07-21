#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

// on some linux distros, you might use just ndbm.h
#include <gdbm-ndbm.h>

#define TEST_DB_FILE "/tmp/dbm1_test"
#define ITEMS_USED 3

/* Part 1, which saves some data, of demoing usage of the dbm database, a
 * very lightweight key-value store that takes arbitrary binary keys and
 * values and stores them in files (somewhat analagous to SQLite)
 */

// dbm works on unstructured byte data. We will use it with string
// keys and `struct test_data` values, but it doesn't care (or even know)
// what types we are using.
struct test_data {
    char misc_chars[15];
    int any_integer;
    char more_chars[21];
};

int main() {
    struct test_data items_to_store[ITEMS_USED];
    struct test_data item_retrieved;
    char key_to_use[20];
    int i, result;
    // a datum is dbm's wrapper around binary data. It consists of a void*
    // to our data and a size_t giving the data size
    datum key_datum;
    datum data_datum;
    // a DBM struct is a wrapper around two files, one used to track indices
    // and another used to store the binary data.
    DBM *dbm_p;

    dbm_p = dbm_open(TEST_DB_FILE, O_RDWR | O_CREAT, 0666);
    if (!dbm_p) {
        fprintf(stderr, "Failed to open database\n");
        exit(EXIT_FAILURE);
    }

    // set all data to zero first, then set some data
    //    note that we *need* all zeros in some cases, e.g. if we use
    //    char arrays as keys. If they are only used for values it's less
    //    important.
    memset(items_to_store, '\0', sizeof(items_to_store));
    // Ff47
    strcpy(items_to_store[0].misc_chars, "First!");
    items_to_store[0].any_integer = 47;
    strcpy(items_to_store[0].more_chars, "foo");
    // bu13
    strcpy(items_to_store[1].misc_chars, "bar");
    items_to_store[1].any_integer = 13;
    strcpy(items_to_store[1].more_chars, "unlucky?");
    // Tb3
    strcpy(items_to_store[2].misc_chars, "Third");
    items_to_store[2].any_integer = 3;
    strcpy(items_to_store[2].more_chars, "baz");

    for (i = 0; i < ITEMS_USED; i++) {
        // make the key string
        sprintf(key_to_use, "%c%c%d",
                items_to_store[i].misc_chars[0],
                items_to_store[i].more_chars[0],
                items_to_store[i].any_integer);
        // make the key. Check that you understand why we *must* use
        // strlen() here and not sizeof()
        key_datum.dptr = (void *) key_to_use;
        key_datum.dsize = strlen(key_to_use);
        // make the data
        data_datum.dptr = (void *) &items_to_store[i];
        data_datum.dsize = sizeof(struct test_data);

        // write the data. Note that we have a choice of what to do
        // if we see a known key; here it doesn't matter.
        result = dbm_store(dbm_p, key_datum, data_datum, DBM_REPLACE);
        if (result != 0) {
            fprintf(stderr, "Failed to write to database\n");
            exit(2);
        }
    }

    // now try retrieving the second entry
    sprintf(key_to_use, "bu%d", 13);
    key_datum.dptr = (void *) key_to_use;
    key_datum.dsize = strlen(key_to_use);
    data_datum = dbm_fetch(dbm_p, key_datum);
    // if we don't find a key, we get a datum with NULL dptr
    if (data_datum.dptr) {
        printf("Data retrieved\n");
        memcpy(&item_retrieved, data_datum.dptr, data_datum.dsize);
        printf("Retrieved item - %s %d %s\n",
                item_retrieved.misc_chars,
                item_retrieved.any_integer,
                item_retrieved.more_chars);
    } else {
        printf("No data found for key %s\n", key_to_use);
    }

    printf("\n\nNow we'll try deleting and then cycling\n\n");

    // delete the entry we just looked at
    if (dbm_delete(dbm_p, key_datum) == 0) {
        printf("Data with key %s deleted\n", key_to_use);
    } else {
        printf("Nothing deleted for key %s\n", key_to_use);
    }

    // now use dbm_firstkey and dbm_nextkey to iterate over keys.
    //   ... the order in which they appear is not defined
    for (key_datum = dbm_firstkey(dbm_p);
         key_datum.dptr;                    // the end condition is a NULL dptr
         key_datum = dbm_nextkey(dbm_p)) {
        // get the data. The data_datum shouldn't be null in this situation,
        // but it could happen due to race conditions and such, so we should
        // still guard against it.
        data_datum = dbm_fetch(dbm_p, key_datum);
        if (data_datum.dptr) {
        printf("Data retrieved for key %s\n", (char *)key_datum.dptr);
        memcpy(&item_retrieved, data_datum.dptr, data_datum.dsize);
        printf("Retrieved item - %s %d %s\n",
                item_retrieved.misc_chars,
                item_retrieved.any_integer,
                item_retrieved.more_chars);

        } else {
            // the authors oopsed here, don't use `key_to_use`
            printf("No data found for key %s\n", (char *) key_datum.dptr);
        }
    }
    
    // don't forget to close the database!
    dbm_close(dbm_p);
    exit(EXIT_SUCCESS);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql.h"
#include "app_mysql.h"

static MYSQL my_connection;
static int dbconnected = 0;

// there's one utility function we didn't expose in the header, so we need
// a prototype for it
static int get_artist_id(char *artist);


/* Start the database. Initialize the MYSQL (which is a global variable in
 * this module) and connect it. Check `dbconnected` first to ensure idempotence
 */
int database_start(char *name, char *pwd) {
    if (dbconnected) return 1;
    mysql_init(&my_connection);
    if (!mysql_real_connect(&my_connection, "localhost", name, pwd,
            "blpcd", 0, NULL, 0)) {
        fprintf(stderr, "Database connection failure: %d, %s\n",
                mysql_errno(&my_connection), mysql_error(&my_connection));
        return 0;
    }
    dbconnected = 1;
    return 1;
}


/* Close the MYSQL, and set the `dbconnected` flag to 0 */
void database_end() {
    if (dbconnected) mysql_close(&my_connection);
    dbconnected = 0;
}


/* Add a cd. Return 1 if successful, if there are any failures.
 * set the address of cd_id to be the id of the cd we added (which is
 * auto-incremented in the database).
 */
int add_cd(char *artist, char *title, char *catalogue, int *cd_id) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;
    int res;
    char is[250];  // insert string
    char es[250];  // escaped string - we escape all string inputs!
    int artist_id = -1;
    int new_cd_id = -1;

    if (!dbconnected) return 0;

    // handle the artist_id. This might do a lookup or an insert, depending.
    artist_id = get_artist_id(artist);

    // save the escaped title in es, the constructed query in is.
    // Then run the query and check the return code.
    mysql_escape_string(es, title, strlen(title));
    sprintf(
        is,
        "INSERT INTO cd(title, artist_id, catalogue) VALUES('%s', %d, '%s')",
        es, artist_id, catalogue
    );
    res = mysql_query(&my_connection, is);
    if (res) {
        fprintf(stderr, "Insert error %d: %s\n",
                mysql_errno(&my_connection), mysql_error(&my_connection));
        return 0;
    }

    // now, get the id, which we 'return' to user via the *cd_id input
    res = mysql_query(&my_connection, "SELECT LAST_INSERT_ID()");
    if (res) {
        printf("SELECT error: %s\n", mysql_error(&my_connection));
        return 0;
    }
    res_ptr = mysql_use_result(&my_connection);
    if (res_ptr) {
        if ((mysqlrow = mysql_fetch_row(res_ptr))) {
            sscanf(mysqlrow[0], "%d", &new_cd_id);
        }
        mysql_free_result(res_ptr); // remember to free result!
    }
    *cd_id = new_cd_id;

    // return 1 if everything worked (id is set and nonzero), 0 otherwise
    if (new_cd_id != -1) return 1;
    return 0;
}


/* Get an artist id for an artist title. Add it to the db if not found,
 * otherwise return the existing id. Return 0 for any error.
 */
static int get_artist_id(char *artist) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;
    int res;
    char qs[250];
    char is[250];
    char es[250];
    int artist_id = -1;

    // look up the artist to see if they exist already.
    // use es for escaped name, qs for the constructed querystring
    // if there are multiple matches, use the first (shouldn't happen)
    mysql_escape_string(es, artist, strlen(artist));
    sprintf(qs, "SELECT id FROM artist WHERE name = '%s'", es);
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(&my_connection));
    } else {
        // use store_result, not use_result, so we can then use mysql_num_rows.
        // we expect the result to be small, so ram isn't an issue.
        res_ptr = mysql_store_result(&my_connection);
        if (res_ptr) {
            if (mysql_num_rows(res_ptr) > 0) {
                if (mysqlrow = mysql_fetch_row(res_ptr)) {
                    sscanf(mysqlrow[0], "%d", &artist_id);
                }
            }
            mysql_free_result(res_ptr); // remember to free result!
        }
    }
    if (artist_id != -1) return artist_id;

    // if we get here, no artist was found. So add a new one
    sprintf(is, "INSERT INTO artist(name) VALUES('%s')", es);
    res = mysql_query(&my_connection, is);
    if (res) {
        fprintf(stderr, "Insert error %d: %s\n",
                mysql_errno(&my_connection), mysql_error(&my_connection));
        return 0;
    }

    // and we need to return the id, so get the auto-incremented value
    res = mysql_query(&my_connection, "SELECT LAST_INSERT_ID()");
    if (res) {
      printf("SELECT error: %s\n", mysql_error(&my_connection));
      return 0;
    } else {
        res_ptr = mysql_use_result(&my_connection);
        if (res_ptr) {
            if ((mysqlrow = mysql_fetch_row(res_ptr))) {
                sscanf(mysqlrow[0], "%d", &artist_id);
            }
            mysql_free_result(res_ptr); // remember to free result!
        }
    }
    return artist_id;
}


/* Add tracks encapsulated in a current_tracks_st to the db. The cd id is
 * embedded in the current_tracks_st.
 * Return 1 for success, 0 for error.
 */
int add_tracks(struct current_tracks_st *tracks) {
    int res;
    char is[250];
    char es[250];
    int i;
    
    if (!dbconnected) return 0;
    
    // add each track. Note that current_tracks_st *must* have an entry
    // '\0' at the first byte in order to indicate that we are finished.
    // Also remember we need to escape the raw inputs.
    i = 0;
    while (tracks->track[i][0]) {
        mysql_escape_string(es, tracks->track[i], strlen(tracks->track[i]));
        sprintf(
            is,
            "INSERT INTO track(cd_id, track_id, title) VALUES(%d, %d, '%s')",
            tracks->cd_id, i + 1, es
        );
        res = mysql_query(&my_connection, is);
        if (res) {
            fprintf(stderr, "Insert error %d: %s\n",
                    mysql_errno(&my_connection), mysql_error(&my_connection));
            return 0;
        }
        i++;
    }
    return 1;
}

/* Fill out a current_cd_st representing a cd, looking up by cd_id.
 * Return 1 for success, 0 for failure.
 *   (Doesn't get the tracks, there's a separate function for that)
 */
int get_cd(int cd_id, struct current_cd_st *dest) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;
    int res;
    char qs[250];
    
    // check connection, zero out `dest`, and set artist_id to -1 (which we
    // use to check for normal execution)
    if (!dbconnected) return 0;
    memset(dest, 0, sizeof(*dest));
    dest->artist_id = -1;
    
    // construct the query string. Note that we need to join on artist id
    // because artist names are in a different table.
    sprintf(qs, "SELECT artist.id, cd.id, artist.name, cd.title, cd.catalogue "
            "FROM artist, cd WHERE artist.id = cd.artist_id and cd.id = %d",
            cd_id);
    
    // execute the query, and copy the resulting entry into dest
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(&my_connection));
    } else {
        // use mysql_store_result becuase we only expect one match, and we
        // want to be able to check mysql_num_rows to see if there was one.
        res_ptr = mysql_store_result(&my_connection);
        if (res_ptr) {
            if (mysql_num_rows(res_ptr) > 0) {
                if (mysqlrow = mysql_fetch_row(res_ptr)) {
                  sscanf(mysqlrow[0], "%d", &dest->artist_id);
                  sscanf(mysqlrow[1], "%d", &dest->cd_id);
                  strcpy(dest->artist_name, mysqlrow[2]);
                  strcpy(dest->title, mysqlrow[3]);
                  strcpy(dest->catalogue, mysqlrow[4]);
                }
            }
            mysql_free_result(res_ptr); // don't forget to free result!
        }
    }
    if (dest->artist_id != -1) return 1;
    return 0;
} 


/* Get the cd tracks for a cd, by cd_id
 * Return the number of tracks we got, or 0 for any errors */
int get_cd_tracks(int cd_id, struct current_tracks_st *dest) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;
    int res;
    char qs[250];
    int i = 0, num_tracks = 0;

    // remember to zero out dest. Set the cd_id to -1 so we can detect errors
    if (!dbconnected) return 0;
    memset(dest, 0, sizeof(*dest));
    dest->cd_id = -1;

    // run the query. Note that we don't do any checks on the number of tracks
    // to avoid buffer overflow - we are counting on the fact that we don't
    // write anything bigger than a current_tracks_st to the db in the first
    // place.
    sprintf(qs, "SELECT track_id, title FROM track "
            "WHERE track.cd_id = %d ORDER BY track_id", cd_id);
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(&my_connection));
    } else {
        res_ptr = mysql_store_result(&my_connection);
        if (res_ptr) {
            if ((num_tracks = mysql_num_rows(res_ptr)) > 0) {
                while (mysqlrow = mysql_fetch_row(res_ptr)) {
                    strcpy(dest->track[i], mysqlrow[1]);
                    i++;
                }
                dest->cd_id = cd_id;
            }
            mysql_free_result(res_ptr); // remember to free result!
        }
    }
    return num_tracks;
}


/* Searches for cds in which *search_str partially matches one of the
 * title, artist name, or catalog id.
 * Returns the total number of matches, and copies the first MAX_CD_RESULT
 * of them into *dest.
 * Returns 0 if no matches found, or if any errors occur.
 */
int find_cds(char *search_str, struct cd_search_st *dest) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;
    int res;
    char qs[500];
    int i = 0;
    char ss[250];
    int num_rows = 0;

    if (!dbconnected) return 0;

    // zero out all data in dest
    memset(dest, -1, sizeof(*dest));

    // make the query string to find cds. Be sure to escape input.
    //
    // The first line in the WHERE is doing a join on artist_id.
    //
    // I actually think the artist id in the result is superfluous; we only use
    // the cd_id in the result. The artist id is needed for the join only.
    mysql_escape_string(ss, search_str, strlen(search_str));
    sprintf(qs,
            "SELECT DISTINCT artist.id, cd.id FROM artist, cd "
            "    WHERE artist.id = cd.artist_id "
            "    AND (artist.name LIKE '%%%s%%' OR cd.title LIKE '%%%s%%' OR "
            "    cd.catalogue LIKE '%%%s%%')",
            ss, ss, ss);

    // now, execute the query.
    // use mysql_store_result so that we can find out how many total rows there
    // were; only add at most MAX_CD_RESULT of them to *dest, but we return the
    // total number of results.
    res = mysql_query(&my_connection, qs);
    if (res) {
      fprintf(stderr, "SELECT error: %s\n", mysql_error(&my_connection));
    } else {
        res_ptr = mysql_store_result(&my_connection);
        if (res_ptr) {
            num_rows = mysql_num_rows(res_ptr);
            if ( num_rows > 0) {
                while ((mysqlrow = mysql_fetch_row(res_ptr)) &&
                        i < MAX_CD_RESULT) {
                    // Note that due to operator precedence, the
                    // & binds last, so &dest->cd_id[i] is &(dest->cd_id[i]),
                    // the address of the int.
                    sscanf(mysqlrow[1], "%d", &dest->cd_id[i]);
                    i++;
                }
            }
            mysql_free_result(res_ptr); // remember to free result!
        }
    }
    return num_rows;
}


/* Removes a cd from the cds table. Also removes all tracks, and removes the
 * artist *if* the artist doesn't appear in any remaining cds.
 * Returns 1 if successful, 0 if anything unexpected happened.
 */
int delete_cd(int cd_id) {

    int res;
    char qs[250];
    int artist_id, num_rows;
    MYSQL_RES *res_ptr;
    MYSQL_ROW mysqlrow;

    if (!dbconnected) return 0;

    // find out whether to delete the artist. -1 means don't delete.
    artist_id = -1;
    sprintf(
        qs,
        "SELECT artist_id FROM cd WHERE artist_id = "
        "(SELECT artist_id FROM cd WHERE id = '%d')",
        cd_id
    );
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(&my_connection));
    } else {
        res_ptr = mysql_store_result(&my_connection);
        if (res_ptr) {
            num_rows = mysql_num_rows(res_ptr);
            if (num_rows == 1) {  // if there's exactly 1 match, we delete.
                mysqlrow = mysql_fetch_row(res_ptr);
                sscanf(mysqlrow[0], "%d", &artist_id);
            }
            mysql_free_result(res_ptr); // remember to free result!
        }
    }

    // delete all the tracks.
    sprintf(qs, "DELETE FROM track WHERE cd_id = '%d'", cd_id);
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "Delete error (track) %d: %s\n",
                mysql_errno(&my_connection), mysql_error(&my_connection));
        return 0;
    }

    // delete the cd
    sprintf(qs, "DELETE FROM cd WHERE id = '%d'", cd_id);
    res = mysql_query(&my_connection, qs);
    if (res) {
        fprintf(stderr, "Delete error (cd) %d: %s\n",
                mysql_errno(&my_connection), mysql_error(&my_connection));
        return 0;
    }

    // delete artist if we found it in the initial search.
    if (artist_id != -1) {
        sprintf(qs, "DELETE FROM artist WHERE id = '%d'", artist_id);
        res = mysql_query(&my_connection, qs);
        if (res) {
            fprintf(stderr, "Delete error (artist) %d: %s\n",
                    mysql_errno(&my_connection), mysql_error(&my_connection));
        }
    }
    return 1;
}

#include <stdlib.h>
#include <stdio.h>

#include "mysql.h"

/* Minimal example of failing to create a conection and printing errors
 */

int main(int argc, char *argv[]) {

    MYSQL my_conn;
    MYSQL_RES *res_ptr;
    MYSQL_ROW sqlrow;
    int res;

    if (!mysql_init(&my_conn)) {
        fprintf(stderr, "mysql_init failed\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(
                &my_conn, "localhost", "trox", "trox", "foo", 0, NULL, 0)) {
        printf("Connection success\n");

        /* update Jenny to be 8 and not 7 */
        res = mysql_query(&my_conn,
                "UPDATE children SET age = 8 WHERE fname = 'Jenny'");
        if (!res) { // return of zero means success
            printf("Updated %lu rows\n",
                   (unsigned long) mysql_affected_rows(&my_conn));
        } else {
            fprintf(stderr, "UPDATE error %d: %s\n",
                    mysql_errno(&my_conn), mysql_error(&my_conn));
        }

        /* Add a new entry, Flora */
        res = mysql_query(
                &my_conn,
                "INSERT INTO children(fname, age) VALUES('Flora', 10)"
        );
        if (!res) { // return of zero means success
            printf("Inserted %lu rows\n",
                   (unsigned long) mysql_affected_rows(&my_conn));
        } else {
            fprintf(stderr, "INSERT error %d: %s\n",
                    mysql_errno(&my_conn), mysql_error(&my_conn));
        }

        /* See what the auto-incremented childno was for Flora */
        res = mysql_query(&my_conn, "SELECT LAST_INSERT_ID()");
        if (!res) { // return of zero means success
            // get the result, in a single batch
            res_ptr = mysql_store_result(&my_conn);
            if (res_ptr) {
                // count the number of rows. This works for a result that
                // was obtained via mysql_store_result, but not if you get
                // it from mysql_use_result (see next example)... the same
                // goes for the mysql_row_seek function, which you will rarely
                // use but lets you seek rows in batched result sets.
                printf("Selecting for last insert id gave %lu rows\n",
                       (unsigned long) mysql_num_rows(res_ptr));
                // there should only be one row, but this is the typical
                // pattern for iterating over a result.
                while ((sqlrow = mysql_fetch_row(res_ptr))) {
                    printf("We inserted childno %s\n", sqlrow[0]);
                }
                // use this method to release allocated resources
                mysql_free_result(res_ptr);
            }
        } else {
            fprintf(stderr, "SELECT error %d: %s\n",
                    mysql_errno(&my_conn), mysql_error(&my_conn));
        }
    } else {
        fprintf(stderr, "Connection failed %d: %s\n",
                mysql_errno(&my_conn), mysql_error(&my_conn));
    }
    return 0;
}

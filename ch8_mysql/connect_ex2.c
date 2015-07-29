#include <stdlib.h>
#include <stdio.h>

#include "mysql.h"

/* Minimal example of failing to create a conection and printing errors
 */

int main(int argc, char *argv[]) {
    MYSQL my_conn;

    // our previous example passed NULL to mysql_init and used the returned
    // MYSQL* pointer. Here we only use the return value for its truthiness
    if (!mysql_init(&my_conn)) {
        fprintf(stderr, "mysql_init failed\n");
        return EXIT_FAILURE;
    }

    // similar to mysql_init, this time we ignore the return value, except
    // to measure its truthiness for error detection
    if (mysql_real_connect(
                &my_conn, "localhost", "trox", "trox", "notadb", 0, NULL, 0)) {
        printf("Connectiion succeeded\n");
        mysql_close(&my_conn);
    } else {
        fprintf(stderr, "Connectiion failed\n");
        // mysql_errno tells us if there's a mysql error code in the global
        // namespace. mysql_error should only be called if there's an errno; it
        // translates the errno into a human-readable string.
        if (mysql_errno(&my_conn)) {
            fprintf(stderr, "Connection error: %d %s\n",
                    mysql_errno(&my_conn), mysql_error(&my_conn));
        }
    }

    // cleanup
    return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <stdio.h>

#include "mysql.h"

/* Minimal example of creating a mysql connection.
 */

int main(int argc, char *argv[]) {
    MYSQL *conn_ptr;

    // mysql initializes the MYSQL struct *before* making a connection. Don't
    // pass a null pointer to mysql_real_connect!
    conn_ptr = mysql_init(NULL);
    if (!conn_ptr) {
        fprintf(stderr, "mysql_init failed\n");
        return EXIT_FAILURE;
    }

    // args are host, username, password, db
    // port number (0 maps to the default 3306), unix_socket_name,
    // and flags.
    conn_ptr = mysql_real_connect(conn_ptr, "localhost", "trox", "trox", "foo",
                                  0, NULL, 0);
    if (conn_ptr) {
        printf("Connectiion succeeded\n");
    } else {
        printf("Connectiion failed\n");
    }

    // cleanup
    mysql_close(conn_ptr);
    return EXIT_SUCCESS;
}

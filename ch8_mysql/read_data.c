#include <stdlib.h>
#include <stdio.h>

#include "mysql.h"


/* display a single row from a mysql result. Assumes the connection is
 * still fresh so that calling mysql_field_count works properly. */
void display_row(MYSQL *conn, MYSQL_ROW *row) {
    unsigned int field_count;
    unsigned int i;
    field_count = mysql_field_count(conn);
    i = 0;
    printf("\t");
    for (i = 0; i < field_count; i++) {
        if ((*row)[i]) printf("%s ", (*row)[i]);
        else printf("null ");
    }
    printf("\n");
}


/* display a one-line header for a query result */
void display_header(MYSQL_RES *res) {
    MYSQL_FIELD *field_ptr;
    printf("  Column details: \n\t");
    while ((field_ptr = mysql_fetch_field(res)) != NULL) {
        printf("%s: ", field_ptr->name);
        // the type logic is kind of convoluted and inconsistent...
        if (IS_NUM(field_ptr->type)) {
            printf("Numeric, ");
        } else {
            switch (field_ptr->type) {
                case FIELD_TYPE_VAR_STRING:
                    printf("VARCHAR, "); break;
                case FIELD_TYPE_LONG:
                    printf("LONG, "); break;
                default:
                    printf("unknown_type_oops, ");
            }
        }
        printf("maxwidth %ld", field_ptr->length);
        if (field_ptr->flags & AUTO_INCREMENT_FLAG) printf(", autoincr; ");
        else printf("; ");
    }
    printf("|\n");
}


/* execute a query, and display it with a header line */
void execute_query_and_show(MYSQL *conn, char *query) {
    MYSQL_RES *res_ptr;
    MYSQL_ROW sqlrow;
    int res;

    // Do the query...
    //   Use mysql_use_result, which fetches rows one at a time, rather than
    //   mysql_store_result. You iterate over the rows in the same way.
    res = mysql_query(conn, query);
    if (res) {
        fprintf(stderr, "SELECT error %d: %s\n",
                mysql_errno(conn), mysql_error(conn));
    } else {
        res_ptr = mysql_use_result(conn);
        if (!res_ptr) {
            fprintf(stderr, "retrieve error %d: %s\n",
                    mysql_errno(conn), mysql_error(conn));
        } else {
            display_header(res_ptr);
            while ((sqlrow = mysql_fetch_row(res_ptr))) {
                display_row(conn, &sqlrow);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    MYSQL my_conn;
    int res;

    if (!mysql_init(&my_conn)) {
        fprintf(stderr, "mysql_init failed\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(
                &my_conn, "localhost", "trox", "trox", "foo", 0, NULL, 0)) {
        printf("Connection success\n");

        printf("All children:\n");
        execute_query_and_show(
                &my_conn,
                "SELECT childno, fname, age FROM children"
        );

        printf("Children 10 and up:\n");
        execute_query_and_show(
                &my_conn,
                "SELECT childno, fname, age FROM children WHERE age >= 10"
        );




    } else {
        fprintf(stderr, "Connection failed %d: %s\n",
                mysql_errno(&my_conn), mysql_error(&my_conn));
    }
    return 0;
}

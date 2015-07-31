/* Required #include header files. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>


#define ERR_TEXT_LEN 80

/* We implement the commands as enumerated types, rather than #defines.  This
 * is a good way of allowing the compiler to do more type checking and also
 * helps in debugging the application, as many debuggers are able to show the
 * name of enumerated constants, but not the name defined by a #define
 * directive.  The first typedef gives the type of request being sent to the
 * server, the second the server response to the client.  */
typedef enum {
    s_create_new_database = 0,
    s_get_cdc_entry,
    s_get_cdt_entry,
    s_add_cdc_entry,
    s_add_cdt_entry,
    s_del_cdc_entry,
    s_del_cdt_entry,
    s_find_cdc_entry
} client_request_e;

/* Server responses are enumerated */
typedef enum {
    r_success = 0,
    r_failure,
    r_find_no_more
} server_response_e;

/* Next, we declare a structure that will form the message passed in both
 * directions between the two processes.
 *
 * Since we don't actually need to return both a cdc_entry and cdt_entry in the
 * same response, we could have combined them in a union. For simplicity, we
 * keep them separate.  This also makes the code easier to maintain.  */
typedef struct {
    pid_t               client_pid;
    client_request_e    request;
    server_response_e   response;
    cdc_entry           cdc_entry_data;
    cdt_entry           cdt_entry_data;
    char                error_text[ERR_TEXT_LEN + 1];
} message_db_t;

/*  Finally, we get to the mqueue interface functions that perform data transfer
 *  implementedg in mqueue_imp.c. These divide into server- and client-side
 *  functions, in the first and second blocks respectively.
 *
 *  Note that we don't have functions for specific database command - these
 *  are all encoded in side of the cient_request and server_response withing
 *  the message_db_t struct. */
int server_starting(void);
void server_ending(void);
int read_request_from_client(message_db_t *rec_ptr);
int start_resp_to_client(const message_db_t mess_to_send);
int send_resp_to_client(const message_db_t mess_to_send);
void end_resp_to_client(void);

int client_starting(void);
void client_ending(void);
int send_mess_to_server(message_db_t mess_to_send);
int start_resp_from_server(void);
int read_resp_from_server(message_db_t *rec_ptr);
void end_resp_from_server(void);



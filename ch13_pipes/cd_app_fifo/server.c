#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cd_data.h"
#include "cliserv.h"

int save_errno;
static int server_running = 1;

static void process_command(const message_db_t mess_command);

void catch_signals()
{
    server_running = 0;
}

/*
Now we come to the main function.

After checking that the signal catching routines,
the program checks to see whether you passed -i on the command line.

If you did, it will create a new database.
If all is well and the server is running,
any requests from the client are fed to the process_command function
that we'll meet in a moment. 

*/

/* The main function loops over reading from the server fifo (which is
 * actually done in the read_request_from_client function). Each time it
 * gets a request, it hands the request to process_command, which handles
 *   - figuring out what operation the client wanted (the same switch
 *     statement that is in main of app_ui.c, in effect)
 *   - delegating to some frunction from cd_dbm.c
 *   - coppying some of the data from the message_db_t struct coming in from 
 *     the client to the response, and then filling out other data depending
 *     on the action and the database results
 *   - sending the response message_db_t struct back to the client
 *
 * The functions that interact directly with the fifo all live in pipe_imp.c;
 * this module provides the main loop and the "glue" between the server-side
 * fifo handling and the server-side data api.
 *
 * Note that the actual calls to the server-side data api from here wind up
 * being identical to the actual calls to the client-side data api (the proxy)
 * made from app_ui.c.
 */
int main(int argc, char *argv[]) {
    struct sigaction new_action, old_action;
    message_db_t mess_command;
    int database_init_type = 0;

    new_action.sa_handler = catch_signals;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    if ((sigaction(SIGINT, &new_action, &old_action) != 0) ||
        (sigaction(SIGHUP, &new_action, &old_action) != 0) ||
        (sigaction(SIGTERM, &new_action, &old_action) != 0)) {
        fprintf(stderr, "Server startup error, signal catching failed\n");
        exit(EXIT_FAILURE);
    }    

    if (argc > 1) {
        argv++;
        if (strncmp("-i", *argv, 2) == 0) database_init_type = 1;
    }
    if (!database_initialize(database_init_type)) {
        fprintf(stderr, "Server error: could not initialize database\n");
        exit(EXIT_FAILURE);
    }

    if (!server_starting()) exit(EXIT_FAILURE);
    
    while(server_running) {
        if (read_request_from_client(&mess_command)) {
            process_command(mess_command);
        } else {
            if(server_running) fprintf(stderr, "Server ended - can not \
                                        read pipe\n");
            server_running = 0;
        }
    } /* while */
    server_ending();
    exit(EXIT_SUCCESS);
}

/* accept a client message `comm`, do a switch based on the action requested,
 * delegate database handling to functions in cd_dbm.c, construct a response
 * message, and send it. */
static void process_command(const message_db_t comm)
{
    message_db_t resp;
    int first_time = 1;

    resp = comm; /* copy command back, then change resp as required */

    if (!start_resp_to_client(resp)) {
        fprintf(stderr, "Server Warning:-\
                 start_resp_to_client %d failed\n", resp.client_pid);
        return;
    }

    // return a message that has either a success or failure flag, and also
    // has data if the operation cals for it (e.g. get_cdc_entry).
    resp.response = r_success;
    memset(resp.error_text, '\0', sizeof(resp.error_text));
    save_errno = 0;

    switch(resp.request) {
        case s_create_new_database:
            if (!database_initialize(1)) resp.response = r_failure;
            break;
        case s_get_cdc_entry:
            resp.cdc_entry_data = 
                           get_cdc_entry(comm.cdc_entry_data.catalog);
            break;
        case s_get_cdt_entry:
            resp.cdt_entry_data = 
                           get_cdt_entry(comm.cdt_entry_data.catalog, 
                                         comm.cdt_entry_data.track_no);
            break;
        case s_add_cdc_entry:
            if (!add_cdc_entry(comm.cdc_entry_data)) resp.response = 
                           r_failure;
            break;
        case s_add_cdt_entry:
            if (!add_cdt_entry(comm.cdt_entry_data)) resp.response = 
                           r_failure;
            break;            
        case s_del_cdc_entry:
            if (!del_cdc_entry(comm.cdc_entry_data.catalog)) resp.response
                         = r_failure;
            break;            
        case s_del_cdt_entry:
            if (!del_cdt_entry(comm.cdt_entry_data.catalog, 
                 comm.cdt_entry_data.track_no)) resp.response = r_failure;
            break;
        case s_find_cdc_entry:
            // notice that unlike all the other commands, which handle request
            // on a 1-1 basis with the ui, this one loops and does all of the
            // search_cdc_entry requests we need based on just a single client
            // request.
            //
            // We do this so that the client side can collect all the data,
            // dump it in a temporary file, and then feed it to the ui one
            // at a time using the same api we use here. See the code in
            // clientif.c to better understand this.
            do {
                resp.cdc_entry_data =
                          search_cdc_entry(comm.cdc_entry_data.catalog,
                                            &first_time);
                if (resp.cdc_entry_data.catalog[0] != 0) {
                    resp.response = r_success;
                    if (!send_resp_to_client(resp)) {
                        fprintf(stderr, "Server Warning:-\
                            failed to respond to %d\n", resp.client_pid);
                        break;
                    }
                } else {
                    resp.response = r_find_no_more;
                }
            } while (resp.response == r_success);
        break;
        default:
            resp.response = r_failure;
            break;
    } /* switch */

    sprintf(resp.error_text, "Command failed:\n\t%s\n", 
             strerror(save_errno));

    if (!send_resp_to_client(resp)) {
        fprintf(stderr, "Server Warning:-\
                 failed to respond to %d\n", resp.client_pid);
    }

    end_resp_to_client();
    return;
}

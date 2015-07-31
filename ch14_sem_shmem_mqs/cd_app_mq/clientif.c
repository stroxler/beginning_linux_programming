/* The file starts with #include files and constants. */

#define _POSIX_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cd_data.h"
#include "cliserv.h"

/* This is the client-side proxy to the data api. It forwards all operations
 * over the server mqueue to server.c, which in turn hands them off to the server
 * side data api. And it gets responses
 *
 * All of the low-level mqueue communication is done via calls to functions
 * in mqueue_imp.c
 */

/* storing mypid in a static var reduces the number of calls to getpid().  */
static pid_t mypid;

/* this is the only function used here not declared in cliserv.h */
static int read_one_response(message_db_t *rec_ptr);

/* database_initialize on the client side opens up the mqueue */
int database_initialize(const int new_database) {
    if (!client_starting()) return(0);
    mypid = getpid();
    return(1);
    
}

/* database_close on the client side closes and cleans up mqueues */
void database_close(void) {
    client_ending();
}

/*  pack up the cd_catalog_ptr into a message, along with an enum telling
 *  the server which function we called, then send it and read response */
cdc_entry get_cdc_entry(const char *cd_catalog_ptr)
{
    cdc_entry ret_val;
    message_db_t mess_send;
    message_db_t mess_ret;

    ret_val.catalog[0] = '\0';
    mess_send.client_pid = mypid;
    mess_send.request = s_get_cdc_entry;
    strcpy(mess_send.cdc_entry_data.catalog, cd_catalog_ptr);

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                ret_val = mess_ret.cdc_entry_data;
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(ret_val);
}


/* read a signle response from the server. Utility function used in
 * many of this file's functions.
 *   Calls, in turn, X_resp_from_server, with X \in {start, read, end}
 *
 * Returns 0 if any of the mqueue functions err out, else 1. */
static int read_one_response(message_db_t *rec_ptr) {

    int return_code = 0;
    if (!rec_ptr) return(0);
    if (start_resp_from_server()) {
        if (read_resp_from_server(rec_ptr)) {
            return_code = 1;
        }
        end_resp_from_server();
    }
    return(return_code);
}
 
/* All of the other get_xxx and set_xxx functions are implemented much like
 * get_cdc_entry: they package up their inputs into a
 * message_db_t struct, send it, and read the response.
 */

cdt_entry get_cdt_entry(const char *cd_catalog_ptr, const int track_no)
{
    cdt_entry ret_val;
    message_db_t mess_send;
    message_db_t mess_ret;

    ret_val.catalog[0] = '\0';
    mess_send.client_pid = mypid;
    mess_send.request = s_get_cdt_entry;
    strcpy(mess_send.cdt_entry_data.catalog, cd_catalog_ptr);
    mess_send.cdt_entry_data.track_no = track_no;

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                ret_val = mess_ret.cdt_entry_data;
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(ret_val);
}


int add_cdc_entry(const cdc_entry entry_to_add) {
    message_db_t mess_send;
    message_db_t mess_ret;

    mess_send.client_pid = mypid;
    mess_send.request = s_add_cdc_entry;
    mess_send.cdc_entry_data = entry_to_add;

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                return(1);
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(0);
}

int add_cdt_entry(const cdt_entry entry_to_add) {
    message_db_t mess_send;
    message_db_t mess_ret;

    mess_send.client_pid = mypid;
    mess_send.request = s_add_cdt_entry;
    mess_send.cdt_entry_data = entry_to_add;

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                return(1);
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(0);
}


int del_cdc_entry(const char *cd_catalog_ptr) {
    message_db_t mess_send;
    message_db_t mess_ret;

    mess_send.client_pid = mypid;
    mess_send.request = s_del_cdc_entry;
    strcpy(mess_send.cdc_entry_data.catalog, cd_catalog_ptr);

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                return(1);
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(0);
}


int del_cdt_entry(const char *cd_catalog_ptr, const int track_no)
{
    message_db_t mess_send;
    message_db_t mess_ret;

    mess_send.client_pid = mypid;
    mess_send.request = s_del_cdt_entry;
    strcpy(mess_send.cdt_entry_data.catalog, cd_catalog_ptr);
    mess_send.cdt_entry_data.track_no = track_no;

    if (send_mess_to_server(mess_send)) {
        if (read_one_response(&mess_ret)) {
            if (mess_ret.response == r_success) {
                return(1);
            } else {
                fprintf(stderr, "%s", mess_ret.error_text);
            }
        } else {
            fprintf(stderr, "Server failed to respond\n");
        }
    } else {
        fprintf(stderr, "Server not accepting requests\n");
    }
    return(0);
}

/* This is the most complicated of the functions.
 *
 * First, remember that we set *first_call_ptr by modifying the address in
 * our impementation of search_cdc_entry. This is more complicated in the
 * client-server setup, because the server can't modify *first_call_ptr for us!
 *
 * We manage first call pointer similarly here. And calls to the server
 * when *first_call_ptr is 0 proceed pretty much like all the functions
 * above, because we just need to ask for the next entry.
 *
 * But when *first_call_ptr is 1, this function gets *all* of the data from the
 * server, and writes it to a (static!) tempfile, keeping track of how many
 * in a (static!) counter. Later calls will read off this data.
 *
 * Note that we don't use read_one_response in this function, because we
 * have to read many responses. */
cdc_entry search_cdc_entry(const char *cd_catalog_ptr, int *first_call_ptr) {

    // notice that these are static: when we call with *first_call_ptr == 1,
    // we set the counter and write all the results to a file. Subsequent
    // calls read this data and decrement the counter.
    static FILE *work_file = (FILE *)0;
    static int entries_matching = 0;

    message_db_t mess_send;
    message_db_t mess_ret;
    
    cdc_entry ret_val;
    ret_val.catalog[0] = '\0';

    // if we've already NULLed out the work_file and this isn't a first call,
    // then just return a placeholder for no result.
    if (!work_file && (*first_call_ptr == 0)) return(ret_val);


    if (*first_call_ptr) { // we are starting a new search

        // close any existing work file, and open a new tmpfile.
        *first_call_ptr = 0;
        if (work_file) fclose(work_file);
        work_file = tmpfile();
        if (!work_file) return(ret_val);

        // set the pid and request (action). Copy the string we are
        // searching for. We copy it to the catalog part of the message, which
        // is I think mostly to save space since we never need a catalog id
        // at the same time that we are perorming a search.
        mess_send.client_pid = mypid;
        mess_send.request = s_find_cdc_entry;
        strcpy(mess_send.cdc_entry_data.catalog, cd_catalog_ptr);

        // send to the server
        //
        // it replies with many structs of responses. So, we write all of
        // them to our temp file (as raw bytes), and increment the
        // (static!!) entries_matching counter for each one.
        if (send_mess_to_server(mess_send)) {
            if (start_resp_from_server()) {
                while (read_resp_from_server(&mess_ret)) {
                    if (mess_ret.response == r_success) {
                       fwrite(&mess_ret.cdc_entry_data,
                              sizeof(cdc_entry), 1, work_file);
                        entries_matching++;
                    } else {
                        break;
                    }
                } /* while */
            } else {
                fprintf(stderr, "Server not responding\n");
            }
        } else {
            fprintf(stderr, "Server not accepting requests\n");
        }


        // reset the file head to the start of the file - we've finished
        // writing the result, now we need to read them back out.
        fseek(work_file, 0L, SEEK_SET);
    }

    // if we've already read all the entries (or, on the first call,
    // if thre are none) then close the file, set the static work_file
    // pointer to NULL, and return an empty result
    if (entries_matching == 0) {
        fclose(work_file);
        work_file = (FILE *)0;
        return(ret_val);
    }      

    fread(&ret_val, sizeof(cdc_entry), 1, work_file);    
    entries_matching--;
    
    return(ret_val);
}


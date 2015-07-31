/* Include files */

#include "cd_data.h"
#include "cliserv.h"

/* define some values that we need in different functions within the file. */

static int server_fd = -1;
static pid_t mypid = 0;
static char client_pipe_name[PATH_MAX + 1] = {'\0'};
static int client_fd = -1;
static int client_write_fd = -1;

/* initialize the server pipe, from the server side
 *   - unlink any old fifo
 *   - make a new fifo
 *   - open the read end fo the fifo, in blocking mode
 *
 * We don't open a write side of the fifo, which means that once any single
 * client connects, the next time there are no clients up the server will
 * read 0 bytes.
 *
 * We handle this issue in the read_request_from_client function,
 * which always closes and reopens after every request. That way if no clients
 * are connected and we read the last data, the next call to read will 
 * once again block until there's data.
 *
 * We could change this by holding a write-side fd on the fifo, if
 * we wanted the server to stay up until explicitly brought down.
 *
 * Returns 0 for error, 1 for success.
 */
int server_starting(void) {
    #if DEBUG_TRACE    
        printf("%d :- server_starting()\n",  getpid());
    #endif

        unlink(SERVER_PIPE);
    if (mkfifo(SERVER_PIPE, 0777) == -1) {
        fprintf(stderr, "Server startup error, no FIFO created\n");
        return(0);
    }

    if ((server_fd = open(SERVER_PIPE, O_RDONLY)) == -1) {
        fprintf(stderr, "Server startup error, no FIFO opened\n");
        return(0);
    }
    return(1);
}


/* server side:
 *
 * When the server exits, close and also unlink the server fifo. */
void server_ending(void)
{
    #if DEBUG_TRACE    
        printf("%d :- server_ending()\n",  getpid());
    #endif

    close(server_fd);
    unlink(SERVER_PIPE);
}


/* read_request_from_client (this is called in main() of server.c, and then
 * the message gets passed to the process_command function)
 *
 * It puts the data into *rec_ptr, and returns 0 if it read no data or if
 * any errors occur.
 *
 * If it reads 0 bytes, it will try to reopen the pipe and read again. This
 * causes read to block if all clients have disconnected but the fifo is still
 * intact, which is the desired behavior.
 *
 * Returns 1 on successful reads, and zero if we read malformed data or if
 * we fail to reopen the fifo */
int read_request_from_client(message_db_t *rec_ptr) {
    int return_code = 0;
    int read_bytes;

    #if DEBUG_TRACE    
        printf("%d :- read_request_from_client()\n",  getpid());
    #endif

    if (server_fd != -1) {
        read_bytes = read(server_fd, rec_ptr, sizeof(*rec_ptr)); 

        if (read_bytes == 0) {
            close(server_fd);
            if ((server_fd = open(SERVER_PIPE, O_RDONLY)) == -1) {
               fprintf(stderr, "Server error, FIFO open failed\n");
               return(0);
            }
            read_bytes = read(server_fd, rec_ptr, sizeof(*rec_ptr));
        }
        if (read_bytes == sizeof(*rec_ptr)) return_code = 1;
    }
    return(return_code);
}


/* Server side:
 *
 * open the write side of a client's fifo.
 *
 * return 0 if fail, 1 if success.
 */
int start_resp_to_client(const message_db_t mess_to_send)
{
    #if DEBUG_TRACE
        printf("%d :- start_resp_to_client()\n", getpid());
    #endif

    sprintf(client_pipe_name, CLIENT_PIPE, mess_to_send.client_pid);
    if ((client_fd = open(client_pipe_name, O_WRONLY)) == -1) return(0);
    return(1);
}

/* Server side:
 *
 * Send a message_db_t struct over the write end of a client's fifo
 * Before you call this you already need to have called start_resp_to_client
 * with the message, which ensures tha the (global) file descriptor is set
 * to the correct fifo.
 *
 * Return 1 for success, 0 for error (write failed, or the fd wasn't set)
 */
int send_resp_to_client(const message_db_t mess_to_send) {
    int write_bytes;

    #if DEBUG_TRACE
        printf("%d :- send_resp_to_client()\n", getpid());
    #endif

    if (client_fd == -1) return(0);
    write_bytes = write(client_fd, &mess_to_send, sizeof(mess_to_send));
    if (write_bytes != sizeof(mess_to_send)) return(0);
    return(1);
}

/* Server side
 *
 * Close the client fifo fd. We do this after sending every message.  */
void end_resp_to_client(void) {
    #if DEBUG_TRACE
        printf("%d :- end_resp_to_client()\n",  getpid());
    #endif

    if (client_fd != -1) {
        (void)close(client_fd);
        client_fd = -1;
    }
}


/* Client side
 *
 * Start up a client. Open the write end of the server fifo, and create
 * (but don't open) the client fifo (which is named via the pid).
 * 
 * Return 0 if there are any errors, else 1. */
int client_starting(void)
{
    #if DEBUG_TRACE
        printf("%d :- client_starting\n",  getpid());
    #endif

    mypid = getpid();
    if ((server_fd = open(SERVER_PIPE, O_WRONLY)) == -1) {
        fprintf(stderr, "Server not running\n");
        return(0);
    }

    (void)sprintf(client_pipe_name, CLIENT_PIPE, mypid);
    (void)unlink(client_pipe_name);
    if (mkfifo(client_pipe_name, 0777) == -1) {
        fprintf(stderr, "Unable to create client pipe %s\n", 
                   client_pipe_name);
        return(0);
    }
    return(1);
}

/* Client side
 *
 * close our write end of the server fd, close the client fd (both ends,
 * if they are open), and unlinke the now unneeded client fd */
void client_ending(void) {
    #if DEBUG_TRACE    
        printf("%d :- client_ending()\n",  getpid()); 
    #endif

    if (client_write_fd != -1) close(client_write_fd);
    if (client_fd != -1) close(client_fd);
    if (server_fd != -1) close(server_fd);
    unlink(client_pipe_name);
}

/* Client side
 *
 * Send a message to the server through the write end of the server fifo.
 *
 * Since we have a struct smaller than the buffer side, this is pretty
 * straightforward, and the server will always read the whole message at once.
 *
 * Multiple clients can send data at the same time, because a single write
 * call is atomic as long as it's no larger than the buffer size.
 */
int send_mess_to_server(message_db_t mess_to_send) {
    int write_bytes;

    #if DEBUG_TRACE    
        printf("%d :- send_mess_to_server()\n",  getpid());
    #endif

    if (server_fd == -1) return(0);
    mess_to_send.client_pid = mypid;
    write_bytes = write(server_fd, &mess_to_send, sizeof(mess_to_send));
    if (write_bytes != sizeof(mess_to_send)) return(0);
    return(1);
}

/* client side:
 *
 * open up the client fifo for reading. We do this fresh after every message.
 * don't actually read, the `read_resp_from_server` does that.
 *
 * It also opens a write-only file descriptor also. The reason it does that
 * is because the server closes the write end after every request. This can
 * caluse a dropped request in race situations where the client might open
 * the read side of its fifo and then try to read data *before* the server
 * has had a chance to open the write side.
 *
 * By ensuring a write side is open before we try to read, we ensure that the
 * read operation will block, rather than returning 0 (which happens when
 * there are no write fd's open on the fifo) in this situation. This is
 * the desired behavior.
 *
 * Returns 0*/
int start_resp_from_server(void) {
    #if DEBUG_TRACE    
        printf("%d :- start_resp_from_server()\n",  getpid());    
    #endif

    if (client_pipe_name[0] == '\0') return(0);
    if (client_fd != -1) return(1);

    client_fd = open(client_pipe_name, O_RDONLY);
    if (client_fd != -1) {
        client_write_fd = open(client_pipe_name, O_WRONLY);
        if (client_write_fd != -1) return(1);
        close(client_fd);
        client_fd = -1;
    }
    return(0);
}


/* client side:
 *
 * read a server response from the pipe, and dump the data in *rec_ptr */
int read_resp_from_server(message_db_t *rec_ptr) {
    int read_bytes;
    int return_code = 0;

    #if DEBUG_TRACE    
        printf("%d :- read_resp_from_server()\n",  getpid());    
    #endif

    if (!rec_ptr) return(0);
    if (client_fd == -1) return(0);

    read_bytes = read(client_fd, rec_ptr, sizeof(*rec_ptr));
    if (read_bytes == sizeof(*rec_ptr)) return_code = 1;
    return(return_code);
}

/* Client side:
 *
 * Function to call when the server response has been read. It's a no-op in
 * this impl (but if we used sockets, for example, we might need it) */
void end_resp_from_server(void) {
    #if DEBUG_TRACE    
        printf("%d :- end_resp_from_server()\n",  getpid());        
    #endif
}



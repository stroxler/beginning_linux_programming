/* First, we include the appropriate headers, declare some message queue keys and define#
 a structure to hold our message data. */

#include "cd_data.h"
#include "cliserv.h"

#include <sys/msg.h>

#define SERVER_MQUEUE 1234
#define CLIENT_MQUEUE 4321

/* implement the same api as we did for fifo.
 *
 * note that the message queue, which is intended for this style of communication,
 * leads to a much simpler implementation. We don't even need big parts of the
 * api!
 */

/* As discussed in the mq example code, messages should always start with a
 * long, which can be considered the message type or a key. The msgrcv
 * function looks for messages with a particular key.
 *
 * This allows us to use a single queue for all clients, by using the pid
 * as the key!
 */
struct msg_passed {
    long int msg_key; /* used for client pid */
    message_db_t real_message;
};

/* Two variables with file scope hold the two queue identifiers returned from the
 msgget function. */

static int serv_qid = -1;
static int cli_qid = -1;


/* server side:
 *
 * In this impl, the server is responsible for creating both of the queues.
 */
int server_starting() {
    #if DEBUG_TRACE
        printf("%d :- server_starting()\n",  getpid());
    #endif

    serv_qid = msgget((key_t)SERVER_MQUEUE, 0666 | IPC_CREAT);
    if (serv_qid == -1) return(0);

    cli_qid = msgget((key_t)CLIENT_MQUEUE, 0666 | IPC_CREAT);
    if (cli_qid == -1) return(0);

    return(1);
}


/* server side:
 *
 * Also the server is responsible for removing both of the queues.
 */
void server_ending() {
    #if DEBUG_TRACE
        printf("%d :- server_ending()\n",  getpid());
    #endif

    (void)msgctl(serv_qid, IPC_RMID, 0);
    (void)msgctl(cli_qid, IPC_RMID, 0);

    serv_qid = -1;
    cli_qid = -1;
}


/* server side:
 *
 * Reading a client request looks a lot like a file read, via msgrcv.
 *
 * Note that we use 0 in the third arg, which is where you set the message
 * type. This is very important, because it means we read the
 * first message from any client (0 is a wildcard).
 */
int read_request_from_client(message_db_t *rec_ptr)
{
    struct msg_passed my_msg;
    #if DEBUG_TRACE
        printf("%d :- read_request_from_client()\n",  getpid());
    #endif

    if (msgrcv(serv_qid, (void *)&my_msg, sizeof(*rec_ptr), 0, 0) == -1) {
        return(0);
    }
    *rec_ptr = my_msg.real_message;
    return(1);
}


/* server side:
 *
 * sending a response requires packaging our message_db_t struct inside
 * a msg_passed struct, because we need to add the pid to the key. But once
 * we've done that, msgsend looks a lot like a file write.
 */
int send_resp_to_client(const message_db_t mess_to_send) {
    struct msg_passed my_msg;
    #if DEBUG_TRACE
        printf("%d :- send_resp_to_client()\n",  getpid());
    #endif

    my_msg.real_message = mess_to_send;
    my_msg.msg_key = mess_to_send.client_pid;

    if (msgsnd(cli_qid, (void *)&my_msg, sizeof(mess_to_send), 0) == -1) {
        return(0);
    }
    return(1);
}


/* client side:
 *
 * The client opens the queues, but does not create them.
 */
int client_starting() {
    #if DEBUG_TRACE
        printf("%d :- client_starting\n",  getpid());
    #endif

    serv_qid = msgget((key_t)SERVER_MQUEUE, 0666);
    if (serv_qid == -1) return(0);

    cli_qid = msgget((key_t)CLIENT_MQUEUE, 0666);
    if (cli_qid == -1) return(0);
    return(1);
}



/* client side:
 *
 * the client doesn't need to remove anything: all it does is set its qids to
 * -1 to end a connection
 */
void client_ending() {
    #if DEBUG_TRACE
        printf("%d :- client_ending()\n",  getpid());
    #endif

    serv_qid = -1;
    cli_qid = -1;
}

/* client side:
 *
 * sending a message to the server is simple, it looks very much like a
 * file write. As with server sending, we package the pid into a struct that
 * wraps the message_db_t struct.
 *
 * The value of the key actually isn't important when sending to the server.
 * The only reason we set it is because the api doesn't allow keys of 0, so
 * it isn't safe to not initialize it.
 */
int send_mess_to_server(message_db_t mess_to_send) {
    struct msg_passed my_msg;
    #if DEBUG_TRACE
        printf("%d :- send_mess_to_server()\n",  getpid());
    #endif

    my_msg.real_message = mess_to_send;
    my_msg.msg_key = mess_to_send.client_pid;

    if (msgsnd(serv_qid, (void *)&my_msg, sizeof(mess_to_send), 0) == -1) {
        perror("Message send failed");
        return(0);
    }
    return(1);
}


/* client side:
 *
 * read a message from the client queue. Note that the third argument is the
 * pid, which means we only pick up messages meant for us.
 */
int read_resp_from_server(message_db_t *rec_ptr) {
    struct msg_passed my_msg;
    #if DEBUG_TRACE
        printf("%d :- read_resp_from_server()\n",  getpid());
    #endif

    if (msgrcv(cli_qid, (void *)&my_msg, sizeof(*rec_ptr), getpid(), 0) == -1) {
        return(0);
    }
    *rec_ptr = my_msg.real_message;
    return(1);
}

/* client side:
 *
 * this isn't needed, but it lets us use the same api as for fifo */
int start_resp_to_client(const message_db_t mess_to_send)
{
    return(1);
}

/* client side:
 *
 * this isn't needed, but it lets us use the same api as for fifo */
void end_resp_to_client(void)
{
}

/* client side:
 *
 * this isn't needed, but it lets us use the same api as for fifo */
int start_resp_from_server(void)
{
    return(1);
}

/* client side:
 *
 * this isn't needed, but it lets us use the same api as for fifo */
void end_resp_from_server(void)
{
}


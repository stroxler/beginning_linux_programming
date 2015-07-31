#include "protocol.h"
#include <ctype.h>

/* the server side of our client-server program. See inside the
 * program for notes on how it works. */

int main() {
    int server_fifo_fd, client_fifo_fd;
    struct data_to_pass_st my_data;
    int read_res;
    char client_fifo_path[256];
    char *tmp_char_ptr;

    // make the server fifo (clients write, server reads), then open it
    mkfifo(SERVER_FIFO_NAME, 0777);
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (server_fifo_fd == -1) {
        fprintf(stderr, "Server fifo failure\n");
        exit(EXIT_FAILURE);
    }

    sleep(10); /* lets clients queue for demo purposes */

    // repeatedly
    //   - reads in chunks of bytes into a
    // `   data_to_pass_st` struct
    //   - looks inside those bytes and, using a pointer to the
    //     message part of the struct, convert the message to
    //     uppercase
    //   - looks inside those bytes and reads the client's pid
    //   - uses the pid to set the client_fifo_path, which is a
    //     string template for the location of a fifo to send
    //     data back to the client. It's assumed that the client
    //     would make it's own fifo, so we don't call mkfifo()
    //   - open the client fifo for writing, using the path that
    //     we just filled out
    //   - write the data back over the client fifo
    //   - close the client fifo fd (we have many clients, and we are
    //     opening new fifo fds all the time, so we can't leave it open
    //     or we might run out of fds.
    do {
        // read bytes into the struct
        read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
        if (read_res > 0) {

            // convert the data to uppercase (in-place) in the struct
            // we just read in
            tmp_char_ptr = my_data.some_data;
            while (*tmp_char_ptr) {
                *tmp_char_ptr = toupper(*tmp_char_ptr);
                tmp_char_ptr++;
            }

            // get the client fifo path from our template and the pid,
            // and open the fifo
            sprintf(client_fifo_path, CLIENT_FIFO_NAME, my_data.client_pid);
            client_fifo_fd = open(client_fifo_path, O_WRONLY);

            // write the struct (which now has an uppercased
            // message) back to the fifo
            if (client_fifo_fd != -1) {
                write(client_fifo_fd, &my_data, sizeof(my_data));
                close(client_fifo_fd);
            }
        }
    } while (read_res > 0);

    // when we are done, close the fifo and also unlink it
    close(server_fifo_fd);
    unlink(SERVER_FIFO_NAME);
    exit(EXIT_SUCCESS);
}


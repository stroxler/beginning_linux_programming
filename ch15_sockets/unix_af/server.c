#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

/* Simple example of a server over a unix domain socket.
 *
 * Most of my discussion is in client.c or in the in-code comments.
 */

int main()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;

    // open the server socket - this is only used to create connections,
    // the actual data transfer happens on a different socket, as we see
    // in the while loop
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    // create the address of the socket
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, "/tmp/server.sock");
    server_len = sizeof(server_address);

    // unlink any existing socket first, then bind to the address
    //    ... this is important. If you forget to do it then it will start
    //        generating a ton of empty connections (it will loop as fast as
    //        it can, printing "server waiting...", which means that the
    //        accept() isn't blocking). I'm not sure what's going on under
    //        the hood here, but clearly it's important that the bind operation
    //        creates a brand-new file
    unlink("/tmp/server.sock");
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    // prep some vars to help us listen for connections
    client_len = sizeof(client_address);
    char ch;

    // start listening. Every connection created in accept() - which will
    // block waiting for connections - creates its own file descriptor.
    //
    // the connect() call in a client corresponds to a return of accept()
    // on the server side.
    listen(server_sockfd, 5);
    while(1) {
        printf("server waiting\n");

        // note that we pass the client address to accept(), which will
        // set it and thereby let us use the client address in handling the
        // connection if desired. Note that we cast to sockaddr * and say
        // the size, much like the connect() function used client-side.
        client_sockfd = accept(server_sockfd, 
                              (struct sockaddr *)&client_address,
                              &client_len);

        // at this point we read and write from the client like we would a
        // file or fifo.
        read(client_sockfd, &ch, 1);
        ch++;
        write(client_sockfd, &ch, 1);

        // don't forget to close it! if you forget this in a real server, you
        // will run out of file descriptors
        close(client_sockfd);
    }
}

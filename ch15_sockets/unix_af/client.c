#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

/* Simple example of a client program, communicating via unix
 * domain socket.
 *
 * The interface between a unix socket and an inet socket is identical, the
 * only difference is in the address. By creating a sockaddr_un struct for
 * the address and setting AF_UNIX in the sun_family, we specify that the
 * socket is a unix domain socket
 *   (why do we need the sun_family if the struct is already specific? Because
 *    we pass the address to functions that take a more general sockaddr,
 *    which is bigger, so the first few bytes need to specify what kind it
 *    is... this is basically C-style inheritance)
 *
 * When we create the sockfd with the call to socket, we likewise specify
 * that the domain is an AF_UNIX socket. We also specify that it's a stream,
 * which over inet means tcp: a two way file-like reliable byte stream. We
 * interact with the file descriptor just as we would with a named pipe.
 *
 * In order to actually hook up our socket file descriptor, we call connect,
 * which give it the address. On the server side, this shows up in a queue
 * under the named socket the server is listening to (and once the server
 * accepts, a new unnamed socket will be created to handle this connection...
 * the named socket is *only* used to detect new ones.
 *
 * We close the socket file descriptor when we are finished, just as we would
 * any other file descriptor.
 */

int main() {
    int sockfd;
    int len;
    struct sockaddr_un address;
    int result;
    char ch = 'A';

    // create the file descriptor. We need to specify the domain and the
    // type (STREAM or DGRAM), but not the address.
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    // create the address and connect
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "/tmp/server.sock");
    len = sizeof(address);
    // note that we cast our address to a general sockaddr *, and we
    // tell connect() it's size.
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: client1");
        exit(1);
    }

    // at this point, we can treat it like any other file descriptor.
    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);
    close(sockfd);
    exit(0);
}


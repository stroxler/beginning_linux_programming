#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Example of a server over an inet socket which forks itself to handle
 * each client connection.
 *
 * A more efficient approach would be to spawn a thread that deals with each
 * connection, which is what most web frameworks do. Forking is simpler if
 * the service is stateless and overhead isn't a concern, so that's what we
 * do in this example.
 *
 * In a later example, we will see how using the `select` utility to listen
 * on many file descriptors, it's also possible for a single-threaded
 * server to handle simultaneous connections.
 */

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);

    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    client_len = sizeof(client_address);
    char ch;

    listen(server_sockfd, 5);
    while(1) {
        printf("server waiting\n");

        client_sockfd = accept(server_sockfd, 
                              (struct sockaddr *)&client_address,
                              &client_len);

        // after accepting, fork and deal with the new connection in the
        // child. Don't forget that both the child *and* the main process
        // need to close the new client_sockfd!
        if (fork() == 0) {
            read(client_sockfd, &ch, 1);
            ch++;
            write(client_sockfd, &ch, 1);
            close(client_sockfd);
        } else {
            close(client_sockfd);
        }
    }
}

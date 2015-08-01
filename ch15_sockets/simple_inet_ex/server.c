#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Simple example of a server over an inet scocket.
 *
 * I discussed all of the meaningful changes already in the client. Note
 * that up to changing the unlink(), and modifying the address code, everything
 * else is identical to the unix_af variant - the socket semantics are
 * the same.
 *
 * Using a SOCK_STREAM type socket over inet means using TCP for transmission,
 * which gives a reliable two-way byte stream.
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

        read(client_sockfd, &ch, 1);
        ch++;
        write(client_sockfd, &ch, 1);

        close(client_sockfd);
    }
}

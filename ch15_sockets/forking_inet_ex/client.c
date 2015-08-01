#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Simple example of a client program, communicating via inet.
 *
 * This client is identical to the one from simple_inet_ex; only the
 * server has changed.
 */

int main() {
    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch = 'A';

    // create the file descriptor. We need to specify the domain and the
    // type (STREAM or DGRAM), but not the address.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // create the address and connect
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    len = sizeof(address);
    // note that the cast hasn't changed.. in fact this line is identical
    // to the unix_af variant, as is everything below.
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: client1");
        exit(1);
    }

    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);
    close(sockfd);
    exit(0);
}


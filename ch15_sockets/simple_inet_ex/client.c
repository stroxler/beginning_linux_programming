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
 * Most of this example is lifted straight from the unix variant. Only the
 * handling of the address differs. Note that we use a completely different
 * struct for the address, but we do still cast to the generic
 * sockaddr type when calling connect().
 *
 * Also note that the header files changed.
 *
 * Note one other thing: we have to call the htons function
 * (read host to network short) port to convert the port from the host's
 * byte ordering (little- or big-endian) to whichever ordering the network
 * is using. The operating system handles this for us, be we have to call
 * it's function.
 *
 * The inet_addr() function is a handy utility that converts from human
 * readable sting ips to integers.
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


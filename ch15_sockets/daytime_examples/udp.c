#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* This program does the same thing as tcp.c, but using the SOCK_DGRAM
 * type of socket, which in the inet domain means using udp.
 *
 * Note the changes:
 *   1. we specify udp in the getservbyname. the host is the same.
 *   2. when we call socket(), we specify SOCK_DGRAM
 *   3. there's no connect() step. We send and recieve messges with the
 *      actual server socket (not a deticated two-way socket as with tcp),
 *      using the `sendto` and `recvfrom` functions.
 */

int main(int argc, char *argv[]) {
    char *host;
    int sockfd;
    int len, result;
    struct sockaddr_in address;
    struct hostent *hostinfo;
    struct servent *servinfo;
    char buffer[128];

    // the hosst is either localhost or anything specified in arg 1
    host = (argc == 1) ? "localhost" : argv[1];

    // look up the host info
    hostinfo = gethostbyname(host);
    if(!hostinfo) {
        fprintf(stderr, "no host: %s\n", host);
        exit(1);
    }

    // look up the service info. Note that we specify the udp version!
    servinfo = getservbyname("daytime", "udp");
    if(!servinfo) {
        fprintf(stderr,"no daytime service\n");
        exit(1);
    }
    printf("daytime port is %d\n", ntohs(servinfo -> s_port));

    // construct the address. Note that (other than s_port) it is the same.
    address.sin_family = AF_INET;
    address.sin_port = servinfo -> s_port;
    address.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;
    len = sizeof(address);

    // make the socket, then
    // get the information. Note there's no connect, hence to 'ask' for the
    // data we have to just send a byte.
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    result = sendto(sockfd, buffer, 1, 0, (struct sockaddr *)&address, len);
    result = recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                      (struct sockaddr *)&address, &len);
    // the `result` is the number of bytes received, so null out the next
    // one (apparently udp doesn't do it for us, which makes sense b/c for
    // binary data we wouldn't care) and then print.
    buffer[result] = '\0';
    printf("read %d bytes: %s", result, buffer);

    close(sockfd);
    exit(0);
}

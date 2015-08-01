#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* This program demonstrates writing a client for a built-in networking
 * service, the "daytime" service that tells us what time a computer thinks
 * it is.
 *
 * In the process, we demonstrate using `struct hostent` and the
 * gethostbyname function to look up information from /etc/hosts, and
 * similarly the `struct servent` and the getservbyname to look up
 * services from /etc/services.
 *
 * In this demo, we use tcp as we do for nearly all of our other examples in
 * this chapter. The other example uses udp, which makes use of SOCK_DGRAM
 * instead of SOCK_STREAM.
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

    // look up the service info. Note that we specify the tcp version!
    servinfo = getservbyname("daytime", "tcp");
    if(!servinfo) {
        fprintf(stderr,"no daytime service\n");
        exit(1);
    }
    printf("daytime port is %d\n", ntohs(servinfo -> s_port));

    // construct the address
    address.sin_family = AF_INET;
    address.sin_port = servinfo -> s_port;
    address.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;
    len = sizeof(address);

    // create the socket and then connect
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: getdate");
        exit(1);
    }

    // get the info!
    result = read(sockfd, buffer, sizeof(buffer));
    buffer[result] = '\0';
    printf("read %d bytes: %s", result, buffer);

    close(sockfd);
    exit(0);
}

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

/* Example of a server over an inet socket which uses select() to wait for
 * contact on multiple sockets in a single thread.
 *
 * The approach we are using - using an fd_set struct - allows waiting for
 * the first contact on a variety of file descriptors, and would work for
 * listening on multiple input devices of any type (not just sockets).
 *
 * None of the socket code here is new, so our comments focus on the
 * fd_set api.
 */

int main() {
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    // I actually haven't found a satisfactory explanation of why we need
    // testfds - which we always set by value to be equal to readfds prior
    // to calling `select` - but it seems to be a part of using the api.
    fd_set readfds, testfds;

    // handle setting up the server socket fd. Nothing new here.
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, 5);
    client_len = sizeof(client_address);
    char ch;

    // initialize the readfds fd_set to be empty
    FD_ZERO(&readfds);
    // then, add the server socket to readfds
    //    we'll add client fds as they form in the loop below
    FD_SET(server_sockfd, &readfds);

    int result;
    int fd;
    int nread;
    while(1) {

        printf("server waiting\n");
        testfds = readfds;

        // this returns -1 if any of the fds experiences an error, otherwise
        // it returns the number of fds that have experienced a result.
        //
        // .. the last input is a timeout. Setting NULL means no timeout.
        // the other two fd_sets are to check for writeablility or errors.
        // If we had set a timeout, we would need to check whether `result` is
        // 0, which would mean we timed out without any data to read.
        //
        // I don't fully undertand this api, but the most common use case
        // for network programming would be to check for reading, as we do
        // here.
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0,
                        (struct timeval *)0);
        if (result < 1) {
            perror("select server");
            exit(1);
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            // FD_ISSET tells us whether a particular fd is
            // (a) part of an fd_set, and (b) had seen 'action' as of the last
            // select call.
            if (FD_ISSET(fd, &testfds)) {
                // handle server socket action - accept new connections
                if (fd == server_sockfd) {
                    client_sockfd = accept(server_sockfd, 
                                          (struct sockaddr *)&client_address,
                                          &client_len);
                    FD_SET(client_sockfd, &readfds);
                    printf("Adding client on fd %d\n", client_sockfd);
                } else {
                    // this call sets nread to be the number of bytes in the
                    // socket's buffer (it also returns an error code, 0 for
                    // success, which we are ignoring here). It's basiclally
                    // a way of 'peeking' to find out how big a buffer we need
                    // to read.
                    //
                    // If we weren't hard-coding a limit on message size, we
                    // could then read the whole buffer by mallocing something
                    // big enough to hold everything.
                    ioctl(fd, FIONREAD, &nread);

                    if (nread == 0) {
                        // there's no data
                        // this can only happen when FD_ISSET returns 1 if the
                        // socket closed. So, we clean up
                        close(fd);
                        FD_CLR(fd, &readfds);
                        printf("Removing client on fd %d\n", fd);
                    } else {
                        // there's data! read it, and reply.
                        read(fd, &ch, 1);
                        ch++;
                        printf("Serving client on fd %d\n", fd);
                        write(fd, &ch, 1);
                    }
                }
            }
        }
    }
}

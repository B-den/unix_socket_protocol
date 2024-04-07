#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char socket_filename_other[] = "mysckt.server";
    char socket_filename[] = "mysckt.client";
    unlink(socket_filename);

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "socket\n");
        return 1;
    }

    struct sockaddr_un cl_addr;
    cl_addr.sun_family = AF_UNIX;
    memcpy(cl_addr.sun_path, socket_filename, sizeof(socket_filename));
    if (bind(sockfd, (const struct sockaddr*) &cl_addr, sizeof(cl_addr)) < 0) {
        perror("binding in sender");
        return 1;
    }

    struct sockaddr_un sc_addrr;
    sc_addrr.sun_family = AF_UNIX;
    memcpy(sc_addrr.sun_path,
           socket_filename_other,
           sizeof(socket_filename_other));

    char msg[] = "hi fella";
    sendto(sockfd,
           msg,
           sizeof(msg),
           0,
           (struct sockaddr*) &sc_addrr,
           sizeof(sc_addrr));

    char buff[1024];
    recvfrom(sockfd, buff, 1024, 0, (struct sockaddr*) &sc_addrr, NULL);
    printf("sender got confirmation: %s\n", buff);

    unlink(socket_filename);
    unlink(socket_filename_other);
    close(sockfd);

    return 0;
}

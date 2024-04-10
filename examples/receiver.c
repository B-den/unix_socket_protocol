#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
    char socket_filename[] = "mysckt.server";
    unlink(socket_filename);

    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "socket\n");
        return 1;
    }

    struct sockaddr_un sc_addrr, client_addr;
    sc_addrr.sun_family = AF_UNIX;
    memcpy(sc_addrr.sun_path, socket_filename, sizeof(socket_filename));
    if (bind(sockfd, (struct sockaddr*) &sc_addrr, sizeof(sc_addrr)) < 0) {
        perror("binding in receiver\n");
        return 1;
    }

    char buff[1024];
    unsigned int client_addr_len = sizeof(client_addr);
    int n = recvfrom(
            sockfd,
            buff,
            1024,
            0,
            (struct sockaddr*) &client_addr,
            &client_addr_len
    );
    buff[n] = 0;
    printf("serv received(%d): %s\n", n, buff);
    char msg[] = "hi received";
    sendto(sockfd,
           msg,
           sizeof(msg),
           0,
           (struct sockaddr*) &client_addr,
           client_addr_len);

    close(sockfd);
    return 0;
}

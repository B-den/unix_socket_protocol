#include "send.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
    const char receiving_sockname[] = "sock.receiving";
    const char sending_sockname[] = "sock.sending";

    const int data_len = 2049;
    char data[2049] = { 1 };

    int sending_scfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sending_scfd == -1) {
        fprintf(stderr, "socket\n");
        return 1;
    }

    struct sockaddr_un send_addr;
    send_addr.sun_family = AF_UNIX;
    memcpy(send_addr.sun_path, sending_sockname, sizeof(sending_sockname));
    unlink(sending_sockname);
    if (bind(sending_scfd, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0) {
        perror("send bind");
        return 1;
    }

    int res = send_data(sending_scfd, receiving_sockname, data, data_len, &send_addr);
    unlink(sending_sockname);
    close(sending_scfd);
    return res;
}

#include "receive.h"
#include "send.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
    const char receiving_sockname[] = "sock.receiving";
    const char sending_sockname[] = "sock.sending";

    int sockfd;

    const int data_len = 2049;
    char data[2049] = { 1 };

    int receiving_scfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un rcv_addr;
    rcv_addr.sun_family = AF_UNIX;
    memcpy(rcv_addr.sun_path, receiving_sockname, sizeof(receiving_sockname));
    unlink(receiving_sockname);
    if (bind(receiving_scfd, (struct sockaddr*) &rcv_addr, sizeof(rcv_addr)) < 0) {
        perror("receiver bind");
        return 1;
    }

    int sending_scfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un send_addr;
    send_addr.sun_family = AF_UNIX;
    memcpy(send_addr.sun_path, sending_sockname, sizeof(sending_sockname));
    unlink(sending_sockname);
    if (bind(sending_scfd, (struct sockaddr*) &send_addr, sizeof(send_addr)) < 0) {
        perror("send bind");
        return 1;
    }

    int pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        sleep(1);
        int res = send_data(sending_scfd, receiving_sockname, data, data_len, &send_addr);
        assert(0 == res);
        return 0;
    } else {
        size_t len;
        char* data_ptr = receive_data(&len, receiving_scfd, receiving_sockname, &rcv_addr);
        assert(data_len == len);
        for (int i = 0; i < data_len; i++) {
            assert(data[i] == data_ptr[i]);
        }
        free(data_ptr);
        return 0;
    }
    return 1;
}

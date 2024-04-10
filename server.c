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

    size_t len;
    char* data_ptr = receive_data(&len, receiving_scfd, receiving_sockname, &rcv_addr);
    assert(NULL != data_ptr);
    for (int i = 0; i < data_len; i++) {
        assert(data[i] == data_ptr[i]);
    }
    free(data_ptr);
    unlink(receiving_sockname);
    close(receiving_scfd);
    return 0;
}

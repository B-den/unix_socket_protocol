
#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "packet.h"

void fill_tmp_server_socket_addr(struct sockaddr_un* addr) {
    // todo (no)
    addr->sun_family = AF_UNIX;
    const char* tmp_name = "/tmp/usp_tmp_server_socket";
    memcpy(addr->sun_path, tmp_name, sizeof(tmp_name));
}

void* receive_data(int sockfd, struct sockaddr_un* addr /*NULL-able*/) {
    int server_socket, to_clear = 0, rc = 0;
    if (addr == NULL) {
        struct sockaddr_un a;
        addr = &a;
        fill_tmp_server_socket_addr(addr);
        server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
        to_clear = 1;
        unlink(addr->sun_path);
        if (bind(server_socket, (const struct sockaddr*) addr, sizeof(*addr))
            < 0) {
            perror("binding in sender");
            rc = 1;
            goto exit;
        }
    }

exit:
    if (to_clear) {
        close(server_socket);
        unlink(addr->sun_path);
    }
}

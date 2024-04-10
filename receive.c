
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "checksum.h"
#include "packet.h"

void fill_tmp_server_socket_addr(struct sockaddr_un* addr) {
    // todo (no)
    addr->sun_family = AF_UNIX;
    const char tmp_name[] = "/tmp/usp_tmp_server_socket";
    memcpy(addr->sun_path, tmp_name, sizeof(tmp_name));
}

void* receive_data(size_t* data_len, int sockfd, const char* target_filename, struct sockaddr_un* local_addr /*NULL-able*/) {
    char* ret_data = NULL;
    *data_len = 0;
    int server_socket, close_socket = 0, unlink_addr = 0, rc = 0;

    if (sockfd == 0) {
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        close_socket = 1;
    }
    if (local_addr == NULL) {
        struct sockaddr_un a;
        local_addr = &a;
        fill_tmp_server_socket_addr(local_addr);
        server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
        close_socket = 1;
        unlink(local_addr->sun_path);
        if (bind(server_socket, (const struct sockaddr*) local_addr, sizeof(*local_addr))
            < 0) {
            perror("binding in sender");
            rc = 1;
            goto exit;
        }
        unlink_addr = 1;
    }

    while (1) {
        result_t result;
        package_t received;
        struct sockaddr_un sender_addr;
        recvfrom(sockfd, &received, sizeof(package_t), 0, (struct sockaddr*) &sender_addr, NULL);
        if (received.datagram.checksum != checksum(0, received.datagram.data, received.datagram.len)) {
            result.error = 1;
            sendto(sockfd, &result, sizeof(result_t), 0, (struct sockaddr*) &sender_addr, sizeof(sender_addr));
            continue;
        }
        result.error = 0;
        sendto(sockfd, &result, sizeof(result_t), 0, (struct sockaddr*) &sender_addr, sizeof(sender_addr));

        ret_data = realloc(ret_data, *data_len + received.datagram.len);
        memcpy(ret_data + *data_len, received.datagram.data, received.datagram.len);
        *data_len += received.datagram.len;
        if (received.order_number == received.total_size) {
            break;
        }
    }

exit:
    if (close_socket) {
        close(sockfd);
    }
    if (unlink_addr) {
        unlink(local_addr->sun_path);
    }
    return ret_data;
}

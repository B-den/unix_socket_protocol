#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "packet.h"

#define RETRY_TIMES 5
#define RETRY_TIMEOUT 1
#define BLOCK_SIZE 1

void send_with_confirm(
        int sockfd, const package_t* package, const scinfo_t* sc_info
) {
    result_t result = { .error = 1 };
    while (result.error) {
        sendto(sockfd,
               package->datagram.data,
               package->datagram.size,
               0,
               sc_info->sc_addr,
               sc_info->sc_size);
        recvfrom(sockfd, &result, sizeof(result_t), 0, sc_info->sc_addr, NULL);
    }
}

int send_data(void* data, size_t len) {
    for (size_t n = 0; n < len; n += BLOCK_SIZE) {
        for (int i = 0; i < RETRY_TIMES; i++) {
        }
        // send data
    }
    return 0;
}

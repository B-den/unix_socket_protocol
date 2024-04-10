#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "checksum.h"
#include "packet.h"

struct send_arg {
    int error;
    int sockfd;
    package_t package;
    struct sockaddr_un* addr;
};

void fill_tmp_client_socket_addr(struct sockaddr_un* addr) {
    // todo (no)
    addr->sun_family = AF_UNIX;
    const char tmp_name[] = "/tmp/usp_tmp_client_socket";
    memcpy(addr->sun_path, tmp_name, sizeof(tmp_name));
}

int send_data(
        int sockfd, /*NULL-able*/
        const char* target_socket_filename,
        void* data,
        size_t len,
        struct sockaddr_un* local_addr /*NULL-able*/ // if not NULL => binded
) {
    if (strlen(target_socket_filename) < 1) {
        perror("invalid argument: target socket filename required");
        return 1;
    }

    struct send_arg s_arg;
    fd_set file_descriptor_set;
    struct timeval timeout = {0, RETRY_TIMEOUT}; // in microsecs

    int close_socket = 0, unlink_addr = 0, rc = 0;
    if (sockfd == 0) {
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        close_socket = 1;
    }
    if (local_addr == NULL || close_socket) {
        struct sockaddr_un a;
        local_addr = &a;
        fill_tmp_client_socket_addr(local_addr);
        unlink(local_addr->sun_path);
        if (bind(sockfd, (const struct sockaddr*) local_addr, sizeof(*local_addr)) < 0) {
            perror("binding in sender");
            rc = 1;
            goto exit;
        }
        unlink_addr = 1;
    }

    FD_ZERO(&file_descriptor_set);
    FD_SET(sockfd, &file_descriptor_set);

    s_arg.sockfd = sockfd;
    s_arg.addr = local_addr;
    s_arg.package.total_size = (unsigned int) (len % BLOCK_SIZE == 0 ? len / BLOCK_SIZE : len / BLOCK_SIZE + 1);

    for (size_t num = 0; num * BLOCK_SIZE < len; num++) {
        size_t actual_size = len - num * BLOCK_SIZE > BLOCK_SIZE
                ? BLOCK_SIZE
                : len - num * BLOCK_SIZE;
        memcpy(s_arg.package.datagram.data,
               (char*) data + num * BLOCK_SIZE,
               actual_size);
        s_arg.package.order_number = num;
        s_arg.package.datagram.checksum = checksum(0, s_arg.package.datagram.data, actual_size);
        s_arg.package.datagram.len = actual_size;

        int resend_counter = 0;
        s_arg.error = 1;  // unsuccess
        for (int i = 0; i < RETRY_TIMES && !s_arg.error; i++) {
            sendto(s_arg.sockfd,
                &s_arg.package,
                sizeof(package_t),
                0,
                (struct sockaddr*)s_arg.addr,
                sizeof(*s_arg.addr));

            timeout.tv_sec = 0; timeout.tv_usec = RETRY_TIMEOUT;
            int retval = select(sockfd, NULL, &file_descriptor_set, NULL, &timeout);
            if (retval == -1) {
                perror("select()");
            } else if (retval) {
                // printf("Data is available now.\n");
                /* FD_ISSET(0, &rfds) will be true. */
            } else {
                // printf("No data within five seconds.\n");
                continue;
            }

            recvfrom(s_arg.sockfd, &s_arg.error, sizeof(s_arg.error), 0, (struct sockaddr*)s_arg.addr, NULL);
            if (s_arg.error && resend_counter < RESEND_TIMES) {
                i--;
                resend_counter++;
            } else {
                break;
            }
        }
        if (s_arg.error) {
            rc = 1;
            goto exit;
        }
    }

exit:
    if (close_socket) {
        close(sockfd);
    }
    if (unlink_addr) {
        unlink(local_addr->sun_path);
    }
    return rc;
}

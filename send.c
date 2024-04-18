#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "checksum.h"
#include "packet.h"

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

    int error;
    package_t package;

    fd_set file_descriptor_set;
    struct timeval timeout = { 0, RETRY_TIMEOUT }; // in microsecs

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
    		if (close_socket) {
				close(sockfd);
			}
			if (unlink_addr) {
				unlink(local_addr->sun_path);
			}
			return rc;
        }
        unlink_addr = 1;
    }

    FD_ZERO(&file_descriptor_set);
    FD_SET(sockfd, &file_descriptor_set);

    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));
    memcpy(addr.sun_path, target_socket_filename, strlen(target_socket_filename) + 1);
    addr.sun_family = AF_UNIX;
    package.total_size = (unsigned int) (len % BLOCK_SIZE == 0 ? len / BLOCK_SIZE : len / BLOCK_SIZE + 1);

    for (size_t num = 0; num * BLOCK_SIZE < len; num++) {
        size_t actual_size = len - num * BLOCK_SIZE > BLOCK_SIZE
                ? BLOCK_SIZE
                : len - num * BLOCK_SIZE;
        memcpy(package.datagram.data,
               (char*) data + num * BLOCK_SIZE,
               actual_size);
        package.order_number = num + 1;
        package.datagram.checksum = checksum(0, package.datagram.data, actual_size);
        package.datagram.len = actual_size;

        int resend_counter = 0;
        error = 1;
        for (int i = 0; i < RETRY_TIMES && error; i++) {
            sendto(sockfd,
                   &package,
                   sizeof(package_t),
                   0,
                   (struct sockaddr*) &addr,
                   sizeof(addr));

            timeout.tv_sec = 0;
            timeout.tv_usec = RETRY_TIMEOUT;
            int retval = select(sockfd + 1, &file_descriptor_set, NULL, NULL, &timeout);
            if (retval == -1) {
                perror("select()");
            } else if (retval) {
                // printf("Data is available now.\n");
                /* FD_ISSET(0, &rfds) will be true. */
            } else {
                fprintf(stderr, "receive timeout expired\n");
                continue;
            }
            recvfrom(sockfd, &error, sizeof(error), 0, (struct sockaddr*) &addr, NULL);
            if (error && resend_counter < RESEND_TIMES) {
                i--;
                resend_counter++;
            } else {
                break;
            }
        }
        if (error) {
            perror("error == 1");
            rc = 1;
    		if (close_socket) {
				close(sockfd);
			}
			if (unlink_addr) {
				unlink(local_addr->sun_path);
			}
			return rc;
        }
    }

    if (close_socket) {
        close(sockfd);
    }
    if (unlink_addr) {
        unlink(local_addr->sun_path);
    }
    return rc;
}

#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "checksum.h"
#include "packet.h"

struct send_arg {
    pthread_t* tid_to_kill;
    int success;
    int sockfd;
    package_t package;
    struct sockaddr_un* addr;
};

void* send_with_confirm(
        /*int sockfd, const package_t* package, const scinfo_t* sc_info*/
        void* _arg
) {
    struct send_arg* arg = (struct send_data*) _arg;

    result_t result = { .error = 1 };
    sendto(arg->sockfd,
           &arg->package,
           sizeof(package_t),
           0,
           arg->addr,
           sizeof(arg->addr));
    recvfrom(arg->sockfd, &result, sizeof(result_t), 0, arg->addr, NULL);

    arg->success = !result.error;
    pthread_cancel(*arg->tid_to_kill);

    return NULL;
}

struct timeout_arg {
    pthread_t* tid_to_kill;
    unsigned int nanosecs;
};

void* timeout(void* _arg) {
    struct timeout_arg* arg = (struct timeout_arg*) _arg;
    struct timespec remaining, request = { 0, arg->nanosecs };
    nanosleep(&request, &remaining);
    pthread_cancel(*arg->tid_to_kill);
    return NULL;
}

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
    struct timeout_arg t_arg;
    pthread_t s_tid, t_tid;
    pthread_attr_t s_attr, t_attr;

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
        if (bind(sockfd, (const struct sockaddr*) local_addr, sizeof(*local_addr))
            < 0) {
            perror("binding in sender");
            rc = 1;
            goto exit;
        }
        unlink_addr = 1;
    }

    s_arg.sockfd = sockfd;
    s_arg.addr = local_addr;
    s_arg.tid_to_kill = &t_tid;
    s_arg.package.total_size = (unsigned int) (len % BLOCK_SIZE == 0 ? len / BLOCK_SIZE : len / BLOCK_SIZE + 1);

    t_arg.nanosecs = RETRY_TIMEOUT;
    t_arg.tid_to_kill = &s_tid;

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
        s_arg.success = 0;
        for (int i = 0; i < RETRY_TIMES && !s_arg.success; i++) {
            s_arg.success = 0;
            pthread_create(&s_tid, &s_attr, send_with_confirm, &s_arg);
            pthread_create(&t_tid, &t_attr, timeout, &t_arg);
            pthread_join(s_tid, NULL);
            if (!s_arg.success && resend_counter < RESEND_TIMES) {
                i--;
            }
        }
        if (!s_arg.success) {
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

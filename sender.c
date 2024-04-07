#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#include "packet.h"

#define RETRY_TIMES 5
#define RESEND_TIMES 3
#define RETRY_TIMEOUT 500 // nanosecs
#define BLOCK_SIZE 1024

struct send_arg {
    pthread_t tid_to_kill;
    int success;
    int sockfd;
    const package_t* package;
    const struct sockaddr_un* addr;
};

void* send_with_confirm(
        /*int sockfd, const package_t* package, const scinfo_t* sc_info*/ void* _arg
) {
    struct send_arg* arg = (struct send_data*)_arg;

    result_t result = { .error = 1 };
    while (result.error) {
        sendto(arg->sockfd,
               arg->package->datagram.data,
               arg->package->datagram.size,
               0,
               arg->addr,
               sizeof(*arg->addr));
        recvfrom(arg->sockfd, &result, sizeof(result_t), 0, arg->addr, NULL);
    }

    pthread_cancel(arg->tid_to_kill);
    return NULL;
}

struct timeout_arg {
    pthread_t tid_to_kill;
    unsigned int nanosecs;
};

void* timeout(void* _arg) {
    struct timeout_arg* arg = (struct timeout_arg*)_arg;
    struct timespec remaining, request = {0, arg->nanosecs};
    nanosleep(&request, &remaining);
    pthread_cancel(arg->tid_to_kill);
    return NULL;
}

int send_data(void* data, size_t len) {
    struct send_arg s_arg;
    struct timeout_arg t_arg;
    pthread_t s_tid, t_tid;
    pthread_attr_t s_attr, t_attr;
    for (size_t offset = 0; offset < len; offset += BLOCK_SIZE) {
        size_t actual_size = len - offset > BLOCK_SIZE? BLOCK_SIZE: len - offset;
        // todo
        int resend_counter = 0;
        for (int i = 0; i < RETRY_TIMES; i++) {
            s_arg.success = 0;
            pthread_create(&s_tid, &s_attr, send_with_confirm, &s_arg);
            pthread_create(&t_tid, &t_attr, timeout, &t_arg);
            pthread_join(s_tid, NULL);
            if (!s_arg.success && resend_counter < RESEND_TIMES) {
                i--;
            }
        }
        if (!s_arg.success) {
            return 1;
        }
    }
    return 0;
}

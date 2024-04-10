#ifndef __SEND_H__
#define __SEND_H__

#include <stddef.h>
#include <sys/un.h>

int send_data(
        int sockfd, /*NULL-able*/
        const char* target_socket_filename,
        void* data,
        size_t len,
        struct sockaddr_un* local_addr /*NULL-able*/ // if not NULL => binded
);

#endif //__SEND_H__

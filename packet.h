#ifndef __PACKET_H__
#define __PACKET_H__

#include <stddef.h>

typedef struct datagram {
    long int checksum;
    size_t size;
    void* data;
} datagram_t;

typedef struct package {
    unsigned int order_number;
    unsigned int total_size; // in datagrams, not bytes
    datagram_t datagram;
} package_t;

typedef struct socket_info {
    struct sockaddr* sc_addr;
    size_t sc_size;
} scinfo_t;

typedef struct result {
    int error;
} result_t;

#endif // __PACKET_H__

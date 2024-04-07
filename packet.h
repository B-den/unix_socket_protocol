#ifndef __PACKET_H__
#define __PACKET_H__

#define BUF_LEN 1024

#include <stddef.h>

typedef struct datagram {
    long int checksum;
    char data[BUF_LEN];
} datagram_t;

typedef struct package {
    unsigned int order_number;
    unsigned int total_size; // in datagrams, not bytes
    datagram_t datagram;
} package_t;

typedef struct result {
    int error;
} result_t;

#endif // __PACKET_H__

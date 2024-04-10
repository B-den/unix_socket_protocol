#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

#define BUF_LEN 1024

#define RETRY_TIMES 5
#define RESEND_TIMES 3
#define RETRY_TIMEOUT 100000 // microsecs     // yet to experiment; context switch takes ~1500ns
#define BLOCK_SIZE 1024

#include <stddef.h>

typedef struct datagram {
    uint64_t checksum;
    size_t len;
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

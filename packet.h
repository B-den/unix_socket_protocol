#pragma once

#define PACKET_BUFFER_SIZE 1024

struct packet {
    unsigned int order_number;
    unsigned int total_size; // in datagrams, not bytes
    char buffer[PACKET_BUFFER_SIZE];
};
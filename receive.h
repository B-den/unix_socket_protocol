#ifndef __RECEIVE_H__
#define __RECEIVE_H__

#include <stddef.h>
#include <sys/un.h>

void* receive_data(size_t* data_len, int sockfd /*NULL-able*/, const char* target_filename, struct sockaddr_un* local_addr /*NULL-able*/);

#endif // __RECEIVE_H__

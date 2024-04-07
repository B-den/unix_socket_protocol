#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]) {
    char socket_filename_other[] = "mysckt.server";
    char socket_filename[] = "mysckt.client";
    unlink(socket_filename);

    int sc = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sc == -1) {
        fprintf(stderr, "socket\n");
        return 1;
    }

    struct sockaddr_un cl_addr;
    cl_addr.sun_family = AF_UNIX;
    memcpy(cl_addr.sun_path, socket_filename, sizeof(socket_filename));
    if (bind(sc, (const struct sockaddr*)&cl_addr, sizeof(cl_addr)) < 0) {
        perror("binding in sender");
        return 1;
    }

    struct sockaddr_un sc_addrr;
    sc_addrr.sun_family = AF_UNIX;
    memcpy(sc_addrr.sun_path, socket_filename_other, sizeof(socket_filename_other));

    struct sockaddr_un to_get_response;

    char msg[] = "hi fella";
    sendto(sc, msg, sizeof(msg), 0, (struct sockaddr*)&sc_addrr, sizeof(sc_addrr));

    char buff[1024];
    recvfrom(sc, buff, 1024, 0, (struct sockaddr*)&sc_addrr, NULL);
    printf("sender got confirmation: %s\n", buff);

    unlink(socket_filename);
    unlink(socket_filename_other);
    close(sc);

    return 0;
}
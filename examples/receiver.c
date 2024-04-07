#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main() {
    char socket_filename[] = "mysckt.server";
    char socket_filename_other[] = "mysckt.client";
    unlink(socket_filename);

    int sc = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sc == -1) {
        fprintf(stderr, "socket\n");
        return 1;
    }

    struct sockaddr_un sc_addrr, client_addr;
    sc_addrr.sun_family = AF_UNIX;
    memcpy(sc_addrr.sun_path, socket_filename, sizeof(socket_filename));
    if (bind(sc, (struct sockaddr*)&sc_addrr, sizeof(sc_addrr)) < 0) {
        perror("binding in receiver\n");
        return 1;
    }

    char buff[1024];
    int client_addr_len = sizeof(client_addr);
    int n = recvfrom(sc, buff, 1024, 0, (struct sockaddr*)&client_addr, &client_addr_len);
    buff[n] = 0;
    printf("serv received(%d): %s\n", n, buff);
    char msg[] = "hi received";
    sendto(sc, msg, sizeof(msg), 0, (struct sockaddr*)&client_addr, client_addr_len);
    
    // close(sc);
    // unlink(socket_filename);
    return 0;


    // int sock, length, n;
    // socklen_t fromlen;
    // struct sockaddr_un server;
    // struct sockaddr_un from;
    // char buf[1024];

    // sock=socket(AF_UNIX, SOCK_DGRAM, 0);
    // if (sock < 0) perror("Opening socket");
    // length = sizeof(AF_UNIX) + sizeof(socket_filename) - 1;
    // bzero(&server,length);
    // server.sun_family = AF_UNIX;
    // memcpy(server.sun_path, socket_filename, sizeof(socket_filename)-1);
    // if (bind(sock,(struct sockaddr *)&server,length)<0) 
    //     perror("binding");
    // fromlen = sizeof(struct sockaddr);
    // n = recvfrom(sock,buf,1024,0, (struct sockaddr *)&from,&fromlen);
    // if (n < 0) perror("recvfrom");
    // write(1,"Received a datagram: ",21);
    // write(1,buf,n);
    // printf("\n%d, %s\n", from.sun_family, from.sun_path);
    // n = sendto(sock,"Got your message\n",17, 0,(struct sockaddr *)&from,fromlen);
    // if (n  < 0) perror("sendto");
    // return 0;
}
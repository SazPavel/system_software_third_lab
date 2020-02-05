#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int sock[3];

void SigintHandler(int sig)
{
    int i;
    for(i = 0; i < 3; i++)
        close(sock[i]);
    printf("Server close\n");
    exit(0);
}

int err_handler(int func, const char *errstr, int quantity)
{
    if(func < 0)
    {
        if(quantity > 0)
        {
            int i;
            for(i = 0; i < quantity; i++)
                close(sock[i]);
        }
        perror(errstr);
        exit(-1);
    }
    return func;
}

int main()
{
    struct sigaction sigint;
    sigint.sa_handler = SigintHandler;
    sigint.sa_flags = 0;
    sigemptyset(&sigint.sa_mask);
    sigaddset(&sigint.sa_mask, SIGINT);
    err_handler(sigaction(SIGINT, &sigint, 0), "sigaction", 0);
    
    int buf, i = 0, n = 100;
    int bytes_read, result, maxfd;
    struct sockaddr_in addr_udp[2];
    socklen_t size_udp[2];
    size_udp[0] = sizeof(addr_udp[0]);
    size_udp[1] = sizeof(addr_udp[1]);
    fd_set readfds;
    sock[0] = err_handler(socket(AF_INET, SOCK_DGRAM, 0), "socket", 0);
    sock[1] = err_handler(socket(AF_INET, SOCK_DGRAM, 0), "socket", 0);
    sock[2] = 0;

    addr_udp[0].sin_family = AF_INET;
    addr_udp[0].sin_port = htons(0);
    addr_udp[0].sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    err_handler(bind(sock[0], (struct sockaddr*)&addr_udp[0], sizeof(addr_udp[0])), "bind", 1);
    err_handler(getsockname(sock[0], (struct sockaddr*)&addr_udp[0], &size_udp[0]), "getsockname", 1);
    printf("UDP port 1: %d\n", ntohs(addr_udp[0].sin_port));

    addr_udp[1].sin_family = AF_INET;
    addr_udp[1].sin_port = htons(0);
    addr_udp[1].sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    err_handler(bind(sock[1], (struct sockaddr*)&addr_udp[1], sizeof(addr_udp[1])), "bind", 2);
    err_handler(getsockname(sock[1], (struct sockaddr*)&addr_udp[1], &size_udp[1]), "getsockname", 2);
    printf("UDP port 2: %d\n", ntohs(addr_udp[1].sin_port));
    
    printf("server ready\n");
    while(1)
    {
        maxfd = sock[0] > sock[1] ? sock[0] + 1: sock[1] + 1;
        FD_ZERO(&readfds);
        FD_SET(sock[0], &readfds);
        FD_SET(sock[1], &readfds);
        result = err_handler(select(maxfd, &readfds, NULL, NULL, NULL), "select", 2);
        if(FD_ISSET(sock[0], &readfds))
        {
            err_handler(recvfrom(sock[0], &buf, sizeof(int), 0, NULL, NULL), "recvfrom", 2);
            printf("UDP 1: %d\n", buf);
        }
        if(FD_ISSET(sock[1], &readfds))
        {
            err_handler(recvfrom(sock[1], &buf, sizeof(int), 0, NULL, NULL), "recvfrom", 2);
            printf("UDP 2: %d\n", buf);
        }
    }
}

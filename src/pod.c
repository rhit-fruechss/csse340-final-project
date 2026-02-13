#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "shared.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6767

typedef char msgid_t;

void process_msg(msgid_t msg) {

}

int main() {
    int sockfd;
    op_packet_t pkt;
    struct sockaddr_in pod_addr;

    puts("-- SuperPump 9000 --");
    puts("Enabling server...");

    // Refer to: https://users.cs.jmu.edu/bernstdh/web/common/lectures/summary_unix_udp.php
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("error: socket\n");
        exit(1);
    }
    
    memset(&pod_addr, 0, sizeof(pod_addr));
    pod_addr.sin_family = AF_INET;
    pod_addr.sin_port = htons(PORT);
    pod_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*) &pod_addr, sizeof(pod_addr)) < 0) {
        perror("error: bind\n");
        exit(1);
    }

    socklen_t pod_addrlen = sizeof(pod_addr);
    int sockname = getsockname(sockfd, (struct sockaddr*) &pod_addr, &pod_addrlen);
    printf("Listening for connections at %d...\n", pod_addr.sin_addr.s_addr);

    while (1) {
        recvfrom(sockfd, (void*)&pkt, 1024, 0, NULL, NULL);
        printf("Received message\n");
        printf("%s\n", pkt.data);
    }

    // Cleanup
    close(sockfd);
    return 0;

}

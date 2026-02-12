#include <stdio.h>
#include <sys/socket.h>
#include "structs.h"

typedef char msgid_t;

void process_msg(msgid_t msg) {

}

int main() {
    int sockfd;
    packet_t buf;
    struct sockaddr_in pod_addr, client_addr;

    puts("-- SuperPump 9000 --");
    puts("Enabling server...");

    // Refer to: https://users.cs.jmu.edu/bernstdh/web/common/lectures/summary_unix_udp.php
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("error: socket\n");
        exit(1);
    }
    
    // Bind socket (source: https://www.linuxhowtos.org/C_C++/socket.htm) 
    pod_addr.sin_family = AF_INET;
    pod_addr.sin_port = htons(6767);
    pod_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*) &pod_addr, sizeof(pod_addr)) < 0) {
        perror("error: bind\n");
        exit(1);
    }

    listen(sockfd, 5);

    puts("Listening for connections...");

    return 0;

}

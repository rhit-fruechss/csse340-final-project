#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "shared.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 6767
#define POD_ID 1

void process_msg(op_msg_t *msg, int *should_continue) {
    for (int i = 0; i < MAX_COMMANDS; i++) {
        op_cmd_t cmd = msg->cmds[i];
        switch (cmd) {
            case CMD_EMPTY:
                puts("-- END OF COMMANDS --");
                return;
            case CMD_SETUP_POD:
                puts("-- SETUP POD --");
                return;
            case CMD_SHUTDOWN:
                puts(" -- SHUTDOWN -- ");
                *should_continue = 0;
                return;
            case CMD_RESCHEDULE:
                puts(" -- RESCHEDULE -- ");
                break;
            case CMD_DEPOSIT_5U:
                puts(" -- DEPOSIT - 5U -- ");
                break;
            case CMD_DEPOSIT_45U:
                puts(" -- DEPOSIT - 45U -- ");
                break;
        }
    }
}

int main() {
    int sockfd;
    op_nonce_t nonce = OP_SHARED_NONCE_START;
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

    int should_continue = 1;
    while (should_continue) {
        size_t msg_len;
        op_pkttype_t msg_pkttype;
        int msg_podid;
        char initial_data[sizeof(op_packet_t)];
        op_nonce_t nonce_to_check;
        op_receive_message_header(sockfd, &msg_len, &msg_pkttype, &msg_podid, &nonce_to_check, initial_data);
        printf("Received packet.\n");

        // Preliminary checks
        if (msg_pkttype != PKTTYPE_PDM) {
            printf("Packet is not from PDM, ignoring...\n");
            continue;
        }
        else if (msg_podid != POD_ID) {
            printf("Packet is not meant for this pod, ignoring...\n");
            continue;
        }
        else if (nonce_to_check != nonce) {
            printf("Packet received invalid nonce (expected: %x, actual: %x)", nonce, nonce_to_check);
            continue;
        }
        puts("Awaiting message body...");

        op_msg_t msg;
        op_receive_message_body(sockfd, msg_len, &msg, POD_ID, initial_data, sizeof(op_packet_t));

        process_msg(&msg, &should_continue);

        nonce = op_next_nonce(nonce);
        printf("New nonce: %x\n", nonce);
    }

    // Cleanup
    close(sockfd);
    return 0;

}

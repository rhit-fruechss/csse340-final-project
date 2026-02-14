#include <netinet/in.h>
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
    struct sockaddr_in pdm_addr;

    puts("-- SuperPump 9000 --");
    puts("Enabling server...");

    // Refer to: https://users.cs.jmu.edu/bernstdh/web/common/lectures/summary_unix_udp.php
    op_makeinetaddr(INADDR_ANY, PORT, &pdm_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sockfd, (struct sockaddr*)&pdm_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("error: connect\n");
        return 0;
    }

    // Connect to the other socket
    printf("Connected. %d\n", getsockname(sockfd, (struct sockaddr*)&pdm_addr, NULL));

    int should_continue = 1;
    while (should_continue) {
        size_t msg_len;
        op_pkttype_t msg_pkttype;
        int msg_podid;
        char initial_data[sizeof(op_packet_t)];
        op_nonce_t nonce_to_check;
        op_receive_message_header(sockfd, &msg_len, &msg_pkttype, &msg_podid, &nonce_to_check, initial_data);
        printf("Received packet: %x, %x, %x.\n", msg_podid, msg_pkttype, nonce_to_check);

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
            printf("Packet received invalid nonce\n(expected: %x, actual: %x)\n", nonce, nonce_to_check);
            continue;
        }
        printf("Awaiting message body (expecting %lu bytes)...\n", msg_len);

        op_msg_t msg;
        op_receive_message_body(sockfd, msg_len, &msg, POD_ID, initial_data, PACKET_DATA_SIZE);

        process_msg(&msg, &should_continue);

        nonce = op_next_nonce(nonce);
        printf("New nonce: %x\n", nonce);
    }

    // Cleanup
    close(sockfd);
    return 0;

}

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

void process_msg(op_msg_t *msg, op_nonce_t nonce, int *should_continue, int *regen_nonce) {
    int ncmds = (msg->msg_len - sizeof(size_t)) / sizeof(op_cmdlet_t);
    for (int i = 0; i < ncmds; i++) {
        op_cmdlet_t cmdlet = msg->cmds[i];
        if (cmdlet.nonce != nonce) {
            perror("Invalid nonce given for command: ignoring...\n");
            *regen_nonce = 0;
            return;
        }

        op_cmd_t cmd = cmdlet.cmd;
        switch (cmd) {
            case CMD_SETUP_POD:
                puts("-- SETUP POD --");
                break;
            case CMD_SHUTDOWN:
                puts("-- SHUTDOWN -- ");
                *should_continue = 0;
                return;
            case CMD_RESCHEDULE:
                puts("-- RESCHEDULE -- ");
                break;
            case CMD_DEPOSIT_5U:
                puts("-- DEPOSIT - 5U -- ");
                break;
            case CMD_DEPOSIT_45U:
                puts("-- DEPOSIT - 45U -- ");
                break;
            default:
                printf("UNKNOWN: ");
                debug_hexdump((void*)&cmdlet, sizeof(op_cmdlet_t));
                break;
        }
    }
    printf("-- END OF MESSAGE -- \n");
}

int main() {
    int sockfd;
    op_nonce_t nonce = OP_SHARED_NONCE_START;
    struct sockaddr_in pdm_addr;

    puts("-- SuperPump 9000 --");
    puts("Enabling server...");

    // Refer to: https://www.geeksforgeeks.org/c/socket-programming-cc/
    op_makeinetaddr(INADDR_ANY, PORT, &pdm_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sockfd, (struct sockaddr*)&pdm_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("error: connect\n");
        return 0;
    }

    // Connect to the other socket
    printf("Connected.\n");

    int should_continue = 1;
    while (should_continue) {
        size_t msglen;
        op_packet_t packet;

        op_receive_message_header(sockfd, &packet, &msglen);
        printf("Received packet: %x, %x.\n", packet.podid, packet.pkttype);

        // Preliminary checks
        if (packet.pkttype != PKTTYPE_PDM) {
            printf("Packet is not from PDM, ignoring...\n");
            continue;
        }
        else if (packet.podid != POD_ID) {
            printf("Packet is not meant for this pod, ignoring...\n");
            continue;
        }
        printf("Awaiting message body (expecting %lu bytes)...\n", msglen);

        op_msg_t msg;
        op_receive_message_body(sockfd, msglen, &msg, POD_ID, packet.data, PACKET_DATA_SIZE);

        int regen_nonce = 1;
        process_msg(&msg, nonce, &should_continue, &regen_nonce);

        if (regen_nonce) {
            nonce = op_next_nonce(nonce);
            printf("New nonce: %x\n", nonce);
        }
    }

    // Cleanup
    close(sockfd);
    return 0;

}

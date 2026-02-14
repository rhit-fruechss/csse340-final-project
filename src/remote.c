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

op_cmd_t make_cmd(char input) {
    switch (input) {
        case 's':
            return CMD_SETUP_POD;
            break;
        case 'S':
            return CMD_RESCHEDULE;
        case '5':
            return CMD_DEPOSIT_5U;
            break;
        case '4':
            return CMD_DEPOSIT_45U;
            break;
        case 'q':
            return CMD_SHUTDOWN;
            break;
        default:
            return CMD_EMPTY;
    }
}

int main() {
    int sockfd, podsockfd;
    op_nonce_t nonce = OP_SHARED_NONCE_START;
    struct sockaddr_in pdr_addr, pod_addr;
    socklen_t pod_addrlen;

    puts("-- SuperPump Manager 9000 --");
    puts("Enabling server...");
    
    op_makeinetaddr(INADDR_ANY, PORT, &pdr_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(sockfd, (struct sockaddr*)&pdr_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("error: bind\n");
        exit(1);
    }

    listen(sockfd, 5);

    if ((podsockfd = accept(sockfd, (struct sockaddr*)&pod_addr, &pod_addrlen)) < 0) {
        perror("error: accept\n");
        exit(1);
    }

    puts("Connected.");

    int should_continue = 1;
    while (should_continue) {
        op_cmdlet_t cmdlets[MAX_COMMANDS];

        // Take commands from stdin
        int i = 0;
        char c;
        while ((c = getc(stdin)) != '\n') {
            cmdlets[i].nonce = nonce;
            cmdlets[i++].cmd = make_cmd(c);
        }

        op_send_commands(sockfd, podsockfd, PKTTYPE_PDM, 1, nonce, cmdlets, i);

        nonce = op_next_nonce(nonce);
        printf("New nonce: %x\n", nonce);
    }

    // Cleanup
    close(sockfd);
    close(podsockfd);
    return 0;

}

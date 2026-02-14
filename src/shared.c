#include "shared.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void op_send_commands(int srcsockfd, int destsockfd, op_pkttype_t src, int podid, op_nonce_t nonce, op_cmdlet_t *cmds, int ncmds) {
    op_packet_t received_packet;
    int msg_len = sizeof(op_msg_t);
    int true_msg_len = sizeof(size_t) + msg_len;
    op_msg_t msg;
    msg.msg_len = msg_len;

    int is_first = 1;
    for (int i = 0; i < true_msg_len; i += PACKET_DATA_SIZE) {
        printf("%d %d\n", true_msg_len, msg_len);
        int pktnum = i / PACKET_DATA_SIZE;
        int is_last = i + PACKET_DATA_SIZE <= true_msg_len;
        char *byte_ptr = ((char*)&msg) + i;
        op_send_packet(destsockfd, podid, is_first ? src : PKTTYPE_CON, (uint8_t)pktnum, byte_ptr, is_last ? (true_msg_len - pktnum) : PACKET_DATA_SIZE);
        is_first = 0;
        if (!is_last) {
            op_receive_packet(srcsockfd, &received_packet); // Should be ACK packet. We don't actually care about the contents.
        }
        usleep(500000); // By waiting 500ms, we make jamming much easier.
    }
}

void op_send_packet(int destsockfd, unsigned int podid, op_pkttype_t pkttype, unsigned int pktnum, char *data, int n) {
    op_packet_t packet;
    packet.podid = podid;
    packet.pkttype = pkttype;
    packet.pktnum = pktnum;
    memcpy(packet.data, data, n);
    send(destsockfd, (void*)&packet, sizeof(op_packet_t), 0);
}

int op_receive_packet(int srcsockfd, op_packet_t *packet) {
    bzero(packet, sizeof(op_packet_t));
    int len = recv(srcsockfd, (void*)packet, sizeof(op_packet_t), 0);
    return len;
}

void op_receive_message_header(int srcsockfd, op_packet_t *packet, size_t *msglen) {
    if (op_receive_packet(srcsockfd, packet) == 0) {
        perror("error: disconnect");
        exit(1);
    }
    puts("");
    puts("");
    memcpy((void*)msglen, (void*)packet->data, sizeof(size_t));
}

void op_receive_message_body(int srcsockfd, size_t msglen, op_msg_t *message, int podid, char *init_data, size_t init_msglen) {
    // Receive packets and put them in order.
    // Since an ACK signal is necessary for sending packets, we can just receive them in order.
    // We just need to know the length.

    // Should copy length
    bzero((void*)message, msglen);
    memcpy((void*)message, (void*)init_data, init_msglen);
    debug_hexdump((void*)message, sizeof(op_msg_t));

    printf("Reading %lu bytes (starting at %lu)...\n", msglen, init_msglen);
    int i = init_msglen;
    int pktnum;
    while (i < msglen) {
        if (i < 0) {
            perror("error: length overflow\n");
            exit(1);
        }
        op_packet_t packet;
        pktnum = i / PACKET_DATA_SIZE;
        op_send_packet(srcsockfd, podid, PKTTYPE_ACK, (uint8_t)pktnum, "", 0);
        op_receive_packet(srcsockfd, &packet);
        debug_hexdump((void*)&packet, sizeof(op_packet_t));
        if (packet.pktnum != pktnum) {
            // Silently drop packets
            printf("Wrong # (E:%u, A:%u)", pktnum, packet.pktnum);
            continue;
        }
        printf("%d/%lu\n", i, msglen);
        memcpy((void*)(((char*)message) + i), (void*)packet.data, PACKET_DATA_SIZE); 
        debug_hexdump((void*)message, sizeof(op_msg_t));
        i += PACKET_DATA_SIZE;
    }
    // Send one final "ACK"
    op_send_packet(srcsockfd, podid, PKTTYPE_ACK, (uint8_t)pktnum, "", 0);
    
}

void op_makeinetaddr(uint32_t inaddr, uint16_t port, struct sockaddr_in *sockaddr) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);
    sockaddr->sin_addr.s_addr = inaddr;
}


op_nonce_t op_next_nonce(op_nonce_t nonce) {
    // They almost certainly do a computation that is FAR more complicated than this.
    // Our solution does not depend on how the nonce internally works.
    return nonce + 1;
}

void debug_hexdump(void *data, int n) {
    unsigned char *charptr = (unsigned char*)data;
    for (int i = 0; i < n; i++) {
        printf("%02x", *(charptr + i));
    }
    puts("");
}

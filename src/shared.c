#include "shared.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

void op_send_commands(int destsockfd, op_pkttype_t src, int podid, op_nonce_t nonce, op_cmd_t *cmds, int ncmds) {
    op_packet_t received_packet;
    int msg_len = sizeof(op_nonce_t) + sizeof(op_cmd_t) * ncmds;
    int true_msg_len = sizeof(size_t) + msg_len;
    op_msg_t msg;
    msg.msg_len = msg_len;
    msg.nonce = nonce;

    int is_first = 1;
    for (int i = 0; i < true_msg_len; i += PACKET_DATA_SIZE) {
        int pktnum = i / PACKET_DATA_SIZE;
        int is_last = i + PACKET_DATA_SIZE <= true_msg_len;
        char *byte_ptr = ((char*)&msg) + i;
        op_send_packet(destsockfd, podid, is_first ? src : PKTTYPE_CON, (uint8_t)pktnum, byte_ptr, is_last ? (true_msg_len - pktnum) : PACKET_DATA_SIZE);
        is_first = 0;
        if (!is_last) {
            op_receive_packet(destsockfd, &received_packet); // Should be ACK packet. We don't actually care about the contents.
        }
        usleep(10000); // By waiting 10ms, we make jamming much easier.
    }
}

void op_send_packet(int destsockfd, uint8_t podid, op_pkttype_t pkttype, uint8_t pktnum, char *data, int n) {
    op_packet_t packet;
    packet.podid = podid;
    packet.pkttype = pkttype;
    packet.pktnum = pktnum;
    memcpy(packet.data, data, n);
    send(destsockfd, (void*)&packet, sizeof(op_packet_t), 0);
}

void op_receive_packet(int srcsockfd, op_packet_t *packet) {
    recvfrom(srcsockfd, (void*)packet, sizeof(op_packet_t), 0, NULL, NULL);
}

void op_receive_message_header(int srcsockfd, size_t *msglen, op_pkttype_t *pkttype, int *podid, op_nonce_t *nonce, char *initial_data) {
    op_packet_t packet;
    op_receive_packet(srcsockfd, &packet);
    *pkttype = packet.pkttype;
    *podid = packet.podid;
    memcpy((void*)msglen, (void*)&packet.data, sizeof(size_t));
    memcpy((void*)nonce, (void*)(&packet.data + sizeof(size_t)), sizeof(op_nonce_t));
    memcpy((void*)initial_data, (void*)&packet.data, PACKET_DATA_SIZE);
}

void op_receive_message_body(int srcsockfd, size_t msglen, op_msg_t *message, int podid, char *init_data, size_t init_msglen) {
    // Receive packets and put them in order.
    // Since an ACK signal is necessary for sending packets, we can just receive them in order.
    // We just need to know the length.

    // Should copy length and nonce
    memcpy((void*)(message), (void*)init_data, init_msglen);
    for (int i = init_msglen; i < msglen; i += PACKET_DATA_SIZE) {
        op_packet_t packet;
        int pktnum = i / PACKET_DATA_SIZE;
        op_send_packet(srcsockfd, podid, PKTTYPE_ACK, (uint8_t)pktnum, "", 0);
        op_receive_packet(srcsockfd, &packet);
        memcpy((void*)((char*)&message + i), (void*)packet.data, PACKET_DATA_SIZE);
    }
}

op_nonce_t op_next_nonce(op_nonce_t nonce) {
    // They almost certainly do a computation that is FAR more complicated than this.
    // Our solution does not depend on how the nonce internally works.
    return nonce + 1;
}

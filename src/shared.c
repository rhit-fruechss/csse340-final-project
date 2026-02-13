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
        int is_last = i + PACKET_DATA_SIZE <= true_msg_len;
        char *byte_ptr = ((char*)&msg) + i;
        op_send_packet(destsockfd, podid, is_first ? src : PKTTYPE_CON, (uint8_t)i, byte_ptr, is_last ? (true_msg_len - i) : PACKET_DATA_SIZE);
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

void op_receive_message(int srcsockfd, op_msg_t *message) {
    // TODO: receive packets and put them together in order based on pktid.
    // Since an ACK signal is necessary for sending packets, we can just receive them in order.
    // We just need to know the length.
}

op_nonce_t op_next_nonce(op_nonce_t nonce) {
    // They almost certainly do a computation that is FAR more complicated than this.
    // Our solution does not depend on how the nonce internally works.
    return nonce + 1;
}

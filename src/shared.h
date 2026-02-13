#ifndef _OP_SHARED_H_
#define _OP_SHARED_H_
#define MAX_COMMANDS 4
#define PACKET_DATA_SIZE 8
#define OP_SHARED_NONCE_START 0xdeadbeef // Assume this was established somehow with Diffie-Hellman.

#include <stdlib.h>
#include <stdint.h>

// Packet type.
typedef enum {
    PKTTYPE_POD = 0,
    PKTTYPE_PDM = 1,
    PKTTYPE_ACK = 2,
    PKTTYPE_CON = 3,
} op_pkttype_t;

// Commands.
// As a note, we are GREATLY simplifying the architecture for the purposes of this project.
typedef enum {
    SHUTDOWN = 1,
    SETUP_POD = 2,
    RESCHEDULE = 3,
    DEPOSIT_5U = 4,
    DEPOSIT_45U = 5,
} op_cmd_t;

// Nonce - just use an unsigned int
typedef uint32_t op_nonce_t;

typedef struct op_msg {
    size_t msg_len; // Message length in bytes
    op_nonce_t nonce; // Normally this is stored per command, but all the commands share the same nonce.
    op_cmd_t cmds[MAX_COMMANDS]; // All commands to send
} op_msg_t;

typedef struct op_packet {
    uint8_t podid; // Packet ID
    op_pkttype_t pkttype; // Packet type
    uint8_t pktnum; // Packet order number
    char data[PACKET_DATA_SIZE];
} op_packet_t;

// Sends commands.
// First sends a PDM packet, then receive ACK and send CON packets.
// 
//      PDM ... -> (PDM packet contains message length) 
//              <- ACK
//      CON ... ->
//              <- ACK
//      CON ... ->
//              <- ACK
//      CON .
//              <- ACK
//
//  - destsockfd: the socket to send the commands to
//  - src: either POD or PDM - the originator of the message
//  - nonce: the nonce to send to check the data
//  - cmds: the commands to send
//  - ncmds: the number of commands to send
void op_send_commands(int destsockfd, op_pkttype_t src, int podid, op_nonce_t nonce, op_cmd_t *cmds, int ncmds);

// Sends a single packet.
void op_send_packet(int destsockfd, uint8_t podid, op_pkttype_t pkttype, uint8_t pktnum, char *data, int n);

void op_receive_packet(int srcsockfd, op_packet_t *packet);

void op_receive_message(int srcsockfd, op_msg_t *message);

op_nonce_t op_next_nonce(op_nonce_t nonce);
#endif

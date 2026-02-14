#ifndef _OP_SHARED_H_
#define _OP_SHARED_H_
#define MAX_COMMANDS 4
#define PACKET_DATA_SIZE 16

// Assume this was established with something like Diffie-Hellman.
#define OP_SHARED_NONCE_START 0xdeadbeef 

#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Packet type.
typedef enum {
    PKTTYPE_POD = 0,
    PKTTYPE_PDM = 1,
    PKTTYPE_ACK = 2,
    PKTTYPE_CON = 3,
} op_pkttype_t;

// Commands.
// As a note, we are GREATLY simplifying the architecture for the purposes of this project.
// We are also assuming ALL commands require a nonce.
typedef enum {
    CMD_EMPTY = 0, // Do nothing
    CMD_SHUTDOWN = 1, // Shut down the pod
    CMD_SETUP_POD = 2, // Set up the pod
    CMD_RESCHEDULE = 3, // Reschedule (this would have parameters but we're ignoring them)
    CMD_DEPOSIT_5U = 4, // Deposit 5 units insulin 
    CMD_DEPOSIT_45U = 5, // Deposit 45 units insulin
} op_cmd_t;

// Nonce - just use an unsigned int
typedef unsigned int op_nonce_t;

typedef struct op_msg {
    size_t msg_len; // Message length in bytes
    op_nonce_t nonce; // Normally this is stored per command, but all the commands share the same nonce.
    op_cmd_t cmds[MAX_COMMANDS]; // All commands to send
} op_msg_t;

typedef struct op_packet {
    unsigned char podid; // Pod ID
    op_pkttype_t pkttype; // Packet type
    unsigned char pktnum; // Packet order number
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
void op_send_commands(int srcsockfd, int destsockfd, op_pkttype_t src, int podid, op_nonce_t nonce, op_cmd_t *cmds, int ncmds);

// Sends a single packet.
void op_send_packet(int destsockfd, unsigned char podid, op_pkttype_t pkttype, unsigned char pktnum, char *data, int n);

int op_receive_packet(int srcsockfd, op_packet_t *packet);

void op_receive_message_header(int srcsockfd, size_t *msglen, op_pkttype_t *pkttype, int *podid, op_nonce_t *nonce, char *initial_data);

void op_receive_message_body(int srcsockfd, size_t msglen, op_msg_t *message, int podid, char *init_data, size_t init_msglen);

void op_makeinetaddr(uint32_t inaddr, uint16_t port, struct sockaddr_in *addr);

op_nonce_t op_next_nonce(op_nonce_t nonce);
#endif

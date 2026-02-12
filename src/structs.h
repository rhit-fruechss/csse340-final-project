// Packet type.
enum op_pkttype {
    POD = 0,
    PDM = 1,
    ACK = 2,
    CON = 3,
};

typedef struct packet {
    char data[1024];
} packet_t;

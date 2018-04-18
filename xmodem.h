#define SOH 0x01 // Start of Header
#define EOT 0x04 // End of Transmission
#define ACK 0x06 // Acknowledge
#define NAK 0x15 // Not Acknowledge
#define CAN 0x18 // Cancel

struct xmodem_packet
{
    unsigned char bytes[132];
    unsigned int addr;
    unsigned int addr2;
    unsigned int number;
    unsigned int byte;
    unsigned int size;
    unsigned int crc;
};

void xmodem_packet_init(struct xmodem_packet *packet, unsigned int output_base, unsigned int output_base2);

void add_checksum_byte(struct xmodem_packet *packet);

void process_byte(struct xmodem_packet *packet);

int read_byte(struct xmodem_packet *packet);
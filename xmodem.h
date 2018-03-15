#define SOH 0x01 // Start of Header
#define EOT 0x04 // End of Transmission
#define ACK 0x06 // Acknowledge
#define NAK 0x15 // Not Acknowledge
#define CAN 0x18 // Cancel

struct xmodem_packet
{
    unsigned char bytes[132];
    unsigned int addr;
    unsigned int number;
    unsigned int byte;
    unsigned int crc;
};
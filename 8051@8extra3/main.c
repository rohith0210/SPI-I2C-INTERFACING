#include <reg52.h>
#include <intrins.h>

// SPI bit-bang pin definitions
sbit SPI_MOSI = P2^0;
sbit SPI_MISO = P2^1;
sbit SPI_SCLK = P2^2;
sbit SPI_CS   = P2^3;   // Chip-select (active low)

// ——— Tiny µs delay ———
void delay_us(unsigned int us) {
    while (us--) {
        _nop_();
        _nop_();
    }
}

// ——— UART @9600bps setup ———
void uart_init(void) {
    TMOD |= 0x20;         // Timer1 Mode 2 (8-bit auto-reload)
    TH1 = TL1 = 0xFD;     // 9600 baud @11.0592MHz
    SCON = 0x50;          // Serial mode 1, REN enabled
    TR1  = 1;             // Start Timer1
}

void uart_send_char(char c) {
    SBUF = c;
    while (!TI);
    TI = 0;
}

void uart_send_str(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_hex(unsigned char b) {
    const char hex[] = "0123456789ABCDEF";
    uart_send_char(hex[b >> 4]);
    uart_send_char(hex[b & 0x0F]);
}

// ——— SPI Init ———
void spi_init(void) {
    SPI_CS   = 1;  // Deselect
    SPI_SCLK = 0;  // Clock idle low
}

// ——— SPI Transfer (Send + Receive) ———
unsigned char spi_transfer_byte(unsigned char byte_out) {
    unsigned char i, byte_in = 0;
    SPI_CS = 0;

    for (i = 0; i < 8; i++) {
        // Send MSB first
        SPI_MOSI = (byte_out & 0x80) ? 1 : 0;
        byte_out <<= 1;

        // Clock high
        SPI_SCLK = 1;
        delay_us(5);

        // Read MISO
        byte_in <<= 1;
        if (SPI_MISO) byte_in |= 1;

        // Clock low
        SPI_SCLK = 0;
        delay_us(5);
    }

    SPI_CS = 1;
    return byte_in;
}

// ——— Main Function ———
void main(void) {
    unsigned char tx = 0xA5;
    unsigned char rx;

    uart_init();
    spi_init();

    uart_send_str("SPI Loopback Test\r\n");
    uart_send_str("Sending 0xA5 ... ");

    rx = spi_transfer_byte(tx);

    uart_send_str("Received 0x");
    uart_send_hex(rx);
    uart_send_str("\r\n");

    while (1);
}

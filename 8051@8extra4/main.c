#include <reg52.h>
#include <intrins.h>

// 1. SPI Bit-Bang Pin Definitions
sbit SPI_MOSI = P2^0;
sbit SPI_MISO = P2^1;
sbit SPI_SCLK = P2^2;
sbit SPI_CS   = P2^3;

// 2. Microsecond Delay
void delay_us(unsigned int us) {
    while (us--) {
        _nop_();
        _nop_();
    }
}

// 3. UART Initialization @9600 baud
void uart_init(void) {
    TMOD |= 0x20;        // Timer1 in Mode2 (8-bit auto-reload)
    TH1 = TL1 = 0xFD;    // 9600 baud @11.0592 MHz
    SCON = 0x50;         // Serial mode1, REN=1
    TR1  = 1;            // Start Timer1
}

void uart_send_char(char c) {
    SBUF = c;
    while (!TI);
    TI = 0;
}

void uart_send_str(char *s) {
    while (*s) {
        uart_send_char(*s++);
    }
}

void uart_send_hex(unsigned char byte) {
    char hex_chars[] = "0123456789ABCDEF";
    uart_send_char(hex_chars[byte >> 4]);
    uart_send_char(hex_chars[byte & 0x0F]);
}

// 4. SPI Initialization (Mode 0)
void spi_init(void) {
    SPI_CS   = 1;   // Deselect EEPROM
    SPI_SCLK = 0;   // Clock idle LOW
}

// 5. SPI Transfer (Send+Receive One Byte)
unsigned char spi_transfer_byte(unsigned char byte_out) {
    unsigned char i, byte_in = 0;

    for (i = 0; i < 8; i++) {
        // Send MSB first
        //SPI_MOSI = (byte_out & 0x80) ? 1 : 0;
			if (byte_out & 0x80){
			   SPI_MOSI = 1;
			 }
			 else{
			  SPI_MOSI = 0;
			}
			
 			byte_out <<= 1;

        // Clock HIGH: slave samples MOSI, shifts out MISO
        SPI_SCLK = 1;
        delay_us(5);

        // Read one bit from MISO
        byte_in <<= 1;
        if (SPI_MISO) byte_in |= 1;

        // Clock LOW: prepare next bit
        SPI_SCLK = 0;
        delay_us(5);
    }

    return byte_in;
}

// 6. EEPROM Command Definitions
#define EEPROM_CMD_WREN   0x06
#define EEPROM_CMD_WRITE  0x02
#define EEPROM_CMD_READ   0x03

// 7. Issue Write Enable
void eeprom_write_enable(void) {
    SPI_CS = 0;
    spi_transfer_byte(EEPROM_CMD_WREN);
    SPI_CS = 1;
}

// 8. Write One Byte to EEPROM
void eeprom_write_byte(unsigned int address, unsigned char data_to_write) {
    unsigned char high_addr = (address >> 8) & 0xFF;
    unsigned char low_addr  =  address       & 0xFF;
    unsigned int  i;

    // a) Enable write
    eeprom_write_enable();

    // b) Send WRITE command + 16-bit address + data
    SPI_CS = 0;
      spi_transfer_byte(EEPROM_CMD_WRITE);
      spi_transfer_byte(high_addr);
      spi_transfer_byte(low_addr);
      spi_transfer_byte(data_to_write);
    SPI_CS = 1;

    // c) Wait ~5 ms for internal write cycle
    for (i = 0; i < 1500; i++) {
        delay_us(5);
    }
}

// 9. Read One Byte from EEPROM
unsigned char eeprom_read_byte(unsigned int address) {
    unsigned char high_addr = (address >> 8) & 0xFF;
    unsigned char low_addr  =  address       & 0xFF;
    unsigned char read_data;

    SPI_CS = 0;
      spi_transfer_byte(EEPROM_CMD_READ);
      spi_transfer_byte(high_addr);
      spi_transfer_byte(low_addr);
      read_data = spi_transfer_byte(0x00);  // send dummy to receive
    SPI_CS = 1;

    return read_data;
}

// 10. Main Function
void main(void) {
    unsigned char written   = 0x5A;
    unsigned char readback;

    uart_init();
    spi_init();

    // Write 0x5A to address 0x0010
    eeprom_write_byte(0x0010, written);
    uart_send_str("Wrote 0x");
    uart_send_hex(written);
    uart_send_str(" to EEPROM address 0x0010\r\n");

    // Read back from 0x0010
    readback = eeprom_read_byte(0x0010);
    uart_send_str("Read  0x");
    uart_send_hex(readback);
    uart_send_str(" from EEPROM address 0x0010\r\n");

    while (1) {
        // Optional: Stay here
    }
}


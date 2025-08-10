/*#include <reg52.h>
#include <intrins.h>  // for _nop_()

// I²C pins
sbit SDA = P2^0;
sbit SCL = P2^1;

// -----------------------------------------------------------------
// Tiny microsecond delay
void delay_us(unsigned int us)
{
    while (us--) {
        _nop_();
        _nop_();
    }
}

// Millisecond delay (~1 ms at 11.0592 MHz)
void delay_ms(unsigned int ms)
{
    unsigned int i;
    while (ms--) {
        for (i = 0; i < 123; i++);
    }
}

// -----------------------------------------------------------------
// I²C START
void i2c_start(void)
{
    SDA = 1; SCL = 1;   delay_us(5);
    SDA = 0;            delay_us(5);
    SCL = 0;            delay_us(5);
}

// I²C STOP
void i2c_stop(void)
{
    SDA = 0; SCL = 1;   delay_us(5);
    SDA = 1;            delay_us(5);
}

// -----------------------------------------------------------------
// Write one byte on I²C (ACK ignored)
void i2c_write(unsigned char d)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        // Drive SDA = MSB of d
        if (d & 0x80)
            SDA = 1;
        else
            SDA = 0;
        // Pulse SCL
        SCL = 1; delay_us(5);
        SCL = 0; delay_us(5);
        d <<= 1;
    }
    // Clock the ACK bit (ignored)
    SDA = 1;
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
}

// Read one byte on I²C then send NACK
unsigned char i2c_read(void)
{
    unsigned char i, val = 0;
    SDA = 1;  // release SDA
    for (i = 0; i < 8; i++) {
        SCL = 1; delay_us(5);
        val <<= 1;
        if (SDA) val |= 1;
        SCL = 0; delay_us(5);
    }
    // Send NACK
    SDA = 1;
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
    return val;
}

// -----------------------------------------------------------------
// BCD?Decimal
unsigned char bcd_to_dec(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// -----------------------------------------------------------------
// UART @9600 baud init
void uart_init(void)
{
    TMOD |= 0x20;   // Timer1 Mode2
    TH1 = 0xFD;     // Reload for 9600
    TL1 = 0xFD;
    SCON = 0x50;    // UART mode1, REN=1
    TR1  = 1;       // Start Timer1
}

// Send one char
void uart_send_char(char c)
{
    SBUF = c;
    while (!TI);
    TI = 0;
}

// Send string
void uart_send_str(const char *s)
{
    while (*s) uart_send_char(*s++);
}

// Send two-digit number 00–99
void uart_send_num(unsigned char num)
{
    uart_send_char((num / 10) + '0');
    uart_send_char((num % 10) + '0');
}

// -----------------------------------------------------------------
void main(void)
{
    unsigned char seconds;

    uart_init();
    while (1) {
        i2c_start();
        i2c_write(0xD0);       // DS1307 write address
        i2c_write(0x00);       // Seconds register
        i2c_start();           // Repeated start
        i2c_write(0xD1);       // DS1307 read address
        seconds = i2c_read();  // Read raw BCD seconds
        i2c_stop();

        seconds = bcd_to_dec(seconds);

        uart_send_str("Seconds: ");
        uart_send_num(seconds);
        uart_send_str("\r\n");

        delay_ms(1000);
    }
}
*/


//THE CODE USING A HARDWARE DELAY!!

/*
#include <reg52.h>
#include <intrins.h>

// I2C pins
sbit SDA = P2^0;
sbit SCL = P2^1;

bit one_sec_flag = 0;
unsigned char tick_count = 0;

// ========== Delay ==========
void delay_us(unsigned int us)
{
	//On an 11.0592 MHz crystal, one machine cycle = 12 oscillator clocks = 1.085 µs.  
  //Each _nop_() burns 1.085 µs. Two of them burn about 2.17 µs.
    while (us--) {
        _nop_(); _nop_();
    }
}

// ========== I2C ==========
void i2c_start(void)
{
    SDA = 1; SCL = 1; delay_us(5);
    SDA = 0; delay_us(5);
    SCL = 0; delay_us(5);
}

void i2c_stop(void)
{
    SDA = 0; SCL = 1; delay_us(5);
    SDA = 1; delay_us(5);
}

void i2c_write(unsigned char d)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {
        // Drive SDA = MSB of d
        if (d & 0x80)
            SDA = 1;
        else
            SDA = 0;
        // Pulse SCL
        SCL = 1; delay_us(5);
        SCL = 0; delay_us(5);
        d <<= 1;
    }
    // Clock ACK bit
    SDA = 1;
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
}

unsigned char i2c_read(void)
{
    unsigned char i, val = 0;
    SDA = 1; // release SDA
    for (i = 0; i < 8; i++) {
        SCL = 1; delay_us(5);
        val <<= 1;
        if (SDA) val |= 1;
        SCL = 0; delay_us(5);
    }
    // NACK
    SDA = 1;
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
    return val;
}

// ========== BCD to Decimal ==========
unsigned char bcd_to_dec(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// ========== UART ==========
void uart_init(void)
{
    TMOD |= 0x20;     // Timer1 mode 2
    TH1 = 0xFD;       // 9600 baud
    TL1 = 0xFD;
    SCON = 0x50;      // 8-bit UART, REN=1
    TR1 = 1;          // Start Timer1
}

void uart_send_char(char c)
{
    SBUF = c;
    while (!TI);
    TI = 0;
}

void uart_send_str(const char *s)
{
    while (*s) uart_send_char(*s++);
}

void uart_send_num(unsigned char num)
{
    uart_send_char((num / 10) + '0');
    uart_send_char((num % 10) + '0');
}

// ========== Timer0 ISR ==========
void timer0_ISR(void) interrupt 1
{
    TH0 = 0x3C;  // Preload for 50ms overflow
    TL0 = 0xB0;

    tick_count++;
    if (tick_count >= 19) {  // 50ms x 20 = 1s
        tick_count = 0;
        one_sec_flag = 1;
    }
}

// ========== Timer Init ==========
void timer0_init(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;  // Mode 1
    TH0 = 0x3C;
    TL0 = 0xB0;
    ET0 = 1;       // Enable Timer0 interrupt
    EA = 1;        // Enable global interrupt
    TR0 = 1;       // Start Timer0
}

// ========== Main ==========
void main(void)
{
    unsigned char seconds;

    uart_init();
    timer0_init();

    while (1) {
        if (one_sec_flag) {
            one_sec_flag = 0;

            i2c_start();
            i2c_write(0xD0);       // Write address
            i2c_write(0x00);       // Seconds register
            i2c_start();
            i2c_write(0xD1);       // Read address
            seconds = i2c_read();
            i2c_stop();

            seconds = bcd_to_dec(seconds);

            uart_send_str("Seconds: ");
            uart_send_num(seconds);
            uart_send_str("\r\n");
        }
    }
}
*/

//poll-on-change method

#include <reg52.h>
#include <intrins.h>
// I2C pins
sbit SDA = P2^0;
sbit SCL = P2^1;

void delay_us(unsigned int us) {
    while (us--) {
        _nop_(); _nop_();
    }
}

// === I2C functions same as before ===
void i2c_start(void) {
    SDA = 1; SCL = 1; delay_us(5);
    SDA = 0; delay_us(5);
    SCL = 0; delay_us(5);
}

void i2c_stop(void) {
    SDA = 0; SCL = 1; delay_us(5);
    SDA = 1; delay_us(5);
}

void i2c_write(unsigned char d) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        SDA = (d & 0x80) ? 1 : 0;
        SCL = 1; delay_us(5);
        SCL = 0; delay_us(5);
        d <<= 1;
    }
    SDA = 1;
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
}

unsigned char i2c_read(void) {
    unsigned char i, val = 0;
    SDA = 1;
    for (i = 0; i < 8; i++) {
        SCL = 1; delay_us(5);
        val <<= 1;
        if (SDA) val |= 1;
        SCL = 0; delay_us(5);
    }
    SDA = 1; // NACK
    SCL = 1; delay_us(5);
    SCL = 0; delay_us(5);
    return val;
}

unsigned char bcd_to_dec(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// UART
void uart_init(void) {
    TMOD |= 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;
    SCON = 0x50;
    TR1 = 1;
}

void uart_send_char(char c) {
    SBUF = c;
    while (!TI);
    TI = 0;
}

void uart_send_str(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_num(unsigned char num) {
    uart_send_char((num / 10) + '0');
    uart_send_char((num % 10) + '0');
}

void main() {
    unsigned char prev_sec = 0xFF; // Invalid initial value
    unsigned char current_sec;

    uart_init();

    while (1) {
        // Read RTC seconds
        i2c_start();
        i2c_write(0xD0); // RTC write addr
        i2c_write(0x00); // seconds register
        i2c_start();
        i2c_write(0xD1); // RTC read addr
        current_sec = i2c_read();
        i2c_stop();

        // Compare only when value changes
         if(current_sec!=prev_sec){
				  prev_sec = current_sec;
					current_sec = bcd_to_dec(current_sec);
					
					uart_send_str("Seconds: ");
					uart_send_num(current_sec);
					uart_send_str("\r\n");
        }
				delay_us(5);
				
    }
}
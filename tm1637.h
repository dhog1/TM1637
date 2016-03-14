/*
        TM1637 6-digs (bits) LED display driver

        Valid (tested) for ATMega328 (ATMega168) @ 16 MHz
*/

#ifndef TM1637_h
#define TM1637_h

#include <avr035.h>
#include <delay.h>

/*
                                // TM1637 instructions
  command byte  B7 B6  B5  B4  B3  B2  B1  B0
                 0  1  x   x                      Data Command Set
                 1  0  x   x                      Display Control Command Set
                 1  1  x   x                      Address command set

                 0  1                   0   0     Write data to the display register
                 0  1                   1   0     Read key scan data
                 0  1               0             Automatic address incrementing
                 0  1               1             Fixed address
                 0  1           0                 Normal mode
                 0  1           1                 Test mode

                                                 -- address (display register, digit nbr, bit nbr) --
                 1  1           0   0   0   0     0x00   the first digit (rightmost) of digital tube (display)
                 1  1           0   0   0   1     0x01
                 1  1           0   0   1   0     0x02
                 1  1           0   0   1   1     0x03
                 1  1           0   1   0   0     0x04
                 1  1           0   1   0   1     0x05   the last digit (leftmost)

                                                 -- display control (brightness and on/off) --
                 1  0               0   0   0    Set the pulse width of  1 /16
                 1  0               0   0   1    Set the pulse width of  2 /16
                 1  0               0   1   0    Set the pulse width of  4 /16
                 1  0               0   1   1    Set the pulse width of  10/16
                 1  0               1   0   0    Set the pulse width of  11/16
                 1  0               1   0   1    Set the pulse width of  12/16
                 1  0               1   1   0    Set the pulse width of  13/16
                 1  0               1   1   1    Set the pulse width of  14/16
                 1  0           0                Display OFF
                 1  0           1                Display ON

*/

#define TM1637_DSC      0x40        // DATA SET COMMAND
    #define _display_write    0x00
    #define _key_scan         0x02
    #define _auto_inc         0x00
    #define _fixed_addr       0x04
    #define _normal_mode      0x00
    #define _test_mode        0x80

#define TM1637_DCC      0x80        // DISPLAY CONTROL COMMAND
    #define _tm1637_disp_off  0x00
    #define _tm1637_disp_on   0x08
    #define _tm1637_brtn_mask 0x07

#define TM1637_ASC      0xC0        // ADDRESS SET COMMAND
    #define _tm1637_r0  0x00
    #define _tm1637_r1  0x01
    #define _tm1637_r2  0x02
    #define _tm1637_r3  0x03
    #define _tm1637_r4  0x04
    #define _tm1637_r5  0x05
    #define _tm1637_rmask 0x03

                             // CLK and DATA pin definition

#define _SCK_PORT     PORTD, PORTD7    // CLK pin   (any MCU pin)
#define _SCK_DDR      DDRD, DDD7
#define _SCK_PIN      PIND, PIND7

#define _DATA_PORT    PORTD, PORTD6    // DIO pin   (any MCU pin)
#define _DATA_DDR     DDRD, DDD6
#define _DATA_PIN     PIND, PIND6

                          //  max TM1637 clock frequecy 500 kHz (typ 450 )
#define _CLOCK_DELAY  1   //  set clock frequency (us) = reciproc(2 * _CLOCK_DELAY) and about 500 kHz @ 16 MHz MCU

#define _CLK_HIGH     C_SETBIT(_SCK_PORT)
#define _CLK_LOW      C_CLEARBIT(_SCK_PORT)
#define _DIO_HIGH     C_SETBIT(_DATA_PORT)
#define _DIO_LOW      C_CLEARBIT(_DATA_PORT)

#define _TM1637_DIGS_ATTACHED    4  // size of digital tube (bits)

#define _FMT_LEFT_BIT       1
#define _FMT_ZERO_BIT       0
#define _FMT_DP_POS(b)      ((b >> 2) & 0x07)
#define _FMT_LENGTH(b)      (b >> 5)

/*
#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000

//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
*/
                                // data - 0 to 9 representation of A - G 7-bit indicator segments
const unsigned char digitToSegment[] = {       // xGFEDCBA     16 bytes
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01000000     // - (minus)
};

/*
        void tm1637_write(int num, unsigned char fmt);

                                // Display a number num -32768 .. 32767 on 6-bit digital tube
                                // (or -999 .. 9999 for 4-bit "clock" tube
                                // fmt ->  l l l dp dp dp a a
                                //      l  - length;  0 - auto or 1.._TM1637_DIGS_ATTACHED
                                //      dp - decimal point position (0 - no dp) 0 .. 5
                                //      a - alignment type
                                //          0-right alignment 1-right alignment with leading zeros
                                //          2 - left alignment
                                //
*/

void tm1637_init(void);                             // the very first call; safe to call again
void tm1637_setBrightness(uint8_t brt);
void tm1637_on(void);
void tm1637_off(void);
void tm1637_clear(void);
void tm1637_write(int num, unsigned char fmt);       // as above
void tm1637_clock(uint32_t tm, unsigned char sec);   // output fotmat:  sec = 0  mm:ss  sec != 0 hh:mm

                                                     // no need to call; internal usage
void tm1637_start(void);                             
void tm1637_stop(void);
void tm1637_writeByte(unsigned char data);


void tm1637_start(void) {

    _CLK_HIGH;
    _DIO_HIGH;
    _delay_us(_CLOCK_DELAY);
    _DIO_LOW;
}

void tm1637_stop(void) {

    _CLK_LOW;
    _delay_us(_CLOCK_DELAY);
    _DIO_LOW;
    _delay_us(_CLOCK_DELAY);
    _CLK_HIGH;
    _delay_us(_CLOCK_DELAY);
    _DIO_HIGH;
}

void tm1637_writeByte(unsigned char data) {

unsigned char i;

    for (i=0; i<8; i++) {
        _CLK_LOW;
        if (data & 0x01) _DIO_HIGH;
        else _DIO_LOW;
        _delay_us(_CLOCK_DELAY);
        data >>= 1;
        _CLK_HIGH;
        _delay_us(_CLOCK_DELAY);
    };
                    // wait for "Chinese ACK" (ignore it)
    _CLK_LOW;
    _delay_us(_CLOCK_DELAY);
    _DIO_LOW;
    _CLK_HIGH;
    _delay_us(_CLOCK_DELAY);
    _CLK_LOW;
}


void tm1637_init(void) {

unsigned char i;

    C_SETBIT(_SCK_DDR);         // OUTPUT
    C_CLEARBIT(_SCK_PORT);      // and LOW
    C_SETBIT(_DATA_DDR);        // OUTPUT
    C_CLEARBIT(_DATA_PORT);     // and LOW

    tm1637_start();
    tm1637_writeByte(TM1637_DSC|_display_write|_auto_inc|_normal_mode);
    tm1637_stop();

    tm1637_start();
    tm1637_writeByte(TM1637_ASC | _tm1637_r0);  // Set the first address
    for(i = 0; i < 6; i++) tm1637_writeByte(0x00);    // Send data to clear
    tm1637_stop();

    tm1637_start();
    tm1637_writeByte(TM1637_DCC | _tm1637_disp_on | 0x03); // Open display, set brightness = 3
    tm1637_stop();
}

void tm1637_setBrightness(uint8_t brt) {

    tm1637_writeByte(TM1637_DCC | (brt & _tm1637_brtn_mask));
}

void tm1637_clear(void) {

unsigned char i;

    tm1637_start();
    tm1637_writeByte(TM1637_ASC | _tm1637_r0);    // Set the first address
    for(i=0; i<6; i++) tm1637_writeByte(0x00);    // Send data to clear
    tm1637_stop();
}

void tm1637_on(void) {

    tm1637_writeByte(TM1637_DCC | _tm1637_disp_on | 0x03);
}

void tm1637_off(void) {

    tm1637_writeByte(TM1637_DCC | _tm1637_disp_off);
}


void tm1637_write(int num, unsigned char fmt) {

unsigned char digs[6];
unsigned char dp, i, k, j = 0;

    if (num < 0) { j = 1; num *= -1; };

    i = 0;
    do {
        digs[i++] = num % 10; num /= 10;
    } while(num);   // i - now is actual digital number

    if (j) digs[i++] = 0x0A;

    k = 0;
    for (j = i; j < _TM1637_DIGS_ATTACHED; j++) {
        if (fmt & (1<<_FMT_ZERO_BIT)) { digs[j] = 0x00; k++; }
        else digs[j] = 0x0F;
    };
    i += k;

    dp = _FMT_DP_POS(fmt);
    for (j = 0; j < i; j++) {
        digs[j] = *(digitToSegment + digs[j]);
        if (dp && (j == _TM1637_DIGS_ATTACHED - dp - 1)) digs[j] |= 0x80;
    };

    k = _FMT_LENGTH(fmt);
    if (!k) k = i;

    tm1637_start();
    if (!(fmt & (1<<_FMT_LEFT_BIT))) tm1637_writeByte(TM1637_ASC | _TM1637_DIGS_ATTACHED - k);
    else tm1637_writeByte(TM1637_ASC);
    for (j = 0; j < k; j++) {
        tm1637_writeByte(digs[k-1-j]);
    };
    tm1637_stop();
}


void tm1637_clock(uint32_t tm, unsigned char sec) {

unsigned char data[4];
unsigned char i, m, s, dp;

    s = (unsigned char) (tm % 60); tm /= 60;      // tm in mins
    m = (unsigned char) (tm % 60); tm /= 60;      // tm in hours

    dp = s & 0x01;          // change colon with sec change

    if (!sec) {
       s = m; m = (unsigned char) (tm % 24);
    };

    i = 0;
    do {
        data[i++] = s % 10; s /= 10;
    } while(s);
    if (i < 2) data[i] = 0x00;
    i = 2;
    do {
        data[i++] = m % 10; m /= 10;
    } while(m);
    if (i<4) data[i] = 0x0F;

    for (i=0; i<4; i++) {
        if (dp) data[i] = *(digitToSegment + data[i]) | 0x80;
        else data[i] = *(digitToSegment + data[i]) & 0x7F;
    };

    tm1637_start();
    tm1637_writeByte(TM1637_ASC);
    for (i=0; i<4; i++) tm1637_writeByte(data[3-i]);
    tm1637_stop();
}

#endif

#include <io.h>
#include <tm1637.h>     // device TM1637
#include <stdio.h>      // Standard Input/Output functions

volatile unsigned int tick_cnt;
volatile unsigned char ready_flag;

// Timer1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
// Reinitialize Timer1 value
TCCR1B &= ~(1<<CS12);
TCNT1H=0x0B;
TCNT1L=0xDC;
TCCR1B |= (1<<CS12);
tick_cnt++;
ready_flag = 1;
}


void menu(void) {
    printf("\n\rn - view -27 (L)\n\r");
    printf("m - view 51 (L)\n\r");
    printf("q - view 12:04\n\r");
    printf("x - mad mix\n\r");
    printf("t - inc count(20ticks, zero)\n\r");
    printf("p - inc count(20ticks, no zero)\n\r");
    printf("d - dec count(20ticks), left\n\r");
    printf("z - set zero\n\r");
    printf("u - clear\n\r");
    printf("c - clock (sec)\n\r");
    printf("k - clock(min)\n\r");
    printf("a - dead timer\n\r");
}


void main(void) {

unsigned char ch;
unsigned int n, n1;
uint32_t t1;

// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 62,500 kHz
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 1 s
// Timer1 Overflow Interrupt: On
TCCR1A = 0;
TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
TCNT1H=0x0B;
TCNT1L=0xDC;
// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (1<<TOIE1);

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART0 Mode: Asynchronous
// USART Baud Rate: 19200
UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
UBRR0H=0x00;
UBRR0L=0x33;

// Global enable interrupts
#asm("sei")

ready_flag = 0;
tm1637_init();

while (1)
{
    menu();
    ch = getchar();
    switch (ch) {
                                // fmt ->  l l l dp dp dp a a
                                //      l  - length  0 - auto  1.._TM1637_DIGS_ATTACHED
                                //      dp - decimal point position (0 - no dp) 0 .. 5
                                //      a - alignment type
                                //      0-right alignment 1-right alignment with leading zeros
                                //      2 - left alignment
        case 'n': n = -27;
                  tm1637_write(n, 0b01100010);
        break;

        case 'm': n = 51;
                  tm1637_write(n, 0b00000010);
        break;

        case 'q': n = 4;
                  tm1637_write(n, 0b01000001);
                  n = 12;
                  tm1637_write(n, 0b01000010);
        break;

        case 'x': n  = 1;
                  n1 = 99;
                  TCNT1H = 0x0B; TCNT1L = 0xDC;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         tm1637_write(n, 0b01000001);
                         n++;
                         tm1637_write(n1, 0b01000010);
                         n1--;
                      };
                  } while (n < 100);
                  TCCR1B &= ~(1<<CS12);
        break;

        case 't':
                  TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; n = 0;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         n = tick_cnt; tm1637_write(n, 0b0000001);
                      };
                  } while (n < 20);
                  TCCR1B &= ~(1<<CS12);
        break;

        case 'a':
                  TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; t1 = 15;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         t1--;
                         tm1637_clock(t1, 1);
                      };
                  } while (t1);
                  TCCR1B &= ~(1<<CS12);
                  printf("\n\rBOOM!!!\n\r");
        break;

        case 'p':
                  TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; n = 0;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         n = tick_cnt; tm1637_write(n, 0b00000000);
                      };
                  } while (n < 20);
                  TCCR1B &= ~(1<<CS12);
        break;

        case 'd':
                  TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; n = 1000;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         tm1637_write(n-tick_cnt, 0b00000010);
                      };
                  } while (tick_cnt < 20);
                  TCCR1B &= ~(1<<CS12);
        break;

        case 'z': n = 0;
                  tm1637_write(n, 0);
        break;

        case 'u': tm1637_clear();
        break;

        case 'c': TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; t1 = 1;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         t1++;
                         tm1637_clock(t1, 1);
                      };
                  } while (t1);
                  TCCR1B &= ~(1<<CS12);
        break;

        case 'k': TCNT1H = 0x0B; TCNT1L = 0xDC;
                  tick_cnt = 0; t1 = 1;
                  TCCR1B |= (1<<CS12);
                  do {
                      if (ready_flag) {
                         ready_flag = 0;
                         t1++;
                         tm1637_clock(t1, 0);
                      };
                  } while (t1);    // 65535 sec
                  TCCR1B &= ~(1<<CS12);
        break;

    };   // switch


}  // loop

}

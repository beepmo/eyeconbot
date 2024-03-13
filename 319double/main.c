#include "msp430.h"
//void UARTSendArray(char *TxArray, char ArrayLength);
//
//static  char data;
//#define     LED1                  BIT0                         //for P1.0 red LED
//#define     LED2                  BIT7                         //for P4.7 green LED
////
#define     TXD                   BIT4                      // TXD on P4.4
#define     RXD                   BIT5                      // RXD on P4.5
void main(void) {
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    /*
    //  set up LEDs
    P1DIR |= LED1;
    P4DIR |= LED2;
    P1OUT &= ~LED1;
    P4OUT &= ~LED2;

*/

    // Configure UART
    P4DIR |= TXD;
    P4OUT |= TXD;
    UCA1CTL1 = UCSWRST; //Recommended to place USCI in reset first
    P4SEL |= BIT4 + BIT5;
    UCA1CTL1 |= UCSSEL_2; // Use SMCLK
    UCA1BR0 = 109; // Set baud rate to 9600 with 1.048MHz clock (Data Sheet 36.3.13)
    UCA1BR1 = 0; // Set baud rate to 9600 with 1.048MHz clock
    UCA1MCTL = UCBRS_2; // Modulation UCBRSx = 2
    UCA1CTL1 &= ~UCSWRST; // Initialize USCI state machine
    UCA1IE = UCRXIE;  // Enable  RX interrupt

    // set up ECHO in p1.3
    P1DIR &= ~BIT3;
    P1IE = BIT3; // set up interrupt
    P1IES = BIT3; // interrupt occurs on falling edge

    while (1) {
        // set up timer
        TA0CTL |= TASSEL__SMCLK; // increment every 1 microsec
        TA0CCR0 = 0xFFFF; // overflow at 0xFFFF
        TA0CTL |= TACLR; // clear TA0R

        // Generate TRIGGER pulse
        P6DIR |= BIT0;
        P6OUT |= BIT0;
        __delay_cycles(20);
        P6OUT &= ~BIT0;

        // start timer
        TA0CTL |= MC__UP;

        // await interrupts
        _BIS_SR (LPM4_bits + GIE);
    }


}


void __attribute__ ((interrupt(PORT1_VECTOR))) PORT1_ISR(void) // Port 1 interrupt service routine
   {

      // read timer
      short x = TA0R;
      UCA1TXBUF = x;  // Transmit time in timer

      __bic_SR_register_on_exit (LPM4_bits);

      P1IFG &= ~BIT3; // Clear P1.1 IFG.If you don't, it just happens again.
   }

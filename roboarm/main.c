#include <msp430.h> 

static short x; // store ADC12MEM0
void UARTConfigure(void);
void UARTSendArray(char *TxArray, char ArrayLength);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	UARTConfigure();

	// set up ADC12 on P6.0
	ADC12CTL0 = ADC12SHT02 + ADC12ON;
	ADC12CTL1 = ADC12SHP;
	ADC12CTL0 |= ADC12ENC;
	ADC12MCTL0 = ADC12INCH_0;
	P6SEL |= BIT0;

	//   set up PWM on P1.2
    P1DIR |= BIT2; // output on bit 2
    P1SEL |= BIT2; // PWM selected on bit 2
    TA0CCR0 = 20000; // period: 20000 1MHz counts = 20ms
    TA0CCTL1 = OUTMOD_7; // periods start on high
    TA0CCR1 = 1500; // start servo on center

    // whir roboarm to center
    TA0CTL = TASSEL__SMCLK + MC_1 + TAIE + ID_0;
    __delay_cycles(1000000);
    TA0CTL = 0;

    _BIS_SR (LPM0_bits | GIE); // CPU off, await interrupts

	return 0;
}

void UARTConfigure(void) {
    /* Configure UART */
    UCA1CTL1 = UCSWRST; //Recommended to place USCI in reset first
    P4SEL |= BIT4 + BIT5;
    UCA1CTL1 |= UCSSEL_2; // Use SMCLK
    UCA1BR0 = 109; // Set baud rate to 9600 with 1.048MHz clock (Data Sheet 36.3.13)
    UCA1BR1 = 0; // Set baud rate to 9600 with 1.048MHz clock
    UCA1MCTL = UCBRS_2; // Modulation UCBRSx = 2
    UCA1CTL1 &= ~UCSWRST; // Initialize USCI state machine
    UCA1IE = UCRXIE;  // Enable  RX interrupt
}

/*
 * Interrupt service routine upon receiving a byte.
 * This ISR controls its own flag; no need to lower it manually.
 * However, don't let it call functions or leave TimerA on, or it won't return control properly to main.
 */
void __attribute__ ((interrupt(USCI_A1_VECTOR))) UCIV1_ISR(void) {
    char data = UCA1RXBUF;

    switch(data){
        case 'd': // "give me a data point!"

            ADC12CTL0 |= ADC12SC; // start sampling

            x = ADC12MEM0;
            // transmit ADC12 in 2 bytes
            char* ptr = &x;
            while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
            UCA1TXBUF = *ptr;
            while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
            UCA1TXBUF = *(ptr + 1);
        break;

        case 'r': // "move right!"
            TA0CCR1 = TA0CCR1 + 10;
            TA0CTL = TASSEL__SMCLK + MC_1 + TAIE + ID_0;
            __delay_cycles(80000);
            TA0CTL = 0;
        break;

        case 'l': // "move left!"
            TA0CCR1 = TA0CCR1 - 10;
            TA0CTL = TASSEL__SMCLK + MC_1 + TAIE + ID_0;
            __delay_cycles(80000);
            TA0CTL = 0;

        break;
    }
}

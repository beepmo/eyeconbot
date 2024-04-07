#include <msp430.h> 

static short x; // store ADC12MEM0

void UARTConfigure(void);
void UARTSendArray(char *TxArray, char ArrayLength);
void Blink_Target_LED(void);

#define CENTER 1500 // 12 o'clock
#define R1 1300 // 1 o'clock
#define L1 1700 // 11 o'clock
#define EPSILON 5 // increment
#define DT 20000 // cycles per increment (0.02s)

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


//	// set up PWM on P1.2
//	P1DIR |= BIT2; // output on bit 2
//    P1SEL |= BIT2; // PWM selected on bit 2
//    TA0CCR0 = 20000; // period: 20000 1MHz counts = 20ms
//    TA0CCTL1 = OUTMOD_7; // periods start on high
//    TA0CCR1 = CENTER; // start servo on center
//    TA0CTL = TASSEL__SMCLK + MC_1 + TAIE + ID_0;  // begin output


    _BIS_SR (LPM0_bits | GIE);

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

void UARTSendArray(char *TxArray,  char ArrayLength){
     // Send number of bytes Specified in ArrayLength in the array at using the  UART
     // Example usage: UARTSendArray("Hello", 5);
     // int data[2]={1023, 235};
     // UARTSendArray(data, 4); // Note because the UART transmits bytes it is necessary to send two bytes for each integer hence the data length is twice the array length

    while(ArrayLength--){ // Loop until StringLength == 0 and post decrement
    while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data

     UCA1TXBUF = *TxArray; //Write the character at the location specified by the pointer
     TxArray++; //Increment the TxString pointer to point to the next character
     }
}

/*
 * Interrupt service routine upon receiving a byte
 * This ISR controls its own flag; no need to lower it manually.
 * However, don't let it call functions, or it won't return control properly to main.
 */
void __attribute__ ((interrupt(USCI_A1_VECTOR))) UCIV1_ISR(void) {
    char data = UCA1RXBUF;

    switch(data){
        case 'd':
            ADC12CTL0 |= ADC12SC; // start sampling

            while (ADC12CTL1 & ADC12BUSY); // poll while busy

            x = ADC12MEM0;
            char* ptr = &x;

            while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
            UCA1TXBUF = *ptr;

            while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
            UCA1TXBUF = *(ptr + 1);

//     case 'r':
//         TA0CCR1 = TA0CCR1 - EPSILON;
//     break;
//     case 'l':
//         TA0CCR1 = TA0CCR1 + EPSILON;
//     break;
    }
}


// Function to Blink the LED
void Blink_Target_LED(void)
{
    P1DIR |= BIT0;
    P1OUT |= BIT0; // Turn ON LED on MSP430
    __delay_cycles(200000); // wait for 0.2 sec
    P1OUT &= ~BIT0; // Turn OFF LED on MSP430
    __delay_cycles(200000); // wait for 0.2 sec
    P1OUT |= BIT0; // Turn ON LED on MSP430
        __delay_cycles(200000); // wait for 0.2 sec
}

#include <msp430.h> 

static short x;
void roboarm(void);
void UARTConfigure(void);
void UARTSendArray(char *TxArray, char ArrayLength);
void Blink_Target_LED(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	UARTConfigure();
	
	int data[2]={1023, 235};
	UARTSendArray(data, 4);

	// set up ADC12 on P6.0
	ADC12CTL0 = ADC12SHT02 + ADC12ON;
	ADC12CTL1 = ADC12SHP;
	ADC12CTL0 |= ADC12ENC;
	ADC12MCTL0 = ADC12INCH_0;
	P6SEL |= BIT0;

	while (1) {
	    ADC12CTL0 |= ADC12SC; // start sampling

	    while (ADC12CTL1 & ADC12BUSY); // poll while busy

        // store ADC12MEM0 register (16 bits long, 4 bits empty) in 16-bit short
	    x = ADC12MEM0;
	    UARTSendArray(&x, 2);

	}

	return 0;
}

void UARTConfigure() {
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

void roboarm() {
    P1DIR |= BIT2; // output on bit 2
    P1SEL |= BIT2; // PWM selected on bit 2

    TA0CCR0 = 512;

    TA0CCR1 = 512;
    TA0CCTL1 = OUTMOD_7;

    TA0CTL = TASSEL_2 + MC_1 + TAIE + ID_0;

    _bis_SR_register(LPM0_bits + GIE);
}

// Function to Blink the LED
void Blink_Target_LED(void)
{
    P1DIR |= BIT0;
    P1OUT |= BIT0; // Turn ON LED on MSP430
    __delay_cycles(200000); // wait for 0.2 sec
    P1OUT &= ~BIT0; // Turn OFF LED on MSP430
    __delay_cycles(200000); // wait for 0.2 sec
}
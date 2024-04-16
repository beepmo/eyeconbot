/* Example code demonstrating the use of the hardware UART on the
 * MSP430F5529 to receive and transmit data back to a host
 * computer over the USB connection on the MSP430 launchpad.
 * Note: After programming it is necessary to stop debugging and
 * reset the uC before connecting the terminal program to
 * transmit and receive characters.
 * This demo will turn on the Red LED if an R is sent and turn it
 * off if a r is sent.
 * Similarly G and g will turn on and off the green LED
 * It also transmits the received character back to the terminal.
 *To use cmd on Windows as terminal open the command prompt
 * Enter    powershell
 * Enter    $port= new-Object System.IO.Ports.SerialPort
 * COM3,9600,None,8,one
 * if COM3 is the port used
 * Than you can use commands:
 * $port.open()
 * $port.WriteLine("some string")
 * $port.ReadLine()
 * $port.Close()
 * One can use a free Windows terminal Tera Term
 */

#include "msp430.h"
void UARTSendArray(char *TxArray, char ArrayLength);

static  char data;
#define     LED1                  BIT0                         //for P1.0 red LED
#define     LED2                  BIT7                         //for P4.7 green LED

#define     TXD                   BIT4                      // TXD on P4.4
#define     RXD                   BIT5                      // RXD on P4.5
void main(void)

{
 WDTCTL = WDTPW + WDTHOLD; // Stop WDT

  P1DIR |= LED1;
  P4DIR |= LED2;
  P1OUT |= LED1;     //LED1 red on
  P4OUT &= ~LED2;    //LED2 green off
  P4DIR |= TXD;
  P4OUT |= TXD;



  /* next  line to use internal calibrated 1.048MHz clock: */

  TA0CTL = TASSEL_2 + MC_1 + TAIE +ID_3;            // Timer A control set to SMCLK, 1MHz and count up mode MC_1



 /* Configure UART */
  UCA1CTL1 = UCSWRST; //Recommended to place USCI in reset first
  P4SEL |= BIT4 + BIT5;
  UCA1CTL1 |= UCSSEL_2; // Use SMCLK
  UCA1BR0 = 109; // Set baud rate to 9600 with 1.048MHz clock (Data Sheet 36.3.13)
  UCA1BR1 = 0; // Set baud rate to 9600 with 1.048MHz clock
  UCA1MCTL = UCBRS_2; // Modulation UCBRSx = 2
  UCA1CTL1 &= ~UCSWRST; // Initialize USCI state machine
  UCA1IE = UCRXIE;  // Enable  RX interrupt

// _BIS_SR (LPM4_bits + GIE);    //Turn on interrupts and go into the lowest
                                    //power mode (the program stops here)
  _BIS_SR (GIE);
     while (1) {
              // bababum
         TA0CCR1 = TA0CCR1 + 1;
          }

  //while(1) {  while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
             // UCA1TXBUF = 0b000001001000;   //H
             // while (! (UCA1IFG & UCTXIFG)); // wait for TX buffer to be ready for new data
             // UCA1TXBUF = 0b00100000;   //space
             // _delay_cycles (2000000); //This function introduces 0.5 s delay
          //  }


}

// Echo back RXed character, confirm TX buffer is ready first


  void __attribute__ ((interrupt(USCI_A1_VECTOR))) UCIV1_ISR(void)    //interrupt routine for received data interrupt

{


data = UCA1RXBUF;                               //received character goes to data
UARTSendArray("Received command: ", 18); //"Received command" is printed, 18 is the number of characters in the printed arrey
UARTSendArray(&data, 1);       //received character is printed
UARTSendArray("\n\r", 2);      //new line and line return

switch(data){
 case 'R':
 {
 P1OUT |= BIT0;
 __delay_cycles(1000000);
 }
 break;
 case 'r':
 {
 P1OUT &= ~BIT0;
 }
 break;
 case 'G':
 {
 P4OUT |= BIT7;
 }
 break;
 case 'g':
 {
 P4OUT &= ~BIT7;
 }
 break;
 default:
 {
 UARTSendArray("Unknown Command: ", 17);
 UARTSendArray(&data, 1);
 UARTSendArray("\n\r", 2);
 }
 break;
 }
 //   _delay_cycles (2000000); //This function introduces 2 s delay
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

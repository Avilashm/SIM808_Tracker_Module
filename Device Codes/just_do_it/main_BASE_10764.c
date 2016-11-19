#include <msp430.h>
#include <string.h>

#define LED BIT0
#define RXD BIT1
#define TXD BIT2


//All credit to http://longhornengineer.com/code/MSP430/UART/ for init and uart handling

volatile unsigned int tx_flag;			//Mailbox Flag for the tx_char.
volatile unsigned char tx_char;			//This char is the most current char to go into the UART
volatile unsigned int rx_flag;			//Mailbox Flag for the rx_char.
volatile unsigned char rx_char;			//This char is the most current char to come out of the UART
char gps_string[100];
char server_resp[100];
char c;
char mob_no[16];
char msg[75];
unsigned int execute = 0;

void ConfigTimerA2(void);


void uart_init(void)
{
	P1SEL = RXD + TXD;					//Setup the I/O
	P1SEL2 = RXD + TXD;

    P1DIR |= LED; 						//P1.0 red LED. Toggle when char received.
    P1OUT |= LED; 						//LED off

	UCA0CTL1 |= UCSSEL_2; 				//SMCLK
										//8,000,000Hz, 9600Baud, UCBRx=52, UCBRSx=0, UCBRFx=1
	UCA0BR0 = 52;                  		//8MHz, OSC16, 9600
	UCA0BR1 = 0;                   	 	//((8MHz/9600)/16) = 52.08333
	UCA0MCTL = 0x10|UCOS16; 			//UCBRFx=1,UCBRSx=0, UCOS16=1
	UCA0CTL1 &= ~UCSWRST; 				//USCI state machine
	IE2 |= UCA0RXIE; 					//Enable USCI_A0 RX interrupt

	rx_flag = 0;						//Set rx_flag to 0
	tx_flag = 0;						//Set tx_flag to 0

	return;
}

unsigned char uart_getc()				//Waits for a valid char from the UART
{
	while (rx_flag == 0);		 		//Wait for rx_flag to be set
	rx_flag = 0;						//ACK rx_flag
    return rx_char;
}

void uart_putc(unsigned char c)
{
	tx_char = c;						//Put the char into the tx_char
	IE2 |= UCA0TXIE; 					//Enable USCI_A0 TX interrupt
	while(tx_flag == 1);				//Have to wait for the TX buffer
	tx_flag = 1;						//Reset the tx_flag
	return;
}

void uart_puts(char *str)
{
	while(*str) uart_putc(*str++);
	return;
}


void uart_putline(char *str)
{
	int idx=0;
	while( str[idx] != '\0' ){
		uart_putc(str[idx]);
		idx++;
	}
	uart_puts((char *)"\n\r");
	return;
}

void send_sms(void)
{
	if (server_resp[2] == '+' )
	{
		strncpy ( mob_no, server_resp+1, 15);
		strncpy ( msg, server_resp+16,strlen(server_resp)-16);

		uart_puts((char *)"AT\r");
	    _delay_cycles(1*16000000);

	    uart_puts((char *)"AT+CFUN=1\r");
	    _delay_cycles(1*16000000);

	    uart_puts((char *)"AT+CMGF=1\r");
	    _delay_cycles(1*16000000);

	    uart_puts((char *)"AT+CSCS=\"GSM\"\r");
	    _delay_cycles(1*16000000);

	    uart_puts((char *)"AT+CMGS=");
	    uart_puts((char *)mob_no);
	    uart_putc(13);
	    uart_putc(10);
	    _delay_cycles(1*16000000);

	    uart_puts((char *)msg);
	    uart_putc(26);
	    _delay_cycles(1*16000000);

	}
}


void get_gps(void)
{

		//uart_puts((char *)"AT+CGNSPWR=1\r");
	    //_delay_cycles(1*16000000);



	    //uart_puts((char *)"AT+CGNSINF\r");
	    //_delay_cycles(1*16000000);

	    uart_puts((char *)"AT+CGNSTST=1\r");

		int idx = 0;

		while(c = uart_getc()){

		    if ( idx == 0 && c != '$' ){ continue; }
			if ( c == '\r')
			{
				gps_string[idx+1] = '\0';
				idx = 0;
				uart_puts((char *)"AT+CGNSTST=0\r");
				_delay_cycles(1*16000000);
				uart_putline(gps_string);
				break;
			}
			else
			{
				gps_string[idx] = c;
			}
			idx++;
		}


}

void to_from_server (void){

    uart_puts((char *)"AT\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+CGATT=1\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+SAPBR=3,1,\"APN\",\"internet\"\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+SAPBR=1,1\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+HTTPINIT\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+HTTPPARA=\"URL\",\"http://thingsdata.co.in/Tracker/action2.php?GPGGA=");
    uart_puts((char *)gps_string);
    uart_puts((char *)"\"\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+HTTPACTION=0\r");
    _delay_cycles(4*16000000);

    uart_puts((char *)"AT+HTTPREAD\r");


    int idx = 0;

    while(c = uart_getc()){

    	if ( idx == 0 && c != '$' ){ continue; }
    	if ( c == 'Z')
    	{
    		server_resp[idx+1] = '\0';
    		idx = 0;
    		uart_putline(server_resp);
    		uart_puts((char *)"\r");
    		break;
    	}
    	else
    	{
    		server_resp[idx] = c;
    	}
    	idx++;
    }

}


int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; 			//Stop WDT
    BCSCTL1 = CALBC1_8MHZ; 				//Set DCO to 8Mhz
    DCOCTL = CALDCO_8MHZ; 				//Set DCO to 8Mhz

    uart_init();						//Initialize the UART connection

    __enable_interrupt();				//Interrupts Enabled

    uart_puts((char *)"AT\r");
    _delay_cycles(1*16000000);

    uart_puts((char *)"AT+CGNSPWR=1\r");
    _delay_cycles(15*16000000);

    uart_puts((char *)"AT\r");
    _delay_cycles(1*16000000);

    ConfigTimerA2();

    __bis_SR_register(GIE);

   while(1){

	   if (execute == 1)
	   {
		   TACTL = MC_0;
		   uart_puts((char *)"AT\r");
		   _delay_cycles(1*16000000);
		   execute = 0;
		   TACTL = TASSEL_1 + MC_2;

	   }
   }


}

void ConfigTimerA2(void)
  {
	CCTL0 = CCIE;                            // CCR0 interrupt enabled

	  CCR0 = 10000;

	  TACTL = TASSEL_1 + MC_2;                 // SMCLK, contmode

  }

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	TACTL = MC_0;
	execute = 1;
	CCR0 +=50000;								// add 12 seconds to the timer
	TACTL = TASSEL_1 + MC_2;
}



#pragma vector = USCIAB0TX_VECTOR		//UART TX USCI Interrupt
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF = tx_char;				//Copy char to the TX Buffer
	tx_flag = 0;						//ACK the tx_flag
	IE2 &= ~UCA0TXIE; 					//Turn off the interrupt to save CPU
}

#pragma vector = USCIAB0RX_VECTOR		//UART RX USCI Interrupt. This triggers when the USCI receives a char.
__interrupt void USCI0RX_ISR(void)
{
	rx_char = UCA0RXBUF;				//Copy from RX buffer, in doing so we ACK the interrupt as well
	rx_flag = 1;						//Set the rx_flag to 1

	P1OUT ^= LED;						//Notify that we received a char by toggling LED
}


/*
 *
GGA : Time, position and fix type data.
GSA : GPS receiver operating mode, active satellites used in the position solution and DOP values.
GSV : The number of GPS satellites in view satellite ID numbers, elevation, azimuth, and SNR values.
RMC : Time, date, position, course and speed data. Recommended Minimum Navigation Information.
VTG : Course and speed information relative to the ground.

*/

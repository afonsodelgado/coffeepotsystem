#include <msp430g2553.h>
#include <legacymsp430.h>
#include <intrinsics.h>
#include <stdlib.h>

#define COFFEE BIT6
#define TX BIT2
#define RX BIT1
#define LED1 BIT0

volatile unsigned char status_coffe = -1, cont = 0;
volatile char hours = -1, minutes = -1, seconds = -1;
volatile char TA_hours = '7', TA_minutes = '0', TA_seconds = '0'; //Change to real time clock

void setTime(volatile char hour, volatile char minute, volatile char second){
  TA_hours = hour;
  TA_minutes = minute;
  TA_seconds = second;
}

/*-----------------------------print_string_serial------------------------------------*
*-------------------------------------------------------------------------------------*
*----This function receive a pointer to a string as argument and will transmit--------*
*------------------------the string through serial port.------------------------------*
*-------------------------------------------------------------------------------------*/
int print_string_serial(const char *str){
  int status = -1;

  if (str != NULL) {
    status = 0;
    while (*str != '\0') {
      while (!(IFG2 & UCA0TXIFG));
      UCA0TXBUF = *str;
      if (*str == '\n') {
        while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = '\r';
      }
      str++;
    }
  }
  return status;
}

/*-------------------------------getchar_serial--------------------------------------*
*------------------------------------------------------------------------------------*
*-This function receive a characther from serial port and return it to the controler-*
*------------------------------------------------------------------------------------*/
int getchar_serial(void){
  int chr = -1;
  if (IFG2 & UCA0RXIFG) {
    chr = UCA0RXBUF;
  }
  return chr;
}


void print_menu_serial(void){
  print_string_serial("\nSelect one of the options above:\n");
  print_string_serial("1- Turn on the coffe maker.\n");
  print_string_serial("2- Turn off the coffe maker.\n");
  print_string_serial("3- Set time to wake.\n");
  print_string_serial("4- Reset timer.\n");
}

void print_welcome_serial(void){
  print_string_serial("*-------------------------------------------*\n");
  print_string_serial("*---------------Good Morning!---------------*\n");
  print_string_serial("*-----Welcome to the auto coffe system------*\n");
  print_string_serial("*-------------------------------------------*\n");
}

int main(){
  int cont_on = 0, cont_off = 0;
  WDTCTL = WDTPW + WDTHOLD;

  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  P1DIR |= BIT0;
  TA0CCR0 = 62500-1;
  TA0CTL = TASSEL_2 + ID_3 + MC_3 + TAIE;
  TACCTL0 |= CCIE;

  P1SEL2 = P1SEL = TX+RX;
  P1DIR = COFFEE + LED1;

  P1OUT = 0;

  UCA0CTL0 = 0;
  UCA0CTL1 = UCSSEL_2;
  UCA0BR0 = 6;
  UCA0BR1 = 0;

  UCA0MCTL = UCBRF_8 + UCOS16;

  IE2 |= UCA0RXIE;

  print_welcome_serial();
  print_menu_serial();


  _BIS_SR(LPM0_bits+GIE);

  while(1){
    print_menu_serial();

    _BIS_SR(LPM0_bits+GIE);
    switch(status_coffe){
    	case '1':
        if(cont_on == 0){
          print_string_serial("\n\nMaking Coffee!\n\n");
      	  P1OUT |= COFFEE;
          cont_off = 0;
          cont_on = 1;
        }
        else{
          print_string_serial("\n\nCoffee pot is alredy on! The coffee is heating!\n\n");
        }
        break;

      	case '2':
          if(cont_off == 0){
            print_string_serial("\n\nCoffee pot is off!\n\n");
            P1OUT &= 0;
            cont_on = 0;
            cont_off = 1;
          }
          else{
            print_string_serial("\n\nCoffee pot is already off!\n\n");
          }
      		break;

        case '3':
          print_string_serial("\n\nInsert the time to wake: ");
                print_string_serial("\nHours: \n");
                while ((IFG2 & UCA0RXIFG) == 0);
                hours = getchar_serial();
                hours = status_coffe;
                UCA0TXBUF = hours;
                print_string_serial(" Hours\n");


                print_string_serial("Minutes: \n");
                while ((IFG2 & UCA0RXIFG) == 0);
                minutes = getchar_serial();
                minutes = status_coffe;
                UCA0TXBUF = minutes;
                print_string_serial(" Minutes\n");

                print_string_serial("Seconds: \n");
                while ((IFG2 & UCA0RXIFG) == 0);
                seconds = getchar_serial();
                seconds = status_coffe;
                UCA0TXBUF = seconds;
                print_string_serial(" Seconds\n");

          print_string_serial("\n\nTimer has been set!\n\n");
          break;

        case '4':
          hours = -1;
          minutes = -1;
          seconds = -1;
          print_string_serial("\n\nTimer has been reseted!\n\n");
          break;

      	default:
      		print_string_serial("\n\nInsert a valid option!\n\n");
        }
       }
    return 0;
}

interrupt(USCIAB0RX_VECTOR) Receiver(void){
    status_coffe = getchar_serial();

    LPM0_EXIT;
}



interrupt(TIMER0_A1_VECTOR) TA0_ISR(void){

  P1OUT ^= LED1;

  if(TA_seconds > 59){

    TA_seconds = 0;

    TA_minutes++;


    if(TA_minutes > 59){

      TA_minutes = 0;

      TA_hours++;



      if(TA_hours > 23){

        TA_hours = 0;

      }

      else;

    }

    else;

  }

  else{

    TA_seconds++;

  }

  if(hours == TA_hours && minutes == TA_minutes && seconds == TA_seconds){

    P1OUT |= COFFEE;

  }

  TA0CTL &= ~TAIFG;

}

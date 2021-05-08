/* ************************************************************************
 *
 *   TTL serial interface (hardware USART)
 *
 *   (c) 2018 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - port and pins for bit-bang serial (software USART)
 *    SERIAL_PORT   port data register
 *    SERIAL_DDR    port data direction register
 *    SERIAL_PIN    port input pins register
 *    SERIAL_TX     pin for Tx (transmit)
 *    SERIAL_RX     pin for Rx (receive) - not supported
 *  - For hardware USART the MCU specific pins are used:
 *    ATmega 328:        RxD PD0 / TxD PD1
 *    ATmega 644: USART0 RxD PD0 / TxD PD1
 *                USART1 RxD PD2 / TxD PD3
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_SERIAL


/*
 *  local constants
 */

/* source management */
#define SERIAL_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  local constants
 */

/* registers and their bits based on USART ID */
#if SERIAL_USART == 0
  /* registers for USART0 */
  #define REG_UDR        UDR0

  #define REG_UCSR_A     UCSR0A
  #define BIT_RXC        RXC0
  #define BIT_TXC        TXC0
  #define BIT_UDRE       UDRE0
  #define BIT_FE         FE0
  #define BIT_DOR        DOR0
  #define BIT_UPE        UPE0

  #define REG_UCSR_B     UCSR0B
  #define BIT_RXEN       RXEN0
  #define BIT_TXEN       TXEN0
  #define BIT_UCSZ_2     UCSZ02

  #define REG_UCSR_C     UCSR0C
  #define BIT_UMSEL_0    UMSEL00
  #define BIT_UMSEL_1    UMSEL01
  #define BIT_UPM_0      UPM00
  #define BIT_UPM_1      UPM01
  #define BIT_USBS       USBS0
  #define BIT_UCSZ_0     UCSZ00
  #define BIT_UCSZ_1     UCSZ01

  #define REG_UBRR_L     UBRR0L
  #define REG_UBRR_H     UBRR0H
  #define REG_UBRR       UBRR0
#endif

#if SERIAL_USART == 1
  /* registers for USART1 */
  #define REG_UDR        UDR1

  #define REG_UCSR_A     UCSR1A
  #define BIT_RXC        RXC1
  #define BIT_TXC        TXC1
  #define BIT_UDRE       UDRE1
  #define BIT_FE         FE1
  #define BIT_DOR        DOR1
  #define BIT_UPE        UPE1

  #define REG_UCSR_B     UCSR1B
  #define BIT_RXEN       RXEN1
  #define BIT_TXEN       TXEN1
  #define BIT_UCSZ_2     UCSZ12

  #define REG_UCSR_C     UCSR1C
  #define BIT_UMSEL_0    UMSEL10
  #define BIT_UMSEL_1    UMSEL11
  #define BIT_UPM_0      UPM10
  #define BIT_UPM_1      UPM11
  #define BIT_USBS       USBS1
  #define BIT_UCSZ_0     UCSZ10
  #define BIT_UCSZ_1     UCSZ11

  #define REG_UBRR_L     UBRR1L
  #define REG_UBRR_H     UBRR1H
  #define REG_UBRR       UBRR1
#endif



/* ************************************************************************
 *   functions for software USART (bit-banging)
 * ************************************************************************ */


#ifdef SERIAL_BITBANG

/*
 *  set up serial interface
 *  - Tx line (Rx not supported yet)
 */

void Serial_Setup(void)
{
  /* set Tx to output mode */
  SERIAL_DDR |= (1 << SERIAL_TX);

  /* preset Tx to idle state (high) */
  SERIAL_PORT |= (1 << SERIAL_TX);
}



/*
 *  send byte
 *  - 9600 8N1
 *
 *  requires:
 *  - Byte: byte to send
 */

void Serial_WriteByte(uint8_t Byte)
{
  uint8_t           n = 8;    /* bit counter */

  /* R_PORT & R_DDR / ADC_PORT & ADC_DDR can interfere (input/HiZ) */
  /* idea: add external pull-up resistor to Tx to keep Tx high while set to HiZ */
  Serial_Setup();        /* quick and dirty */
  /* todo: find better solution? */

  /* start bit (0) */
  SERIAL_PORT &= ~(1 << SERIAL_TX);     /* clear Tx */
  wait100us();                          /* delay for 9600bps */
  wait3us();

  /* 8 data bits (LSB first) */
  while (n > 0)
  {
    if (Byte & 0b00000001)    /* 1 */
    {
      SERIAL_PORT |= (1 << SERIAL_TX);  /* set Tx */
    }
    else                      /* 0 */
    {
      SERIAL_PORT &= ~(1 << SERIAL_TX); /* clear Tx */
    }

    wait100us();              /* delay for 9600bps */
    wait3us();

    Byte >>= 1;               /* shift right */
    n--;                      /* next bit */
  }

  /* 1 stop bit (1) and stay idle (high) */
  SERIAL_PORT |= (1 << SERIAL_TX);      /* set Tx */
  wait100us();                          /* delay for 9600bps */
  wait3us();
}



#ifdef SERIAL_RW

/*
 *  receive byte
 */

#endif

#endif



/* ************************************************************************
 *   functions for hardware USART
 * ************************************************************************ */


#ifdef SERIAL_HARDWARE

/*
 *  set up serial interface
 *  - 9600 8N1
 *  - Tx line (Rx not supported yet)
 */

void Serial_Setup(void)
{
  /*
   *  asynchronous normal mode
   *  - prescaler = (f_MCU / (16 * bps)) - 1
   *  - 2400bps, 8 data bits, no parity, 1 stop bit
   *  - overrides normal port operation of Tx pin
   */

  REG_UBRR = (CPU_FREQ / (16UL * 9600)) - 1;
  REG_UCSR_C = (1 << BIT_UCSZ_1) | (1 << BIT_UCSZ_0);
  REG_UCSR_B = (1 << BIT_TXEN);
}



/*
 *  send byte
 *
 *  requires:
 *  - Byte: byte to send
 */

void Serial_WriteByte(uint8_t Byte)
{
  /* wait for empty Tx buffer */
  while (! (REG_UCSR_A & (1 << BIT_UDRE)));

  /* clear USART Transmit Complete flag */
  //REG_UCSR_A = (1 << BIT_TXC);

  /* copy byte to Tx buffer, triggers sending */
  REG_UDR = Byte;
}



#ifdef SERIAL_RW

/*
 *  receive byte
 */

#endif

#endif



/* ************************************************************************
 *   high level functions for TX
 * ************************************************************************ */


#ifdef UI_SERIAL_COPY

/*
 *  write one char
 *  - converts custom LCD characters to standard ones
 *
 *  requires:
 *  - Char: character to output
 */

void Serial_Char(unsigned char Char)
{
  switch (Char)
  {
    case LCD_CHAR_DIODE_AC:        /* diode icon '>|' */
      Serial_WriteByte('>');
      Serial_WriteByte('|');
      break;

    case LCD_CHAR_DIODE_CA:        /* diode icon '|<' */
      Serial_WriteByte('|');
      Serial_WriteByte('<');
      break;

    case LCD_CHAR_CAP:             /* capacitor icon '||' */
      Serial_WriteByte('|');
      Serial_WriteByte('|');
      break;

    case LCD_CHAR_OMEGA:           /* omega */
      Serial_WriteByte('R');
      break;

    case LCD_CHAR_MICRO:           /* µ / micro */
      Serial_WriteByte('u');
      break;

    case LCD_CHAR_RESISTOR_L:      /* resistor left icon '[' */
      Serial_WriteByte('[');       /* or maybe 'V'? */
      break;

    case LCD_CHAR_RESISTOR_R:      /* resistor right icon ']' */
      Serial_WriteByte(']');       /* or maybe 'V'? */
      break;

    default:                       /* standard char */
      Serial_WriteByte(Char);
  }
}



/*
 *  send carriage return and linefeed
 */

void Serial_NewLine(void)
{
  Serial_WriteByte('\r');          /* send CR */
  Serial_WriteByte('\n');          /* send LF */
}

#endif



#if 0
/*
 *  send a fixed string stored in EEPROM
 *
 *  requires:
 *  - pointer to fixed string
 */

void Serial_EEString(const unsigned char *String)
{
  unsigned char     Char;

  while (1)
  {
    Char = eeprom_read_byte(String);    /* read character */

    /* check for end of string */
    if (Char == 0) break;

    Serial_WriteByte(Char);             /* send character */
    String++;                           /* next one */
  }
}
#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef SERIAL_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

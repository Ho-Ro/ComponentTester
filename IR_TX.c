/* ************************************************************************
 *
 *   IR remote control: sender
 *
 *   (c) 2015-2021 by Markus Reschke
 *
 * ************************************************************************ */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef SW_IR_TRANSMITTER


/*
 *  local constants
 */

/* source management */
#define IR_TX_C


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

/* IR protocols */
/* basic protocols */
#define IR_JVC                    1     /* JVC C8D8 */
#define IR_KASEIKYO               2     /* Kaseikyo (Japanese Code) */
#define IR_MATSUSHITA             3     /* Matsushita (Panasonic) */
#define IR_MOTOROLA               4     /* Motorola */
#define IR_NEC_STD                5     /* NEC standard */
#define IR_NEC_EXT                6     /* NEC extended */
#define IR_PROTON                 7     /* Proton (Mitsubishi) */
#define IR_RC5                    8     /* RC-5 Standard */
#define IR_RC6                    9     /* RC-6 Standard, Mode 0 */
#define IR_SAMSUNG               10     /* Samsung / Toshiba */
#define IR_SHARP                 11     /* Sharp (Denon) */
#define IR_SIRC_12               12     /* Sony SIRC-12 */
#define IR_SIRC_15               13     /* Sony SIRC-15 */
#define IR_SIRC_20               14     /* Sony SIRC-20 */
/* extra protocols */
#define IR_THOMSON               15     /* Thomson */

#ifndef SW_IR_TX_EXTRA
  #define IR_PROTO_MAX           14     /* number of basic protocols */
#else
  #define IR_PROTO_MAX           15     /* number of all protocols */  
#endif

/* code bit mode */
#define IR_LSB                    1     /* LSB */
#define IR_MSB                    2     /* MSB */

/* bi-phase modes (bitfield) */
#define IR_IEEE          0b00000001     /* IEEE bit encoding */
#define IR_THOMAS        0b00000010     /* Thomas bit encoding */
#define IR_PRE_PAUSE     0b00000100     /* heading pause */

/* signal types */
#define IR_PAUSE         0b00000001     /* pause */
#define IR_PULSE         0b00000010     /* pulse */


/*
 *  local variables
 */

/* key toggle feature */
uint8_t             IR_Toggle = 0;           /* key toggle flag */



/* ************************************************************************
 *   IR remote control tool (sender)
 * ************************************************************************ */



/*
 *  send single pause/pulse
 *
 *  requires:
 *  - type: IR_PAUSE or IR_PULSE
 *  - time: duration in 탎
 */

void IR_Send_Pulse(uint8_t Type, uint16_t Time)
{
  if (Type & IR_PULSE)        /* create pulse */
  {
    /* enable output via OC1B pin */
    TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);

    /* start Timer1 for carrier frequency */
    TCNT1 = 0;                     /* set counter to 0 */
    /* enable Timer1 by setting prescaler 1:1 */
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  }


  /*
   *  wait (delay loop)
   *  - loop (without NOPs) burns 7 cycles per run and 4 cycles for last run
   *  - total (without NOPs): 7 cycles * Time + 4 cycles
   *  - add NOPs for 1탎
   *  - don't care about missing cycles of last run
   */

  #ifndef SW_IR_TX_ALTDELAY

  while (Time > 0)
  {
    #if CPU_FREQ == 8000000
      /* add 1 cycle (one cycle is 62.5ns) */
      asm volatile("nop");
    #elif CPU_FREQ == 16000000
      /* add 9 cycles (one cycle is 62.5ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #elif CPU_FREQ == 20000000
      /* add 13 cycles (one cycle is 50ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #else
      #error <<< IR_Send_Pulse(): no supported MCU clock >>>
    #endif

    Time--;
  }
  #endif


  /*
   *  alternative delay loop
   *  - provided by Vitaliy
   */

  #ifdef SW_IR_TX_ALTDELAY

  #include <util/delay.h>

  while (Time >= 100)         /* 100탎 chunks */
  {
    _delay_us(100);
    Time -= 100;
  }

  while (Time >= 10)          /* 10탎 chunks */
  {
    _delay_us(10);
    Time -= 10;
  }

  while (Time > 0)            /* 1탎 chunks */
  {
    _delay_us(1);
    Time--;
  }
  #endif


  if (Type & IR_PULSE)        /* create pulse */
  {
    /* stop Timer1 */
    TCCR1B = (1 << WGM13) | (1 << WGM12);    /* clear prescaler */

    /* disable output via OC1B pin */
    TCCR1A = (1 << WGM11) | (1 << WGM10);
  }
}



/*
 *  send IR code using Bi-Phase modulation
 *  - similar to Manchester encoding
 *  - special case of BPSK (binary phase-shift keying)
 *
 *  required:
 *  - Code: pointer to code data
 *  - Bits: number of bits in code data
 *  - Mode:
 *    IR_IEEE   - IEEE
 *    IR_THOMAS - G.E. Thomas
 *  - tP: time units of pulse/pause in 탎
 */

void IR_Send_BiPhase(uint8_t *Code, uint8_t Bits, uint8_t Mode, uint16_t tP)
{
  uint8_t           Count;         /* bit counter */
  uint8_t           Byte;          /* current code byte */
  uint8_t           One;           /* type of first half for 0 */
  uint8_t           Zero;          /* type of first half for 1 */

  /*
   *  Bi-Phase Modulation:
   *  - fixed time slot for each bit
   *  - bit is encoded as a phase shift (H-L or L-H transition)
   *  - two encoding conventions
   *    - G.E. Thomas (Manchester II or Biphase-L)
   *      0 -> pause - pulse
   *      1 -> pulse - pause
   *    - IEEE 802.3
   *      0 -> pulse - pause 
   *      1 -> pause - pulse
   */

  if (Mode & IR_IEEE)         /* IEEE */
  {
    Zero = IR_PULSE;          /* 0 starts with pulse */
    One = IR_PAUSE;           /* 1 starts with pause */
  }
  else                        /* Thomas */
  {
    Zero = IR_PAUSE;          /* 0 starts with pause */
    One = IR_PULSE;           /* 1 starts with pulse */
  }

  Count = 0;                  /* reset bit counter */
  Byte = *Code;               /* get first code byte */

  while (Bits > 0)            /* loop through bits */
  {
    if (Byte & 0b10000000)    /* bit is 1 */
    {
      Mode = One;             /* set type of first half for 1 */
    }
    else                      /* bit is 0 */
    {
      Mode = Zero;            /* set type of first half for 0 */
    }

    IR_Send_Pulse(Mode, tP);       /* send first half */
    Mode = ~Mode;                  /* reverse type */
    IR_Send_Pulse(Mode, tP);       /* send second half */

    Byte <<= 1;               /* shift byte one step left */
    Count++;                  /* another bit done */

    if (Count == 8)           /* complete byte done */
    {
      Code++;                 /* next code byte */
      Byte = *Code;           /* get new code byte */
      Count = 0;              /* reset bit counter */
    }

    Bits--;                   /* next bit */
  }
}



/*
 *  send IR code using PWM
 *  - Pulse Width Modulation
 *
 *  required:
 *  - Code: pointer to code data
 *  - Bits: number of bits in code data
 *  - tP: time units of pause in 탎
 *  - t0: time units of pulse for 0 in 탎
 *  - t1: time units of pulse for 1 in 탎
 */

void IR_Send_PWM(uint8_t *Code, uint8_t Bits, uint16_t tP, uint16_t t0, uint16_t t1)
{
  uint8_t           Count;         /* bit counter */
  uint8_t           Byte;          /* current code byte */

  /*
   *  PWM / pulse encoding:
   *  - fixed pause time
   *  - two different pulse times to encode 0/1
   */

  Count = 0;                  /* reset bit counter */
  Byte = *Code;               /* get first code byte */

  while (Bits > 0)            /* loop through bits */
  {
    IR_Send_Pulse(IR_PAUSE, tP);        /* send pause */

    if (Byte & 0b10000000)    /* bit is 1 */
    {
      IR_Send_Pulse(IR_PULSE, t1);      /* send pulse for 1 */
    }
    else                      /* bit is 0 */
    {
      IR_Send_Pulse(IR_PULSE, t0);      /* send pulse for 0 */
    }

    Byte <<= 1;               /* shift byte one step left */
    Count++;                  /* another bit done */

    if (Count == 8)           /* complete byte done */
    {
      Code++;                 /* next code byte */
      Byte = *Code;           /* get new code byte */
      Count = 0;              /* reset bit counter */
    }

    Bits--;                   /* next bit */
  }
}



/*
 *  send IR code using PDM
 *  - Pulse Distance Modulation
 *
 *  required:
 *  - Code: pointer to code data
 *  - Bits: number of bits in code data
 *  - tP: time units of pulse in 탎
 *  - t0: time units of pause for 0 in 탎
 *  - t1: time units of pause for 1 in 탎
 */

void IR_Send_PDM(uint8_t *Code, uint8_t Bits, uint16_t tP, uint16_t t0, uint16_t t1)
{
  uint8_t           Count;         /* bit counter */
  uint8_t           Byte;          /* current code byte */

  /*
   *  PDM / space encoding:
   *  - fixed pulse time
   *  - two different pause times to encode 0/1
   *  - last item is pulse (stop bit)
   *    required to indicate end of pause of last data bit
   */

  Count = 0;                  /* reset bit counter */
  Byte = *Code;               /* get first code byte */

  while (Bits > 0)            /* loop through bits */
  {
    IR_Send_Pulse(IR_PULSE, tP);        /* send pulse */

    if (Byte & 0b10000000)    /* bit is 1 */
    {
      IR_Send_Pulse(IR_PAUSE, t1);      /* send pause for 1 */
    }
    else                      /* bit is 0 */
    {
      IR_Send_Pulse(IR_PAUSE, t0);      /* send pause for 0 */
    }

    Byte <<= 1;               /* shift byte one step left */
    Count++;                  /* another bit done */

    if (Count == 8)           /* complete byte done */
    {
      Code++;                 /* next code byte */
      Byte = *Code;           /* get new code byte */
      Count = 0;              /* reset bit counter */
    }

    Bits--;                   /* next bit */
  }

  /* send a stop pulse (to signal end of last pause) */
  IR_Send_Pulse(IR_PULSE, tP);
}




/*
 *  get code timing for PDM/PWM
 *  - max. time is 65ms
 *
 *  required:
 *  - Code: pointer to code data
 *  - Bits: number of bits in code data
 *  - tP: time units of fixed pulse/pause in 탎
 *  - t0: time units of pulse/pause for 0 in 탎
 *  - t1: time units of pulse/pause for 1 in 탎
 *
 *  returns:
 *  - time in ms
 */

uint16_t CodeTime(uint8_t *Code, uint8_t Bits, uint16_t tP, uint16_t t0, uint16_t t1)
{
  uint16_t          Time = 0;      /* time */
  uint8_t           Count;         /* bit counter */
  uint8_t           Byte;          /* current code byte */

  Count = 0;                  /* reset bit counter */
  Byte = *Code;               /* get first code byte */

  while (Bits > 0)            /* loop through bits */
  {
    Time += tP;               /* add time for fixed pulse/pause */

    if (Byte & 0b10000000)    /* bit is 1 */
    {
      Time += t1;             /* add time for 1 */
    }
    else                      /* bit is 0 */
    {
      Time += t0;             /* add time for 0 */
    }

    Byte <<= 1;               /* shift byte one step left */
    Count++;                  /* another bit done */

    if (Count == 8)           /* complete byte done */
    {
      Code++;                 /* next code byte */
      Byte = *Code;           /* get new code byte */
      Count = 0;              /* reset bit counter */
    }

    Bits--;                   /* next bit */
  }

  /* todo: for PDM we should add the end pulse */
  return Time; 
}



/*
 *  put bits into IR Code
 *
 *  required:
 *  - Data: source data
 *  - Bits: number of bits to copy (1-16)
 *  - StartBit: start bit position in IR Code (1-?)
 *  - Mode: bit mode
 *    IR_LSB - LSB
 *    IR_MSB - MSB
 */

void PutBits(uint16_t Data, uint8_t Bits, uint8_t StartBit, uint8_t Mode)
{
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Count;         /* bit position in code byte */
  uint8_t           Byte;          /* current code byte */
  uint8_t           Mask;          /* bitmask */
  uint8_t           n;             /* counter */

  /*
   *  IR_Code: bits stored in sending order (bit #7 is first bit to be sent)
   *           IR_Code[0] is first byte 
   *  LSB: bit #0 of Data goes to bit #7 of IR_Code
   *       bit #7 of Data goes to bit #0 of IR_Code
   *  MSB: bit #7 of Data goes to bit #7 of IR_Code
   *       bit #0 of Data goes to bit #0 of IR_Code
   */

  /* determine start position in IR_Code */
  StartBit--;                      /* align */
  n = StartBit / 8;                /* start byte (0-) */
  Count = StartBit % 8;            /* start bit in byte (0-7) */
  Code = &IR_Code[n];              /* get start address */
  Byte = *Code;                    /* get current code byte */

  /* prepare Data for MSB */
  if (Mode == IR_MSB)              /* MSB mode */
  {
    n = 16 - Bits;                 /* unused bits */
    Data <<= n;                    /* shift to left */
  }

  /* prepare mask */
  Mask = 0b10000000;               /* reset bitmask */
  Mask >>= Count;                  /* shift 1 to position of start bit */

  /* processing loop */
  while (Bits > 0)                 /* loop through bits */
  {
    if (Mode == IR_LSB)            /* LSB mode */
    {
      if (Data & 0b0000000000000001)    /* bit set in data */
      {
        Byte |= Mask;                   /* set bit in code byte */
      }
      else                              /* bit not set in data */
      {
        Byte &= ~Mask;                  /* clear bit in code byte */
      }

      Data >>= 1;                       /* shift one bit right */
    }
    else                           /* MSB mode */
    {
      if (Data & 0b1000000000000000)    /* bit set in data */
      {
        Byte |= Mask;                   /* set bit in code byte */
      }
      else                              /* bit not set in data */
      {
        Byte &= ~Mask;                  /* clear bit in code byte */
      }

      Data <<= 1;                       /* shift one bit left */
    }

    Mask >>= 1;               /* shift one bit right */
    Count++;                  /* next bit position */
    Bits--;                   /* next bit */

    if ((Count == 8) || (Bits == 0))    /* byte done or last bit */
    {
      *Code = Byte;           /* save current code byte */
      Code++;                 /* next byte in IR_Code[] */
      Byte = *Code;           /* get new code byte */
      Count = 0;              /* start at first bit again */
      Mask = 0b10000000;      /* reset bitmask */
    }
  }
}



/*
 *  send IR code
 *
 *  required:
 *  - Proto: protocol ID
 *  - Data:  pointer to array of data fields 
 */

void IR_Send_Code(uint8_t Proto, uint16_t *Data)
{
  uint16_t          Temp;          /* temporary value */
  uint8_t           n;             /* counter */


  /*
   *  JVC C8D8
   *  - start: pulse 8.44ms, pause 4.22ms
   *  - PDM: pulse 525탎, pause 0=525탎 1=1575탎
   *  - bit mode: LSB
   *  - stop: pulse 525탎
   *  - format: <start><address:8><command:8><stop>
   *  - repeat sequence format: <address:8><command:8><stop>
   *  - repeat sequence delay is 46.42ms (start to start)
   */

  if (Proto == IR_JVC)                  /* JVC */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 8, 1, IR_LSB);        /* address, 8 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 17, IR_LSB);       /* command, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 8440);      /* start */
    IR_Send_Pulse(IR_PAUSE, 4220);
    IR_Send_PDM(&IR_Code[0], 16, 525, 525, 1575);
  }


  /*
   *  Kaseikyo (Japanese Code)
   *  - start: pulse 3456탎, pause 1728탎
   *  - PDM: pulse 432탎, pause 0=432탎 1=1296탎
   *  - bit mode: LSB
   *  - stop: pulse 432탎
   *  - format (48 bits):
   *    <start><manufacturer code:16><parity:4><system:4><product:8><function:8><check:8><stop>
   *  - parity: 0000
   *    ?: <mc 0-3> ^ <mc 4-7> ^ <mc 8-11> ^ <mc 12-15>                 
   *  - check: <system:4><parity:4> ^ <product:8> ^ <function:8>
   *    ?: <system 0-3> + <product 0-3> + <product 4-7> + <function 0-3> + <function 4-7>
   *  - code becomes valid after sending it 2 (or 3) times
   *    code delay is 74.62ms (end to start)
   *  - repeat sequence format: <pulse 3456탎><pause 3456탎><pulse 432탎>
   *  - repeat delay is 42.2ms (end to start)
   */

  else if (Proto == IR_KASEIKYO)        /* Kaseikyo */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: manufacturer code */
    PutBits(Temp, 16, 1, IR_LSB);       /* manufacturer, 16 bits */

    n = 0;                              /* 0000 */
    PutBits(n, 4, 17, IR_LSB);          /* parity, 4 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: system */
    PutBits(Temp, 4, 21, IR_LSB);       /* system, 4 bits */
    n = Temp;                           /* use n for check value */
    n <<= 4;                            /* check: <system><parity> */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #2: product */
    PutBits(Temp, 8, 25, IR_LSB);       /* product, 8 bits */
    n ^= (uint8_t)Temp;                 /* check: ^ <product> */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #2: function */
    PutBits(Temp, 8, 33, IR_LSB);       /* function, 8 bits */
    n ^= (uint8_t)Temp;                 /* check: ^ <function> */

    PutBits(n, 8, 41, IR_LSB);          /* check, 8 bits */

    /* send code three times */
    n = 3;

    while (n > 0)
    {
      IR_Send_Pulse(IR_PULSE, 3456);      /* start */
      IR_Send_Pulse(IR_PAUSE, 1728);
      IR_Send_PDM(&IR_Code[0], 48, 432, 432, 1296);

      MilliSleep(74);         /* delay for next packet */

      n--;                    /* next run */
    }
  }


  /*
   *  Matsushita (Panasonic, MN6014 C6D6)
   *  - start: pulse 3.5ms, pause 3.5ms
   *  - PDM: pulse 872탎, pause 0=872탎 1=2616탎
   *  - bit mode: LSB
   *  - stop: pulse 872탎
   *  - format:
   *    <start><custom code:6><data code:6><inverted custom code:6><inverted data code:6><stop>
   *  - repeat delay is 104.7ms (start to start) or 34ms (end to start)
   */

  else if (Proto == IR_MATSUSHITA)      /* Matsushita */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: custom code */
    PutBits(Temp, 6, 1, IR_LSB);        /* address, 6 bits */
    Temp = ~Temp;                       /* invert address */
    PutBits(Temp, 6, 13, IR_LSB);       /* inverted address, 6 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: key data */
    PutBits(Temp, 6, 7, IR_LSB);        /* command, 6 bits */
    Temp = ~Temp;                       /* invert command */
    PutBits(Temp, 6, 19, IR_LSB);       /* inverted command, 6 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 3500);      /* start */
    IR_Send_Pulse(IR_PAUSE, 3500);
    IR_Send_PDM(&IR_Code[0], 24, 872, 872, 2616);
  }


  /*
   *  Motorola
   *  - start: pulse 512탎, pause 2560탎
   *  - Bi-Phase:
   *    0: pause 512탎, pulse 512탎
   *    1: pulse 512탎, pause 512탎 
   *  - bit mode: LSB
   *  - a valid code consists of a start packet, one or more command packets
   *    and an end packet
   *  - command packets are repeated as long as key is pressed
   *  - start/end packet format: <start><start "1":1><all 1s:9>
   *  - command packet format: <start><start "1":1><command:9>
   *  - packet delay is 32.8ms (start to start) or 19.5ms (end to start) between
   *    command packets and 131ms (start to start) or 117.7ms (end to start) for
   *    start-command and command-end
   */

  else if (Proto == IR_MOTOROLA)        /* Motorola */
  {
    /* build code */
    /* IR_Code[0] for command packet, IR_Code[2] for start/end packet */
    Temp = 0b0000001111111111;
    PutBits(Temp, 10, 17, IR_LSB);      /* all 1s for start/end packet */

    PutBits(Temp, 1, 1, IR_LSB);        /* start bit, 1 bit */

    Temp = *Data;                       /* Data #0: command */
    PutBits(Temp, 9, 2, IR_LSB);        /* command, 9 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 512);       /* start */
    IR_Send_Pulse(IR_PAUSE, 2560);
    IR_Send_BiPhase(&IR_Code[2], 10, IR_THOMAS, 512);  /* start packet */
    MilliSleep(118);                                   /* delay */
    IR_Send_BiPhase(&IR_Code[0], 10, IR_THOMAS, 512);  /* command packet */
    MilliSleep(118);                                   /* delay */
    IR_Send_BiPhase(&IR_Code[2], 10, IR_THOMAS, 512);  /* end packet */ 
  }


  /*
   *  NEC Standard (킦D6121/킦D6122)
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - format:
   *    <start><address:8><inverted address:8><command:8><inverted command:8><stop>
   *  - repeat sequence:
   *    <pulse 9ms><pause 2.25ms><stop>
   *  - repeat delay is 108ms (start to start)
   */

  if (Proto == IR_NEC_STD)              /* NEC Standard */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 8, 1, IR_LSB);        /* address, 8 bits */
    Temp = ~Temp;                       /* invert address */
    PutBits(Temp, 8, 9, IR_LSB);        /* inverted address, 8 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 17, IR_LSB);       /* command, 8 bits */
    Temp = ~Temp;                       /* invert command */
    PutBits(Temp, 8, 25, IR_LSB);       /* inverted command, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 9000);      /* start */
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
  }


  /*
   *  NEC Extended (킦D6121/킦D6122)
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - format:
   *    <start><low address:8><high address:8><command:8><inverted command:8><stop>
   *  - repeat sequence:
   *    <pulse 9ms><pause 2.25ms><stop>
   *  - repeat delay is 108ms (start to start)
   */

  else if (Proto == IR_NEC_EXT)         /* NEC Extended */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 16, 1, IR_LSB);       /* address, 16 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 17, IR_LSB);       /* command, 8 bits */
    Temp = ~Temp;                       /* invert command */
    PutBits(Temp, 8, 25, IR_LSB);       /* inverted command, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 9000);      /* start */
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
  }


  /*
   *  Proton
   *  - also: Mitsubishi (M50560)
   *  - start: pulse 8ms, pause 4ms
   *  - sync/separator between address and command: pause 4ms
   *  - PDM: pulse 500탎, pause 0=500탎 1=1500탎
   *  - bit mode: LSB
   *  - stop: pulse 500탎
   *  - format: <start><address:8><stop><sync><command:8><stop>
   *  - code repeat delay is 60ms (start to start)
   */

  else if (Proto == IR_PROTON)          /* Proton */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 8, 1, IR_LSB);        /* address, 8 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 9, IR_LSB);        /* command, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 8000);           /* start */
    IR_Send_Pulse(IR_PAUSE, 4000);
    IR_Send_PDM(&IR_Code[0], 8, 500, 500, 1500);
    IR_Send_Pulse(IR_PAUSE, 4000);           /* sync */
    IR_Send_PDM(&IR_Code[1], 8, 500, 500, 1500);
  }


  /*
   *  Philips RC-5 Standard 
   *  - 2 start bits: (889탎 L) 889탎 H, 889탎 L (889탎 H)
   *  - Bi-Phase (IEEE 802.3):
   *    0: pulse 889탎, pause 889탎
   *    1: pause 889탎, pulse 889탎
   *  - bit mode: MSB
   *  - format: <s1 "1":1><s2 "1":1><toggle:1><address:5><command:6>
   *  - toggle: inverted each time a key is pressed
   *            stays same when key is still pressed
   *  - repeat delay is 114ms (start to start) or 89ms (end to start)
   */

  else if (Proto == IR_RC5)             /* RC-5 Standard */
  {
    /* build code */
    Temp = 0b0000000000000110;          /* two start bits & cleared toggle bit */
    Temp |= IR_Toggle;                  /* update toggle bit */
    PutBits(Temp, 3, 1, IR_MSB);        /* start & toggle, 3 bits */

    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 5, 4, IR_MSB);        /* address, 5 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 6, 9, IR_MSB);        /* command, 6 bits */

    /* send code */
    IR_Send_BiPhase(&IR_Code[0], 14, IR_IEEE, 889);
  }


  /*
   *  Philips RC-6 Standard (RC6-0-16)
   *  - start: pulse 2664탎, pause 888탎
   *  - Bi-Phase (Thomas):
   *    normal bit 0: pause 444탎, pulse 444탎
   *    normal bit 1: pulse 444탎, pause 444탎
   *    toggle bit 0: pause 888탎, pulse 888탎
   *    toggle bit 1: pulse 888탎, pause 888탎
   *  - bit mode: MSB
   *  - format (Mode 0, 16 bit):
   *    <start><start bit "1":1><mode:3><toggle:1><address:8><command:8>
   *  - mode: 000 for Mode 0
   *  - toggle: inverted each time a key is pressed
   *            stays the same when key is still pressed
   *  - delay between codes is 2.666ms (end to start)
   */

  else if (Proto == IR_RC6)             /* RC-6 Standard */
  {
    /* build code */
    /* IR_Code[0] for start & mode, IR_Code[2] for address & command */
    Temp = 0b0000000000001000;          /* start bit & mode */
    PutBits(Temp, 4, 1, IR_MSB);        /* start & mode, 4 bits */

    PutBits(IR_Toggle, 1, 7, IR_MSB);   /* toggle bit */

    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 8, 17, IR_MSB);       /* address, 8 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 25, IR_MSB);       /* command, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 2664);      /* start */
    IR_Send_Pulse(IR_PAUSE, 888);
    IR_Send_BiPhase(&IR_Code[0], 4, IR_THOMAS, 444);   /* first part */
    IR_Send_BiPhase(&IR_Code[1], 1, IR_THOMAS, 888);   /* toggle bit */
    IR_Send_BiPhase(&IR_Code[2], 16, IR_THOMAS, 444);  /* second part */
  }


  /*
   *  Samsung / Toshiba (TC9012)
   *  - start: pulse 4.5ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - stop: pulse 560탎
   *  - format:
   *    <start><custom:8><copy of custom:8><data:8><inverted data:8><stop>
   *  - repeat sequence format:
   *    <start><inverted bit #0 of custom><stop>
   *  - repeat delay is 108ms (start to start)
   */

  else if (Proto == IR_SAMSUNG)         /* Samsung */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: custom */
    PutBits(Temp, 8, 1, IR_LSB);        /* custom, 8 bits */
    PutBits(Temp, 8, 9, IR_LSB);        /* copy of custom, 8 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: data */
    PutBits(Temp, 8, 17, IR_LSB);       /* data, 8 bits */
    Temp = ~Temp;                       /* invert data */
    PutBits(Temp, 8, 25, IR_LSB);       /* inverted data, 8 bits */

    /* send code */
    IR_Send_Pulse(IR_PULSE, 4500);      /* start */
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
  }


  /*
   *  Sharp (LR3715M)
   *  - also: Denon
   *  - no start / AGC burst
   *  - PDM: pulse 264탎, pause 0=786탎 1=1836탎
   *  - bit mode: LSB
   *  - stop: pulse 320탎
   *  - a valid code consists of two packets (#1 and #2)
   *  - format packet #1:
   *    <address:5><command:8><mask:1><type:1><stop>
   *  - format packet #2:
   *    <address:5><inverted command:8><inverted mask:1><inverted type:1><stop>
   *  - mask: 0, but also seen 1
   *  - type: indicater for packet (0 for first packet, 1 for second packet)
   *  - delay for second packet is 67.5ms (start to start), 
   *    40ms (end to start) could work too
   */

  else if (Proto == IR_SHARP)           /* Sharp */
  {
    /* build code */
    /* IR_Code[0] for packet #1, IR_Code[2] for packet #2 */
    Temp = *Data;                       /* Data #0: address */
    PutBits(Temp, 5, 1, IR_LSB);        /* address, 5 bits */
    PutBits(Temp, 5, 17, IR_LSB);       /* address, 5 bits (packet #2) */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: command */
    PutBits(Temp, 8, 6, IR_LSB);        /* command, 8 bits */
    Temp = ~Temp;                       /* invert command */
    PutBits(Temp, 8, 22, IR_LSB);       /* inverted command, 8 bits (packet #2) */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #2: mask */
    /* bit #1 should be 0 -> use as type bit */
    PutBits(Temp, 2, 14, IR_LSB);       /* mask & type, 2 bits */
    Temp = ~Temp;                       /* invert mask & type */
    PutBits(Temp, 2, 30, IR_LSB);       /* inverted mask & type, 2 bits (packet #2) */

    /* send code */
    IR_Send_PDM(&IR_Code[0], 15, 264, 786, 1836);      /* packet #1 */
    MilliSleep(40);                                    /* delay */
    IR_Send_PDM(&IR_Code[2], 15, 264, 786, 1836);      /* packet #2 */
  }


  /*
   *  Sony SIRC-12
   *  - start: pulse 2.4ms (pause 600탎)
   *  - PWM: pause 600탎, pulse 0=600탎 1=1200탎
   *  - bit mode: LSB
   *  - format: <start><command:7><address:5>
   *  - code becomes valid after sending it 3 times
   *    code delay is 45ms (start to start)
   */

  else if (Proto == IR_SIRC_12)         /* Sony SIRC 12 bits */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: command */
    PutBits(Temp, 7, 1, IR_LSB);        /* command, 7 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: address */
    PutBits(Temp, 5, 8, IR_LSB);        /* address, 5 bits */

    /* calculate delay between code frames */
    Temp = 45000 - 2400;                /* time for start pulse */
    Temp -= CodeTime(&IR_Code[0], 12, 600, 600, 1200);

    /* send code three times */
    n = 3;
    while (n > 0)
    {
      IR_Send_Pulse(IR_PULSE, 2400);    /* start */
      IR_Send_PWM(&IR_Code[0], 12, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);    /* delay for next packet */

      n--;               /* next run */
    }
  }


  /*
   *  Sony SIRC-15
   *  - start: pulse 2.4ms (pause 600탎)
   *  - PWM: pause 600탎, pulse 0=600탎 1=1200탎
   *  - bit mode: LSB
   *  - format: <start><command:7><address:8>
   *  - code becomes valid after sending it 3 times
   *    code delay is 45ms (start to start)
   */

  else if (Proto == IR_SIRC_15)         /* Sony SIRC 15 bits */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: command */
    PutBits(Temp, 7, 1, IR_LSB);        /* command, 7 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: address */
    PutBits(Temp, 8, 8, IR_LSB);        /* address, 8 bits */

    /* calculate delay between code frames */
    Temp = 45000 - 2400;                /* time for start pulse */
    Temp -= CodeTime(&IR_Code[0], 15, 600, 600, 1200);

    /* send code three times */
    n = 3;
    while (n > 0)
    {
      IR_Send_Pulse(IR_PULSE, 2400);    /* start */
      IR_Send_PWM(&IR_Code[0], 15, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);    /* delay for next packet */

      n--;               /* next run */
    }
  }


  /*
   *  Sony SIRC-20
   *  - start: pulse 2.4ms (pause 600탎)
   *  - PWM: pause 600탎, pulse 0=600탎 1=1200탎
   *  - bit mode: LSB
   *  - format: <start><command:7><address:5><extended:8>
   *  - code becomes valid after sending it 3 times
   *    code delay is 45ms (start to start)
   */

  else if (Proto == IR_SIRC_20)         /* Sony SIRC 20 bits */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: command */
    PutBits(Temp, 7, 1, IR_LSB);        /* command, 7 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: address */
    PutBits(Temp, 5, 8, IR_LSB);        /* address, 5 bits */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #2: extended */
    PutBits(Temp, 8, 13, IR_LSB);       /* extended, 8 bits */

    /* calculate delay between code frames */
    Temp = 45000 - 2400;                /* - time for start pulse */
    Temp -= CodeTime(&IR_Code[0], 20, 600, 600, 1200);

    /* send code three times */
    n = 3;
    while (n > 0)
    {
      IR_Send_Pulse(IR_PULSE, 2400);    /* start */
      IR_Send_PWM(&IR_Code[0], 20, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);    /* delay for next packet */

      n--;               /* next run */
    }
  }


  #ifdef SW_IR_TX_EXTRA
  /*
   *  Thomson
   *  - no start / AGC burst
   *  - PDM: pulse 500탎, pause 0:2ms 1:4.5ms
   *  - stop: pulse 500탎
   *  - bit mode: LSB
   *  - a valid code consists of at least 2 packets
   *  - format: <device:4><toggle:1><function:7><stop>
   *  - toggle: inverted each time a key is pressed
   *            stays the same when key is still pressed
   *  - packet repeat delay is 80ms (start to start)
   */

  else if (Proto == IR_THOMSON)         /* Thomson */
  {
    /* build code */
    Temp = *Data;                       /* Data #0: command */
    PutBits(Temp, 4, 1, IR_LSB);        /* device, 4 bits */

    PutBits(IR_Toggle, 1, 5, IR_LSB);   /* toggle bit */

    Data++;                             /* next data field */
    Temp = *Data;                       /* Data #1: function */
    PutBits(Temp, 7, 6, IR_LSB);        /* function, 7 bits */

    /* calculate delay between code frames */
    /* half all times to prevent overflow */
    Temp = 40000 - 250;                 /* - time for stop pulse */
    Temp -= CodeTime(&IR_Code[0], 12, 250, 1000, 2250);
    Temp *= 2;                          /* double for real time */

    /* send code two times */
    n = 2;
    while (n > 0)
    {
      IR_Send_PDM(&IR_Code[0], 12, 500, 2000, 4500);
      IR_Send_Pulse(IR_PAUSE, Temp);    /* delay for next packet */

      n--;               /* next run */
    }
  }
  #endif


  #if 0
  /* debugging */
  LCD_ClearLine(6);
  LCD_CharPos(1, 6);
  Display_HexByte(IR_Code[0]);
  Display_Space();
  Display_HexByte(IR_Code[1]);
  Display_Space();
  Display_HexByte(IR_Code[2]);
  Display_Space();
  Display_HexByte(IR_Code[3]);
  Display_Space();
  #endif
}



/*
 *  send IR remote control codes/signals
 *  - uses probe #2 (OC1B) as output for IR LED
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - requires additional keys (e.g. rotary encoder)
 *    and display with more than ? lines
 *  - requires idle sleep mode to keep timer running when MCU is sleeping
 */

void IR_RemoteControl(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Mode;               /* UI */
  uint8_t           n;                  /* counter */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Proto_ID;           /* protocol ID */
  uint8_t           Carrier;            /* carrier frequency */
  uint8_t           DutyCycle;          /* carrier duty cycle */
  unsigned char     *ProtoStr = NULL;   /* string pointer (EEPROM) */
  uint16_t          Step = 0;           /* step size */
  uint16_t          Temp;               /* temporary value */
  /* data fields for IR code */
  #define FIELDS                  4     /* number of data fields */
  uint16_t          Data[FIELDS];       /* data fields */
  uint8_t           Bits[FIELDS];       /* bit depth of data fields */
  uint8_t           Fields = 0;         /* number of fields used */
  uint16_t          Max = 0;            /* upper limit of field value */
  uint8_t           Field_ID = 0;       /* data index */

  /* local constants for Flag */
  #define RUN_FLAG       0b00000001     /* keep running */
  #define CHANGE_PROTO   0b00000010     /* change protocol */
  #define DISPLAY_PROTO  0b00000100     /* display protocol */
  #define UPDATE_FREQ    0b00001000     /* update carrier frequency */
  #define DISPLAY_DATA   0b00010000     /* display IR data */
  #define SEND_CODE      0b10000000     /* send IR command/code */

  /* local constants for Mode */
  #define MODE_PROTO              1     /* protocol */
  #define MODE_FREQ               2     /* carrier frequency */
  #define MODE_DUTYCYCLE          3     /* carrier duty cycle */
  #define MODE_DATA               4     /* code data */

  ShortCircuit(0);                      /* make sure probes are not shorted */

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: IR sender */
    Display_ColoredEEString_Space(IR_Transmitter_str, COLOR_TITLE);
  #else
    Display_EEString_Space(IR_Transmitter_str);   /* display: IR sender */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* display pinout (1: Gnd / 2: LED / 3: Gnd) */
  Display_NextLine();
  Show_SimplePinout('-', 's', '-');
  TestKey(3000, CHECK_BAT);             /* wait 3s or for key press */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* set up probes: #1 and #3 are signal ground, #2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* set up signal output: dedicated output via OC1B */
  SIGNAL_PORT &= ~(1 << SIGNAL_OUT);    /* low by default */
  SIGNAL_DDR |= (1 << SIGNAL_OUT);      /* enable output */
  #endif


  /*
   *  set up Timer1 for PWM with variable duty cycle (carrier)
   *  - fast PWM mode 
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   *  - f_PWM = f_MCU / (prescaler * (1 + top))
   *  - fixed prescaler: 1:1
   *  - top = (f_MCU / (prescaler * f_PWM)) - 1
   *    range: (2^2 - 1) up to (2^16 - 1)
   */

  /* enable OC1B pin and set timer mode */
  /* TCCR1A is set by IR_Send_Pulse() */
  /* TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1); */
  TCCR1B = (1 << WGM13) | (1 << WGM12);


  /* set start values */
  Proto_ID = IR_NEC_STD;                /* NEC Standard */
  Carrier = 38;                         /* 38 kHz */
  DutyCycle = 3;                        /* 1/3 */
  Mode = MODE_PROTO;                    /* select protocol */
  Flag = RUN_FLAG | CHANGE_PROTO | DISPLAY_PROTO | UPDATE_FREQ | DISPLAY_DATA;


  /*
   *  processing loop
   */

  while (Flag & RUN_FLAG)          /* loop */
  {
    wdt_reset();                   /* reset watchdog */

    /*
     *  update display and settings
     */

    if (Flag & CHANGE_PROTO)       /* change protocol */
    {
      /* init variables based on protocol */
      switch (Proto_ID)
      {
        case IR_JVC:               /* JVC */
          ProtoStr = (unsigned char *)IR_JVC_str;
          Bits[0] = 8;             /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 38;            /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_KASEIKYO:          /* Kaseikyo */
          ProtoStr = (unsigned char *)IR_Kaseikyo_str;
          Bits[0] = 16;            /* manufacturer */
          Bits[1] = 4;             /* system */
          Bits[2] = 8;             /* product */
          Bits[3] = 8;             /* function */
          Fields = 4;              /* 4 data fields */
          Carrier = 37;            /* 36.666kHz (440kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_MATSUSHITA:        /* Matsushita */
          ProtoStr = (unsigned char *)IR_Matsushita_str;
          Bits[0] = 6;             /* address */
          Bits[1] = 6;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 37;            /* 36.666kHz (440kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_MOTOROLA:          /* Motorola */
          ProtoStr = (unsigned char *)IR_Motorola_str;
          Bits[0] = 9;             /* command */
          Fields = 1;              /* 1 data field */
          Carrier = 32;            /* 32kHz (500kHz/16=31.250) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_NEC_STD:           /* NEC Standard */
          ProtoStr = (unsigned char *)IR_NEC_Std_str;
          Bits[0] = 8;             /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 38;            /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_NEC_EXT:           /* NEC Extended */
          ProtoStr = (unsigned char *)IR_NEC_Ext_str;
          Bits[0] = 16;            /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 38;            /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_PROTON:            /* Proton */
          ProtoStr = (unsigned char *)IR_Proton_str;
          Bits[0] = 8;             /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 40;            /* 40kHz (480kHz/12) */
                                   /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_RC5:               /* Philips RC-5 Standard */
          ProtoStr = (unsigned char *)IR_RC5_str;
          Bits[0] = 5;             /* address */
          Bits[1] = 6;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 36;            /* 36kHz (432kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_RC6:               /* Philips RC-6 Standard */
          ProtoStr = (unsigned char *)IR_RC6_str;
          Bits[0] = 8;             /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          Carrier = 36;            /* 36kHz (432kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SAMSUNG:           /* Samsung / Toshiba */
          ProtoStr = (unsigned char *)IR_Samsung_str;
          Bits[0] = 8;             /* custom (address) */
          Bits[1] = 8;             /* data (command) */
          Fields = 2;              /* 2 data fields */
          Carrier = 38;            /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SHARP:             /* Sharp */
          ProtoStr = (unsigned char *)IR_Sharp_str;
          Bits[0] = 5;             /* address */
          Bits[1] = 8;             /* command */
          Bits[2] = 1;             /* mask */
          Fields = 3;              /* 2 data fields */
          Carrier = 38;            /* 38kHz (455kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_12:           /* Sony SIRC 12 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_12_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 5;             /* address */
          Fields = 2;              /* 2 data fields */
          Carrier = 40;            /* 40kHz (480kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_15:           /* Sony SIRC 15 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_15_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 8;             /* address */
          Fields = 2;              /* 2 data fields */
          Carrier = 40;            /* 40kHz (480kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_20:           /* Sony SIRC 20 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_20_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 5;             /* address */
          Bits[2] = 8;             /* extended */
          Fields = 3;              /* 3 data fields */
          Carrier = 40;            /* 40kHz (480kHz/12) */
          DutyCycle = 3;           /* 1/3 */
          break;

        #ifdef SW_IR_TX_EXTRA
        case IR_THOMSON:           /* Thomson */
          ProtoStr = (unsigned char *)IR_Thomson_str;
          Bits[0] = 4;             /* device */
          Bits[1] = 7;             /* function */
          Fields = 2;              /* 2 data fields */
          Carrier = 33;            /* 33kHz */
          DutyCycle = 3;           /* 1/3 */
          break;
        #endif
      }

      /* reset data fields */
      for (n = 0; n < FIELDS; n++) Data[n] = 0;   

      Flag |= UPDATE_FREQ;              /* update carrier & duty cycle */
      Flag &= ~CHANGE_PROTO;            /* clear flag */
    }

    if (Flag & DISPLAY_PROTO)      /* display protocol */
    {
      LCD_ClearLine2();                 /* line #2 */
      MarkItem(MODE_PROTO, Mode);       /* mark mode if selected */
      Display_EEString(ProtoStr);       /* display protocol */

      Flag &= ~DISPLAY_PROTO;           /* clear flag */
    }

    if (Flag & UPDATE_FREQ)        /* update carrier frequency */
    {
      /* todo: remove user selectable carrier frequency? */
      /* just let the protocol set the carrier frequency */ 

      /* display frequency */
      LCD_ClearLine(3);                 /* line #3 */
      LCD_CharPos(1, 3);                /* start of line #3 */
      MarkItem(MODE_FREQ, Mode);        /* mark mode if selected */
      Display_Value(Carrier, 3, 0);     /* display frequency */
      Display_EEString(Hertz_str);      /* display: Hz */

      /* display duty cycle */
      MarkItem(MODE_DUTYCYCLE, Mode);   /* mark mode if selected */
      Display_Char('1');                /* display: 1 */
      Display_Char('/');                /* display: / */
      Display_Char('0' + DutyCycle);    /* display denominator */

      /* calculate top value for Timer1 (carrier) */
      /* top = (f_MCU / (prescaler * f_PWM)) - 1 */
      Temp = CPU_FREQ / 1000;           /* MCU clock in kHz */
      Temp /= Carrier;                  /* / f_PWM (in kHz) */
      Temp--;                           /* -1 */

      /* update Timer1 */
      OCR1A = Temp;                     /* top value for frequency */
      Temp /= DutyCycle;                /* apply duty cycle */
      OCR1B = Temp;                     /* top value for duty cycle */

      Flag &= ~UPDATE_FREQ;             /* clear flag */
    }

    if (Flag & DISPLAY_DATA)       /* display IR data */
    {
      LCD_ClearLine(4);                 /* line #4 */
      LCD_CharPos(1, 4);                /* start of line #4 */

      n = 0;
      while (n < Fields)           /* loop through data fields */
      {
        MarkItem(n + MODE_DATA, Mode);  /* mark mode if selected */

        /* display data field */
        Display_HexValue(Data[n], Bits[n]);

        n++;             /* next field */
      }

      Flag &= ~DISPLAY_DATA;            /* clear flag */
    }


    /*
     *  user feedback
     */

    /* wait for user feedback */
    Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

    if (Mode >= MODE_DATA)              /* code data mode */
    {
      /* consider rotary encoder's turning velocity (1-7) */
      n = UI.KeyStep;                   /* get velocity (1-7) */
      Step = n;
      Step *= n;                        /* n^2 */

      /* get details for data field */
      Field_ID = Mode - MODE_DATA;      /* data index */
      n = Bits[Field_ID];               /* bit depth */

      /* more speed up for large ranges */
      if (n >= 12)
      {
        Step *= Step;                   /* n^4 */
      }

      /* calculate max. value for data field (2^n - 1) */
      Max = 0;
      while (n > 0)           /* loop through bit depth */ 
      {
        Max <<= 1;            /* shift one bit left */
        Max += 1;             /* set lowest bit */
        n--;                  /* next bit */
      }
    }

    /* process user input */
    if (Test == KEY_SHORT)              /* short key press */
    {
      /* switch parameter */
      Mode++;                           /* next one */
      n = (MODE_DATA - 1) + Fields;     /* number of current modes */
      if (Mode > n) Mode = MODE_PROTO;  /* overflow */

      Flag |= DISPLAY_PROTO | UPDATE_FREQ | DISPLAY_DATA;   /* update display */
    }
    else if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    else if (Test == KEY_LONG)          /* long key press */
    {
      Flag |= SEND_CODE;                /* set flag to send code */
    }
    else if (Test == KEY_RIGHT)         /* right key */
    {
      if (Mode == MODE_PROTO)           /* protocol mode */
      {
        Proto_ID++;                     /* next one */
        if (Proto_ID > IR_PROTO_MAX)    /* overflow */
        {
          Proto_ID = 1;                 /* reset to first one */
        }

        Flag |= CHANGE_PROTO | DISPLAY_PROTO | DISPLAY_DATA;
      }
      else if (Mode == MODE_FREQ)       /* carrier frequency mode */
      {
        Carrier++;                      /* + 1kHz */
        if (Carrier > 56)               /* overflow */
        {
          Carrier = 30;                 /* reset to 30kHz */
        }

        Flag |= UPDATE_FREQ;
      }
      else if (Mode == MODE_DUTYCYCLE)  /* carrier duty cycle mode */
      {
        DutyCycle++;                    /* next one */
        if (DutyCycle > 4)              /* overflow */
        {
          DutyCycle = 2;                /* reset to 1/2 (50%) */
        }

        Flag |= UPDATE_FREQ;
      }
      else                              /* code data mode */
      {
        Temp = Max - Data[Field_ID];
        if (Temp > Step)                /* within range */
        {
          Data[Field_ID] += Step;       /* add step */
        }
        else                            /* overflow */
        {
          Data[Field_ID] = Max;         /* set max */
        }

        Flag |= DISPLAY_DATA;
      }
    }
    else if (Test == KEY_LEFT)          /* left key */
    {
      if (Mode == MODE_PROTO)           /* protocol mode */
      {
        Proto_ID--;                     /* previous one */
        if (Proto_ID == 0)              /* underflow */
        {
          Proto_ID = IR_PROTO_MAX;      /* reset to last one */
        }

        Flag |= CHANGE_PROTO | DISPLAY_PROTO | DISPLAY_DATA;
      }
      else if (Mode == MODE_FREQ)       /* carrier frequency mode */
      {
        if (Carrier > 30)               /* within range */
        {
          Carrier--;                    /* - 1kHz */
        }
        else                            /* underflow */
        {
          Carrier = 56;                 /* reset to 56kHz */
        }

        Flag |= UPDATE_FREQ;
      }
      else if (Mode == MODE_DUTYCYCLE)  /* carrier duty cycle mode */
      {
        DutyCycle--;                    /* previous one */
        if (DutyCycle < 2)              /* underflow */
        {
          DutyCycle = 4;                /* reset to 1/4 (25%) */
        }

        Flag |= UPDATE_FREQ;
      }
      else                              /* code data mode */
      {
        if (Data[Field_ID] > Step)      /* within range */
        {
          Data[Field_ID] -= Step;       /* substract step */
        }
        else                            /* underflow */
        {
          Data[Field_ID] = 0;           /* set min */
        }

        Flag |= DISPLAY_DATA;
      }
    }


    /*
     *  send IR Code
     */

    if (Flag & SEND_CODE)     /* send IR code */
    {
      n = 1;

      while (n > 0)                /* send/repeat */
      {
        /* send code */
        LCD_CharPos(1, 5);              /* line #5 */
        Display_EEString(IR_Send_str);  /* display: sending... */

        IR_Send_Code(Proto_ID, &Data[0]);

        LCD_ClearLine(5);               /* clear line #5 */
        n = 0;

        IR_Toggle ^= 0b00000001;        /* toggle flag */

        /* check if we should repeat sending */
        Test = TestKey(100, CHECK_BAT); /* get user feedback */
        if (Test == KEY_LONG)           /* long key press */
        {
          n = 2;                        /* repeat code */
        }

        /* smooth UI or delay repeated code */
        MilliSleep(200);           /* take a short nap */
      }

      Flag &= ~SEND_CODE;          /* clear flag */
    }
  }


  /* clean up */
  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  R_DDR = 0;                            /* set HiZ mode */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  SIGNAL_DDR &= ~(1 << SIGNAL_OUT);     /* set HiZ mode */
  #endif

  /* clean up local constants */
  #undef FIELDS

  #undef RUN_FLAG
  #undef CHANGE_PROTO
  #undef DISPLAY_PROTO
  #undef UPDATE_FREQ
  #undef DISPLAY_DATA
  #undef SEND_CODE

  #undef MODE_PROTO
  #undef MODE_FREQ
  #undef MODE_DUTYCYCLE
  #undef MODE_DATA
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef IR_TX_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

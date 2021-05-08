/* ************************************************************************
 *
 *   IR remote control functions
 *
 *   (c) 2015-2017 by Markus Reschke
 *
 * ************************************************************************ */


/* local includes */
#include "config.h"           /* global configuration */

#if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)


/*
 *  local constants
 */

/* source management */
#define IR_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  local defines
 */

/* important constants */
#define IR_SAMPLE_PERIOD         50     /* 50 탎 */
#define IR_CODE_BYTES             6     /* 6 bytes = 48 bit */

/* code bit mode */
#define IR_LSB                    1     /* LSB */
#define IR_MSB                    2     /* MSB */

/* bi-phase modes (bitmask) */
#define IR_IEEE          0b00000001     /* IEEE bit encoding */
#define IR_THOMAS        0b00000010     /* Thomas bit encoding */
#define IR_PRE_PAUSE     0b00000100     /* heading pause */

/* timing control flags (bitmask) */
#define IR_RELAX_SHORT   0b00000001     /* relax short pulses */
#define IR_RELAX_LONG    0b00000010     /* relax long pulses */



/*
 *  local variables
 */

/* demodulated IR code */
uint8_t             IR_Code[IR_CODE_BYTES];  /* raw data */

/* decoding logic */
uint8_t             IR_State = 0;            /* multi packet protocol state */
uint8_t             IR_RelaxTime = 0;        /* timing control flag */



/* ************************************************************************
 *   IR detection tool
 * ************************************************************************ */


/*
 *  display singe hex digit
 *
 *  requires:
 *  - value to display (0-15)
 */

void DisplayHexDigit(uint8_t Digit)
{
  /*
   *  0-9: ascii 48-57
   *  A-F: ascii 65-70
   */

  if (Digit < 10) Digit += 48;     /* 0-9 */
  else Digit += (65 - 10);         /* A-F */
  LCD_Char(Digit);  
}



/*
 *  display byte as hex
 *
 *  requires:
 *  - value to display
 */

void DisplayHex(uint8_t Value)
{
  uint8_t           Digit;

  /* first digit */
  Digit = Value / 16;
  DisplayHexDigit(Digit);

  /* second digit */
  Digit = Value % 16;
  DisplayHexDigit(Digit);
}



/*
 *  check if pulse duration matches reference value
 *
 *  requires:
 *  - time units of pause/pulse
 *  - time units of reference
 *  returns:
 *  - 0 for no match
 *  - 1 for match
 */

uint8_t PulseCheck(uint8_t PulseWidth, uint8_t Ref)
{
  uint8_t           Flag;          /* return value */
  uint8_t           Ref2;          /* reference value */

  /* define tolerance based on value */
  if (Ref > 10)          /* long pulse */
  {
    Flag = 3;                      /* default: 3 units */
  }
  else                   /* short pulse */
  {
    Flag = 1;                      /* default: 1 unit */

    if (IR_RelaxTime & IR_RELAX_SHORT)  /* relax timing */
    {
      Flag = 3;                    /* increase to 3 units */
    }

    if (Flag > Ref) Flag = Ref;    /* prevent underflow */
  }

  Ref2 = Ref;
  Ref2 += Flag;               /* upper limit */
  Ref -= Flag;                /* lower limit */

  Flag = 0;                   /* reset flag */

  /* check if pulse duration is within allowed time window */
  if ((PulseWidth >= Ref) && (PulseWidth <= Ref2))
  {
    Flag = 1;
  }

  return Flag;
}



/*
 *  Bi-Phase demodulation
 *
 *  requires:
 *  - pointer to pulse duration data
 *    first item has to be a pulse
 *  - number of pulses
 *  - mode: Thomas or IEEE, heading pause
 *  - time units of clock half cycle
 *  returns:
 *  - 0 for any error
 *  - number of bits
 */

uint8_t BiPhase_Demod(uint8_t *PulseWidth, uint8_t Pulses, uint8_t Mode, uint8_t Clock)
{
  uint8_t           Flag = 1;      /* return value & control flag */
  uint8_t           Counter = 1;   /* pulse counter */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Width;         /* 1x / 2x pulse width */
  uint8_t           Dir;           /* direction of pulse change */
  uint8_t           PrePulse = 0;  /* pulse for half-bit */
  uint8_t           Data = 0;      /* code byte */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Bits = 0;      /* counter */
  uint8_t           Bytes = 0;     /* counter */

  Code = &IR_Code[0];               /* set start address */

  /*
   *  Bi-Phase Modulation / Manchester encoding
   *  - G.E. Thomas
   *    0: pause pulse
   *    1: pulse pause
   *  - IEEE 802.3
   *    0: pulse pause 
   *    1: pause pulse
   */

  /* take care about heading pause */
  if (Mode & IR_PRE_PAUSE) PrePulse = 1;

  while (Counter <= Pulses)   /* process all data items */
  {
    Time = *PulseWidth;       /* get duration */
    Dir = 0;                  /* reset pulse change direction */
    Width = 1;                /* reset pulse width */

    /* check for pulse duration */
    if (PulseCheck(Time, Clock))             /* half clock cycle */
    {
      /* do nothing */
    }
    else if (PulseCheck(Time, Clock * 2))    /* full clock cycle */
    {
      Width++;                /* double pulse */
    }
    else
    {
      Flag = 0;               /* signal error */
    }

    if (Counter % 2 == 0)     /* pause */
    {
      if (PrePulse == 2)      /* prior pulse */
      {
        Dir = 1;              /* H/L change */

        if (Width == 1)       /* second half of clock cycle */
        {
          PrePulse = 0;       /* clock cycle done */
        }
        else                  /* full clock cycle */
        {
          PrePulse = 1;       /* first half of the next clock cycle */
        }
      }
      else                    /* new clock cycle */
      {
        if (Width == 1)       /* first half of clock cycle */
        {
          PrePulse = 1;       /* first half of clock cycle */
        }
        else                  /* full clock cycle */
        {
          Flag = 0;           /* signal error */
        }
      }
    }
    else                      /* pulse */
    {
      if (PrePulse == 1)      /* prior pause */
      {
        Dir = 2;              /* L/H change */

        if (Width == 1)       /* second half of clock cycle */
        {
          PrePulse = 0;       /* clock cycle done */
        }
        else                  /* full clock cycle */
        {
          PrePulse = 2;       /* first half of the next clock cycle */
        }
      }
      else                    /* new clock cycle */
      {
        if (Width == 1)       /* first half of clock cycle */
        {
          PrePulse = 2;       /* first half of clock cycle */
        }
        else                  /* full clock cycle */
        {
          Flag = 0;           /* signal error */
        }
      }
    }

    /* process pulse change */
    if (Dir)                  /* got a pulse change */
    {
      Bits++;                      /* got another bit */
      Data <<= 1;                  /* shift left for new bit */

      if (Mode & IR_THOMAS)        /* Thomas */
      {
        if (Dir == 1)              /* H/L is 1 */
        {
          Data |= 0b00000001;      /* set bit */
        }
      }
      else                         /* IEEE */
      {
        if (Dir == 2)              /* L/H is 1 */
        {
          Data |= 0b00000001;      /* set bit */
        }
      }

      if (Bits == 8)          /* got a byte */
      {
        *Code = Data;         /* save code byte */
        Bytes++;              /* got another byte */
        Data = 0;             /* reset code byte */
        Bits = 0;             /* reset bit counter */

        if (Bytes < IR_CODE_BYTES)      /* prevent overflow */
        {
          Code++;             /* next code byte */
        }
      }
    }

    /* special case: missing pause at end */
    if (Counter == Pulses)       /* last pause/pulse */
    {
      /* only for pulse */
      if (PrePulse == 2)         /* first half of cycle */
      {
        /* assume that pause follows and simulate it */
        *PulseWidth = Clock;     /* half cycle */
        PulseWidth--;            /* stay here */
        Pulses++;                /* add trailing pause */
      }
    }

    if (Flag == 0)            /* error */
    {
      Pulses = 0;             /* end loop */
    }

    PulseWidth++;             /* next one */
    Counter++;                /* next one */
  }

  /* check result */
  if (Flag)                   /* success */
  {
    Flag = Bytes * 8;         /* return number of bits */
    Flag += Bits;
  }

  /* shift remaining bits to left end of current byte */
  if (Bits > 0)                 /* some bits left */
  {
    while (Bits < 8)
    {
      Data <<= 1;               /* shift left */
      Bits++;                   /* next bit */
    }

    *Code = Data;               /* save code byte */
  }

  return Flag;
}



/*
 *  PDM/PWM demodulation
 *
 *  requires:
 *  - pointer to pulse duration data
 *    first item has to be a pulse for PDM or a pause for PWM
 *  - number of pulses (pauses/pulses)
 *  - time units of spacer
 *  - time units of 0
 *  - time units of 1
 *  returns:
 *  - 0 for any error
 *  - number of bits
 */

uint8_t PxM_Demod(uint8_t *PulseWidth, uint8_t Pulses, uint8_t tS, uint8_t t0, uint8_t t1)
{
  uint8_t           Flag = 1;      /* return value & control flag */
  uint8_t           Counter = 1;   /* pulse counter */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Data = 0;      /* code byte */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Bits = 0;      /* counter */
  uint8_t           Bytes = 0;     /* counter */

  Code = &IR_Code[0];              /* set start address */

  /*
   *  PWM / pulse encoding:
   *  - fixed pause time
   *  - two pulse times to encode 0/1
   *  - even number of data items
   *
   *  PDM / space encoding
   *  - fixed pulse time
   *  - two pause times to encode 0/1
   *  - last item is pulse
   *  - odd number of data items
   */

  while (Counter <= Pulses)   /* process all data items */
  {
    Time = *PulseWidth;       /* get duration */

    if (Counter % 2 == 0)     /* item with variable time */
    {
      Bits++;                           /* got another bit */
      Data <<= 1;                       /* shift left for new bit */

      if (PulseCheck(Time, t0))         /* 0 */
      {
        /* set 0 ;) */
      }
      else if (PulseCheck(Time, t1))    /* 1 */
      {
        Data |= 0b00000001;             /* set bit */
      }
      else                              /* invalid pulse */
      {
        Flag = 0;                       /* signal error */
      }

      if (Bits == 8)          /* got a byte */
      {
        *Code = Data;         /* save code byte */
        Bytes++;              /* got another byte */
        Data = 0;             /* reset code byte */
        Bits = 0;             /* reset bit counter */

        if (Bytes < IR_CODE_BYTES)      /* prevent overflow */
        {
          Code++;             /* next code byte */
        }
      }
    }
    else                      /* item with fixed time */
    {
      if (! PulseCheck(Time, tS))  /* time doesn't matches */
      {
        Flag = 0;                  /* signal error */
      }
    }

    #if 0
    /* debugging output */
    if (! Flag)               /* error */
    {
      LCD_NextLine();
      DisplayValue(Counter, 0, 0);      /* display pulse number */
      LCD_Char(':');
      DisplayValue(Time, 0, 0);         /* display pulse duration */
    }
    #endif

    PulseWidth++;             /* next one */
    Counter++;                /* next one */
  }

  /* check result */
  if (Flag)                   /* success */
  {
    Flag = Bytes * 8;         /* return number of bits */
    Flag += Bits;
  }

  /* shift remaining bits to left end of current byte */
  if (Bits > 0)                 /* some bits left */
  {
    while (Bits < 8)
    {
      Data <<= 1;               /* shift left */
      Bits++;                   /* next bit */
    }

    *Code = Data;               /* save code byte */
  }

  return Flag;
}



/*
 *  adjust special bi-phase pulse pair to standard timing
 *
 *  requires:
 *  - pointer to pulse duration data
 *    first item has to be a pulse
 *  - number of pulses
 *  - offset to special pulse pair (half cycles)
 *  - time units of normal pulse
 *  - time units of special pulse
 *  returns:
 *  - number of special pulses
 */

uint8_t SpecialBiPhasePulse(uint8_t *PulseWidth, uint8_t Pulses, uint8_t Offset, uint8_t Normal, uint8_t Special)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Mixed;         /* duration of mixed pulse */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Cycles = 0;    /* half cycles */
  uint8_t           n = 0;         /* counter */

  Mixed = Normal + Special;        /* mixed pulse */

  while (Pulses > 0)
  {
    Time = *PulseWidth;       /* get duration */

    if (Cycles <= Offset)     /* offset not reached yet */
    {
      if (PulseCheck(Time, Normal))     /* normal pulse */
      {
        Cycles++;
      }
      else                              /* double or mixed pulse */
      {
        Cycles += 2;
      }
    }

    if (Cycles > Offset)      /* reached offset */
    {
      if (PulseCheck(Time, Special))    /* special pulse */
      {
        *PulseWidth = Normal;           /* adjust to normal */
        Flag++;

      }
      else if (PulseCheck(Time, Mixed)) /* mixed pulse */
      {
        *PulseWidth = 2 * Normal;       /* adjust to normal double pulse */
        Flag++;
      }

      n++;                         /* increase counter */
      if (n == 2) Pulses = 1;      /* end loop for pulse pair */
    }

    Pulses--;                 /* next pulse */
    PulseWidth++;
  }

  return(Flag);
}



/*
 *  get specific number of bits from IR code
 *
 *  requires:
 *  - start bit (1-...)
 *  - number of bits (1-8)
 *  - bit mode (LSB/MSB)
 *  returns:
 *  - code byte
 */

uint8_t Codebits(uint8_t StartBit, uint8_t Bits, uint8_t Mode)
{
  uint8_t           Data = 0;      /* return value */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           CodeBit;       /* bit counter */
  uint8_t           CodeByte;      /* byte counter */
  uint8_t           Temp;
  uint8_t           n;             /* counter */


  /*
   *  IR_Code: bits stored as received (bit #7 is first)
   *           IR_Code[0] is first byte
   *  LSB: first bit from IR_Code goes to bit #0
   *       last bit from IR_Code goes to highest bit #
   *  MSB: first bit from IR_Code goes to highest bit #
   *       last bit from IR_Code goes to bit #0 
   */

  /* determine start position in IR_Code */
  StartBit--;                      /* align */
  CodeByte = StartBit / 8;         /* start byte (0-) */
  CodeBit = StartBit % 8;          /* start bit in byte (0-) */
  Code = &IR_Code[CodeByte];       /* set start address */  
  Temp = *Code;                    /* copy code byte */

  /* shift start bit to bit #7 */
  n = CodeBit;                /* number of positions to shift */
  while (n > 0)
  {
    Temp <<= 1;               /* shift left */
    n--;                      /* next bit */
  }

  /* get bits */
  CodeBit = 8 - CodeBit;      /* remaining bits in first byte */
  n = 1;
  while (n <= Bits)
  {
    Data <<= 1;               /* shift destination left */

    if (Temp & 0b10000000)    /* bit set */
    {
      Data |= 0b00000001;     /* set bit */
    }

    Temp <<= 1;               /* shift source left */

    if (n == CodeBit)         /* byte overflow */
    {
      Code++;                 /* next byte */
      Temp = *Code;           /* copy code byte */
      /* CodeBit += 8; */
    }

    n++;                      /* next bit */
  }

  /* post processing */
  if (Mode == IR_LSB)         /* LSB mode */
  {
    /* reverse bit sequence */
    Temp = Data;                   /* Temp becomes source */
    Data = 0;                      /* Data becomes destination */
    n = Bits;                      /* number of bits to process */

    while (n > 0)
    {
      Data <<= 1;                  /* shift destination left */

      if (Temp & 0b00000001)       /* if bit in source is set */
      {
        Data |= 0b00000001;        /* set it in destination too */
      }

      Temp >>= 1;                  /* shift source right */
      n--;                         /* next bit */
    }
  } 

  return Data;
}



/*
 *  detect and decode IR protocol
 *
 *  requires:
 *  - pointer to array of pulse duration data
 *  - number of pulses (pauses/pulses)
 */

void IR_Decode(uint8_t *PulseWidth, uint8_t Pulses)
{
  uint8_t           Flag = 0;
  uint8_t           *Pulse;        /* pointer to pause/pulse data */
  uint8_t           Time1;         /* time units #1 */
  uint8_t           Time2;         /* time units #2 */
  uint8_t           Bits = 0;      /* number of bits received */
  uint8_t           Address;       /* RC address */
  uint8_t           Command;       /* RC command */
  uint8_t           Extras = 0;    /* RC extra stuff */
  uint8_t           Data = Pulses; /* temporary value */

  if (Pulses < 2) return;     /* not enough pulses */

  if (IR_State == 0)          /* no multi packet */
  {
    LCD_NextLine();           /* display result in a new line */
  }

  /*
   *  figure out IR protocol by checking the start pulse-pause pair
   */

  Pulse = PulseWidth;         /* first pulse */
  Time1 = *Pulse;             /* duration of first pulse */
  Pulse++;                    /* first pause */
  Time2 = *Pulse;             /* duration of first pause */


  /*
   *  NEC
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - format: 
   *    <start><address:8><inverted address:8><command:8><inverted command:8><end pulse>
   *  - extended format:
   *    <start><low address:8><high address:8><command:8><inverted command:8><end pulse>
   */

  if (PulseCheck(Time1, 180))           /* pulse 9ms */
  {
    if (PulseCheck(Time2, 90))          /* pause 4.5ms */
    {
      LCD_EEString_Space(IR_NEC_str);   /* display protocol */
      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 33);

      if (Bits == 32)              /* we expect 32 bits */
      {
        Address = Codebits(1, 8, IR_LSB);    /* address */
        Extras = Codebits(9, 8, IR_LSB);     /* inverted address */
        Command = Codebits(17, 8, IR_LSB);   /* command */

        /* determine protocol version */
        Data = ~Extras;                 /* invert */
        if (Address != Data)            /* address is not inverted */
        {
          /* extended format with 16 bit address */
          DisplayHex(Extras);           /* display high address */
        }

        Flag = 3;             /* confirmed + standard ouput */
      }
    }
    else if (PulseCheck(Time2, 45))          /* pause 2.25ms */
    {
      /* check for repeat sequence */
      if (Pulses == 3)                       /* just 3 pulses */
      {
        PulseWidth += 2;                     /* skip start pulse */
        if (PulseCheck(*PulseWidth, 11))     /* pulse 560탎 */
        {
          LCD_EEString_Space(IR_NEC_str);    /* display protocol */
          LCD_Char('R');
          Flag = 2;                          /* confirmed */
        }
      }
    }
  }


  /*
   *  Proton
   *  - also Mitsubishi/X-Sat (M50560)
   *  - start: pulse 8ms, pause 4ms
   *  - sync/separator between address and command: pulse 500탎, pause 4ms
   *  - PDM: pulse 500탎, pause 0=500탎 1=1500탎
   *  - bit mode: LSB
   *  - format: <start><address:8><sync><command:8><end pulse>
   */

  else if (PulseCheck(Time1, 160))      /* pulse 8ms */
  {
    if (PulseCheck(Time2, 80))          /* pause 4ms */
    {
      LCD_EEString_Space(IR_Proton_str);     /* display protocol */
      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;

      if (Pulses > 18)        /* enough pulses for 1st part and sync */
      {
        /* first part */
        Bits = PxM_Demod(PulseWidth, 16, 11, 11, 30);

        if (Bits == 8)             /* we expect 8 bits */
        {
          Address = Codebits(1, 8, IR_LSB);       /* address */
          PulseWidth += 17;        /* 16 pulses + terminating pulse */
          Pulses -= 17;

          /* check for separator after first part */
          if (PulseCheck(*PulseWidth, 80))     /* pause 4ms */
          {
            PulseWidth++;               /* skip pause */
            Pulses--;

            /* second part */
            Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 30);

            if (Bits == 8)             /* we expect 8 bits */
            {
              Command = Codebits(1, 8, IR_LSB);   /* command */

              Flag = 3;            /* confirmed + standard output */
            }
          }
        }
      }
    }
  }


  /*
   *  JVC
   *  - start: pulse 8.4ms, pause 4.2ms
   *  - PDM: pulse 525탎, pause 0=525탎 1=1575탎
   *  - bit mode: LSB
   *  - format: <start><address:8><command:8><end pulse>
   */

  else if (PulseCheck(Time1, 168))      /* pulse 8.4ms */
  {
    if (PulseCheck(Time2, 84))          /* pause 4.2ms */
    {
      LCD_EEString_Space(IR_JVC_str);   /* display protocol */
      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 32);

      if (Bits == 16)              /* we expect 16 bits */
      {
        Address = Codebits(1, 8, IR_LSB);    /* address */
        Command = Codebits(9, 8, IR_LSB);    /* command */

        Flag = 3;             /* confirmed + standard ouput */
      }
    }
  }


  /*
   *  Matsushita / Emerson
   *  - start: pulse 3.5ms, pause 3.5ms
   *  - PDM: pulse 872탎, pause 0=872탎 1=2616탎
   *  - bit mode: LSB
   *  - format:
   *    <start><custom code:6><data code:6><inverted custom code:6><inverted data code:6><end pulse>
   */

  /*
   *  Kaseikyo (Japanese Code)
   *  - start: pulse 3456탎, pause 1728탎
   *  - PDM: pulse 432탎, pause 0=432탎 1=1296탎
   *  - bit mode: LSB
   *  - 48 bit format:
   *    <start><OEM-1:8><OEM-2:8><parity:4><system:4>
   *    <product:8><function:8><x:4><checksum:4><end pulse>
   *  - parity: (OEM-1 0-3) ^ (OEM-1 4-7) ^ (OEM-2 0-3) ^ (OEM-2 4-7)
   *  - checksum: (system 0-3) ^ (product 0-3) ^ (product 4-7)
   *              ^ (function 0-3) ^ (function 4-7) ^ (x 0-3)
   */

  else if (PulseCheck(Time1, 70))       /* pulse 3.5ms */
  {
    /* Matsushita / Emerson */ 
    if (PulseCheck(Time2, 70))          /* pause 3.5ms */
    {
      LCD_EEString_Space(IR_Matsushita_str); /* display protocol */
      Flag++;                           /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 17, 17, 52);

      if (Bits == 24)              /* we expect 24 bits */
      {
        Address = Codebits(1, 6, IR_LSB);    /* address */
        Command = Codebits(7, 6, IR_LSB);    /* command */
        /* todo: check inverted address and command */

        Flag = 3;             /* confirmed + standard ouput */
      }
    }

    /* Kaseikyo */
    else if (PulseCheck(Time2, 34))     /* pause 1728탎 */
    {
      LCD_EEString_Space(IR_Kaseikyo_str);   /* display protocol */
      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      IR_RelaxTime = IR_RELAX_SHORT;
      Bits = PxM_Demod(PulseWidth, Pulses, 8, 8, 26);
      IR_RelaxTime = 0;

      if (Bits == 48)              /* we expect 48 bits */
      {
        Address = Codebits(1, 8, IR_LSB);    /* OEM 1 */
        Extras = Codebits(9, 8, IR_LSB);     /* OEM 2 */

        DisplayHex(Address);       /* display OEM 1 */
        DisplayHex(Extras);        /* display OEM 2 */
        LCD_Char(':');

        Address = Codebits(21, 4, IR_LSB);   /* system */
        Extras = Codebits(25, 8, IR_LSB);    /* product */
        Command = Codebits(33, 8, IR_LSB);   /* command */      

        DisplayHexDigit(Address);  /* display system */
        DisplayHex(Extras);        /* display product */
        LCD_Char(':');
        DisplayHex(Command);       /* display command */

        Flag = 2;                  /* confirmed */
      }
    }
  }


  /*
   *  Motorola
   *  - start: pulse 512탎, pause 2560탎
   *  - Bi-Phase:
   *    0: pause 512탎, pulse 512탎
   *    1: pulse 512탎, pause 512탎 
   *  - bit mode: LSB
   *  - second packet with 34ms - 12.8ms delay
   *  - third packet with 135.6ms - 12.8ms delay
   *  - format packet #1: <start><start "1":1><all 1s:9>
   *  - format packet #2: <start><start "1":1><command:9>
   *  - format packet #3: <start><start "1":1><all 1s:9>
   */

  else if (PulseCheck(Time1, 11))       /* pulse 512탎 */
  {
    if (PulseCheck(Time2, 52))          /* pause 2560탎 */
    {
      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;

      Bits = BiPhase_Demod(PulseWidth, Pulses, IR_THOMAS, 11);

      if (Bits == 10)              /* we expect 10 bits */
      {
        /* todo: check start bit */
        Command = Codebits(2, 8, IR_LSB);    /* command LSB */
        Extras = Codebits(10, 1, IR_LSB);    /* command MSB */

        Flag = 4;             /* confirmed multi packet */

        /*
         *  multi packet logic
         */

        /* check for all "1"s */
        if ((Command == 0b11111111) && (Extras == 0b00000001))
        {
          /* packet #1 or #3: all 1s */
          if (IR_State == 0)            /* packet #1 */
          {
            IR_State = 1;               /* got packet #1 */
          }
          else if (IR_State == 2)       /* packet #3 */
          {
            Flag = 2;                   /* confirmed */
          }
          /* else: packet missing/broken */
        }
        else
        {
          /* packet #2: command */
          if (IR_State == 2)          /* repeated packet */
          {
            LCD_NextLine();           /* display result in a new line */
          }

          LCD_EEString_Space(IR_Motorola_str);    /* display protocol */
          DisplayHexDigit(Extras);      /* display command MSB */
          DisplayHex(Command);          /* display command LSB */

          IR_State = 2;                 /* got packet #2 */
        }
      }
      else                              /* broken packet */
      {
        if (IR_State <= 1)         /* not for third packet */
        {
          /* for the "?" */
          LCD_EEString_Space(IR_Motorola_str);    /* display protocol */        
        }
      }
    }
  }


  /*
   *  Samsung / Toshiba
   *  - start: pulse 4.5ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - old format: <start><manufacturer code:12><command:8><end pulse>
   *  - 32 bit format (TC9012):
   *    <start><custom:8><copy of custom:8><data:8><inverted data:8><end pulse>
   */

  else if (PulseCheck(Time1, 89))       /* pulse 4.5ms */
  {
    if (PulseCheck(Time2, 89))          /* pause 4.5ms */
    {
      LCD_EEString_Space(IR_Samsung_str); /* display protocol */
      Flag++;                           /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 34);

      if (Bits == 32)              /* we expect 32 bits */
      {
        Address = Codebits(1, 8, IR_LSB);    /* address */
        Command = Codebits(17, 8, IR_LSB);   /* command */
        /* todo: check copy of address and inverted command */

        Flag = 3;                  /* confirmed + standard ouput */
      }
    }
  }


  /*
   *  Sony SIRC
   *  - start: pulse 2.4ms (pause 600탎)
   *  - PWM: pause 600탎, pulse 0=600탎 1=1200탎
   *  - bit mode: LSB
   *  - 12 bit format: <start><command:7><address:5>
   *  - 15 bit format: <start><command:7><address:8>
   *  - 20 bit format: <start><command:7><address:5><extended:8>
   */

  else if (PulseCheck(Time1, 48))       /* pulse 2.4ms */
  {
    if (PulseCheck(Time2, 12))          /* pause 600탎 */
    {
      LCD_EEString(IR_SIRC_str);        /* display protocol */
      Flag = 1;                         /* detected protocol */
      PulseWidth += 1;                  /* skip start pulse */
      Pulses -= 1;
      Bits = PxM_Demod(PulseWidth, Pulses, 12, 12, 24);

      /* 12, 15 or 20 bit format */
      Data = 5;                    /* 5 bit address */
      if (Bits == 12)
      {
        Flag = 2;                  /* confirmed */
      }
      else if (Bits == 15)
      {
        Data = 8;                  /* 8 bit address */
        Flag = 2;                  /* confirmed */
      }
      else if (Bits == 20)
      {
        Extras = Codebits(13, 8, IR_LSB);    /* extended */
        Flag = 2;                  /* confirmed */
      }

      Command = Codebits(1, 7, IR_LSB);      /* command */
      Address = Codebits(8, Data, IR_LSB);   /* address */

      /* display format */
      if (Flag == 2)          /* valid packet */
      {
        DisplayValue(Bits, 0, 0);
      }

      LCD_Space();            /* display space */

      /* display data */
      if (Flag == 2)          /* valid packet */
      {
        DisplayHex(Address);       /* display address */
        LCD_Char(':');
        DisplayHex(Command);       /* display command */

        if (Bits == 20)            /* 20 bit format */
        {
          LCD_Char(':');
          DisplayHex(Extras);      /* display extended data */
        }
      }
    }
  }


  /*
   *  Sharp
   *  - no start / AGC burst
   *  - PDM: pulse 320탎, pause 0=680탎 1=1680탎
   *  - bit mode: LSB
   *  - second packet with 40ms delay
   *  - format packet #1:
   *    <address:5><command:8><expansion:1><check:1><end pulse>
   *  - format packet #2:
   *    <address:5><inverted command:8><inverted expansion:1><inverted check:1><end pulse>
   */

  else if (PulseCheck(Time1, 6))        /* pulse 320탎 */
  {
    /* pause 680탎 or 1680탎 */
    if ((PulseCheck(Time2, 14)) || (PulseCheck(Time2, 35)))
    {
      Flag = 1;                         /* detected protocol */

      Bits = PxM_Demod(PulseWidth, Pulses, 6, 14, 35);

      if (Bits == 15)              /* we expect 15 bits */
      {
        if (IR_State == 0)         /* packet #1 */
        {
          Address = Codebits(1, 5, IR_LSB);    /* address */
          Command = Codebits(6, 8, IR_LSB);    /* command */
          /* todo: expansion & check bits */

          LCD_EEString_Space(IR_Sharp_str);      /* display protocol */
          DisplayHex(Address);
          LCD_Char(':');
          DisplayHex(Command);

          IR_State = 1;                 /* got packet #1 */
          Flag = 4;                     /* multi packet  */
        }
        else                       /* packet #2 */
        {
          /* we don't check the inverted command and extra bits */
          Flag = 2;                     /* confirmed */
        }
      }
    }
  }


  /*
   *  standard RC-5
   *  - 2 start bits: (889탎 L) 889탎 H, 889탎 L (889탎 H)
   *  - Bi-Phase (IEEE 802.3):
   *    0: pulse 889탎, pause 889탎
   *    1: pause 889탎, pulse 889탎
   *  - bit mode: MSB
   *  - format: <s1 "1":1><s2 "1":1><toggle:1><address:5><command:6>
   */

  else if (PulseCheck(Time1, 17))       /* pulse 889탎 */
  {
    if (PulseCheck(Time2, 17))          /* pause 889탎 */
    {
      LCD_EEString_Space(IR_RC5_str);   /* display protocol */
      Flag = 1;                         /* detected protocol */
      Bits = BiPhase_Demod(PulseWidth, Pulses, IR_IEEE | IR_PRE_PAUSE, 17);

      if (Bits == 14)              /* we expect 14 bits */
      {
        Address = Codebits(4, 5, IR_MSB);    /* address */
        Command = Codebits(9, 6, IR_MSB);    /* command */

        Flag = 3;             /* confirmed + standard ouput */
      }
    }
  }


  /*
   *  standard RC-6
   *  - start: pulse 2664탎, pause 888탎
   *  - Bi-Phase (Thomas):
   *    normal bit 0: pause 444탎, pulse 444탎
   *    normal bit 1: pulse 444탎, pause 444탎
   *    toggle bit 0: pause 888탎, pulse 888탎
   *    toggle bit 1: pulse 888탎, pause 888탎
   *  - bit mode: MSB
   *  - format (Mode 0, 16 bit):
   *    <start><start bit "1":1><mode:3><toggle:1><address:8><command:8>
   */

  else if (PulseCheck(Time1, 53))       /* pulse 2664탎 */
  {
    if (PulseCheck(Time2, 17))          /* pause 888탎 */
    {
      LCD_EEString_Space(IR_RC6_str);   /* display protocol */

      Flag = 1;                         /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;

      /* convert toggle bit to standard timing */
      Data = SpecialBiPhasePulse(PulseWidth, Pulses, 8, 8, 17);

      if (Data == 2)               /* we expect 1 special bit (= 2 pulses) */
      {
        IR_RelaxTime = IR_RELAX_SHORT;
        Bits = BiPhase_Demod(PulseWidth, Pulses, IR_THOMAS, 8);
        IR_RelaxTime = 0;

        if (Bits == 21)       /* we expect 21 bits */
        {
          Address = Codebits(6, 8, IR_MSB);       /* address */
          Command = Codebits(14, 8, IR_MSB);      /* command */

          Flag = 3;             /* confirmed + standard ouput */
        }
      }
    }
  }


  if (Flag <= 1)              /* some issue: unknown protocol/broken frame */
  {
    LCD_Char('?');    
  }

  if (Flag == 0)              /* unknown protocol */
  {
    LCD_Space();
    DisplayValue(Data, 0, 0);      /* display number of pulses */
    LCD_Char(':');
    DisplayValue(Time1, 0, 0);     /* display time units of first pulse */
    LCD_Char('-');
    DisplayValue(Time2, 0, 0);     /* display time units of first pause */

    #if 0
    /* debugging output */
    LCD_NextLine();
    Pulse++;
    DisplayValue(*Pulse, 0, 0);
    LCD_Space();
    Pulse++;
    DisplayValue(*Pulse, 0, 0);
    LCD_Space();
    Pulse++;
    DisplayValue(*Pulse, 0, 0);
    LCD_Space();
    Pulse++;
    DisplayValue(*Pulse, 0, 0);
    LCD_Space();
    Pulse++;
    DisplayValue(*Pulse, 0, 0);
    #endif
  }

  if (Flag == 3)              /* known protocol & standard output */
  {
    DisplayHex(Address);      /* display address */
    LCD_Char(':');
    DisplayHex(Command);      /* display command */
  }

  if (Flag < 4)               /* protocol done */
  {
    IR_State = 0;             /* reset state for multi packet protocols */

    /* slow down display updates and try to skip early repeats */
    MilliSleep(200);          /* don't proceed too soon */
  }
} 



/*
 *  detect IR remote control using a TSOP IR receiver module
 *  - pinout:
 *    probe #1  Gnd
 *    probe #2  Vs/+5V (limit current by Rl)
 *    probe #3  Out/Data (inverted)
 */

void IR_Detector(void)
{
  /* maximum number of pauses/pulses: 2 start + (2 * 48) data + 1 stop */
  #define MAX_PULSES          100

  uint8_t           Run = 1;            /* loop control */
  uint8_t           Flag;               /* IR signal */
  uint8_t           OldFlag = 0;        /* former IR signal */
  uint8_t           Cycles;             /* delay loop */
  uint8_t           n;                  /* counter */
  uint8_t           Pulses = 0;         /* pulse counter */
  uint8_t           Period = 0;         /* pulse duration */
  uint8_t           *Pulse = NULL;      /* pointer to pulse data */
  uint8_t           PulseWidth[MAX_PULSES];  /* pulse duration data */     

  /* inform user */
  ShortCircuit(0);                      /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString(IR_Detector_str);        /* display: IR detector */
  LCD_NextLine_Mode(LINE_KEEP);         /* line mode: keep first line */
  #ifdef SW_IR_RECEIVER
    /* display module pinout (1: Gnd / 2: Vcc / 3: Data) */
    LCD_NextLine();
    Show_SimplePinout('-', '+', 'd');
  #endif

  /*
   *  setup module
   */

  #ifdef SW_IR_RECEIVER
    #ifdef SW_IR_DISABLE_RESISTOR
    /* unsafe mode without current limiting resistor for Vs */
    /* set probes: probe-1 -- Gnd / probe-2 -- Vcc / probe-3 (HiZ) -- Rh -- Gnd */
    ADC_PORT = (1 << TP2);                /* pull down probe-1, pull up probe-2 */
    ADC_DDR = (1 << TP1) | (1 << TP2);    /* enable direct pull down/up */
    R_DDR = (1 << R_RH_3);                /* enable Rh for probe-3 */
    R_PORT = 0;                           /* pull down probe-3 */
    #else
    /* safe mode with current limiting resistor for Vs */
    /* set probes: probe-1 -- Gnd / probe-2 -- Rl -- Vcc / probe-3 (HiZ) -- Rh -- Gnd */
    ADC_PORT = 0;                         /* pull down directly: */
    ADC_DDR = (1 << TP1);                 /* probe-1 */
    /* pull up probe-2 via Rl, pull down probe-3 via Rh */
    R_DDR = (1 << R_RL_2) | (1 << R_RH_3);     /* enable resistors */
    R_PORT = (1 << R_RL_2);                    /* pull up probe-2, pull down probe-3 */
    #endif
  #endif

  #ifdef HW_IR_RECEIVER
    /* set data pin to input mode */
    IR_DDR &= ~(1 << IR_DATA);          /* clear bit for data pin */
  #endif

  /* wait for IR receiver module or key press */
  n = 1;
  while (n)
  {
    #ifdef SW_IR_RECEIVER
    if (ADC_PIN & (1 << TP3))           /* check for high level */
    #else
    if (IR_PIN & (1 << IR_DATA))        /* check for high level */
    #endif
    {
      n = 0;                            /* end this loop */
    }
    else
    {
      Flag = TestKey(100, CURSOR_NONE); /* wait 100ms for key press */
      if (Flag)                         /* key pressed */
      {
        Run = 0;                        /* skip decoder loop */
        n = 0;                          /* end this loop */
      }
    }
  }

  LCD_ClearLine2();                /* clear line #2 */
  LCD_CharPos(1, 1);               /* move to first line */


  /*
   *  adaptive sampling delay for 10탎 considering processing loop
   *  - processing loop needs about 24 MCU cycles (3탎@8MHz)
   */

  Cycles = MCU_CYCLES_PER_US;           /* MCU cycles per 탎 */
  Cycles *= 10;                         /* cycles for 10탎 */
  Cycles -= 24;                         /* consider loop */
  Cycles /= 4;                          /* delay loop is 4 cycles */


  /*
   *  Since we deal with data pulses in the range of 0.5 up to 10ms
   *  we sample data every 50탎 and log how long each pulse or pause
   *  lasts. Timeout at 12ms.
   */

  while (Run > 0)
  {
    /* data logic is inverted by IR receiver */
    #ifdef SW_IR_RECEIVER
    Flag = ADC_PIN & (1 << TP3);       /* poll data pin */
    #else
    Flag = IR_PIN & (1 << IR_DATA);    /* poll data pin */
    #endif

    if (Run == 1)             /* wait for IR */
    {
      if (!Flag)              /* low: H / IR signal */
      {
        Run = 2;              /* start sampling */
        OldFlag = Flag;       /* first one is always a pulse */
        Pulses = 0;           /* reset pulse counter */
        Period = 0;           /* reset duration */
        Pulse = &PulseWidth[0];    /* set start address */
      }
      else                    /* high: L / no IR signal */
      {
        Run = 4;              /* check for test key */
      }
    }
    else                      /* sample IR */
    {
      if (Flag == OldFlag)    /* same pause/pulse */
      {
        Period++;             /* add one sampling period */

        if (Period > 240)     /* 12ms timeout */
        {
          Run = 3;            /* switch to decoding mode */

          if (!Flag)          /* IR signal or removed receiver module */
          {
            Run = 4;          /* check for test key */
          }
        }
      }
      else                    /* new pause/pulse */
      {
        OldFlag = Flag;       /* update flag */

        if (Pulses < MAX_PULSES)   /* prevent overflow */
        {
          Pulses++;             /* got another one */
          *Pulse = Period;      /* save duration */
          Pulse++;              /* next one */
          Period = 0;           /* reset duration */
        }
        else                  /* max number of pulses exceeded */
        {
          Run = 3;              /* switch to decoding mode */
        }
      }
    }

    if (Run == 2)             /* sampling mode */
    {
      wait40us();             /* wait sampling period */

      /*
       *  adaptive delay for 10탎 considering processing loop
       *  - a loop run needs 4 cycles, the last loop run just 3
       *  - time delay: 10탎 - 1 MCU cycle
       */

      n = Cycles;
      while (n > 0)
      {
        n--;
        asm volatile("nop\n\t"::);
      }
    }
    else if (Run == 3)        /* decode mode */
    {
      IR_Decode(&PulseWidth[0], Pulses);     /* try to decode */
      Run = 1;                          /* switch back to waiting mode */
    }
    else if (Run == 4)        /* check for test key */
    {
      Run = 1;                          /* switch back to waiting mode */

      /* check test button */
      while (!(CONTROL_PIN & (1 << TEST_BUTTON)))
      {
        MilliSleep(50);                 /* take a nap */
        Run = 0;                        /* end loop */
      }
    }

    wdt_reset();            /* reset watchdog */
  }

  #undef MAX_PULSES
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef IR_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

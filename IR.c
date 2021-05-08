/* ************************************************************************
 *
 *   IR remote control functions
 *
 *   (c) 2015-2017 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define IR_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  local constants
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

/* signal types */
#define IR_PAUSE         0b00000001     /* pause */
#define IR_PULSE         0b00000010     /* pulse */

/* IR protocols */
#define IR_PROTOMAX               6     /* number of protocols */
#define IR_NEC_STD                1     /* NEC standard */
#define IR_NEC_EXT                2     /* NEC extended */
#define IR_SAMSUNG                3     /* Samsung / Toshiba */
#define IR_SIRC_12                4     /* Sony SIRC-12 */
#define IR_SIRC_15                5     /* Sony SIRC-15 */
#define IR_SIRC_20                6     /* Sony SIRC-20 */

#define IR_PROTON                 0     /* Proton (Mitsubishi/X-Sat) */
#define IR_JVC                    0     /* JVC */
#define IR_MATSUSHITA             0     /* Matsushita / Emerson */
#define IR_KASEIKYO               0     /* Kaseikyo (Japanese Code) */
#define IR_MOTOROLA               0     /* Motorola */
#define IR_SHARP                  0     /* Sharp */
#define IR_RC5                    0     /* standard RC-5 */
#define IR_RC6                    0     /* standard RC-6 */



/*
 *  local variables
 */

#if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (SW_IR_TRANSMITTER)
/* demodulated/raw IR code */
uint8_t             IR_Code[IR_CODE_BYTES];  /* raw data */
#endif

#if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
/* decoding logic */
uint8_t             IR_State = 0;            /* multi packet protocol state */
uint8_t             IR_RelaxTime = 0;        /* timing control flag */
#endif

#ifdef SW_IR_TRANSMITTER
#endif



/* ************************************************************************
 *   IR detection/decoder tool (receiver)
 * ************************************************************************ */


#if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)

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

  Code = &IR_Code[0];              /* set start address */

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
   *  - two different pulse times to encode 0/1
   *  - even number of data items
   *
   *  PDM / space encoding
   *  - fixed pulse time
   *  - two different pause times to encode 0/1
   *  - last item is pulse (stop bit)
   *    required to indicate end of pause of last data bit
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
 *  - start bit in IR code (1-...)
 *  - number of bits (1-8)
 *  - bit mode
 *    IR_LSB - LSB
 *    IR_MSB - MSB
 *  returns:
 *  - code byte
 */

uint8_t GetBits(uint8_t StartBit, uint8_t Bits, uint8_t Mode)
{
  uint8_t           Data = 0;      /* return value */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Count;         /* bit counter */
  uint8_t           Temp;
  uint8_t           n;             /* counter */


  /*
   *  IR_Code: bits stored as received (bit #7 is first bit received)
   *           IR_Code[0] is first byte
   *  LSB: bit #7 of IR_Code goes to bit #0 of Data
   *       bit #0 of IR_Code goes to bit #7 of Data
   *  MSB: bit #7 of IR_Code goes to bit #7 of Data
   *       bit #0 of IR_Code goes to bit #0 of Data
   */

  /* determine start position in IR_Code */
  StartBit--;                      /* align */
  Temp = StartBit / 8;             /* start byte (0-) */
  Count = StartBit % 8;            /* start bit in byte (0-) */
  Code = &IR_Code[Temp];           /* get start address */
  Temp = *Code;                    /* copy code byte */

  /* shift start bit to bit #7 */
  n = Count;                  /* number of positions to shift */
  while (n > 0)
  {
    Temp <<= 1;               /* shift left */
    n--;                      /* next bit */
  }

  /* get bits */
  Count = 8 - Count;          /* remaining bits in first byte */
  n = 1;
  while (n <= Bits)
  {
    Data <<= 1;               /* shift destination left */

    if (Temp & 0b10000000)    /* bit set */
    {
      Data |= 0b00000001;     /* set bit */
    }

    Temp <<= 1;               /* shift source left */

    if (n == Count)           /* byte overflow */
    {
      Code++;                 /* next byte */
      Temp = *Code;           /* copy code byte */
      /* Count += 8; */
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
 *  - uses IR_State to keep track of multi-packet protocols
 *
 *  requires:
 *  - pointer to array of pulse duration data
 *  - number of pulses (pauses/pulses)
 */

void IR_Decode(uint8_t *PulseWidth, uint8_t Pulses)
{
  uint8_t           Flag = 0;      /* control flag */
  uint8_t           *Pulse;        /* pointer to pause/pulse data */
  uint8_t           Time1;         /* time units #1 */
  uint8_t           Time2;         /* time units #2 */
  uint8_t           Bits = 0;      /* number of bits received */
  uint8_t           Address;       /* RC address */
  uint8_t           Command;       /* RC command */
  uint8_t           Extras = 0;    /* RC extra stuff */
  uint8_t           Data = Pulses; /* temporary value */

  /* local constants for Flag */
  #define PROTO_UNKNOWN       0    /* unknown protocol */
  #define PROTO_DETECTED      1    /* protocol detected */
  #define PACKET_OK           2    /* valid packet */
  #define PACKET_DISPLAY      3    /* valid packet & display standard values */
  #define PACKET_MULTI        4    /* multi packet (more to follow) */

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
   *  - stop: pulse 560탎
   *  - standard 2format: 
   *    <start><address:8><inverted address:8><command:8><inverted command:8><stop>
   *  - extended format:
   *    <start><low address:8><high address:8><command:8><inverted command:8><stop>
   *  - repeat sequence:
   *    <pulse 9ms><pause 2.25ms><stop>
   */

  if (PulseCheck(Time1, 180))           /* pulse 9ms */
  {
    if (PulseCheck(Time2, 90))          /* pause 4.5ms */
    {
      LCD_EEString_Space(IR_NEC_str);   /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 33);

      if (Bits == 32)              /* we expect 32 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* address */
        Extras = GetBits(9, 8, IR_LSB);      /* inverted address */
        Command = GetBits(17, 8, IR_LSB);    /* command */

        /* determine protocol version */
        Data = ~Extras;                 /* invert */
        if (Address != Data)            /* address is not inverted */
        {
          /* extended format with 16 bit address */
          DisplayHexByte(Extras);       /* display high address */
        }

        Flag = PACKET_DISPLAY;          /* packet ok & default output */
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
          Flag = PACKET_OK;                  /* packet ok */
        }
      }
    }
  }


  /*
   *  Proton
   *  - also Mitsubishi/X-Sat (M50560)
   *  - start: pulse 8ms, pause 4ms
   *  - sync/separator between address and command: pause 4ms
   *  - PDM: pulse 500탎, pause 0=500탎 1=1500탎
   *  - bit mode: LSB
   *  - stop: pulse 500탎
   *  - format: <start><address:8><stop><sync><command:8><stop>
   */

  else if (PulseCheck(Time1, 160))      /* pulse 8ms */
  {
    if (PulseCheck(Time2, 80))          /* pause 4ms */
    {
      LCD_EEString_Space(IR_Proton_str);     /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;

      if (Pulses > 18)        /* enough pulses for 1st part and sync */
      {
        /* first part */
        Bits = PxM_Demod(PulseWidth, 16, 11, 11, 30);

        if (Bits == 8)             /* we expect 8 bits */
        {
          Address = GetBits(1, 8, IR_LSB);        /* address */
          PulseWidth += 17;        /* 16 pulses + terminating pulse */
          Pulses -= 17;

          /* check for separator after first part */
          if (PulseCheck(*PulseWidth, 80))     /* pause 4ms */
          {
            PulseWidth++;               /* skip pause */
            Pulses--;

            /* second part */
            Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 30);

            if (Bits == 8)              /* we expect 8 bits */
            {
              Command = GetBits(1, 8, IR_LSB);    /* command */

              Flag = PACKET_DISPLAY;    /* packet ok & default output */
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
   *  - stop: pulse 525탎
   *  - format: <start><address:8><command:8><stop>
   */

  else if (PulseCheck(Time1, 168))      /* pulse 8.4ms */
  {
    if (PulseCheck(Time2, 84))          /* pause 4.2ms */
    {
      LCD_EEString_Space(IR_JVC_str);   /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 32);

      if (Bits == 16)              /* we expect 16 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* address */
        Command = GetBits(9, 8, IR_LSB);     /* command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
      }
    }
  }


  /*
   *  Matsushita (Panasonic) / Emerson
   *  - start: pulse 3.5ms, pause 3.5ms
   *  - PDM: pulse 872탎, pause 0=872탎 1=2616탎
   *  - bit mode: LSB
   *  - stop: pulse 872탎
   *  - format:
   *    <start><custom code:6><data code:6><inverted custom code:6><inverted data code:6><stop>
   */

  /*
   *  Kaseikyo (Japanese Code)
   *  - start: pulse 3456탎, pause 1728탎
   *  - PDM: pulse 432탎, pause 0=432탎 1=1296탎
   *  - bit mode: LSB
   *  - stop: pulse 432탎
   *  - 48 bit format:
   *    <start><manufacturer code:16><mc-parity:4><system:4>
   *      <equipment:8><command:8><parity:8><stop>
   *  - mc-parity: 0000 (fixed)
   *  - parity: <mc-parity:4><system:4> ^ <equipment:8> ^ <command:8>
   *  - other possible format:
   *    <start><OEM-1:8><OEM-2:8><parity:4><system:4>
   *      <product:8><function:8><x:4><checksum:4><stop>
   *    parity: <OEM-1 0-3> ^ <OEM-1 4-7> ^ <OEM-2 0-3> ^ <OEM-2 4-7>
   *    checksum: <system 0-3> ^ <product 0-3> ^ <product 4-7>
   *              ^ >function 0-3> ^ <function 4-7> ^ <x 0-3>
   */

  else if (PulseCheck(Time1, 70))       /* pulse 3.5ms */
  {
    /* Matsushita (Panasonic) / Emerson */ 
    if (PulseCheck(Time2, 70))          /* pause 3.5ms */
    {
      LCD_EEString_Space(IR_Matsushita_str); /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 17, 17, 52);

      if (Bits == 24)              /* we expect 24 bits */
      {
        Address = GetBits(1, 6, IR_LSB);     /* address */
        Command = GetBits(7, 6, IR_LSB);     /* command */
        /* todo: check inverted address and command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
      }
    }

    /* Kaseikyo */
    else if (PulseCheck(Time2, 34))     /* pause 1728탎 */
    {
      LCD_EEString_Space(IR_Kaseikyo_str);   /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      IR_RelaxTime = IR_RELAX_SHORT;
      Bits = PxM_Demod(PulseWidth, Pulses, 8, 8, 26);
      IR_RelaxTime = 0;

      if (Bits == 48)              /* we expect 48 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* manufacturer LSB */
        Extras = GetBits(9, 8, IR_LSB);      /* manufacturer MSB */

        DisplayHexByte(Extras);    /* display manufacturer MSB */
        DisplayHexByte(Address);   /* display manufacturer LSB */

        LCD_Char(':');

        Extras = GetBits(21, 4, IR_LSB);     /* system */
        Address = GetBits(25, 8, IR_LSB);    /* equipment */
        Command = GetBits(33, 8, IR_LSB);    /* command */      

        DisplayHexDigit(Extras);   /* display system */
        DisplayHexByte(Address);   /* display equipment */
        LCD_Char(':');
        DisplayHexByte(Command);   /* display command */

        Flag = PACKET_OK;          /* packet ok */
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
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;

      Bits = BiPhase_Demod(PulseWidth, Pulses, IR_THOMAS, 11);

      if (Bits == 10)              /* we expect 10 bits */
      {
        /* todo: check start bit */
        Command = GetBits(2, 8, IR_LSB);     /* command LSB */
        Extras = GetBits(10, 1, IR_LSB);     /* command MSB */

        Flag = PACKET_MULTI;       /* confirmed multi packet */

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
            Flag = PACKET_OK;           /* packet ok */
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
          DisplayHexByte(Command);      /* display command LSB */

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
   *  - stop: pulse 560탎
   *  - old format: <start><manufacturer code:12><command:8><stop>
   *  - 32 bit format (TC9012):
   *    <start><custom:8><copy of custom:8><data:8><inverted data:8><stop>
   */

  else if (PulseCheck(Time1, 89))       /* pulse 4.5ms */
  {
    if (PulseCheck(Time2, 89))          /* pause 4.5ms */
    {
      LCD_EEString_Space(IR_Samsung_str); /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 2;                  /* skip start pulse */
      Pulses -= 2;
      Bits = PxM_Demod(PulseWidth, Pulses, 11, 11, 34);

      if (Bits == 32)              /* we expect 32 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* address */
        Command = GetBits(17, 8, IR_LSB);    /* command */
        /* todo: check copy of address and inverted command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
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
   *  - code becomes valid after receiving it 3 times at least
   *    delay is 45ms between start and start
   */

  else if (PulseCheck(Time1, 48))       /* pulse 2.4ms */
  {
    if (PulseCheck(Time2, 12))          /* pause 600탎 */
    {
      LCD_EEString(IR_SIRC_str);        /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */
      PulseWidth += 1;                  /* skip start pulse */
      Pulses -= 1;
      Bits = PxM_Demod(PulseWidth, Pulses, 12, 12, 24);

      /* 12, 15 or 20 bit format */
      Data = 5;                    /* 5 bit address */
      if (Bits == 12)
      {
        Flag = PACKET_OK;          /* packet ok */
      }
      else if (Bits == 15)
      {
        Data = 8;                  /* 8 bit address */
        Flag = PACKET_OK;          /* packet ok */
      }
      else if (Bits == 20)
      {
        Extras = GetBits(13, 8, IR_LSB);     /* extended */
        Flag = PACKET_OK;          /* packet ok */
      }

      Command = GetBits(1, 7, IR_LSB);       /* command */
      Address = GetBits(8, Data, IR_LSB);    /* address */

      /* display format */
      if (Flag == PACKET_OK)       /* valid packet */
      {
        DisplayValue(Bits, 0, 0);
      }

      /* we accept the first code and don't wait for another 2 repeats */

      LCD_Space();                 /* display space */

      /* display data */
      if (Flag == PACKET_OK)       /* valid packet */
      {
        DisplayHexByte(Address);   /* display address */
        LCD_Char(':');
        DisplayHexByte(Command);   /* display command */

        if (Bits == 20)            /* 20 bit format */
        {
          LCD_Char(':');
          DisplayHexByte(Extras);  /* display extended data */
        }
      }
    }
  }


  /*
   *  Sharp
   *  - no start / AGC burst
   *  - PDM: pulse 320탎, pause 0=680탎 1=1680탎
   *  - bit mode: LSB
   *  - stop: pulse 320탎
   *  - second packet with 40ms delay
   *  - format packet #1:
   *    <address:5><command:8><expansion:1><check:1><stop>
   *  - format packet #2:
   *    <address:5><inverted command:8><inverted expansion:1><inverted check:1><stop>
   */

  else if (PulseCheck(Time1, 6))        /* pulse 320탎 */
  {
    /* pause 680탎 or 1680탎 */
    if ((PulseCheck(Time2, 14)) || (PulseCheck(Time2, 35)))
    {
      Flag = PROTO_DETECTED;       /* detected protocol */

      Bits = PxM_Demod(PulseWidth, Pulses, 6, 14, 35);

      if (Bits == 15)              /* we expect 15 bits */
      {
        if (IR_State == 0)         /* packet #1 */
        {
          Address = GetBits(1, 5, IR_LSB);     /* address */
          Command = GetBits(6, 8, IR_LSB);     /* command */
          /* todo: expansion & check bits */

          LCD_EEString_Space(IR_Sharp_str);      /* display protocol */
          DisplayHexByte(Address);
          LCD_Char(':');
          DisplayHexByte(Command);

          IR_State = 1;                 /* got packet #1 */
          Flag = PACKET_MULTI;          /* multi packet */
        }
        else                       /* packet #2 */
        {
          /* we don't check the inverted command and extra bits */
          Flag = PACKET_OK;             /* packet ok */
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
      Flag = PROTO_DETECTED;            /* detected protocol */
      Bits = BiPhase_Demod(PulseWidth, Pulses, IR_IEEE | IR_PRE_PAUSE, 17);

      if (Bits == 14)              /* we expect 14 bits */
      {
        Address = GetBits(4, 5, IR_MSB);     /* address */
        Command = GetBits(9, 6, IR_MSB);     /* command */

        Flag = PACKET_DISPLAY;     /* packet ok & default ouput */
      }
    }
  }


  /*
   *  standard RC-6 (RC6-0-16)
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

      Flag = PROTO_DETECTED;            /* detected protocol */
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
          Address = GetBits(6, 8, IR_MSB);   /* address */
          Command = GetBits(14, 8, IR_MSB);  /* command */

          Flag = PACKET_DISPLAY;   /* packet ok & default ouput */
        }
      }
    }
  }

  if (Flag <= PROTO_DETECTED)      /* some issue: unknown protocol/bad packet */
  {
    LCD_Char('?');
  }

  if (Flag == PROTO_UNKNOWN)       /* unknown protocol */
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

  if (Flag == PACKET_DISPLAY) /* known protocol & standard output */
  {
    DisplayHexByte(Address);  /* display address */
    LCD_Char(':');
    DisplayHexByte(Command);  /* display command */
  }

  if (Flag < PACKET_MULTI)    /* no packets to follow (protocol done) */
  {
    IR_State = 0;             /* reset state for multi packet protocols */

    /* slow down display updates and try to skip early repeats */
    MilliSleep(200);          /* don't proceed too soon */
  }

  /* clean up local constants */
  #undef PROTO_UNKNOWN
  #undef PROTO_DETECTED
  #undef PACKET_OK
  #undef PACKET_DISPLAY
  #undef PACKET_MULTI
} 



/*
 *  detect & decode IR remote control signals
 *  using a TSOP IR receiver module
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

#endif



/* ************************************************************************
 *   IR remote control tool (sender)
 * ************************************************************************ */

#ifdef SW_IR_TRANSMITTER

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
   *  wait
   *  - loop burns 7 cycles per run and 4 cycles for last run
   *  - total: 7 cycles * Time + 4 cycles
   *  - add NOPs for 1탎
   */

  while (Time > 0)
  {
    #if CPU_FREQ == 8000000
      /* add 1 cycle */
      asm volatile("nop");
    #endif

    #if CPU_FREQ == 16000000
      /* add 9 cycles */
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
    #endif

    #if CPU_FREQ == 20000000
      /* add 13 cycles */
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
    #endif

    Time--;
  }

  if (Type & IR_PULSE)        /* create pulse */
  {
    /* stop Timer1 */
    TCCR1B = (1 << WGM13) | (1 << WGM12);    /* clear prescaler */

    /* disable output via OC1B pin */
    TCCR1A = (1 << WGM11) | (1 << WGM10);
  }
}



#if 0
/*
 *  send IR code using Bi-Phase modulation
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
   *  Bi-Phase Modulation / Manchester encoding
   *  - G.E. Thomas
   *    0: pause - pulse
   *    1: pulse - pause
   *  - IEEE 802.3
   *    0: pulse - pause 
   *    1: pause - pulse
   */

  if (Mode & IR_IEEE)         /* IEEE */
  {
    Zero = IR_PULSE           /* 0 starts with pulse */
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
#endif



/*
 *  send IR code using PWM
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
 *  - tP: time units of pulse/pause in 탎
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

  /* todo: for PDM we should the end pulse */
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
   *  NEC Standard
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - format:
   *    <start><address:8><inverted address:8><command:8><inverted command:8><stop>
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
    IR_Send_Pulse(IR_PULSE, 9000);
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
  }


  /*
   *  NEC Extended
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - format:
   *    <start><low address:8><high address:8><command:8><inverted command:8><stop>
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
    IR_Send_Pulse(IR_PULSE, 9000);
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
  }


  /*
   *  Proton
   *  - also Mitsubishi/X-Sat (M50560)
   *  - start: pulse 8ms, pause 4ms
   *  - sync/separator between address and command: pause 4ms
   *  - PDM: pulse 500탎, pause 0=500탎 1=1500탎
   *  - bit mode: LSB
   *  - stop: pulse 500탎
   *  - format: <start><address:8><stop><sync><command:8><stop>
   */


  /*
   *  JVC
   *  - start: pulse 8.4ms, pause 4.2ms
   *  - PDM: pulse 525탎, pause 0=525탎 1=1575탎
   *  - bit mode: LSB
   *  - stop: pulse 525탎
   *  - format: <start><address:8><command:8><stop>
   */


  /*
   *  Matsushita (Panasonic) / Emerson
   *  - start: pulse 3.5ms, pause 3.5ms
   *  - PDM: pulse 872탎, pause 0=872탎 1=2616탎
   *  - bit mode: LSB
   *  - stop: pulse 872탎
   *  - format:
   *    <start><custom code:6><data code:6><inverted custom code:6><inverted data code:6><stop>
   */


  /*
   *  Kaseikyo (Japanese Code)
   *  - start: pulse 3456탎, pause 1728탎
   *  - PDM: pulse 432탎, pause 0=432탎 1=1296탎
   *  - bit mode: LSB
   *  - stop: pulse 432탎
   *  - 48 bit format:
   *    <start><manufacturer code:16><mc-parity:4><system:4>
   *      <equipment:8><command:8><parity:8><stop>
   *  - mc-parity: 0000 (fixed)
   *  - parity: <mc-parity:4><system:4> ^ <equipment:8> ^ <command:8>
   *  - other possible format:
   *    <start><OEM-1:8><OEM-2:8><parity:4><system:4>
   *      <product:8><function:8><x:4><checksum:4><stop>
   *    parity: <OEM-1 0-3> ^ <OEM-1 4-7> ^ <OEM-2 0-3> ^ <OEM-2 4-7>
   *    checksum: <system 0-3> ^ <product 0-3> ^ <product 4-7>
   *              ^ >function 0-3> ^ <function 4-7> ^ <x 0-3>
   */


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


  /*
   *  Samsung / Toshiba
   *  - start: pulse 4.5ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - stop: pulse 560탎
   *  - 32 bit format (TC9012):
   *    <start><custom:8><copy of custom:8><data:8><inverted data:8><stop>
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
    IR_Send_Pulse(IR_PULSE, 4500);
    IR_Send_Pulse(IR_PAUSE, 4500);
    IR_Send_PDM(&IR_Code[0], 32, 560, 560, 1690);
    /* todo: check if we have to send the code twice */
  }


  /*
   *  Sony SIRC-12
   *  - start: pulse 2.4ms (pause 600탎)
   *  - PWM: pause 600탎, pulse 0=600탎 1=1200탎
   *  - bit mode: LSB
   *  - format: <start><command:7><address:5>
   *  - code becomes valid after sending it 3 times
   *    delay is 45ms between start and start
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
      IR_Send_Pulse(IR_PULSE, 2400);
      IR_Send_PWM(&IR_Code[0], 12, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);

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
   *    delay is 45ms between start and start
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
      IR_Send_Pulse(IR_PULSE, 2400);
      IR_Send_PWM(&IR_Code[0], 15, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);

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
   *    delay is 45ms between start and start
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
    PutBits(Temp, 5, 13, IR_LSB);       /* extended, 8 bits */

    /* calculate delay between code frames */
    Temp = 45000 - 2400;                /* time for start pulse */
    Temp -= CodeTime(&IR_Code[0], 20, 600, 600, 1200);

    /* send code three times */
    n = 3;
    while (n > 0)
    {
      IR_Send_Pulse(IR_PULSE, 2400);
      IR_Send_PWM(&IR_Code[0], 20, 600, 600, 1200);
      IR_Send_Pulse(IR_PAUSE, Temp);

      n--;               /* next run */
    }
  }


  /*
   *  Sharp
   *  - no start / AGC burst
   *  - PDM: pulse 320탎, pause 0=680탎 1=1680탎
   *  - bit mode: LSB
   *  - stop: pulse 320탎
   *  - second packet with 40ms delay
   *  - format packet #1:
   *    <address:5><command:8><expansion:1><check:1><stop>
   *  - format packet #2:
   *    <address:5><inverted command:8><inverted expansion:1><inverted check:1><stop>
   */


  /*
   *  standard RC-5
   *  - 2 start bits: (889탎 L) 889탎 H, 889탎 L (889탎 H)
   *  - Bi-Phase (IEEE 802.3):
   *    0: pulse 889탎, pause 889탎
   *    1: pause 889탎, pulse 889탎
   *  - bit mode: MSB
   *  - format: <s1 "1":1><s2 "1":1><toggle:1><address:5><command:6>
   */


  /*
   *  standard RC-6 (RC6-0-16)
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


  #if 0
  /* debugging */
  LCD_ClearLine(6);
  LCD_CharPos(1, 6);
  DisplayHexByte(IR_Code[0]);
  LCD_Space();
  DisplayHexByte(IR_Code[1]);
  LCD_Space();
  DisplayHexByte(IR_Code[2]);
  LCD_Space();
  DisplayHexByte(IR_Code[3]);
  LCD_Space();
  #endif
}



/*
 *  send IR remote control codes/signals
 *  - uses probe #2 (OC1B) as output for IR LED
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - requires additional keys (e.g. rotary encoder)
 *    and display with more than ? lines
 *  - 
 */

void IR_RemoteControl(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Mode;               /* UI */
  uint8_t           n;                  /* counter */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Proto_ID;           /* protocol ID */
  /* carrier frequency */
  uint8_t           FreqTable[6] = {30, 33, 36, 38, 40, 56};
  uint8_t           FreqIndex;          /* index for FreqTable */
  uint8_t           DutyCycle;          /* carrier duty cycle */
  unsigned char     *ProtoStr = NULL;   /* string pointer (EEPROM) */
  uint16_t          Step = 0;           /* step size */
  uint16_t          Temp;               /* temporary value */
  /* data fields for IR code */
  #define FIELDS         3              /* number of data fields */
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
  LCD_Clear();
  LCD_EEString_Space(IR_Transmitter_str);    /* display: IR sender */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* display pinout (1: Gnd / 2: LED / 3: Gnd) */
  LCD_NextLine();
  Show_SimplePinout('-', 's', '-');
  TestKey(3000, CURSOR_NONE);        /* wait 3s / for key press */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* dedicated output via OC1B */
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


  /* power save mode would disable timer1 */
  Cfg.SleepMode = SLEEP_MODE_IDLE;      /* change sleep mode to Idle */

  /* enable OC1B pin and set timer mode */
  /* TCCR1A is set by IR_Send_Pulse() */
  /* TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1); */
  TCCR1B = (1 << WGM13) | (1 << WGM12);


  /* set start values */
  Proto_ID = IR_NEC_STD;                /* NEC Standard */
  FreqIndex = 3;                        /* 38 kHz */
  DutyCycle = 3;                        /* 1/3 */
  Mode = MODE_PROTO;                    /* */
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
        case IR_NEC_STD:           /* NEC Standard */
          ProtoStr = (unsigned char *)IR_NEC_Std_str;
          Bits[0] = 8;             /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          FreqIndex = 3;           /* 38kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_NEC_EXT:           /* NEC Extended */
          ProtoStr = (unsigned char *)IR_NEC_Ext_str;
          Bits[0] = 16;            /* address */
          Bits[1] = 8;             /* command */
          Fields = 2;              /* 2 data fields */
          FreqIndex = 3;           /* 38kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SAMSUNG:           /* Samsung / Toshiba */
          ProtoStr = (unsigned char *)IR_Samsung_str;
          Bits[0] = 8;             /* custom (address) */
          Bits[1] = 8;             /* data (command) */
          Fields = 2;              /* 2 data fields */
          FreqIndex = 3;           /* 38kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_12:           /* Sony SIRC 12 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_12_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 5;             /* address */
          Fields = 2;              /* 2 data fields */
          FreqIndex = 4;           /* 40kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_15:           /* Sony SIRC 15 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_15_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 8;             /* address */
          Fields = 2;              /* 2 data fields */
          FreqIndex = 4;           /* 40kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

        case IR_SIRC_20:           /* Sony SIRC 20 Bits */
          ProtoStr = (unsigned char *)IR_SIRC_20_str;
          Bits[0] = 7;             /* command */
          Bits[1] = 5;             /* address */
          Bits[2] = 8;             /* extended */
          Fields = 3;              /* 3 data fields */
          FreqIndex = 4;           /* 40kHz */
          DutyCycle = 3;           /* 1/3 */
          break;

// JVC 38kHz 1/3
// Kaseikyo 36kHz 1/3 (36.7)
// Matsushita 36kHz (36.7)
// Motorola 32kHz
// Proton 40kHz 1/3
// Sharp 38kHz 1/3
// RC-5 36kHz 1/3
// RC-6 36kHz 1/3
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
      LCD_EEString(ProtoStr);

      Flag &= ~DISPLAY_PROTO;           /* clear flag */
    }

    if (Flag & UPDATE_FREQ)        /* update carrier frequency */
    {
      Test = FreqTable[FreqIndex];      /* get frequency (in kHz) */

      /* display frequency */
      LCD_ClearLine(3);                 /* line #3 */
      LCD_CharPos(1, 3);
      MarkItem(MODE_FREQ, Mode);        /* mark mode if selected */
      DisplayValue(Test, 3, 'H');
      LCD_Char('z');

      /* display duty cycle */
      MarkItem(MODE_DUTYCYCLE, Mode);   /* mark mode if selected */
      LCD_Char('1');
      LCD_Char('/');
      LCD_Char('0' + DutyCycle);

      /* calculate top value for Timer1 (carrier) */
      /* top = (f_MCU / (prescaler * f_PWM)) - 1 */
      Temp = CPU_FREQ / 1000;           /* MCU clock in kHz */
      Temp /= Test;                     /* / f_PWM (in kHz) */
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
      LCD_CharPos(1, 4);

      n = 0;
      while (n < Fields)           /* loop through data fields */
      {
        MarkItem(n + MODE_DATA, Mode);  /* mark mode if selected */

        /* display data field */
        DisplayHexValue(Data[n], Bits[n]);

        n++;             /* next field */
      }

      Flag &= ~DISPLAY_DATA;            /* clear flag */
    }


    /*
     *  user feedback
     */

    Test = TestKey(0, CURSOR_NONE);     /* wait for user feedback */

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
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, CURSOR_NONE); /* check for second key press */

      if (Test > KEY_TIMEOUT)           /* second key press */
      {
        Flag = 0;                       /* end loop */
      }
      else                              /* single key press */
      {
        /* switch parameter */
        Mode++;                              /* next one */
        n = (MODE_DATA - 1) + Fields;        /* number of current modes */
        if (Mode > n) Mode = MODE_PROTO;     /* overflow */
        Flag |= DISPLAY_PROTO | UPDATE_FREQ | DISPLAY_DATA; /* update display */
      }
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
        if (Proto_ID > IR_PROTOMAX)     /* overflow */
        {
          Proto_ID = IR_NEC_STD;        /* reset to first one */
        }

        Flag |= CHANGE_PROTO | DISPLAY_PROTO | DISPLAY_DATA;
      }
      else if (Mode == MODE_FREQ)       /* carrier frequency mode */
      {
        FreqIndex++;                    /* next one */
        if (FreqIndex > 5)              /* overflow */
        {
          FreqIndex = 0;                /* reset to first one */
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
          Proto_ID = IR_PROTOMAX;       /* reset to last one */
        }

        Flag |= CHANGE_PROTO | DISPLAY_PROTO | DISPLAY_DATA;
      }
      else if (Mode == MODE_FREQ)       /* carrier frequency mode */
      {
        if (FreqIndex > 0)              /* within range */
        {
          FreqIndex--;                  /* previous one */
        }
        else                            /* underflow */
        {
          FreqIndex = 4;                /* reset to last one */
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
        LCD_CharPos(1, 5);           /* line #5 */
        LCD_EEString(IR_Send_str);   /* display: sending... */

        IR_Send_Code(Proto_ID, &Data[0]);

        LCD_ClearLine(5);            /* clear line #5 */
        n = 0;

        /* check if we should repeat sending */
        Test = TestKey(100, CURSOR_NONE);    /* get user feedback */
        if (Test == KEY_LONG)                /* long key press */
        {
          n = 2;              /* repeat code */
        }

        /* smooth UI or delay repeated code */
        MilliSleep(200);      /* take a short nap */
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

  Cfg.SleepMode = SLEEP_MODE_PWR_SAVE;  /* reset sleep mode to default */

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

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef IR_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

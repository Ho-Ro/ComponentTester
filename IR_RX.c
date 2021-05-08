/* ************************************************************************
 *
 *   IR remote control: receiver
 *
 *   (c) 2015-2021 by Markus Reschke
 *
 * ************************************************************************ */


/* local includes */
#include "config.h"           /* global configuration */

#if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)


/*
 *  local constants
 */

/* source management */
#define IR_RX_C


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

/* important constants */
#define IR_SAMPLE_PERIOD         50     /* 50 탎 */

/* code bit mode */
#define IR_LSB                    1     /* LSB */
#define IR_MSB                    2     /* MSB */

/* bi-phase modes (bitfield) */
#define IR_IEEE          0b00000001     /* IEEE bit encoding */
#define IR_THOMAS        0b00000010     /* Thomas bit encoding */
#define IR_PRE_PAUSE     0b00000100     /* heading pause */

/* timing control flags (bitfield) */
#define IR_STD_TOLER     0b00000000     /* use default tolerance */
#define IR_RELAX_SHORT   0b00000001     /* relax short pulses */
#define IR_RELAX_LONG    0b00000010     /* relax long pulses */

/* signal types */
#define IR_PAUSE         0b00000001     /* pause */
#define IR_PULSE         0b00000010     /* pulse */


/*
 *  local variables
 */

/* decoding logic */
uint8_t             IR_State = 0;            /* multi packet protocol state */

/* multi packet data fields */
uint8_t             IR_Data_1;               /* data field #1 */



/* ************************************************************************
 *   IR detection/decoder tool (receiver)
 * ************************************************************************ */


/*
 *  check if pulse/pause duration matches reference value
 *
 *  requires:
 *  - time units of pause/pulse
 *  - time units of reference period
 *  - timing control
 *    IR_STD_TOLER   - use standard tolerance
 *    IR_RELAX_SHORT - increase tolerance for short pulses/pauses
 *    IR_RELAX_LONG  - increase tolerance for long pulses/pauses
 *
 *  returns:
 *  - 0 for no match
 *  - 1 for match
 */

uint8_t PulseCheck(uint8_t PulseWidth, uint8_t Ref, uint8_t Control)
{
  uint8_t           Flag;          /* return value */

  /* define tolerance based on value */
  if (Ref > 10)               /* long pulse */
  {
    Flag = 3;                 /* default: 3 units */

    /* be tolerent with long pulses/pauses */
    if (Control & IR_RELAX_LONG)   /* relax timing */
    {
      Flag = 5;               /* increase to 5 units */
    }
  }
  else                        /* short pulse */
  {
    Flag = 1;                 /* default: 1 unit */

    /* be tolerent with short pulses/pauses */
    if (Control & IR_RELAX_SHORT)  /* relax timing */
    {
      Flag = 3;               /* increase to 3 units */
    }
  }

  /* prevent underflow for very short pulses/pauses */
  if (Flag > Ref)
  {
    Flag = Ref;               /* set to reference value */
  }

  /* calculate min and max values */
  Control = Ref;
  Control += Flag;            /* upper limit */
  Ref -= Flag;                /* lower limit */

  Flag = 0;                   /* reset flag */

  /* check if pulse duration is within allowed time window */
  if ((PulseWidth >= Ref) && (PulseWidth <= Control))
  {
    Flag = 1;
  }

  return Flag;
}



/*
 *  Bi-Phase demodulation
 *  - similar to Manchester encoding
 *  - special case of BPSK (binary phase-shift keying)
 *
 *  requires:
 *  - pointer to pulse/pause duration data
 *    first item has to be a pulse
 *  - number of pulses/pauses
 *  - mode: Thomas or IEEE, heading pause
 *    IR_IEEE      - decoding based on IEEE 802.3
 *    IR_THOMAS    - decoding based on G.E. Thomas
 *    IR_PRE_PAUSE - heading pause (not logged)
 *  - time units of clock half cycle
 *  - timing control
 *    IR_STD_TOLER   - use standard tolerance
 *    IR_RELAX_SHORT - increase tolerance for short pulses/pauses
 *    IR_RELAX_LONG  - increase tolerance for long pulses/pauses
 *
 *  returns:
 *  - 0 for any error
 *  - number of bits
 */

uint8_t BiPhase_Demod(uint8_t *PulseData, uint8_t Pulses, uint8_t Mode, uint8_t tH, uint8_t Control)
{
  uint8_t           Counter = 0;   /* return value: number of bits */
  uint8_t           Pulse = 1;     /* pulse counter */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Width;         /* 1x / 2x pulse width */
  uint8_t           Dir;           /* direction of pulse change */
  uint8_t           PrePulse = 0;  /* pulse for half-bit */
  uint8_t           Data = 0;      /* code byte */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Bits = 0;      /* counter for bits */
  uint8_t           Bytes = 0;     /* counter for bytes */

  Code = &IR_Code[0];              /* set start address */

  /*
   *  Bi-Phase Modulation:
   *  - fixed time slot for each bit
   *  - bit is encoded as a phase shift (H-L or L-H transition)
   *  - two encoding conventions
   *    - G.E. Thomas (Manchester II or Biphase-L)
   *      pause - pulse -> 0
   *      pulse - pause -> 1
   *    - IEEE 802.3
   *      pulse - pause -> 0
   *      pause - pulse -> 1
   */

  /* take care about heading pause */
  if (Mode & IR_PRE_PAUSE) PrePulse = 1;

  while (Pulse <= Pulses)     /* process all data items */
  {
    Time = *PulseData;        /* get duration */
    Dir = 0;                  /* reset pulse change direction */
    Width = 1;                /* reset pulse width */

    /* check for pulse duration */
    if (PulseCheck(Time, tH, Control))            /* half clock cycle */
    {
      /* do nothing */
    }
    else if (PulseCheck(Time, tH * 2, Control))   /* full clock cycle */
    {
      Width++;                /* double pulse */
    }
    else                                          /* wrong duration */
    {
      Counter = 0;            /* signal error */
      break;                  /* exit loop */
    }

    if (Pulse % 2 == 0)       /* pause */
    {
      if (PrePulse == 2)      /* prior pulse */
      {
        Dir = 1;              /* H-L change */

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
          Counter = 0;        /* signal error */
          break;              /* exit loop */
        }
      }
    }
    else                      /* pulse */
    {
      if (PrePulse == 1)      /* prior pause */
      {
        Dir = 2;              /* L-H change */

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
          Counter = 0;        /* signal error */
          break;              /* exit loop */
        }
      }
    }

    /* process pulse change */
    if (Dir)                  /* got a pulse change */
    {
      Bits++;                      /* got another bit */
      Counter++;                   /* increase counter */
      Data <<= 1;                  /* shift left for new bit */

      if (Mode & IR_THOMAS)        /* Thomas */
      {
        if (Dir == 1)              /* H-L is 1 */
        {
          Data |= 0b00000001;      /* set bit */
        }
      }
      else                         /* IEEE */
      {
        if (Dir == 2)              /* L-H is 1 */
        {
          Data |= 0b00000001;      /* set bit */
        }
      }

      if (Bits == 8)          /* completed byte */
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
    if (Pulse == Pulses)         /* last pause/pulse */
    {
      /* only for pulse */
      if (PrePulse == 2)         /* first half of cycle */
      {
        /* assume that pause follows and simulate it */
        *PulseData = tH;         /* half cycle */
        PulseData--;             /* stay here */
        Pulses++;                /* add trailing pause */
      }
    }

    PulseData++;              /* next one */
    Pulse++;                  /* next one */
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

  return Counter;
}



#ifdef SW_IR_RX_EXTRA

/*
 *  PPM demodulation
 *  - Pulse Position Modulation
 *
 *  requires:
 *  - pointer to pulse/pause duration data
 *    (first one has to be a pulse)
 *  - number of pulses/pauses
 *  - time units of pulse/pause
 *    should be lower end of valid range
 *  - number of time slots (= bits) expected
 *  - timing control
 *    IR_STD_TOLER   - use standard tolerance
 *    IR_RELAX_SHORT - increase tolerance for short pulses/pauses
 *    IR_RELAX_LONG  - increase tolerance for long pulses/pauses
 *
 *  returns:
 *  - 0 for any error
 *  - number of bits
 */

uint8_t PPM_Demod(uint8_t *PulseData, uint8_t Pulses, int8_t tP, uint8_t Slots, uint8_t Control)
{
  uint8_t           Counter = 0;   /* return value */
  uint8_t           Pulse = 1;     /* pulse counter */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Data = 0;      /* code byte */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Bits = 0;      /* counter for bits */
  uint8_t           Bytes = 0;     /* counter for bytes */
  uint8_t           n;             /* counter */

  Code = &IR_Code[0];              /* set start address */

  /*
   *  PPM:
   *  - fixed time slot for each bit
   *  - pause -> 0
   *  - pulse -> 1
   */

  while (Pulse <= Pulses)     /* process all data items */
  {
    Time = *PulseData;        /* get duration */

    /* estimate number of bits for current pulse/pause */
    n = Time / tP;            /* number of bits */
    Time = Time % tP;         /* remaining time */
    Time /= n;                /* offset per bit */

    /* check if time is in tolerance */
    if (! PulseCheck(tP + Time, tP, Control))
    {
      Counter = 0;            /* signal error */
      break;                  /* exit loop */
    }

    Bits += n;                /* add to number of bits */
    Counter += n;             /* add to totals */

    /* copy bits */
    while (n > 0)
    {
      Data <<= 1;             /* shift left for new bit */

      /* set bit in data */
      if (Pulse % 2 != 0)     /* pulse */
      {
        Data |= 0b00000001;        /* "1" */
      }
      /* otherwise stays "0" for pause */

      if (Bits == 8)          /* completed byte */
      {
        *Code = Data;              /* save code byte */
        Bytes++;                   /* got another byte */
        Data = 0;                  /* reset code byte */
        Bits = 0;                  /* reset bit counter */

        if (Bytes < IR_CODE_BYTES)      /* prevent overflow */
        {
          Code++;                  /* next code byte */
        }
      }

      n--;                    /* next bit */
    }

    /* manage trailing pauses */
    if (Pulse == Pulses)      /* last pulse */
    {
      if (Counter < Slots)         /* we have trailing pauses */
      {
        /* create pseudo pause(s) */
        Time = Slots - Counter;    /* number of bits missing */
        Time *= tP;                /* * pause duration */
        *PulseData = Time;         /* save time */
        PulseData--;               /* re-use last item */
        Pulses++;                  /* add pseudo pause */
      }
    }

    PulseData++;              /* next one */
    Pulse++;                  /* next one */
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

  return Counter;
}

#endif



/*
 *  PDM/PWM demodulation
 *  - PDM - Pulse Distance Modulation
 *  - PWM - Pulse Width Modulation
 *
 *  requires:
 *  - pointer to pulse/pause duration data
 *    first item has to be a pulse for PDM or a pause for PWM
 *  - number of pulses/pauses
 *  - time units of spacer
 *  - time units of 0
 *  - time units of 1
 *  - timing control
 *    IR_STD_TOLER   - use standard tolerance
 *    IR_RELAX_SHORT - increase tolerance for short pulses/pauses
 *    IR_RELAX_LONG  - increase tolerance for long pulses/pauses
 *
 *  returns:
 *  - 0 for any error
 *  - number of bits
 */

uint8_t PxM_Demod(uint8_t *PulseData, uint8_t Pulses, uint8_t tS, uint8_t t0, uint8_t t1, uint8_t Control)
{
  uint8_t           Counter = 0;   /* return value: number of bits */
  uint8_t           Pulse = 1;     /* pulse counter */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Data = 0;      /* code byte */
  uint8_t           *Code;         /* pointer to code data */
  uint8_t           Bits = 0;      /* counter for bits */
  uint8_t           Bytes = 0;     /* counter for bytes */

  Code = &IR_Code[0];              /* set start address */

  /*
   *  PWM / pulse encoding:
   *  - fixed pause time
   *  - two different pulse times to encode 0/1
   *  - even number of data items
   *
   *  PDM / space encoding:
   *  - fixed pulse time
   *  - two different pause times to encode 0/1
   *  - last item is pulse (stop bit)
   *    required to indicate end of pause of last data bit
   *  - odd number of data items
   */

  while (Pulse <= Pulses)     /* process all data items */
  {
    Time = *PulseData;        /* get duration */

    if (Pulse % 2 == 0)       /* pulse/pause with variable time */
    {
      Bits++;                           /* got another bit */
      Counter++;                        /* increase counter for total bits */
      Data <<= 1;                       /* shift left for new bit */

      if (PulseCheck(Time, t0, Control))          /* 0 */
      {
        /* set 0 ;) */
      }
      else if (PulseCheck(Time, t1, Control))     /* 1 */
      {
        Data |= 0b00000001;             /* set bit */
      }
      else                              /* invalid pulse */
      {
        Counter = 0;                    /* signal error */
        break;                          /* exit loop */
      }

      if (Bits == 8)          /* completed byte */
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
    else                      /* pulse/pause with fixed time */
    {
      if (! PulseCheck(Time, tS, Control))   /* time doesn't match */
      {
        Counter = 0;               /* signal error */
        break;                     /* exit loop */
      }
    }

    PulseData++;              /* next one */
    Pulse++;                  /* next one */
  }

  #if 0
  /* debugging output */
  if (Counter == 0)           /* error */
  {
    Display_NextLine();
    Display_Value(Counter, 0, 0);  /* display pulse number */
    Display_Colon();               /* display; : */
    Display_Value(Time, 0, 0);     /* display pulse duration */
  }
  #endif


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

  return Counter;
}



/*
 *  adjust special bi-phase pulse pair to standard timing
 *
 *  requires:
 *  - pointer to pulse/pause duration data
 *    first item has to be a pulse
 *  - number of pulses
 *  - offset to special pulse-pause pair (half cycles)
 *  - time units of normal pulse/pause
 *  - time units of special pulse/pause
 *
 *  returns:
 *  - number of special pulses
 */

uint8_t SpecialBiPhasePulse(uint8_t *PulseData, uint8_t Pulses, uint8_t Offset, uint8_t Normal, uint8_t Special)
{
  uint8_t           Counter = 0;   /* return value: number of pulses */
  uint8_t           Mixed;         /* duration of mixed pulse */
  uint8_t           Time;          /* pulse duration */
  uint8_t           Cycles = 0;    /* half cycles */
  uint8_t           n = 0;         /* counter */

  Mixed = Normal + Special;        /* mixed pulse */

  while (Pulses > 0)
  {
    Time = *PulseData;        /* get duration */

    if (Cycles <= Offset)     /* offset not reached yet */
    {
      if (PulseCheck(Time, Normal, IR_STD_TOLER))      /* normal pulse */
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
      if (PulseCheck(Time, Special, IR_STD_TOLER))     /* special pulse */
      {
        *PulseData = Normal;            /* adjust to normal */
        Counter++;                      /* increase counter */

      }
      else if (PulseCheck(Time, Mixed, IR_STD_TOLER))  /* mixed pulse */
      {
        *PulseData = 2 * Normal;        /* adjust to normal double pulse */
        Counter++;                      /* increase counter */
      }

      n++;                         /* increase counter */
      if (n == 2) break;           /* exit loop for pulse pair */
    }

    Pulses--;                 /* next pulse */
    PulseData++;
  }

  return(Counter);
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
  uint8_t           Counter;       /* bit counter */
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
  Counter = StartBit % 8;          /* start bit in byte (0-) */
  Code = &IR_Code[Temp];           /* get start address */
  Temp = *Code;                    /* copy code byte */

  /* shift start bit to bit #7 */
  n = Counter;                /* number of positions to shift */
  while (n > 0)
  {
    Temp <<= 1;               /* shift left */
    n--;                      /* next bit */
  }

  /* get bits */
  Counter = 8 - Counter;      /* remaining bits in first byte */
  n = 1;
  while (n <= Bits)
  {
    Data <<= 1;               /* shift destination left */

    if (Temp & 0b10000000)    /* bit set */
    {
      Data |= 0b00000001;     /* set bit */
    }

    Temp <<= 1;               /* shift source left */

    if (n == Counter)         /* byte overflow */
    {
      Code++;                 /* next byte */
      Temp = *Code;           /* copy code byte */
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
 *  - pointer to array of pulse/pause duration data
 *  - number of pulses/pauses
 */

void IR_Decode(uint8_t *PulseData, uint8_t Pulses)
{
  uint8_t           Flag = 0;      /* control flag */
  uint8_t           *Pulse;        /* pointer to pause/pulse data */
  uint8_t           PulsesLeft;    /* remaining pulses/pauses */
  uint8_t           Time1;         /* time units #1 */
  uint8_t           Time2;         /* time units #2 */
  uint8_t           Bits = 0;      /* number of bits received */
  uint8_t           Address;       /* RC address */
  uint8_t           Command;       /* RC command */
  uint8_t           Extras = 0;    /* RC extra stuff */
  uint8_t           Temp;          /* temporary value */

  /* local constants for Flag */
  #define PROTO_UNKNOWN       0    /* unknown protocol */
  #define PROTO_DETECTED      1    /* protocol detected */
  #define PACKET_OK           2    /* valid packet */
  #define PACKET_DISPLAY      3    /* valid packet & display standard values */
  #define PACKET_MULTI        4    /* multi packet (more to follow) */

  if (Pulses < 2) return;     /* not enough pulses */


  /*
   *  figure out the IR protocol by checking the first pulse-pause pair,
   *  number of pulses/pauses and other features
   */

  /* get first pulse-pause pair */
  Pulse = PulseData;          /* first pulse */
  Time1 = *Pulse;             /* duration of first pulse */
  Pulse++;                    /* first pause */
  Time2 = *Pulse;             /* duration of first pause */
  Pulse++;                    /* second pulse (skip start pair) */
  PulsesLeft = Pulses - 2;    /* start pair done */


  /*
   *  NEC (킦D6121/킦D6122)
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - stop: pulse 560탎
   *  - standard format: 
   *    <start><address:8><inverted address:8><command:8><inverted command:8><stop>
   *  - extended format:
   *    <start><low address:8><high address:8><command:8><inverted command:8><stop>
   *  - repeat sequence:
   *    <pulse 9ms><pause 2.25ms><stop>
   *  - repeat delay is 108ms (start to start)
   */

  /*
   *  Sanyo (LC7461)
   *  - start: pulse 9ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - stop: pulse 560탎
   *  - format: 
   *    <start><custom code:13><inverted custom code:13><key data:8><inverted key data:8><stop>
   *  - repeat sequence:
   *    <start><stop>
   *  - repeat delay is 108ms (start to start) or 23.6ms (end to start)
   *  - carrier 38kHz (455kHz/12), duty cycle 1/3
   */

  if (PulseCheck(Time1, 179, IR_RELAX_LONG))      /* pulse 9ms */
  {
    if (PulseCheck(Time2, 89, IR_RELAX_LONG))     /* pause 4.5ms */
    {
      /* try to demodulate NEC */
      Bits = PxM_Demod(Pulse, PulsesLeft, 11, 11, 33, IR_STD_TOLER);

      if (Bits == 32)              /* we expect 32 bits for NEC */
      {
        Display_NL_EEString_Space(IR_NEC_str);    /* display protocol */

        Address = GetBits(1, 8, IR_LSB);     /* get address */
        Extras = GetBits(9, 8, IR_LSB);      /* get inverted address */
        Command = GetBits(17, 8, IR_LSB);    /* get command */

        /* determine protocol version */
        Temp = ~Extras;                 /* invert */
        if (Address != Temp)            /* address is not inverted */
        {
          /* extended format with 16 bit address */
          Display_HexByte(Extras);      /* display high address */
        }

        Flag = PACKET_DISPLAY;          /* packet ok & default output */
        goto result;                    /* skip other checks */
      }
      #ifdef SW_IR_RX_EXTRA
      else if (Bits == 42)         /* we expect 42 bits for Sanyo */
      {
        Display_NL_EEString_Space(IR_Sanyo_str);  /* display protocol */

        Address = GetBits(1, 8, IR_LSB);     /* get custom code LSB */
        Extras = GetBits(9, 5, IR_LSB);      /* get custom code MSB */
        Command = GetBits(27, 8, IR_LSB);    /* get key data */

        Display_HexByte(Extras);        /* display custom code MSB */
        Flag = PACKET_DISPLAY;          /* packet ok & default output */
        goto result;                    /* skip other checks */
      }
      #endif
    }
    else if (PulseCheck(Time2, 45, IR_STD_TOLER))      /* pause 2.25ms */
    {
      /* check for NEC repeat sequence */
      if (Pulses == 3)                       /* just 3 pulses */
      {
        if (PulseCheck(*Pulse, 11, IR_STD_TOLER))      /* pulse 560탎 */
        {
          Display_NL_EEString_Space(IR_NEC_str);       /* display protocol */
          Display_Char('R');                 /* display: R (for repeat) */
          Flag = PACKET_OK;                  /* packet ok */
          goto result;                       /* skip other checks */
        }
      }
    }
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

  if (PulseCheck(Time1, 162, IR_RELAX_LONG))      /* pulse 8ms */
  {
    if (PulseCheck(Time2, 80, IR_STD_TOLER))      /* pause 4ms */
    {
      if (PulsesLeft == 35)        /* correct number of pulses/pauses */
      {
        /* check if we got the sync pause */
        Pulse += 17;          /* 16 pulses for first part + terminating pulse */
        if (PulseCheck(*Pulse, 80, IR_STD_TOLER))    /* pause 4ms */
        {
          *Pulse = 11;                  /* change sync into "0" */
          Pulse -= 17;                  /* back to second pulse */

          Display_NL_EEString_Space(IR_Proton_str);    /* display protocol */
          Flag = PROTO_DETECTED;                  /* detected protocol */

          Bits = PxM_Demod(Pulse, PulsesLeft, 11, 11, 30, IR_STD_TOLER);
          if (Bits == 17)               /* we expect 8 + 1 + 8 bits */
          {
            Address = GetBits(1, 8, IR_LSB);      /* get address */
            Command = GetBits(10, 8, IR_LSB);     /* get command */
            Flag = PACKET_DISPLAY;                /* packet ok & default output */
          }

          goto result;                       /* skip other checks */
        }
      }
    }
  }


  /*
   *  JVC C8D8
   *  - start: pulse 8.44ms, pause 4.22ms
   *  - PDM: pulse 525탎, pause 0=525탎 1=1575탎
   *  - bit mode: LSB
   *  - stop: pulse 525탎
   *  - format: <start><address:8><command:8><stop>
   *  - repeat sequence format: <address:8><command:8><stop>
   *  - repeat sequence delay is <start> + 46.42ms (start to start)
   *  - alternative timings:
   *    - start: pulse 9.4ms, pause 4.05ms (188/81)
   *      PDM: pulse 560탎, pause 0:600탎 1:1620탎 
   *    - start: pulse 9ms, pause 4.2ms (180/84)
   *      PDM: pulse 550탎, pause 0:550탎 1:1580탎
   */

  if ((PulseCheck(Time1, 168, IR_STD_TOLER)) ||   /* pulse 8.44ms */
      (PulseCheck(Time1, 184, IR_RELAX_LONG)))    /* 9 - 9.5ms */
  {
    if (PulseCheck(Time2, 84, IR_STD_TOLER))      /* pause 4.22ms */
    {
      /* try to demodulate JVC */
      Bits = PxM_Demod(Pulse, PulsesLeft, 11, 11, 32, IR_STD_TOLER);

      if (Bits == 16)              /* we expect 16 bits */
      {
        Display_NL_EEString_Space(IR_JVC_str);    /* display protocol */

        Address = GetBits(1, 8, IR_LSB);     /* get address */
        Command = GetBits(9, 8, IR_LSB);     /* get command */

        Flag = PACKET_DISPLAY;               /* packet ok & default output */
        goto result;                         /* skip other checks */
      }
    }
  }


  /*
   *  Matsushita (Panasonic, MN6014)
   *  - start: pulse 3.5ms, pause 3.5ms
   *  - PDM: pulse 872탎, pause 0=872탎 1=2616탎
   *  - bit mode: LSB
   *  - stop: pulse 872탎
   *  - format 12 bits / C6D6:
   *    <start><custom code:6><data code:6><inverted custom code:6><inverted data code:6><stop>
   *  - format 11 bits / C5D6:
   *    <start><custom code:5><data code:6><inverted custom code:5><inverted data code:6><stop>
   *  - repeat delay is 104.7ms (start to start), or 34ms (end to start) for 12 bit format
   *    and 39.24ms (end to start) for 11 bit format
   */

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

  if (PulseCheck(Time1, 70, IR_STD_TOLER))        /* pulse 3.5ms */
  {
    /* Matsushita (Panasonic) */ 
    if (PulseCheck(Time2, 70, IR_STD_TOLER))      /* pause 3.5ms */
    {
      Display_NextLine();                         /* new line */
      Display_EEString(IR_Matsushita_str);        /* display protocol */
      Flag = PROTO_DETECTED;                      /* detected protocol */

      /* try to demodulate Matsushita */
      Bits = PxM_Demod(Pulse, PulsesLeft, 17, 17, 52, IR_RELAX_LONG);
      Temp = 0;

      if (Bits == 24)              /* we expect 24 bits for C6D6 */
      {
        Temp = '6';                /* char 6 */

        Address = GetBits(1, 6, IR_LSB);     /* get address */
        Command = GetBits(7, 6, IR_LSB);     /* get command */
        /* todo: check inverted address and command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
      }
      #ifdef SW_IR_RX_EXTRA
      else if (Bits == 22)         /* we expect 22 bits for C5D6 */
      {
        Temp = '5';                /* char 5 */

        Address = GetBits(1, 5, IR_LSB);     /* get address */
        Command = GetBits(6, 6, IR_LSB);     /* get command */
        /* todo: check inverted address and command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
      }
      #endif

      if (Temp > 0)                /* display version */
      {
        Display_Minus();           /* display: - */
        Display_Char(Temp);        /* display: 5 or 6 */
      }

      Display_Space();             /* display space */
      goto result;                 /* skip other checks */
    }

    /* Kaseikyo */
    if (PulseCheck(Time2, 34, IR_STD_TOLER))      /* pause 1728탎 */
    {
      Display_NL_EEString_Space(IR_Kaseikyo_str);      /* display protocol */
      Flag = PROTO_DETECTED;                      /* detected protocol */

      /* try to demodulate Kaseikyo */
      Bits = PxM_Demod(Pulse, PulsesLeft, 8, 8, 26, IR_RELAX_SHORT);

      if (Bits == 48)              /* we expect 48 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* get manufacturer LSB */
        Extras = GetBits(9, 8, IR_LSB);      /* get manufacturer MSB */

        Display_HexByte(Extras);        /* display manufacturer MSB */
        Display_HexByte(Address);       /* display manufacturer LSB */
        Display_Colon();                /* display: : */

        Extras = GetBits(21, 4, IR_LSB);     /* get system */
        Address = GetBits(25, 8, IR_LSB);    /* get product (address) */
        Command = GetBits(33, 8, IR_LSB);    /* get function (command) */      

        Display_HexDigit(Extras);  /* display system */
        Display_Minus();           /* display: - */
        Display_HexByte(Address);  /* display product */
        Display_Colon();           /* display: : */
        Display_HexByte(Command);  /* display function */

        Flag = PACKET_OK;          /* packet ok */
      }

      goto result;                 /* skip other checks */
    }
  }


  /*
   *  RCA
   *  - start: pulse 4ms, pause 4ms
   *  - PDM: pulse 500탎, pause 0=1ms 1=2ms
   *  - stop: pulse 500탎
   *  - bit mode: MSB
   *  - format: <start><address:4><command:8><inverted address:4><inverted command:8><stop>
   *  - repeat delay is 64ms (start to start) or 8ms (end to start)
   *  - alternative timing:
   *    - start: pulse 3.75ms, pause 3.9ms
   *    - PDM: pulse 560탎, pause 0:1ms 1:2ms
   *    - repeat delay 10ms (end to start)
   *  - carrier 56kHz or 38.7kHz, duty cycle 1/3
   */

  #ifdef SW_IR_RX_EXTRA
  if (PulseCheck(Time1, 79, IR_RELAX_LONG))       /* pulse 4ms */
  {
    if (PulseCheck(Time2, 79, IR_RELAX_LONG))     /* pause 4ms */
    {
      /*
       *  Packet repeat delay is shorter than sample timeout. So the sample
       *  data could include a partial second packet.
       */

      if (PulsesLeft > 49)         /* more pulses than we need */
      {
        /* check for repeat delay */
        Pulse += 49;               /* 24 bits + stop */

        if (*Pulse >= 158)         /* longer than 7.9ms */
        {
          PulsesLeft = 49;         /* hide second packet */
        } 

        Pulse -=  49;              /* back to second pulse */
      }

      /* try to demodulate RCA */
      Bits = PxM_Demod(Pulse, PulsesLeft, 10, 20, 40, IR_RELAX_SHORT);

      if (Bits == 24)              /* we expect 24 bits */
      {
        Display_NL_EEString_Space(IR_RCA_str);    /* display protocol */  

        Address = GetBits(1, 4, IR_MSB);     /* get address */
        Command = GetBits(5, 8, IR_MSB);     /* get command */

        /* todo: check inverted address and command? */

        Flag = PACKET_DISPLAY;               /* packet ok & default output */
        goto result;                         /* skip other checks */
      }
    }
  }
  #endif


  /*
   *  Motorola
   *  - start: pulse 512탎, pause 2560탎
   *  - Bi-Phase (Thomas):
   *    0: pause 512탎, pulse 512탎
   *    1: pulse 512탎, pause 512탎 
   *  - bit mode: LSB
   *  - a valid code consists of a start packet, one or more command packets
   *    and an end packet
   *  - command packets are repeated as long as key is pressed
   *  - start/end packet format: <start><start "1":1><all 1s:9>
   *  - command packet format: <start><start "1":1><command:9>
   *  - packet delay is 32.8ms (start to start) between command packets
   *    and 131ms (start to start) for start-command and command-end
   */

  /*
   *  IR60 (SDA2008/MC14497)
   *  - start: pulse 550탎, pause 2.5ms
   *  - Bi-Phase (Thomas):
   *    0: pause 550탎, pulse 550탎
   *    1: pulse 550탎, pause 550탎
   *  - bit mode: LSB
   *  - a valid code consists of a start packet, one or more command packets
   *    and an end packet
   *  - packet delay is 34ms (start to start) or 24ms (end to start) between
   *    start packet and first command packet
   *  - packet delay is 120ms (end to start) for all other packets following the
   *    first command packet
   *  - start/end packet format: <start><start "1":1><command "62":6>
   *  - command packet format: <start><start "1":1><command:6>
   *  - carrier 31kHz, duty cycle 1/4
   */

  if (PulseCheck(Time1, 11, IR_STD_TOLER))        /* pulse 512탎 */
  {
    if (PulseCheck(Time2, 52, IR_STD_TOLER))      /* pause 2560탎 */
    {
      /* try to demodulate Motorola or IR60 */
      Bits = BiPhase_Demod(Pulse, PulsesLeft, IR_THOMAS, 11, IR_STD_TOLER);

      if (Bits == 10)              /* we expect 10 bits for Motorola */
      {
        /* todo: check start bit? */
        Command = GetBits(2, 8, IR_LSB);     /* get command LSB */
        Extras = GetBits(10, 1, IR_LSB);     /* get command MSB */

        /*
         *  multi packet logic
         */

        Flag = PACKET_MULTI;       /* multi packet protocol */

        /* check for packet type */
        if ((Command == 0b11111111) && (Extras == 0b00000001))
        {
          /* start or end packet: all 1s */
          if (IR_State == 0)            /* no packet yet */
          {
            IR_State = 1;               /* we have a start packet */
          }
          else if (IR_State == 2)       /* command packet */
          {
                                        /* we have a stop packet */
            Flag = PACKET_OK;           /* packet ok */
          }
          /* else: packet missing/broken */
        }
        else                            /* command packet */
        {
          /* command packet */
          Display_NL_EEString_Space(IR_Motorola_str);  /* display protocol */
          Display_HexDigit(Extras);     /* display command MSB */
          Display_HexByte(Command);     /* display command LSB */

          IR_State = 2;                 /* we have a command packet */
        }

        goto result;               /* skip other checks */
      }
      #ifdef SW_IR_RX_EXTRA
      else if (Bits == 7)               /* we expect 7 bits for IR60 */
      {
        /* todo: check start bit? */
        Command = GetBits(2, 6, IR_LSB);     /* get command */

        /*
         *  multi packet logic
         */

        Flag = PACKET_MULTI;       /* multi packet protocol */

        /* check for packet type */
        if (Command == 62)              /* start/end packet (command 62) */
        {
          if (IR_State == 0)            /* no packet yet */
          {
            IR_State = 1;               /* we have a start packet */
          }
          else if (IR_State == 2)       /* command packet */
          {
                                        /* we have a stop packet */
            Flag = PACKET_OK;           /* packet ok */
          }
        }
        else                            /* command packet */
        {
          Display_NL_EEString_Space(IR_IR60_str);   /* display protocol */
          Display_HexByte(Command);                 /* display command */

          IR_State = 2;                 /* we have a command packet */
        }

        goto result;               /* skip other checks */
      }
      #endif
    }
  }


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
   *  - alternative timing:
   *    PDM: pulse 470탎, pause 0:2.05ms 1:4.6ms
   *    stop: pulse 470탎
   */

  #ifdef SW_IR_RX_EXTRA
  if (PulseCheck(Time1, 10, IR_STD_TOLER))        /* pulse 500탎 */
  {
    if ((PulseCheck(Time2, 40, IR_STD_TOLER)) ||  /* pause 2ms */
        (PulseCheck(Time2, 90, IR_STD_TOLER)))    /* pause 4.5ms */
    {
      /* try to demodulate Thomson */
      Bits = PxM_Demod(PulseData, Pulses, 10, 40, 90, IR_RELAX_SHORT);

      if (Bits == 12)                   /* we expect 12 bits */
      {
        Display_NL_EEString_Space(IR_Thomson_str);     /* display protocol */

        Address = GetBits(1, 4, IR_LSB);     /* get device (address) */
        Command = GetBits(6, 7, IR_LSB);     /* get function (command) */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
        goto result;               /* skip other checks */
      }
    }
  }
  #endif


  /*
   *  Samsung / Toshiba  (TC9012)
   *  - start: pulse 4.5ms, pause 4.5ms
   *  - PDM: pulse 560탎, pause 0=560탎 1=1690탎
   *  - bit mode: LSB
   *  - stop: pulse 560탎
   *  - old format: <start><manufacturer code:12><command:8><stop>
   *  - format:
   *    <start><custom:8><copy of custom:8><data:8><inverted data:8><stop>
   *  - repeat sequence format:
   *    <start><inverted bit #0 of custom><stop>
   *  - repeat delay is 108ms (start to start)
   */

  if (PulseCheck(Time1, 89, IR_STD_TOLER))        /* pulse 4.5ms */
  {
    if (PulseCheck(Time2, 89, IR_STD_TOLER))      /* pause 4.5ms */
    {
      Display_NL_EEString_Space(IR_Samsung_str);  /* display protocol */
      Flag = PROTO_DETECTED;                      /* detected protocol */

      /* try to demodulate Samsung */
      Bits = PxM_Demod(Pulse, PulsesLeft, 11, 11, 34, IR_STD_TOLER);

      if (Bits == 32)              /* we expect 32 bits */
      {
        Address = GetBits(1, 8, IR_LSB);     /* get address */
        Command = GetBits(17, 8, IR_LSB);    /* get command */
        /* todo: check copy of address and inverted command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
      }

      goto result;                 /* skip other checks */
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
   *    code delay is 45ms (start to start)
   */

  if (PulseCheck(Time1, 48, IR_STD_TOLER))        /* pulse 2.4ms */
  {
    if (PulseCheck(Time2, 12, IR_STD_TOLER))      /* pause 600탎 */
    {
      Display_NextLine();               /* new line */
      Display_EEString(IR_SIRC_str);    /* display protocol */
      Flag = PROTO_DETECTED;            /* detected protocol */

      /* try to demodulate SIRC */
      Pulse--;                          /* back to first pause */
      PulsesLeft++;                     /* undo first pause */
      Bits = PxM_Demod(Pulse, PulsesLeft, 12, 12, 24, IR_STD_TOLER);

      /* 12, 15 or 20 bit format */
      Temp = 5;                    /* 5 bit address */
      if (Bits == 12)              /* SIRC-12 */
      {
        Flag = PACKET_OK;          /* packet ok */
      }
      else if (Bits == 15)         /* SIRC-15 */
      {
        Flag = PACKET_OK;          /* packet ok */
        Temp = 8;                  /* 8 bit address */
      }
      else if (Bits == 20)         /* SIRC-20 */
      {
        Flag = PACKET_OK;          /* packet ok */
      }

      Command = GetBits(1, 7, IR_LSB);       /* get command */
      Address = GetBits(8, Temp, IR_LSB);    /* get address */

      /* display format */
      if (Flag == PACKET_OK)       /* valid packet */
      {
        Display_Value(Bits, 0, 0); /* display number of bits */
      }

      /* we accept the first code and don't wait for another 2 repeats */

      Display_Space();             /* display space */

      /* display data */
      if (Flag == PACKET_OK)       /* valid packet */
      {
        Display_HexByte(Command);  /* display command */
        Display_Colon();           /* display: : */
        Display_HexByte(Address);  /* display address */

        if (Bits == 20)            /* 20 bit format */
        {
          Display_Colon();                   /* display: : */
          Extras = GetBits(13, 8, IR_LSB);   /* get extended */
          Display_HexByte(Extras);           /* display extended data */
        }
      }

      goto result;                 /* skip other checks */
    }
  }


  /*
   *  RECS80 Standard (SAA3008, M3004)
   *  - no start / AGC burst
   *  - PDM: pulse 140.8탎, pause 0:5060탎 1:7590ms
   *  - stop: pulse 140.8탎
   *  - bit mode: MSB
   *  - format: <reference "1":1><toggle:1><address:3><command:6><stop>
   *  - toggle: inverted each time a key is pressed
   *            stays the same when key is still pressed
   *  - packet repeat delay is 121.44ms (start to start)
   *  - alternative timing:
   *    PDM: pulse 180탎, pause 0:4850탎 1:7.4ms
   *  - carrier 38kHz (455kHz/12), duty cycle 1/3
   */

  /*
   *  RECS80 Extended (SAA3008)
   *  - start: pulse 140.8탎, pause 3659.2탎, pulse 140.8탎, pause 3659.2탎
   *  - PDM: pulse 140.8탎, pause 0=4919.2탎 1=7459.2탎
   *  - bit mode: MSB
   *  - stop: pulse 140.8탎
   *  - format: <start><toggle:1><address:4><command:6><stop>
   *  - toggle: inverted each time a key is pressed
   *            stays same when key is still pressed
   *  - repeat delay is 132.56ms (start to start)
   *  - carrier 38kHz (455kHz/12), duty cycle 1/3
   */

  #ifdef SW_IR_RX_EXTRA
  if (PulseCheck(Time1, 4, IR_STD_TOLER))         /* pulse 141탎 */
  {
    if (PulseCheck(Time2, 149, IR_RELAX_LONG))    /* pause 7.6ms (for 1) */
    {
      /* try to demodulate RECS80 Standard */
      Bits = PxM_Demod(PulseData, Pulses, 4, 98, 149, IR_RELAX_SHORT | IR_RELAX_LONG);

      if (Bits == 11)                   /* we expect 11 bits */
      {
        Display_NL_EEString_Space(IR_RECS80_str);      /* display protocol */

        Address = GetBits(3, 3, IR_MSB);     /* get subsystem address */
        Command = GetBits(6, 6, IR_MSB);     /* get command */

        Flag = PACKET_DISPLAY;     /* packet ok & default output */
        goto result;               /* skip other checks */
      }
    }
    else if (PulseCheck(Time2, 173, IR_RELAX_LONG))  /* pause 3659탎 */
    {
      /* todo: check timing for second half of start? */

      if (PulsesLeft == 25)        /* 2 + 22 + 1 */
      {
        /* skip second half of start */
        Pulse += 2;
        PulsesLeft -= 2;

        /* try to demodulate RECS80 Extended */
        Bits = PxM_Demod(Pulse, PulsesLeft, 4, 98, 149, IR_RELAX_SHORT | IR_RELAX_LONG);

        if (Bits == 11)                   /* we expect 11 bits */
        {
          Display_NL_EEString_Space(IR_RECS80_str);    /* display protocol */
          Display_Char('x');                           /* display: x */
          Display_Space();

          Address = GetBits(2, 4, IR_MSB);        /* get subsystem address */
          Command = GetBits(6, 6, IR_MSB);        /* get command */

          Flag = PACKET_DISPLAY;     /* packet ok & default output */
          goto result;               /* skip other checks */
        }
      }
    }
  }
  #endif


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
   *  - alternative timing:
   *    - pulse 320탎, pause 0=680탎 1=1680탎
   */

  if (PulseCheck(Time1, 6, IR_STD_TOLER))    /* pulse 320탎 */
  {
    /* pause 680탎 or 1680탎 */
    if ((PulseCheck(Time2, 14, IR_STD_TOLER)) ||
        (PulseCheck(Time2, 35, IR_STD_TOLER)))
    {
      Flag = PROTO_DETECTED;       /* detected protocol */

      /* try to demodulate Sharp */
      Bits = PxM_Demod(PulseData, Pulses, 6, 14, 35, IR_STD_TOLER);

      if (Bits == 15)              /* we expect 15 bits */
      {
        if (IR_State == 0)         /* packet #1 */
        {
          Address = GetBits(1, 5, IR_LSB);     /* get address */
          Command = GetBits(6, 8, IR_LSB);     /* get command */
          /* todo: mask & type bits */

          Display_NL_EEString_Space(IR_Sharp_str);  /* display protocol */
          Display_HexByte(Address);                 /* display address */
          Display_Colon();                          /* display: : */
          Display_HexByte(Command);                 /* display command */

          IR_State = 1;                 /* got packet #1 */
          Flag = PACKET_MULTI;          /* multi packet */
        }
        else                       /* packet #2 */
        {
          /* we don't check the inverted command and extra bits */
          Flag = PACKET_OK;             /* packet ok */
        }
      }

      goto result;                 /* skip other checks */
    }
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
   *            stays the same when key is still pressed
   *  - repeat delay is 114ms (start to start) or 89ms (end to start)
   */

  /*
   *  Philips RC-5 Extended
   *  - 2 start bits: (889탎 L) 889탎 H + 889탎 H (889탎 L)
   *  - Bi-Phase (IEEE 802.3):
   *    0: pulse 889탎, pause 889탎
   *    1: pause 889탎, pulse 889탎
   *  - bit mode: MSB
   *  - format: <s1 "1":1><s2 "0":1><toggle:1><address:5><command:6>
   *  - s2: 0 to indicate RC-5 Extended
   *        inverted s2 becomes bit #6 of command (| 0b1000000)
   *  - toggle: inverted each time a key is pressed
   *            stays the same when key is still pressed
   *  - repeat delay is 114ms (start to start) or 89ms (end to start)
   */

  if (PulseCheck(Time1, 17, IR_STD_TOLER))        /* pulse 889탎 */
  {
    if (PulseCheck(Time2, 17, IR_STD_TOLER))      /* pause 889탎 */
    {
      Display_NL_EEString_Space(IR_RC5_str);      /* display protocol */
      Flag = PROTO_DETECTED;                 /* detected protocol */

      /* try to demodulate RC-5 */
      Bits = BiPhase_Demod(PulseData, Pulses, IR_IEEE | IR_PRE_PAUSE, 17, IR_STD_TOLER);

      if (Bits == 14)              /* we expect 14 bits */
      {
        Address = GetBits(4, 5, IR_MSB);     /* get address */
        Command = GetBits(9, 6, IR_MSB);     /* get command */

        Flag = PACKET_DISPLAY;     /* packet ok & default ouput */
      }

      goto result;                 /* skip other checks */
    }
  }


  /*
   *  NEC 킦D1986C (NTE1758)
   *  - no start / AGC burst
   *  - PPM: 1120탎, 0:pause 1:pulse
   *  - bit mode: LSB
   *  - 2 packets (original plus one repeat)
   *  - format: <header "101":3><data:5>
   *  - repeat delay is 36ms (start to start)
   *  - carrier 38kHz (455kHz/12), duty cycle 1/2
   *  - alternative timing:
   *    PPM: 0:pause 1:pulse 1590탎
   *    packet delay 51,2ms (start to start)
   *    carrier 26.7kHz (320kHz/12)
   */

  #ifdef SW_IR_RX_EXTRA
  if (PulseCheck(Time1, 22, IR_STD_TOLER))        /* pulse 1120탎 (for 1) */
  {
    if (PulseCheck(Time2, 22, IR_STD_TOLER))      /* pause 1120탎 (for 0) */
    {
      /* try to demodulate */
      Bits = PPM_Demod(PulseData, Pulses, 22, 8, IR_STD_TOLER);

      if (Bits == 8)                    /* we expect 8 bits */
      {
        Display_NL_EEString_Space(IR_uPD1986C_str);    /* display protocol */

        /* todo: check header? */
        Command = GetBits(4, 5, IR_LSB);     /* get command */
        Display_HexByte(Command);            /* display command */

        Flag = PACKET_OK;          /* packet ok */
        goto result;               /* skip other checks */
      }
    }
  }
  #endif


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

  if (PulseCheck(Time1, 53, IR_STD_TOLER))        /* pulse 2664탎 */
  {
    if (PulseCheck(Time2, 17, IR_STD_TOLER))      /* pause 888탎 */
    {
      Display_NL_EEString_Space(IR_RC6_str);      /* display protocol */
      Flag = PROTO_DETECTED;                 /* detected protocol */

      /* convert toggle bit to standard timing */
      Temp = SpecialBiPhasePulse(Pulse, PulsesLeft, 8, 8, 17);

      if (Temp == 2)               /* we expect 1 special bit (= 2 pulses) */
      {
        /* try to demodulate */
        Bits = BiPhase_Demod(Pulse, PulsesLeft, IR_THOMAS, 8, IR_RELAX_SHORT);

        if (Bits == 21)       /* we expect 21 bits */
        {
          Address = GetBits(6, 8, IR_MSB);   /* get address */
          Command = GetBits(14, 8, IR_MSB);  /* get command */

          Flag = PACKET_DISPLAY;   /* packet ok & default ouput */
        }
      }

      /* don't need to "goto" - this is the last protocol */
    }
  }


  /*
   *  process result
   */

result:

  if (Flag == PROTO_UNKNOWN)       /* unknown protocol */
  {
    Display_NextLine();            /* new line */
    Display_Value(Pulses, 0, 0);   /* display number of pulses */
    Display_Colon();               /* display: : */
    Display_Value(Time1, 0, 0);    /* display time units of first pulse */
    Display_Minus();               /* display: - */
    Display_Value(Time2, 0, 0);    /* display time units of first pause */

    #if 0
    /* debugging output */
    Display_NextLine();
    Pulse++;
    Display_Value(*Pulse, 0, 0);
    Display_Space();
    Pulse++;
    Display_Value(*Pulse, 0, 0);
    Display_Space();
    Pulse++;
    Display_Value(*Pulse, 0, 0);
    Display_Space();
    Pulse++;
    Display_Value(*Pulse, 0, 0);
    Display_Space();
    Pulse++;
    Display_Value(*Pulse, 0, 0);
    #endif
  }

  if (Flag == PROTO_DETECTED)      /* bad packet */
  {
    Display_Char('?');             /* display: ? */
  }

  if (Flag == PACKET_DISPLAY)      /* known protocol & standard output */
  {
    Display_HexByte(Address);      /* display address */
    Display_Colon();               /* display: : */
    Display_HexByte(Command);      /* display command */
  }

  if (Flag < PACKET_MULTI)         /* no packets to follow (protocol done) */
  {
    IR_State = 0;                  /* reset multi packet state */

    /* slow down display updates and try to skip early repeats */
    MilliSleep(200);               /* don't proceed too soon */
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

  /* processing modes */
  #define MODE_WAIT           1         /* wait for next packet */
  #define MODE_SAMPLE         2         /* sample */
  #define MODE_DECODE         3         /* decode packet */
  #define MODE_KEY            4         /* check test button */

  uint8_t           Run = MODE_WAIT;    /* loop control */
  uint8_t           Flag;               /* IR signal */
  uint8_t           OldFlag = 0;        /* former IR signal */
  uint8_t           Cycles;             /* delay loop */
  uint8_t           n;                  /* counter */
  uint8_t           Pulses = 0;         /* pulse counter */
  uint8_t           Period = 0;         /* pulse duration */
  uint8_t           *Pulse = NULL;      /* pointer to pulse data */
  uint8_t           PulseData[MAX_PULSES];  /* pulse duration data */     

  ShortCircuit(0);                      /* make sure probes are not shorted */

  /* inform user */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: IR detector */
    Display_ColoredEEString(IR_Detector_str, COLOR_TITLE);
  #else
    Display_EEString(IR_Detector_str);  /* display: IR detector */
  #endif
  UI.LineMode = LINE_KEEP;              /* next-line mode: keep first line */
  #ifdef SW_IR_RECEIVER
    /* display module pinout (1: Gnd / 2: Vcc / 3: Data) */
    Display_NextLine();
    Show_SimplePinout('-', '+', 'd');
  #endif


  /*
   *  set up module
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
    if (ADC_PIN & (1 << TP3))      /* check for high level */
    #else
    if (IR_PIN & (1 << IR_DATA))   /* check for high level */
    #endif
    {
      n = 0;                            /* end this loop */
    }
    else                           /* check test key */
    {
      /* wait 100ms for key press */
      Flag = TestKey(100, CHECK_BAT);
      /* also delay for next loop run */

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
   *  Since we deal with data pulses in the range of 0.5 up to 10ms,
   *  we sample IR data every 50탎 and log how long each pulse or pause
   *  lasts. A pulse/pause exceeding 12ms triggers a timeout and won't
   *  be logged.
   */

  while (Run > 0)             /* processing loop */
  {
    /*
     *  read IR receiver module
     *  - data logic is inverted by IR receiver
     *    High: no IR signal / pause
     *    Low: IR signal / pulse
     */
   
    #ifdef SW_IR_RECEIVER
      /* IR receiver connected to probes */
      Flag = ADC_PIN & (1 << TP3);       /* poll data pin */
    #else
      /* fixed IR receiver */
      Flag = IR_PIN & (1 << IR_DATA);    /* poll data pin */
    #endif

    /*
     *  control logic for sampling
     *  - wait for new packet and sample
     */

    if (Run == MODE_WAIT)     /* wait for IR data */
    {
      if (!Flag)              /* got IR signal */
      {
        Run = MODE_SAMPLE;         /* start sampling */
        OldFlag = Flag;            /* first one is always a pulse */
        Pulses = 0;                /* reset pulse counter */
        Period = 0;                /* reset duration */
        Pulse = &PulseData[0];     /* set start address */
      }
      else                    /* no IR signal */
      {
        Run = MODE_KEY;            /* check for test key */
      }
    }
    else                      /* sample IR data */
    {
      if (Flag == OldFlag)    /* same pause/pulse */
      {
        Period++;             /* increase period */

        if (Period > 240)     /* 12ms timeout */
        {
          Run = MODE_DECODE;       /* switch to decoding mode */

          if (!Flag)               /* removed receiver module */
          {
            Run = MODE_KEY;        /* check for test key */
          }
        }
      }
      else                    /* new pause/pulse */
      {
        OldFlag = Flag;       /* update flag */

        if (Pulses < MAX_PULSES)   /* prevent buffer overflow */
        {
          Pulses++;                /* got another one */
          *Pulse = Period;         /* save duration */
          Pulse++;                 /* next one */
          Period = 0;              /* reset duration */
        }
        else                  /* max number of pulses exceeded */
        {
          Run = MODE_DECODE;       /* switch to decoding mode */
        }
      }
    }

    /*
     *  manage tasks
     */

    if (Run == MODE_SAMPLE)             /* sampling mode */
    {
      wait40us();             /* wait sampling period */

      /*
       *  adaptive delay for 10탎 considering processing loop
       *  - a loop run needs 4 cycles, the last loop run just 3
       *  - time delay: 10탎 - 1 MCU cycle
       */

      n = Cycles;             /* number loop runs */
      while (n > 0)           /* waiting loop */
      {
        n--;
        asm volatile("nop\n\t"::);
      }
    }
    else if (Run == MODE_DECODE)        /* decoding mode */
    {
      IR_Decode(&PulseData[0], Pulses);    /* try to decode */
      Run = MODE_WAIT;                     /* switch back to waiting mode */
    }
    else if (Run == MODE_KEY)           /* check for test key */
    {
      Run = MODE_WAIT;                     /* switch back to waiting mode */

      /* check test button */
      while (!(BUTTON_PIN & (1 << TEST_BUTTON)))  /* key pressed */
      {
        MilliSleep(50);            /* take a nap */
        Run = 0;                   /* end loop */
      }
    }

    wdt_reset();                   /* reset watchdog */
  }

  /* clean up local constants */
  #undef MAX_PULSES

  #undef MODE_WAIT
  #undef MODE_SAMPLE
  #undef MODE_DECODE
  #undef MODE_KEY
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef IR_RX_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

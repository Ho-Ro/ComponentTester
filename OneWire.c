/* ************************************************************************
 *
 *   OneWire communication and tools
 *
 *   (c) 2018-2020 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - DQ (data line) requires a 4.7kOhms pull-up resistor to Vcc
 *  - pin assignment for probes (ONEWIRE_PROBES)
 *    probe #1      Gnd
 *    probe #2      DQ (requires 4.7kOhms pull-up resistor to Vcc)
 *    probe #3      Vcc (via Rl to limit current)
 *  - port and pins for dedicated MCU pin (ONEWIRE_IO_PIN)
 *    ONEWIRE_PORT  port data register
 *    ONEWIRE_DDR   port data direction register
 *    ONEWIRE_PIN   port input pins register
 *    ONEWIRE_DQ    pin for DQ
 *  - standard-speed (accurate 1탎 delay)
 *  - external power for clients (no parasite power)
 */


/* local includes */
#include "config.h"           /* global configuration */

#if defined (ONEWIRE_IO_PIN) || defined (ONEWIRE_PROBES)


/*
 *  local constants
 */

/* source management */
#define ONEWIRE_C

/* operation modes */
#define MODE_MANUAL      0    /* manual mode */
#define MODE_AUTO        1    /* automatic mode */


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "OneWire.h"          /* OneWire specifics */


/*
 *  local variables
 */

/* CRC */
uint8_t        CRC8;          /* current CRC-8 */

#if defined (ONEWIRE_READ_ROM) || defined (SW_ONEWIRE_SCAN)
/* ROM code */
uint8_t        ROM_Code[8];   /* ROM code */
uint8_t        LastCon;       /* bit position of last code conflict */
#endif



/* ************************************************************************
 *   low level functions
 * ************************************************************************ */


#ifdef ONEWIRE_IO_PIN

/*
 *  set up OneWire bus
 *  - DQ line
 */

void OneWire_Setup(void)
{
  /*
   *  DQ is driven as open-drain output
   *  and pulled up by external 4.7kOhm resistor
   */

  /* set DQ to input mode */
  ONEWIRE_DDR &= ~(1 << ONEWIRE_DQ);         /* clear bit */

  /* preset DQ to low for output mode */
  ONEWIRE_PORT &= ~(1 << ONEWIRE_DQ);        /* clear bit */
}

#endif



#ifdef ONEWIRE_PROBES

/*
 *  set up probes for OneWire bus
 *  - probe-1: Gnd
 *  - probe-2: DQ (pull-up resistor required)
 *  - probe-3: Vcc (current limited by Rl)
 *
 *  requires:
 *  - String: string stored in EEPROM for first display line 
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t OneWire_Probes(const unsigned char *String)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Run = 1;       /* loop control */

  /* inform user in line #1 */
  ShortCircuit(0);                      /* make sure probes are not shorted */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    Display_ColoredEEString(String, COLOR_TITLE);   /* display String */
  #else
    Display_EEString(String);           /* display String */
  #endif

  /* display module pinout (1: Gnd / 2: Data / 3: Vcc) */
  Display_NextLine();
  Show_SimplePinout('-', 'd', '+');

  /* set probes: probe-1 -- Gnd / probe-3 -- Rl -- Vcc / probe-2 (HiZ) -- Rh -- Gnd */
  /* pull up probe-3 via Rl, pull down probe-2 via Rh */
  R_DDR = (1 << R_RL_3) | (1 << R_RH_2);     /* enable resistors */
  R_PORT = (1 << R_RL_3);                    /* pull up probe-3, pull down probe-2 */
  ADC_PORT = 0;                              /* pull down directly: */
  ADC_DDR = (1 << TP1);                      /* probe-1 */
  wait20ms();                                /* time to settle */

  /* wait for external pull-up resistor or key press */
  while (Run)
  {
    if (ADC_PIN & (1 << TP2))      /* check for high level of probe-2 */
    {
      Flag = 1;                              /* signal "bus ok" */
      Run = 0;                               /* end loop */
    }
    else                           /* check test key */
    {
      /* wait 100ms for key press */
      Flag = TestKey(100, CHECK_BAT);
      if (Flag)                              /* key pressed */
      {
        Flag = 0;                            /* signal "skipped" */
        Run = 0;                             /* end loop */
      }
    }
  }

  /* remove pull-down via Rh for probe-2 */
  R_DDR = (1 << R_RL_3);      /* disable Rh for probe-2 */

  return Flag;
}

#endif



/*
 *  reset bus and check for presence pulse from client(s)
 *
 *  returns:
 *  - 0: no presence pulse from client
 *  - 1: presence pulse from client
 */

uint8_t OneWire_ResetBus(void)
{
  uint8_t           Flag = 0;      /* return value */

  /*
   *  pull down DQ for >=480탎 and release it again
   */

  #ifdef ONEWIRE_IO_PIN
  /* change DQ to output mode (port pin is low) */
  ONEWIRE_DDR |= (1 << ONEWIRE_DQ);          /* set bit */
  #endif

  #ifdef ONEWIRE_PROBES
  /* change probe-2 (DQ) to output mode (port pin is low) */
  ADC_DDR |= (1 << TP2);                     /* set bit */
  #endif

  wait500us();                /* delay of 500탎 */

  #ifdef ONEWIRE_IO_PIN
  /* change DQ back to input mode */
  ONEWIRE_DDR &= ~(1 << ONEWIRE_DQ);         /* clear bit */
  #endif

  #ifdef ONEWIRE_PROBES
  /* change probe-2 (DQ) back to input mode */
  ADC_DDR &= ~(1 << TP2);                    /* clear bit */
  #endif


  /*
   *  check for presence pulse from client(s)
   *  - time slot >=480탎
   *  - client responds after 15-60탎 with a low pulse of 60-240탎
   *  - we check DQ after 70탎
   */

  wait50us();                 /* read delay of 70탎 */
  wait20us();

  #ifdef ONEWIRE_IO_PIN
  /* read DQ */
  if (! (ONEWIRE_PIN & (1 << ONEWIRE_DQ)))   /* low */
  {
    Flag = 1;                 /* signal ok */
  }
  #endif

  #ifdef ONEWIRE_PROBES
  /* read DQ (probe-2) */
  if (! (ADC_PIN & (1 << TP2)))              /* low */
  {
    Flag = 1;                 /* signal ok */
  }
  #endif

  /* end the time slot */
  wait400us();                /* delay of 430탎 (500탎 - 70탎) */
  wait30us();

  return Flag;
}



/*
 *  send bit
 *
 *  requires:
 *  - Bit: 0/1
 */

void OneWire_SendBit(uint8_t Bit)
{
  /*
   *  - write time slot: 60-120탎
   *  - recovery time after write: >=1탎
   *  - write "1":
   *    - 1-15탎 low pulse
   *    - release DQ and wait until end of time slot
   *  - write "0":
   *    - 60-120탎 low pulse (complete time slot)
   *    - release DQ
   */

  /*
   *  pull down DQ
   */

  #ifdef ONEWIRE_IO_PIN
  /* change DQ to output mode (port pin is low) */
  ONEWIRE_DDR |= (1 << ONEWIRE_DQ);     /* set bit */
  #endif    

  #ifdef ONEWIRE_PROBES
  /* change probe-2 (DQ) to output mode (port pin is low) */
  ADC_DDR |= (1 << TP2);                /* set bit */
  #endif


  if (Bit)          /* 1 */
  {
    /*
     *  pull down DQ for 5탎 and release it again
     */

    wait5us();                /* pulse delay of 5탎 */

    #ifdef ONEWIRE_IO_PIN
    /* change DQ back to input mode */
    ONEWIRE_DDR &= ~(1 << ONEWIRE_DQ);       /* clear bit */
    #endif

    #ifdef ONEWIRE_PROBES
    /* change probe-2 (DQ) back to input mode */
    ADC_DDR &= ~(1 << TP2);                  /* clear bit */
    #endif
 
    /* end the time slot + recovery time */
    wait50us();               /* delay of 65탎 */
    wait5us();
    wait10us();
  }
  else              /* 0 */
  {
    /*
     *  pull down DQ for 60탎 and release it again
     */

    wait50us();               /* pulse delay of 60탎 */
    wait10us();

    #ifdef ONEWIRE_IO_PIN
    /* change DQ back to input mode */
    ONEWIRE_DDR &= ~(1 << ONEWIRE_DQ);       /* clear bit */
    #endif

    #ifdef ONEWIRE_PROBES
    /* change probe-2 (DQ) back to input mode */
    ADC_DDR &= ~(1 << TP2);                  /* clear bit */
    #endif

    /* recovery time */
    wait10us();               /* delay of 10탎 */
  }
}



/*
 *  read bit
 *
 *  returns:
 *  - Bit: 0/1
 */

uint8_t OneWire_ReadBit(void)
{
  uint8_t           Bit = 0;       /* return value */

  /*
   *  - read time slot: 60-120탎
   *  - recovery time after read: >=1탎
   *  - master starts read process by a low pulse >=1탎
   *  - client's response is valid for 15탎 after falling edge of
   *    the master's low pulse
   *  - we read DQ after 13탎
   */


  /*
   *  pull down DQ for 5탎 and release it again
   */

  #ifdef ONEWIRE_IO_PIN
  /* change DQ to output mode (port pin is low) */
  ONEWIRE_DDR |= (1 << ONEWIRE_DQ);     /* set bit */
  #endif    

  #ifdef ONEWIRE_PROBES
  /* change probe-2 (DQ) to output mode (port pin is low) */
  ADC_DDR |= (1 << TP2);                /* set bit */
  #endif

  wait5us();                  /* pulse delay of 5탎 */

  #ifdef ONEWIRE_IO_PIN
  /* change DQ back to input mode */
  ONEWIRE_DDR &= ~(1 << ONEWIRE_DQ);    /* clear bit */
  #endif

  #ifdef ONEWIRE_PROBES
  /* change probe-2 (DQ) back to input mode */
  ADC_DDR &= ~(1 << TP2);               /* clear bit */
  #endif


  /*
   *  read client's response
   *  - data bit valid for 15탎 starting with master's read pulse
   *  - high -> 1
   *  - low  -> 0
   */

  wait5us();                /* read delay of 8탎 */
  wait3us();

  #ifdef ONEWIRE_IO_PIN
  /* read DQ */
  if (ONEWIRE_PIN & (1 << ONEWIRE_DQ))  /* high */
  {
    Bit = 1;                /* 1 */
  }
  #endif

  #ifdef ONEWIRE_PROBES
  /* read DQ (probe-2) */
  if (ADC_PIN & (1 << TP2))             /* high */
  {
    Bit = 1;                /* 1 */
  }
  #endif

  /* end the time slot + recovery time */
  wait50us();               /* delay of 57탎 */
  wait5us();
  wait2us();

  return Bit;
}



/*
 *  send byte
 *  - sending bit order: LSB
 *
 *  requires:
 *  - Byte: data byte
 */

void OneWire_SendByte(uint8_t Byte)
{
  uint8_t           n = 0;         /* counter */

  while (n < 8)          /* 8 bits */
  {
    OneWire_SendBit(Byte & 0x01);       /* send LSB */
    Byte >>= 1;                         /* shift to right */
    n++;                                /* next bit */
  }
}



/*
 *  read byte
 *  - reading bit order: LSB
 *
 *  returns:
 *  - Byte: data byte
 */

uint8_t OneWire_ReadByte(void)
{
  uint8_t           Byte = 0;      /* return value */
  uint8_t           n = 0;         /* counter */

  while (n < 8)          /* 8 bits */
  {
    Byte >>= 1;                         /* shift to right */

    if (OneWire_ReadBit())              /* read "1" */
    {
      Byte |= 0b10000000;               /* set bit */
    }

    n++;                                /* next bit */
  }

  return Byte;
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


#if 0

/*
 *  address client
 *  - includes reset of the bus
 *  - transaction steps: initialization and ROM command
 *
 *  requires:
 *  - ROM_Code: pointer to ROM code stored in an array of 8 bytes
 *
 *  returns:
 *  - 1
 *  - 0
 */

uint8_t OneWire_AddressClient(uint8_t *ROM_Code)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           n = 0;         /* counter */

  /* transaction: initialization */
  /* reset bus and check for presence pulse */
  Flag = OneWire_ResetBus();

  if (Flag)              /* detected client(s) */
  {
    /* transaction: ROM command */
    if (ROM_Code)            /* valid pointer */
    {
      /* address specific client on the bus */
      OneWire_SendByte(CMD_MATCH_ROM);  /* select client with ROM code */

      /* send client's ROM code */
      while (n < 8)          /* 8 bytes */
      {
        OneWire_SendByte(*ROM_Code);   /* send one byte of ROM code */
        ROM_Code++;                    /* next byte */
        n++;                           /* next byte */
      }
    }
    else                     /* NULL pointer */
    {
      /* assumes a single client on the bus */
      OneWire_SendByte(CMD_SKIP_ROM);   /* select all clients */
    }
  }

  return Flag;
}

#endif



/*
 *  calculate CRC-8
 *  - CRC = X^8 + X^5 + X^4 + 1
 *  - start value: 0x00
 *  - uses variable CRC8 to track current CRC
 *
 *  requires:
 *  - Byte: new input byte
 */

void OneWire_CRC8(uint8_t Byte)
{
  uint8_t           n = 0;         /* counter */
  uint8_t           Bit;           /* LSB */ 

  while (n < 8)          /* 8 bits */
  {
    /* XOR current LSB of input with CRC8's current X^8 */
    Bit = CRC8 ^ Byte;        /* XOR */
    Bit &= 0b00000001;        /* filter LSB */

    /* shift CRC right */
    CRC8 >>= 1;               /* for next bit */

    if (Bit)                  /* XORed LSB is 1 */
    {
      /*
       *  XOR CRC's X^5 and X^4 with 1
       *  - XOR with 0b00011000
       *  - since CRC is already shifted right: XOR with 0b00001100
       *  - since we have to feed the XORed LSB back into the CRC
       *    and the MSB is 0 after shifting: XOR with 0b10001100
       */

      CRC8 ^= 0b10001100;     /* XOR */
    }
    /*  when 0:
     *  - XOR would keep the original bits
     *  - MSB will be 0 after a right shift anyway
     */

    /* shift input right */
    Byte >>= 1;               /* for next input bit */

    n++;                      /* next bit */
  }
}



/* ************************************************************************
 *   Search ROM
 * ************************************************************************ */


#ifdef SW_ONEWIRE_SCAN

/*
 *  search for next device (ROM code)
 *
 *  returns:
 *  - 2 found last device
 *  - 1 found next device
 *  - 0 on any problem
 */

uint8_t OneWire_Next_ROM_Code(void)
{
  uint8_t      Flag;          /* return value */
  uint8_t      BitPos = 1;    /* bit position of ROM code */
  uint8_t      NewCon = 0;    /* bit position of code conflict */
  uint8_t      Bit1;          /* bit */
  uint8_t      Bit2;          /* bit complement */
  uint8_t      NewBit;        /* bit to send */
  uint8_t      CodeByte = 0;  /* byte counter for ROM code */
  uint8_t      CodeBit = 1;   /* bitmask for ROM code */
  uint8_t      Temp;          /* temporary value */

  wdt_reset();                /* reset watchdog */

  /* transaction: initialization */
  /* reset bus and check for presence pulse */
  Flag = OneWire_ResetBus();

  if (Flag)                   /* detected client */
  {
    /* transaction: ROM command */
    OneWire_SendByte(CMD_SEARCH_ROM);   /* search ROM */

    /*
     *  processing loop
     */

    while (BitPos < 65)       /* 64 bits */
    {
      NewBit = 2;                  /* default: invalid */

      /* read bit and its complement */
      Bit1 = OneWire_ReadBit();         /* read bit */
      Bit2 = OneWire_ReadBit();         /* read complement */

      /* process bits */
      if (Bit1 != Bit2)            /* valid bit (01 or 10) */
      {
        NewBit = Bit1;                 /* copy bit */
      }
      else if (Bit1 & Bit2)        /* no response (11) */
      {
        BitPos = 64;               /* end loop */
        Flag = 0;                  /* signal bus error */
      }
      else                         /* code conflict (00) */
      {
        if (LastCon < BitPos)           /* code conflict further up */
        {
          /* start branch with 0 */
          NewBit = 0;                   /* set bit to 0 */
          NewCon = BitPos;              /* update bit position */
        }
        else if (LastCon == BitPos)     /* hit pos of last code conflict */
        {
          /* change branch to 1 */
          NewBit = 1;                   /* set bit to 1 */
        }
        else  /* LastCon > BitPos */    /* code conflict further down */
        {
          /* get bit from ROM code */
          Temp = ROM_Code[CodeByte] & CodeBit;

          if (Temp == 0)                /* bit is 0 */
          {
            NewCon = BitPos;            /* update bit position */
            /* this is going to be 1 in the next search run */
          }

          NewBit = Temp;                /* use bit from ROM code */
        }
      }

      /* process bit to send */
      if (NewBit < 2)              /* valid bit */
      {
        /* update ROM code */
        if (NewBit == 0)           /* bit is 0 */
        {
          ROM_Code[CodeByte] &= ~CodeBit;    /* clear bit in ROM code */
        }
        else                       /* bit is 1 */
        {
          ROM_Code[CodeByte] |= CodeBit;     /* set bit in ROM code */
        }

        /* send bit to select branch */
        OneWire_SendBit(NewBit);
      }

      /* loop management */
      if (CodeBit & 0b10000000)    /* bitmask overflow */
      {
        CodeBit = 0b00000001;      /* reset bitmask to first bit */
        CodeByte++;                /* next byte */
      }
      else                         /* no overflow */
      {
        CodeBit <<= 1;             /* shift left by one */
      }

      BitPos++;               /* next bit */
    }

    /* manage branching for next run */
    LastCon = NewCon;         /* update bit position of code conflict */

    if (LastCon == 0)         /* no devices left */
    {
      if (Flag)               /* don't overwrite error */
      {
        Flag = 2;             /* signal last device */
      }
    }

    /* special case: all zero (pull-up resistor suddenly missing) */
    if (ROM_Code[0] == 0)     /* family code 00 */
    {
      Flag = 0;               /* signal bus error */
    }
  }

  return Flag;
}



/*
 *  read ROM codes of connected devices
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t OneWire_Scan_Tool(void)
{
  uint8_t           Flag;          /* return value */
  uint8_t           Run;           /* loop control */
  uint8_t           Test;          /* key / feedback */
  uint8_t           n;             /* counter */

  /* local constants */
  #define RUN_FLAG       0b00000001     /* run */
  #define RESET_FLAG     0b00000010     /* reset values */
  #define OUTPUT_FLAG    0b00000100     /* output ROM code */
  #define DONE_FLAG      0b00001000     /* last device */

  #ifdef ONEWIRE_IO_PIN
  Flag = 1;                   /* set default */
  #endif

  #ifdef ONEWIRE_PROBES
  /* inform user about pinout and check for external pull-up resistor */
  Flag = OneWire_Probes(OneWire_Scan_str);

  if (Flag == 0)              /* bus error */
  {
    return Flag;              /* exit tool and signal error */
  }
  #endif

  LCD_ClearLine2();                /* clear line #2 */
  Display_EEString(Start_str);     /* display: Start */
  UI.LineMode = LINE_STD | LINE_KEEP;   /* next-line mode: keep first line */


  /*
   *  processing loop
   */

  Run = RUN_FLAG | RESET_FLAG;

  while (Run)
  {
    n = UI.CharPos_Y;              /* get current line number */

    /* user input */

    /* wait for user input */
    Test = TestKey(0, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      break;                       /* end loop */
    }

    /* reset values */
    if (Run & RESET_FLAG)         /* reset requested */
    {
      /* clear ROM code */
      for (n = 0; n < 8; n++)     /* 8 bytes */
      {
        ROM_Code[n] = 0;
      }

      LastCon = 0;                /* reset bit position of last code conflict */

      Run &= ~RESET_FLAG;         /* clear flag */
    }
    else                          /* consecutive run */
    {
      LCD_CharPos(1, n);          /* move to beginning of former line */
    }

    /* manage screen */
    Display_NextLine();           /* move to next line */

    /* search for next device  */
    if (Run)             /* ok to proceed */
    {
      Test = OneWire_Next_ROM_Code();   /* get next device */

      if (Test >= 1)          /* got ROM code */
      {
        /* check CRC */
        CRC8 = 0x00;          /* reset CRC to start value */
        n = 0;
        while (n < 7)         /* 7 data bytes */
        {
          OneWire_CRC8(ROM_Code[n]);    /* process byte */
          n++;                          /* next byte */
        }

        if (ROM_Code[7] == CRC8)   /* CRC matches */
        {
          Run |= OUTPUT_FLAG;      /* output ROM code */ 
        }
        else                       /* mismatch */
        {
          Display_EEString_Space(CRC_str);   /* display: CRC */
          Display_EEString(Error_str);       /* display: error */
        }

        if (Test == 2)             /* no devices left */
        {
          Run |= DONE_FLAG;        /* last ROM code */
        }
      }
      else                    /* error */
      {
        Display_EEString_Space(Bus_str);     /* display: Bus */
        Display_EEString(Error_str);         /* display: error */
        Run |= RESET_FLAG;                   /* reset values for next scan */
      }
    }

    /* display ROM code */
    if (Run & OUTPUT_FLAG)         /* output requested */
    {
      /* display family code */
      Display_HexByte(ROM_Code[0]);     /* display family */

      Display_Space();                  /* display space */

      /* display serial number (MSB left) */
      for (n = 6; n > 0; n--)           /* 6 bytes */
      {
        Display_HexByte(ROM_Code[n]);   /* display S/N */
      }

      Run &= ~OUTPUT_FLAG;              /* clear flag */
    }

    /* indicate end of scan */
    if (Run & DONE_FLAG)           /* no devices left */
    {
      Display_NL_EEString(Done_str);    /* display: done */
      Run |= RESET_FLAG;                /* reset values for next scan */

      Run &= ~DONE_FLAG;                /* clear flag */
    }
  }

  return Flag;

  /* clean up */
  #undef RUN_FLAG
  #undef RESET_FLAG
  #undef OUTPUT_FLAG
  #undef DONE_FLAG
}

#endif



/* ************************************************************************
 *   Read ROM
 * ************************************************************************ */


#ifdef ONEWIRE_READ_ROM

/*
 *  read ROM code of connected device
 *  - single client on the bus
 *  - also display ROM code
 */

void OneWire_Read_ROM_Code(void)
{
  uint8_t           Flag = 0;           /* control flag */
  uint8_t           n;                  /* counter */

  wdt_reset();                /* reset watchdog */

  /* transaction: initialization */
  /* reset bus and check for presence pulse */
  Flag = OneWire_ResetBus();    

  if (Flag)                   /* detected client */
  {
    /* transaction: ROM command */
    OneWire_SendByte(CMD_READ_ROM);     /* read ROM */

    /* read 8 bytes */
    n = 0;
    while (n < 8)             /* 8 bytes */
    {
      ROM_Code[n] = OneWire_ReadByte();      /* read byte */
      n++;                                   /* next byte */
    }

    /* check CRC */
    CRC8 = 0x00;              /* reset CRC to start value */
    n = 0;
    while (n < 7)             /* 7 data bytes */
    {
      OneWire_CRC8(ROM_Code[n]);        /* process byte */
      n++;                              /* next byte */
    }

    if (ROM_Code[7] == CRC8)   /* CRC matches */
    {
      /* output ROM code */ 

      /* display family code */
      Display_HexByte(ROM_Code[0]);     /* display family */

      Display_Space();                  /* display space */

      /* display serial number (MSB left) */
      for (n = 6; n > 0; n--)           /* 6 bytes */
      {
        Display_HexByte(ROM_Code[n]);   /* display S/N */
      }
    }
    else                       /* mismatch */
    {
      Display_Minus();                  /* display n/a */
    }
  }
}

#endif



/* ************************************************************************
 *   DS18B20
 * ************************************************************************ */


#ifdef SW_DS18B20

/*
 *  DS18B20: read temperature
 *  - single client on the bus
 *
 *  requires:
 *  - Value: pointer to temperature in 캜
 *  - Scale: pointer to scale factor (*10^x)
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t DS18B20_ReadTemperature(int32_t *Value, int8_t *Scale)
{
  uint8_t           Flag = 0;           /* return value / control flag */
  uint8_t           Run = 0;            /* loop control */
  uint8_t           n;                  /* counter */
  uint8_t           ScratchPad[9];      /* scratchpad */
  uint8_t           Sign;               /* sign flag */
  int16_t           Temp;               /* temperature */

  wdt_reset();                /* reset watchdog */

  /* transaction: initialization */
  /* reset bus and check for presence pulse */
  Flag = OneWire_ResetBus();

  if (Flag)                   /* detected client */
  {
    /* transaction: ROM command */
    OneWire_SendByte(CMD_SKIP_ROM);     /* select all clients */

    /* transaction: function command */
    /* start conversion */
    OneWire_SendByte(CMD_DS18B20_CONVERT_T);

    /*
     *  maximum conversion time
     *  -  9 bits  93.75ms  (t_conv/8)
     *  - 10 bits  187.5ms  (t_conv/4)
     *  - 11 bits  375ms    (t_conv/2)
     *  - 12 bits  750ms    (t_conv)
     */

    #if 0
    /* fixed delay for conversion (required when parasitic-powered) */
    MilliSleep(750);          /* 750ms */
    Run = 3;                  /* three read attempts */
    #endif

    /*
     *  check the conversion status to minimize delay
     *  - requires external power
     *  - this way we don't need to know the bit depth in advance
     *    to determine the conversion time
     */

    Flag = 0;                           /* reset flag */
    n = 50;                             /* 750ms / 15ms = 50 */
    while (n > 0)
    {
      MilliSleep(15);                   /* wait 15ms */

      /* check conversion state */
      Sign = OneWire_ReadBit();         /* single read slot */
      if (Sign == FLAG_CONV_DONE)       /* conversion finished */
      {
        Flag = 1;                       /* signal "ok" */
        Run = 3;                        /* three read attempts */
        n = 1;                          /* end loop */
      }

      n--;                              /* next round */
    }
  }


  /*
   *  read scratchpad
   */

  while (Run)
  {
    /* transaction: initialization */
    /* reset bus and check for presence pulse */
    Flag = OneWire_ResetBus();    

    if (Flag)                 /* detected client */
    {
      /* transaction: ROM command */
      OneWire_SendByte(CMD_SKIP_ROM);        /* select all clients */

      /* transaction: function command */
      /* read scratchpad to get temperature */
      OneWire_SendByte(CMD_DS18B20_READ_SCRATCHPAD);
      n = 0;
      while (n < 9)           /* 9 bytes */
      {
        ScratchPad[n] = OneWire_ReadByte();  /* read byte */
        n++;                                 /* next byte */
      }

      /* check CRC of scratchpad */
      CRC8 = 0x00;            /* reset CRC to start value */
      n = 0;
      while (n < 8)           /* 8 data bytes */
      {
        OneWire_CRC8(ScratchPad[n]);    /* process byte */
        n++;                            /* next byte */
      }

      if (ScratchPad[8] == CRC8)        /* CRC matches */
      {
        Run = 1;                        /* end loop */
      }
      else                              /* mismatch */
      {
        Flag = 0;                       /* signal error */
      }
    }
    else                      /* no client detected */
    {
      Run = 1;                /* end loop */
    }

    Run--;                    /* another try */
  }


  /*
   *  get temperature from scratchpad (in 캜)
   */

  if (Flag)                   /* valid scratchpad */
  {
    /*
     *  get bit depth (resolution)
     *  - 9-12 bits mean 1-4 binary places after the dot
     */

    Run = 0;                       /* reset */
    n = ScratchPad[4];             /* get configuration register */
    if (n & FLAG_DS18B20_R0)       /* R0 set */
    {
      Run |= 0b00000001;           /* set lower bit */
    }
    if (n & FLAG_DS18B20_R1)       /* R1 set */
    {
      Run |= 0b00000010;           /* set upper bit */
    }


    /*
     *  build signed integer from LSB and MSB
     *  - two's complement
     *  - t_LSB: Byte #0 = ScratchPad[0]
     *  - t_MSB: Byte #1 = ScratchPad[1]
     */

    int16_t         *Ptr;          /* pointer to mitigate compiler warning */

    Ptr = (int16_t *)&ScratchPad[0];    /* combine LSB and MSB */
    Temp = *Ptr;                        /* copy value */  


    /*
     *  remove undefined bits based on bit depth and 
     *  also calculate scaling factor
     */

    n = 3 - Run;                        /* number of bits to ignore */
    *Value = 10000;                     /* 4 decimal places */  
    *Scale = -4;                        /* 4 decimal places */
    Run = 16;                           /* 4 binary places (*16) */
    Sign = ScratchPad[1] & 0b11111000;  /* set flag when negative */
    while (n)
    {
      /* remove undefined bit */
      Temp >>= 1;                       /* shift right */
      if (Sign)                         /* negative */
      {
        /* keep signed integer negative */
        Temp |= 0b1000000000000000;     /* set MSB */
      }

      /* scale by one digit */
      *Value /= 10;                /* one decimal place less */
      (*Scale)++;                  /* update scale too */
      Run >>=1;                    /* one binary place less */

      n--;                         /* next bit */
    }

    *Value *= Temp;                /* scale temperature */ 
    *Value /= Run;                 /* and adjust for binary scaling */
  }

  return Flag;
}



/*
 *  temperature sensor DS18B20
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any error
 */

uint8_t DS18B20_Tool(void)
{
  uint8_t           Flag;          /* return value / control flag */
  uint8_t           Run = 1;       /* loop control */
  uint8_t           Test;          /* key / feedback */
  int8_t            Scale;         /* temperature scale 10^x */
  int32_t           Value;         /* temperature value */
  uint8_t           Mode = MODE_MANUAL; /* operation mode */
  uint16_t          Timeout = 0;        /* timeout for user feedback */

  #ifdef ONEWIRE_IO_PIN
  Flag = 1;                   /* set default */
  #endif

  #ifdef ONEWIRE_PROBES
  /* inform user about pinout and check for external pull-up resistor */
  Flag = OneWire_Probes(DS18B20_str);

  if (Flag == 0)              /* bus error */
  {
    return Flag;              /* exit tool and signal error */
  }
  #endif

  /* start info */
  LCD_ClearLine2();                     /* clear line #2 */
  Display_EEString(Start_str);          /* display: Start */


  /*
   *  processing loop
   */

  while (Run)
  {
    /*
     *  user input
     */

    /* wait for user input */
    Test = TestKey(Timeout, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_LONG)          /* long key press */
    {
      /* display mode in line #1 */
      LCD_ClearLine(1);            /* clear line #1 */
      LCD_CharPos(1, 1);           /* move to line #1 */
      #ifdef UI_COLORED_TITLES
        Display_ColoredEEString_Space(DS18B20_str, COLOR_TITLE);
      #else
        Display_EEString_Space(DS18B20_str);
      #endif

      /* change mode */
      if (Mode == MODE_MANUAL)     /* manual mode */
      {
        Mode = MODE_AUTO;          /* set automatic mode */
        Timeout = 1000;            /* wait for max. 1s */

        /* indicate auto mode */
        Display_Char('*');         /* display: * */
      }
      else                         /* automatic mode */
      {
        Mode = MODE_MANUAL;        /* set manual mode again */
        Timeout = 0;               /* wait for user */
      }

      MilliSleep(500);             /* smooth UI */
    }
    else if (Test == KEY_TWICE)    /* two short key presses */
    {
      Run = 0;                     /* end loop */
    }


    /* clear text lines for new output */
    LCD_ClearLine2();                   /* clear line #2 */
    #ifdef ONEWIRE_READ_ROM
    LCD_ClearLine(3);                   /* clear line #3 */
    LCD_CharPos(1, 2);                  /* move to line #2 */
    #endif

    /*
     *  read and show temperature
     */

    if (Run)            /* ok to proceed */
    {
      /* get temperature from DS18B20 (in 캜) */
      Test = DS18B20_ReadTemperature(&Value, &Scale);

      if (Test)                    /* got temperature */
      {
        /* Scale is -1 to -4: 1-4 decimal places */
        Scale = -Scale;

        #ifdef UI_FAHRENHEIT
        /* convert Celsius into Fahrenheit */
        Value = Celsius2Fahrenheit(Value, Scale);
        #endif

        #ifdef UI_ROUND_DS18B20
        /* round value and scale to 0.1� */
        Value = RoundSignedValue(Value, Scale, 1);
        Scale = 1;                 /* 1 decimal */
        #endif

        /* todo: add degree symbol to bitmap fonts */
        Display_SignedFullValue(Value, Scale, '�');

        #ifdef UI_FAHRENHEIT
          Display_Char('F');       /* display: F (Fahrenheit) */
        #else
          Display_Char('C');       /* display: C (Celsius) */
        #endif
      }
      else                         /* some error */
      {
        Display_Minus();           /* display n/a */
      }

      #ifdef ONEWIRE_READ_ROM
      /* read and display ROM code */
      LCD_CharPos(1, 3);           /* move to line #3 */
      OneWire_Read_ROM_Code();     /* read and display ROM code */
      #endif
    }
  }

  return Flag;
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local constants */
#undef MODE_MANUAL
#undef MODE_AUTO

/* source management */
#undef ONEWIRE_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

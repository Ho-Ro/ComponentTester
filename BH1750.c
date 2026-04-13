/* ************************************************************************
 *
 *   functions for BH1750FVI (ambient light sensor, I2C bus)
 *
 *   (c) 2024-2025 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    SCL      I2C: SCL
 *    SDA      I2C: SDA
 *    ADDR     Gnd or Vcc  
 *  - I2C clock modes: standard (100kHz) and fast (400kHz) 
 *  - Vcc 3.3V
 *    I2C pull-up resistors to 3.3V should be ok for a 5V ATmega
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_BH1750


/*
 *  local constants
 */

/* source management */
#define BH1750_C

/* operation modes */
#define MODE_MANUAL      0    /* manual mode */
#define MODE_AUTO        1    /* automatic mode */

/* I2C addresses */
#define BH1750_I2C_ADDR_0     0b00100011     /* ADDR=0 0x23 */
#define BH1750_I2C_ADDR_1     0b01011100     /* ADDR=1 0x5c */


/*
 *  instruction set
 */

/* power down */
#define BH1750_POWER_DOWN          0b00000000

/* power on (and wait for measurement command) */
#define BH1750_POWER_UP            0b00000001

/* reset data register (not in power down mode) */
#define BH1750_RESET               0b00000111

/* run measurement in continuous high resolution mode 1 (1 lx) */
#define BH1750_CONT_HIRES_1        0b00010000

/* run measurement in continuous high resolution mode 2 (0.5 lx) */
#define BH1750_CONT_HIRES_2        0b00010001

/* run measurement in continuous low resolution mode (4 lx) */
#define BH1750_CONT_LOWRES         0b00010011

/* run measurement in one-time high resolution mode 1 (1 lx) */
#define BH1750_SINGLE_HIRES_1      0b00100000

/* run measurement in one-time high resolution mode 2 (0.5 lx) */
#define BH1750_SINGLE_HIRES_2      0b00100001

/* run measurement in one-time low resolution mode (4 lx) */
#define BH1750_SINGLE_LOWRES       0b00100011

/* change measurement time, high bits (bits 7-5) */
#define BH1750_TIME_HIGH           0b01000000

/* change measurement time, low bits (bits 4-0) */
#define BH1750_TIME_LOW            0b01100000


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   low level functions for I2C interface
 * ************************************************************************ */


/*
 *  send command to BH1750
 *
 *  requires:
 *  - Command
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t BH1750_SendCommand(uint8_t Command)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           OldTimeout;         /* old ACK timeout */

  OldTimeout = I2C.Timeout;             /* save old timeout */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    I2C.Byte = BH1750_I2C_ADDR << 1;              /* address (7 bits) & write (0) */
    I2C.Timeout = 1;                              /* ACK timeout 10µs */

    /* send address & write bit, expect ACK from BH1750 */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address BH1750 */
    {
      I2C.Byte = Command;                         /* set command */

      /* send command, expect ACK from BH1750 */
      if (I2C_WriteByte(I2C_DATA) == I2C_ACK)     /* send data */
      {
        Flag = 1;                                 /* signal success */
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



/*
 *  read light intensity value from BH1750
 *
 *  requires:
 *  - Value: pointer to raw value
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t BH1750_ReadValue(uint16_t *Value)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           HighByte;           /* high byte */
  uint16_t          RawValue;           /* raw value */
  uint8_t           OldTimeout;         /* old ACK timeout */

  OldTimeout = I2C.Timeout;             /* save old timeout */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    /* address (7 bits) & read (1) */
    I2C.Byte = (BH1750_I2C_ADDR << 1) | 0b00000001;
    I2C.Timeout = 1;                              /* ACK timeout 10µs */

    /* send address & read bit, expect ACK from BH1750 */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address BH1750 */
    {
      /* read high byte and ACK */
      if (I2C_ReadByte(I2C_ACK) == I2C_OK)        /* read first byte */
      {
        HighByte = I2C.Byte;                      /* save byte */

        /* read low byte and NACK */
        if (I2C_ReadByte(I2C_NACK) == I2C_OK)     /* read second byte */
        {
          /* pre-process data */
          RawValue = HighByte;                    /* copy high byte */
          RawValue <<= 8;                         /* and shift to MSB */
          RawValue |= I2C.Byte;                   /* copy low byte */
          *Value = RawValue;                      /* save result */

          Flag = 1;                               /* signal success */          
        }
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  measurement modes and times:
 *  - resolution mode    resolution  typ. time  max. time
 *    high resolution 1     1 lx      120 ms     180 ms
 *    high resolution 2   0.5 lx      120 ms     180 ms
 *    low resolution        4 lx       16 ms      24 ms
 *  - modes
 *    continuously:  stay in power on mode
 *    one-time:      automaticly power down after measurement
 */


/*
 *  BH1750: read light intensity
 *
 *  requires:
 *  - Value: pointer to light intensity in Lux
 *  - Scale: pointer to scale factor / decimal places
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t BH1750_ReadLux(uint32_t *Value, uint8_t *Scale)
{
  uint8_t           Flag = 0;           /* return value */
  uint16_t          RawValue;           /* raw value */
  uint32_t          Temp;               /* temporary value */


  /*
   *  measure light intensity
   */

  /* send command: measure in continuous high resolution mode 1 */
  if (BH1750_SendCommand(BH1750_CONT_HIRES_1) == 1)    /* done */
  {
    /* conversion time 120-180ms */
    MilliSleep(180);                         /* wait 180ms */

    /* read raw value */
    if (BH1750_ReadValue(&RawValue) == 1)    /* done */
    {
      Flag = 1;                              /* signal success */
    }
  }


  /*
   *  convert raw value to Lux
   */

  Temp = RawValue;                      /* copy raw value */
  Temp *= 100;                          /* scale to 10^-2 */
  /* apply accuracy factor (/1.2) */
  Temp /= 12;                           /* /12 (scaled to 10^-1) */
  *Value = Temp;                        /* save Lux value */
  *Scale = 1;                           /* 1 decimal place (10^-1) */

  return Flag;
}



/* ************************************************************************
 *   tool
 * ************************************************************************ */


/*
 *  BH1750 tool
 *  - reads and displays light intensity (in Lux)
 */

void BH1750_Tool(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* key / feedback */
  uint8_t           Mode = MODE_MANUAL; /* operation mode */
  uint16_t          Timeout = 0;        /* timeout for user feedback */
  uint8_t           Scale;              /* Lux scale / decimal places */
  uint32_t          Value;              /* Lux value */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define POWERUP_FLAG        0b00000010     /* power up BH1750 */


  /*
   *  display info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    Display_ColoredEEString(BH1750_str, COLOR_TITLE);  /* display: BH1750 */
  #else
    Display_EEString(BH1750_str);       /* display: BH1750 */
  #endif
  LCD_CharPos(1, 2);                    /* move to line #2 */
  Display_EEString(Start_str);          /* display: Start */


  /*
   *  processing loop
   */

  /* enter loop, power up BH1750 */
  Flag = RUN_FLAG | POWERUP_FLAG;

  while (Flag)
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
        Display_ColoredEEString_Space(BH1750_str, COLOR_TITLE);
      #else
        Display_EEString_Space(BH1750_str);
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
      Flag = 0;                    /* end loop */
    }

    LCD_ClearLine2();                   /* clear line #2 */


    /*
     *  power on BH1750
     *  - only once after start
     *  - BH1750 enters power-down mode after Vcc is applied
     *  - this also checks if BH1750 is connected and responsive
     */

    if (Flag & POWERUP_FLAG)       /* power-up requested */
    {
      Flag &= ~POWERUP_FLAG;            /* clear flag */

      /* send command: power up */
      if (BH1750_SendCommand(BH1750_POWER_UP) == 0)    /* can't power on */
      {
        /* inform user */
        Display_EEString(Error_str);        /* display: Error */
        WaitKey();                          /* wait for user feedback */

        Flag = 0;                           /* don't proceed */
      }
      /* else: ok */
    }


    /*
     *  read and show light intensity
     */

    if (Flag)            /* ok to proceed */
    {
      /* get light intensity from BH1750 (in Lux) */
      Test = BH1750_ReadLux(&Value, &Scale);

      if (Test)                    /* got value */
      {
        /* display Lux value */
        Display_FullValue(Value, Scale, 'l');
        Display_Char('x');
      }
      else                         /* some error */
      {
        Display_Minus();           /* display: - */
      }
    }
  }


  /*
   *  power down BH1750
   *  - ignore any errors
   */

  BH1750_SendCommand(BH1750_POWER_DOWN);     /* send command: power down */


  /*
   *  clean up
   */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef POWERUP_FLAG
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local constants */
#undef MODE_MANUAL
#undef MODE_AUTO

#undef BH1750_I2C_ADDR_0
#undef BH1750_I2C_ADDR_1

/* source management */
#undef BH1750_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

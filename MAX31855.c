/* ************************************************************************
 *
 *   functions for MAX31855 (thermocouple ADC)
 *   - supports MAX31855K, J, N, T, S, R and E
 *
 *   (c) 2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    SCK           SPI: SCK
 *    SO            SPI: MISO
 *    /CS           MAX31855_CS
 *  - max. SPI clock: 5 MHz
 *  - Vcc 3.3V (requires level shifter)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_MAX31855


/*
 *  local constants
 */

/* source management */
#define MAX31855_C

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


/*
 *  local variables
 */

#ifdef SPI_HARDWARE
/* SPI */
uint8_t             ClockRate;     /* SPI clock rate bits */
uint8_t             OldClockRate;  /* SPI clock rate bits */
#endif



/* ************************************************************************
 *   low level functions for SPI interface
 * ************************************************************************ */


/*
 *  protocol (read only):
 *  - /CS low (also starts new conversion)
 *  - read 32 bits on falling edge of SCK
 *  - data is MSB
 *    - D31-18  temperature, 14 bits (in 0.25°C)
 *      D31     sign bit (0: pos / 1: neg)
 *    - D17     reserved (always 0)
 *    - D16     fault bit (0: ok / 1: fault)
 *    - D15-4   reference junction temperature, 12 bits (in 0.0625°C)
 *      D15     sign bit (0: pos / 1: neg)
 *    - D3      reserved (always 0)
 *    - D2      short to Vcc (0: no short / 1: short)
 *    - D1      short to Gnd (0: no short / 1: short)
 *    - D0      open (0: closed / 1: open)
 *  - conversion time (in background): 70 - 100 ms
 */


/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void MAX31855_BusSetup(void)
{
  /*
   *  set control signals
   */

  /* set directions for required pins: */
  MAX31855_DDR |= (1 << MAX31855_CS);   /* /CS */

  /* set default levels */
  MAX31855_PORT |= (1 << MAX31855_CS);  /* set /CS high */


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */

  #ifdef SPI_HARDWARE

  /*
   *  set SPI clock rate (max. 2MHz)
   */

  /* 1MHz -> f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 1
    ClockRate = SPI_CLOCK_2X;
  #endif

  /* 8MHz -> f_osc/4 (SPR1 = 0, SPR0 = 0, SPI2X = 0) */
  #if CPU_FREQ / 1000000 == 8
    ClockRate = 0;
  #endif

  /* 16MHz -> f_osc/8 (SPR1 = 0, SPR0 = 1, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 16
    ClockRate = SPI_CLOCK_R0 | SPI_CLOCK_2X;
  #endif

  /* 20MHz -> f_osc/16 (SPR1 = 0, SPR0 = 1, SPI2X = 0) */
  #if CPU_FREQ / 1000000 == 20
    ClockRate = SPI_CLOCK_R0;
  #endif

  #endif
}



/*
 *  select MAX31855
 *  - also update clock for hardware SPI
 */

void MAX31855_SelectChip(void)
{
  /* select chip */
  MAX31855_PORT &= ~(1 << MAX31855_CS);      /* set /CS low */

  wait1us();                                 /* wait >100ns */

  #ifdef SPI_HARDWARE
  /* change SPI clock for touch controller */
  OldClockRate = SPI.ClockRate;         /* save old clock settings */
  SPI.ClockRate = ClockRate;            /* set new clock rate */
  SPI_Clock();                          /* update SPI clock */
  #endif
}



/*
 *  deselect MAX31855
 *  - also update clock for hardware SPI
 */

void MAX31855_DeselectChip(void)
{
  /* disable chip */
  MAX31855_PORT |= (1 << MAX31855_CS);       /* set /CS high */

  wait1us();                                 /* wait >100ns */

  #ifdef SPI_HARDWARE
  /* change SPI clock for display controller */
  SPI.ClockRate = OldClockRate;         /* set old clock rate */
  SPI_Clock();                          /* update SPI clock */
  #endif
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  MAX31855: read temperature
 *
 *  requires:
 *  - Value: pointer to temperature in °C
 *  - Scale: pointer to scale factor / decimal places
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t MAX31855_ReadTemperature(int32_t *Value, int8_t *Scale)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           n;                  /* counter */
  uint8_t           Data[4];            /* data (4 bytes) */
  int16_t           Temp;               /* temperature */


  /*
   *  read data from MAX31855
   *  - MSB to LSB
   */

  MAX31855_SelectChip();                /* select chip */

  /* read four bytes */ 
  n = 0;
  while (n < 4)           /* 4 bytes */
  {
    Data[n] = SPI_WriteRead_Byte(0);    /* read byte */
    n++;                                /* next byte */
  }

  MAX31855_DeselectChip();              /* deselect chip */


  /*
   *  process data
   */

  /* check fault bit (D16) */
  if ((Data[1] & 0b00000001) == 0)      /* 0: no fault */
  {
    /* get temperature (D31-18, in 0.25°C) */
    Temp = Data[0];                /* copy MSB */
    Temp <<= 8;                    /* move to MSB */
    Temp |= Data[1];               /* copy LSB */
    Temp /= 4;                     /* get rid of D17-16 while keeping sign */
    *Value = Temp;                 /* copy value */
    *Value *= 25;                  /* scale to 0.01, * 0.25°C */
    *Scale = 2;                    /* two decimal places (10^-2) */

    Flag = 1;                      /* signal success */
  }
  /* todo: check D2-0 error bits? */

  return Flag;
}



/* ************************************************************************
 *   tool
 * ************************************************************************ */


/*
 *  MAX31855 tool
 *  - reads and displays temperature of thermocouple
 */

void MAX31855_Tool(void)
{
  uint8_t           Flag = 1;           /* loop control */
  uint8_t           Test;               /* key / feedback */
  uint8_t           Mode = MODE_MANUAL; /* operation mode */
  uint16_t          Timeout = 0;        /* timeout for user feedback */
  int8_t            Scale;              /* temperature scale / decimal places */
  int32_t           Value;              /* temperature value */

  /*
   *  display info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    Display_ColoredEEString(MAX31855_str, COLOR_TITLE);  /* display: MAX31855 */
  #else
    Display_EEString(MAX31855_str);     /* display: MAX31855 */
  #endif
  LCD_CharPos(1, 2);                    /* move to line #2 */
  Display_EEString(Start_str);          /* display: Start */


  /*
   *  processing loop
   */

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
        Display_ColoredEEString_Space(MAX31855_str, COLOR_TITLE);
      #else
        Display_EEString_Space(MAX31855_str);
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
     *  read and show temperature
     */

    if (Flag)            /* ok to proceed */
    {
      /* get temperature from MAX31855 (in °C) */
      Test = MAX31855_ReadTemperature(&Value, &Scale);

      if (Test)                    /* got temperature */
      {
        #ifdef UI_FAHRENHEIT
        /* convert Celsius into Fahrenheit */
        Value = Celsius2Fahrenheit(Value, Scale);
        #endif

        /* todo: add degree symbol to bitmap fonts */
        Display_SignedFullValue(Value, Scale, '°');

        #ifdef UI_FAHRENHEIT
          Display_Char('F');       /* display: F (Fahrenheit) */
        #else
          Display_Char('C');       /* display: C (Celsius) */
        #endif
      }
      else                         /* some error */
      {
        Display_Minus();           /* display: - */
      }
    }
  }
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local constants */
#undef MODE_MANUAL
#undef MODE_AUTO

/* source management */
#undef MAX6675_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

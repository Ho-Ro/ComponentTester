/* ************************************************************************
 *
 *   functions for MAX6675 (K thermocouple ADC)
 *
 *   (c) 2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    SCK           SPI: SCK
 *    SO            SPI: MISO
 *    /CS           MAX6675_CS
 *  - max. SPI clock: 4.3 MHz
 *  - Vcc 5V or 3.3V (requires level shifter)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_MAX6675


/*
 *  local constants
 */

/* source management */
#define MAX6675_C

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
 *  - read 16 bits on falling edge of SCK
 *  - data is MSB
 *    - D15    dummy sign bit (always 0)
 *    - D14-3  temperature, 12 bits (in 0.25°C / 0 - 1023.75°C)
 *    - D2     thermocouple input (0: closed / 1: open)
 *    - D1     device ID (always 0)
 *    - D0     state (three-state)
 *  - conversion time (in background): 0.17 - 0.22 s
 */


/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void MAX6675_BusSetup(void)
{
  /*
   *  set control signals
   */

  /* set directions for required pins: */
  MAX6675_DDR |= (1 << MAX6675_CS);     /* /CS */

  /* set default levels */
  MAX6675_PORT |= (1 << MAX6675_CS);    /* set /CS high */


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
 *  select MAX6675
 *  - also update clock for hardware SPI
 */

void MAX6675_SelectChip(void)
{
  /* select chip */
  MAX6675_PORT &= ~(1 << MAX6675_CS);   /* set /CS low */

  wait1us();                            /* wait >100ns */

  #ifdef SPI_HARDWARE
  /* change SPI clock for touch controller */
  OldClockRate = SPI.ClockRate;         /* save old clock settings */
  SPI.ClockRate = ClockRate;            /* set new clock rate */
  SPI_Clock();                          /* update SPI clock */
  #endif
}



/*
 *  deselect MAX6675
 *  - also update clock for hardware SPI
 */

void MAX6675_DeselectChip(void)
{
  /* disable chip */
  MAX6675_PORT |= (1 << MAX6675_CS);    /* set /CS high */

  wait1us();                            /* wait >100ns */

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
 *  MAX6675: read temperature
 *
 *  requires:
 *  - Value: pointer to temperature in °C
 *  - Scale: pointer to scale factor / decimal places
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t MAX6675_ReadTemperature(int32_t *Value, int8_t *Scale)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           HighByte;      /* MSB */
  uint8_t           LowByte;       /* LSB */
  uint16_t          Temp;          /* temperature */

  /*
   *  read data from MAX6675
   *  - MSB to LSB
   */

  MAX6675_SelectChip();                 /* select chip */

  /* read two bytes */ 
  HighByte = SPI_WriteRead_Byte(0);     /* read MSB */
  LowByte = SPI_WriteRead_Byte(0);      /* read LSB */

  MAX6675_DeselectChip();               /* deselect chip */


  /*
   *  process data
   */

  /* check thermocouple input (D2) */
  if ((LowByte & 0b00000100) == 0)      /* 0: closed */
  {
    /* check dummy sign (D15) */
    if ((HighByte & 0b10000000) == 0)   /* expected to be 0 */
    {
      /* get temperature (D14-3, in 0.25°C) */
      Temp = HighByte;             /* copy MSB */
      Temp <<= 8;                  /* shift to MSB */
      Temp |= LowByte;             /* copy LSB */
      Temp >>= 3;                  /* get rid of D2-0 */
      *Value = Temp;               /* copy value */
      *Value *= 25;                /* scale to 0.01, * 0.25°C */
      *Scale = 2;                  /* two decimal places (10^-2) */

      Flag = 1;                    /* signal success */
    }
  }

  return Flag;
}



/* ************************************************************************
 *   tool
 * ************************************************************************ */


/*
 *  MAX6675 tool
 *  - reads and displays temperature of K thermocouple
 */

void MAX6675_Tool(void)
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
    Display_ColoredEEString(MAX6675_str, COLOR_TITLE);  /* display: MAX6675 */
  #else
    Display_EEString(MAX6675_str);      /* display: MAX6675 */
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
        Display_ColoredEEString_Space(MAX6675_str, COLOR_TITLE);
      #else
        Display_EEString_Space(MAX6675_str);
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
      /* get temperature from MAX6675 (in °C) */
      Test = MAX6675_ReadTemperature(&Value, &Scale);

      if (Test)                    /* got temperature */
      {
        #ifdef UI_FAHRENHEIT
        /* convert Celsius into Fahrenheit */
        Value = Celsius2Fahrenheit(Value, Scale);
        #endif

        /* todo: add degree symbol to bitmap fonts */
        Display_FullValue(Value, Scale, '°');

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

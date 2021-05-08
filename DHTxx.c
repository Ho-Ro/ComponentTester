/* ************************************************************************
 *
 *   DHTxx Temperature and Humidity Sensors
 *   - DHT11: DHT11, RHT01
 *   - DHT22: DHT22, RHT03, AM2302
 *            DHT21, RHT02, AM2301, HM2301
 *            DHT33, RHT04, AM2303
 *            DHT44, RHT05
 *
 *   (c) 2019-2021 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  hints:
 *  - Data line requires a 4.7kOhms pull-up resistor to Vdd (3.3-5.5V)
 *    (datasheet says 5.1k)
 *  - pin assignment for probes
 *    probe #1:  Gnd
 *    probe #2:  Data
 *    probe #3:  Vdd (current not limited)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef SW_DHTXX


/*
 *  local constants
 */

/* source management */
#define DHTXX_C

/* sensor models */
#define DHT11            1    /* DHT11 */
#define DHT22            2    /* DHT22 and compatibles */

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



/* ************************************************************************
 *   low level functions
 * ************************************************************************ */


/*
 *  set up probes for bus
 *  - probe-1: Gnd
 *  - probe-2: Data (pull-up resistor required)
 *  - probe-3: Vdd
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t DHTxx_Probes(void)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Run = 1;       /* loop control */

  /* inform user */
  ShortCircuit(0);                      /* make sure probes are not shorted */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: DHTxx */
    Display_ColoredEEString(DHTxx_str, COLOR_TITLE);
  #else
    Display_EEString(DHTxx_str);        /* display: DHTxx */
  #endif

  /* display module pinout (1: Gnd / 2: Data / 3: Vdd) */
  Display_NextLine();
  Show_SimplePinout('-', 'd', '+');

  /* set probes: probe-1 -- Gnd / probe-2 -- Rh -- Gnd / probe-3 -- Vcc */
  R_PORT = 0;                                /* pull down probe-2 */
  R_DDR = (1 << R_RH_2);                     /* enable Rh for probe-2 */
  ADC_PORT = (1 << TP3);                     /* pull down probe-1 & pull up probe-3 */
  ADC_DDR = (1 << TP1) | (1 << TP3);         /* enable pull up/down */

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

  /* set probes: probe-1 -- Gnd / probe-2 -- HiZ / probe-3 -- Vcc */
  /* remove pull-down via Rh for probe-2 */
  R_DDR = (1 << R_RL_3);      /* disable Rh for probe-2 */

  return Flag;
}



/*
 *  wait for DATA line to be pulled down by sensor
 *
 *  requires:
 *  - Timeout: timeout (in 10탎 steps)
 *
 *  returns:
 *  - 0  error or timeout exceeded
 *  - >0 time (in 10탎 steps)
 */

uint8_t DHTxx_WaitPullDown(uint8_t Timeout)
{
  uint8_t           Ticks = 0;     /* return value */

  while (Timeout > 0)
  {
    wait10us();                    /* wait 10탎 */
    Ticks++;                       /* increase time */

    if (! (ADC_PIN & (1 << TP2)))  /* check for low level of probe-2 */
    {
      break;                       /* end loop */
    }

    Timeout--;                     /* decrease timeout */
  }

  /* manage timeout */
  if (Timeout == 0)           /* timeout exceeded */
  {
    Ticks = 0;                /* reset time */
  }

  return Ticks;
}



/*
 *  wait for DATA line to be released by sensor
 *
 *  requires:
 *  - Timeout: timeout (in 10탎 steps)
 *
 *  returns:
 *  - 0  error or timeout exceeded
 *  - >0 time (in 10탎 steps)
 */

uint8_t DHTxx_WaitRelease(uint8_t Timeout)
{
  uint8_t           Ticks = 0;     /* return value */

  while (Timeout > 0)
  {
    wait10us();                    /* wait 10탎 */
    Ticks++;                       /* increase time */

    if (ADC_PIN & (1 << TP2))      /* check for high level of probe-2 */
    {
      break;                       /* end loop */
    }

    Timeout--;                     /* decrease timeout */
  }

  /* manage timeout */
  if (Timeout == 0)           /* timeout exceeded */
  {
    Ticks = 0;                /* reset time */
  }

  return Ticks;
}



/*
 *  trigger sensor and read measurements
 *
 *  requires:
 *  - Data: pointer to array of 5 bytes
 *
 *  returns:
 *  - 0 on any error
 *  - 1 on success
 */

uint8_t DHTxx_GetData(uint8_t *Data)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Ticks;         /* time ticks (in 10탎) */
  uint8_t           Bytes = 0;     /* byte counter */
  uint8_t           Bits = 1;      /* bits counter */
  uint8_t           Byte;          /* data byte */

  /* make sure that Data line is pulled up (by external resistor) */
  if (ADC_PIN & (1 << TP2))        /* check for high level of probe-2 */
  {  
    /*
     *  send start signal:
     *  - pull down Data line for >18ms
     *  - release Data line again
     */

    /* pull down Data line */
    /* change probe-2 to output mode (port pin is low) */
    ADC_DDR |= (1 << TP2);              /* set bit */

    wait20ms();                         /* wait 20ms */

    /* and release Data line */
    /* change probe-2 back to input mode */
    ADC_DDR &= ~(1 << TP2);             /* clear bit */


    /*
     *  sensor sends response:
     *  - 20-40탎 after MCU released Data line
     *  - pulls down Data line for 80탎
     *  - releases Data line for 80탎
     */

    /* wait up to 40탎 for sensor to pull down DATA line */
    Ticks = DHTxx_WaitPullDown(5);

    if (Ticks > 0)                 /* time ok */
    {
      /* wait about 80탎 for sensor to release DATA line */
      Ticks = DHTxx_WaitRelease(9);

      if (Ticks >= 6)              /* time ok */
      {
        /* wait about 80탎 for sensor to pull down DATA line again */
        Ticks = DHTxx_WaitPullDown(9);

        if (Ticks >= 6)            /* time ok */
        {
          Bytes = 5;               /* read 5 data bytes */
        }
      }
    }
  }


  /*
   *  sensor sends 5 bytes (40 bits) data:
   *  - each bit starts with pull-down of Data line for 50탎
   *  - 0 is indicated by release of Data for 26-28탎
   *  - 1 is indicated by release of Data for 70탎
   *  - after the last bit Data line is pulled down for 50탎
   *  - bit order: MSB
   */

  while (Bytes > 0)                /* 5 bytes */
  {
    Bits = 8;                      /* 1 byte equals 8 bits */
    Byte = 0;                      /* reset data byte */

    while (Bits > 0)               /* 8 bits */
    {
      /* wait about 50탎 for sensor to release Data line */
      Ticks = DHTxx_WaitRelease(6);

      if (Ticks >= 4)              /* time ok */
      {
        /* wait up to 70탎 for sensor to pull down Data line again */
        Ticks = DHTxx_WaitPullDown(8);

        if (Ticks >= 6)            /* 70 탎 -> 1 */
        {
          /* set bit */
          Byte <<= 1;              /* shift one bit left */
          Byte |= 0b00000001;      /* set bit */
        }
        else if ((Ticks >= 1) && (Ticks <= 3))    /* 26-28탎 -> 0 */
        {
          /* clear bit */
          Byte <<= 1;              /* shift one bit left */
        }
        else                       /* timing issue */
        {
          goto after_loop;         /* exit nested loops */
        }
      }
      else                         /* timing issue */
      {
        goto after_loop;           /* exit nested loops */
      }

      Bits--;                      /* decrease counter */
    }

    *Data = Byte;                  /* save byte to data buffer */
    Data++;                        /* next byte of data buffer */
    Bytes--;                       /* decrease counter */
  }

after_loop:

  if ((Bytes == 0) && (Bits == 0))   /* got all bytes */
  {
    /* wait about 50탎 for sensor to release Data line */
    Ticks = DHTxx_WaitRelease(6);

    if (Ticks >= 4)                /* time ok */
    {
      Flag = 1;                    /* signal success */
    }
  }

  return Flag;
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  check checksum
 *
 *  requires:
 *  - Data: pointer to array of 5 bytes
 *
 *  returns:
 *  - 0 on bad checksum
 *  - 1 on correct checksum
 */

uint8_t DHTxx_Checksum(uint8_t *Data)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Sum = 0;       /* checksum */
  uint8_t           n = 4;         /* counter */

  /*
   *  checksum (byte 5) is sum of bytes 1 to 4
   */

  while (n > 0)              /* calculate checksum */
  {
    Sum += *Data;            /* add data byte */
    Data++;                  /* next data byte */
    n--;                     /* decrease counter */
  }

  if (Sum == *Data)          /* checksum correct */
  {
    Flag = 1;                /* signal correct checksum */
  }

  return Flag;
}



/* ************************************************************************
 *   tools
 * ************************************************************************ */


/*
 *  DHT11
 *
 *  ranges:
 *  - humidity 20 - 80 %RH
 *  - temperature 0 - 50 캜
 *  data format:
 *  - byte 1: humidity high byte (integer part, in %RH)
 *  - byte 2: humidity low byte (decimal part, always 0)
 *  - byte 3: temperature high byte (integer part, in 캜)
 *  - byte 4: temperature low byte (decimal part, always 0)
 *  - byte 5: check sum (sum of bytes 1-4)
 */


/*
 *  display measurement values for DHT11
 *
 *  requires:
 *  - H_High: humidity high byte (in %RH)
 *  - T_High: temperature high byte (in 캜)
 */

void DHT11_DisplayValues(uint8_t H_High, uint8_t T_High)
{
  uint16_t          Temp;          /* temporary value */

  /* display temperature */
  Temp = T_High;

  #ifdef UI_FAHRENHEIT
  /* convert Celsius into Fahrenheit */
  Temp = Celsius2Fahrenheit(Temp, 0);
  #endif

  /* todo: add degree symbol to bitmap fonts */
  Display_FullValue(Temp, 0, '�');      /* display temperature */

  #ifdef UI_FAHRENHEIT
    Display_Char('F');             /* display: F (Fahrenheit) */
  #else
    Display_Char('C');             /* display: C (Celsius) */
  #endif


  /* display humidity */
  Display_Space();
  Display_FullValue(H_High, 0, '%');    /* display humidity */
  Display_EEString(RH_str);             /* display: RH */
}



/*
 *  DHT22
 *
 *  ranges:
 *  - humidity 0 - 99 %RH
 *  - temperature -40 - 80 캜
 *  data format:
 *  - byte 1: humidity high byte (in 0.1 %RH)
 *  - byte 2: humidity low byte
 *  - byte 3: temperature high byte (in 0.1 캜)
 *  - byte 4: temperature low byte
 *  - byte 5: check sum (sum of bytes 1-4)
 *  - bit 15 of humidity and temperature indicates negative value
 */


/*
 *  display measurement values for DHT22
 *
 *  requires:
 *  - H_High: humidity high byte (in %RH)
 *  - H_Low:  humidity low byte
 *  - T_High: temperature high byte (in 0.1 캜)
 *  - T_Low:  temperature low byte
 */

void DHT22_DisplayValues(uint8_t H_High, uint8_t H_Low, uint8_t T_High, uint8_t T_Low)
{
  int16_t           Temp;          /* temporary value */

  /*
   *  display temperature
   */

  /* build value */
  Temp = T_High & 0x7f;       /* take high byte and remove sign bit */
  Temp <<= 8;                 /* shift one byte left */
  Temp |= T_Low;              /* add low byte  */
  if (T_High & 0x80)          /* sign bit set */
  {
    Temp = -Temp;             /* negative value */
  }

  #ifdef UI_FAHRENHEIT
  /* convert Celsius into Fahrenheit */
  Temp = Celsius2Fahrenheit(Temp, 1);
  #endif

  /* todo: add degree symbol to bitmap fonts */
  Display_SignedFullValue(Temp, 1, '�');

  #ifdef UI_FAHRENHEIT
    Display_Char('F');             /* display: F (Fahrenheit) */
  #else
    Display_Char('C');             /* display: C (Celsius) */
  #endif


  /*
   *  display humidity
   */

  /* build value */
  Temp = H_High;              /* take high byte */
  Temp <<= 8;                 /* shift one byte left */
  Temp |= H_Low;              /* add low byte  */

  Display_Space();
  Display_FullValue(Temp, 1, '%');
  Display_EEString(RH_str);        /* display: RH */
}



/*
 *  display sensor type in first line
 *
 *  requires:
 *  - Sensor: sensor type
 *    DHT11
 *    DHT22
 *  - Mode: operation mode
 *    MODE_MANUAL
 *    MODE_AUTO
 */

void DHTxx_DisplaySensor(uint8_t Sensor, uint8_t Mode)
{
  /* show tool name */
  LCD_ClearLine(1);                     /* clear line #1 */
  LCD_CharPos(1, 1);                    /* move to first line */

  /* show sensor type */
  if (Sensor == DHT11)                  /* DHT11 */
  {
    #ifdef UI_COLORED_TITLES
      Display_ColoredEEString_Space(DHT11_str, COLOR_TITLE);
    #else
      Display_EEString_Space(DHT11_str);     /* display: DHT11 */
    #endif
  }
  else                                  /* DHT22 */
  {
    #ifdef UI_COLORED_TITLES
      Display_ColoredEEString_Space(DHT22_str, COLOR_TITLE);
    #else
      Display_EEString_Space(DHT22_str);     /* display: DHT22 */
    #endif
  }

  /* show mode */
  if (Mode == MODE_AUTO)           /* automatic mode */
  {
    Display_Char('*');             /* display: * */
  }
}



/*
 *  tool for DHTxx sensors
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any error
 */

uint8_t DHTxx_Tool(void)
{
  uint8_t           Flag;          /* control flag */
  uint8_t           Test;          /* key / feedback */
  uint8_t           Data[5];       /* sensor data */
  uint8_t           Sensor = DHT11;     /* sensor model */
  uint8_t           Mode = MODE_MANUAL; /* operation mode */
  uint16_t          Timeout;       /* timeout for user feedback */

  /* inform user about pinout and check for external pull-up resistor */
  Flag = DHTxx_Probes();

  if (Flag == 0)              /* bus error */
  {
    return 0;                 /* exit tool and signal error */
  }

  DHTxx_DisplaySensor(Sensor, Mode);    /* update display */
  LCD_ClearLine2();                     /* clear line #2 */
  MilliSleep(1000);                     /* power-up delay for sensor */
  Display_EEString(Start_str);          /* display: Start */

  /*
   *  processing loop
   */

  while (Flag)
  {
    /*
     *  user feedback
     */
 
    if (Mode == MODE_MANUAL)       /* manual mode */
    {
      Timeout = 0;                 /* wait for user */
    }
    else                           /* automatic mode */
    {
      Timeout = 1000;              /* wait max. 1s */
    }

    Test = TestKey(Timeout, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_LONG)          /* long key press */
    {
      if (Mode == MODE_MANUAL)     /* manual mode */
      {
        Mode = MODE_AUTO;          /* set automatic mode */
      }
      else                         /* automatic mode */
      {
        Sensor++;                  /* select next sensor model */
        if (Sensor > DHT22)        /* overflow */
        {
          Sensor = DHT11;          /* select DHT11 */
        }

        Mode = MODE_MANUAL;        /* set manual mode again */
      }

      DHTxx_DisplaySensor(Sensor, Mode);     /* update display */
      MilliSleep(500);                       /* smooth UI */
    }
    else if (Test == KEY_TWICE)    /* two short key presses */
    {
      /* exit tool */
      Flag = 0;                    /* end loop */
    }

    /* there should be a delay of >1s between measurements (DHT11) */

    /*
     *  read sensor
     */

    if (Flag)            /* ok to proceed */
    {
      LCD_ClearLine2();            /* clear line #2 */

      /* get measurement data from sensor */
      Test = DHTxx_GetData(&Data[0]);

      if (Test)                    /* got data */
      {
        /* check checksum */
        Test = DHTxx_Checksum(&Data[0]);

        if (Test == 1)             /* checksum correct */
        {
          /* display measurement values */
          if (Sensor == DHT11)     /* DHT11 */
          {
            DHT11_DisplayValues(Data[0], Data[2]);
          }
          else                     /* DHT22 */
          {
            DHT22_DisplayValues(Data[0], Data[1], Data[2], Data[3]);
          }
        }
        else                       /* checksum error */
        {
         Display_Minus();          /* display n/a */
        }
      }
      else                         /* some error */
      {
       Display_Minus();            /* display n/a */
      }
    }
  }

  return 1;                   /* signal success */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local constants */
#undef DHT_11
#undef DHT_22
#undef MODE_MANUAL
#undef MODE_AUTO

/* source management */
#undef DHTXX_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

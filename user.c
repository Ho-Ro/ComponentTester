/* ************************************************************************
 *
 *   user interface functions
 *
 *   (c) 2012-2021 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define USER_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */


/*
 *  local constants
 */

/* rotary encoder */
#define DIR_NONE         0b00000000     /* no turn or error */
#define DIR_RESET        0b00000001     /* reset state */



/* ************************************************************************
 *   values and scales
 * ************************************************************************ */


/*
 *  get number of digits of a value
 */

uint8_t NumberOfDigits(uint32_t Value)
{
  uint8_t           Counter = 1;   /* return value */

  while (Value >= 10)
  {
    Value /= 10;
    Counter++;
  }

  return Counter;
}



/*
 *  compare two scaled values
 *
 *  requires:
 *  - first pair of value and scale (*10^x)
 *  - second pair of value and scale (*10^x)
 *
 *  returns:
 *  - -1 if first value is smaller than seconds one
 *  - 0 if equal
 *  - 1 if first value is larger than second one
 */

int8_t CmpValue(uint32_t Value1, int8_t Scale1, uint32_t Value2, int8_t Scale2)
{
  int8_t            Flag;               /* return value */
  int8_t            Len1, Len2;         /* length */

  /* determine virtual length */
  Len1 = NumberOfDigits(Value1) + Scale1;
  Len2 = NumberOfDigits(Value2) + Scale2;

  if ((Value1 == 0) || (Value2 == 0))    /* special case */
  {
    Flag = 10;                /* perform direct comparison */
  }
  else if (Len1 > Len2)       /* more digits -> larger */
  {
    Flag = 1;
  }
  else if (Len1 == Len2)      /* same length */
  {
    /* re-scale to longer value */
    Len1 -= Scale1;
    Len2 -= Scale2;

    while (Len1 > Len2)       /* up-scale Value #2 */
    {
      Value2 *= 10;
      Len2++;
      /* Scale2-- */
    }

    while (Len2 > Len1)       /* up-scale Value #1 */
    {
      Value1 *= 10;
      Len1++;
      /* Scale1-- */
    }   

    Flag = 10;                /* perform direct comparison */
  }
  else                        /* less digits -> smaller */
  {
    Flag = -1;
  }

  if (Flag == 10)             /* perform direct comparison */
  {
    if (Value1 > Value2) Flag = 1;
    else if (Value1 < Value2) Flag = -1;
    else Flag = 0;
  }

  return Flag;
}



/*
 *  rescale value
 *
 *  requires:
 *  - value and scale (*10^x)
 *  - new scale (*10^x)
 */

uint32_t RescaleValue(uint32_t Value, int8_t Scale, int8_t NewScale)
{
  uint32_t          NewValue;      /* return value */

  NewValue = Value;           /* take old value */

  while (Scale != NewScale)   /* processing loop */
  {
    if (NewScale > Scale)     /* upscale */
    {
      NewValue /= 10;
      Scale++;
    }
    else                      /* downscale */
    {
      NewValue *= 10;
      Scale--;
    }
  }

  return NewValue;
}



#ifdef UI_ROUND_DS18B20

/*
 *  round value while also scaling
 *
 *  requires:
 *  - Value: value
 *  - Scale: number of decimal places (0 = none)
 *  - RoundScale: number of decimal places to round to
 *
 *  returns:
 *  - rounded value
 */

int32_t RoundSignedValue(int32_t Value, uint8_t Scale, uint8_t RoundScale)
{
  int8_t            Offset = 5;         /* rounding offset */

  while (Scale < RoundScale)  /* not enough decimal digits */
  {
    Value *= 10;              /* scale by 0.1 */
    Scale++;                  /* one decimal digit more */
  }

  if (Value < 0)              /* negative value */
  {
    Offset = -5;              /* negative rounding offset */
  }

  while (Scale > RoundScale)  /* rounding loop */
  {
    Value += Offset;          /* for rounding */
    Value /= 10;              /* /10 */
    Scale--;                  /* one digit less */
  }

  return Value;
}

#endif



/* ************************************************************************
 *   conversion functions
 * ************************************************************************ */


#ifdef UI_FAHRENHEIT
  #if defined (SW_DS18B20) || defined (SW_DHTXX)

/*
 *  convert temperature value from Celcius to Fahrenheit
 *
 *  requires:
 *  - Value: temperature in °C
 *  - Scale: number of decimal places (0 = none)
 *
 *  returns:
 *  - temperature in °F
 */

int32_t Celsius2Fahrenheit(int32_t Value, uint8_t Scale)
{
  int32_t           Offset = 32;   /* offset */

  /*
   *  convert °C to °F
   *  - T[°F] = T[°C] * 9/5 + 32
   */

  /* scale offset to match temperature's scale */
  while (Scale > 0)
  {
    Offset *= 10;             /* scale by 10^1 */
    Scale--;                  /* next digit */
  }

  /* convert into Fahrenheit */
  Value *= 9;
  Value /= 5;
  Value += Offset;            /* add scaled offset */

  return Value;
}

  #endif
#endif



/* ************************************************************************
 *   string functions
 * ************************************************************************ */


#ifdef UI_KEY_HINTS

/*
 *  get length of a fixed string stored in EEPROM/Flash
 *
 *  requires:
 *  - pointer to fixed string
 */

uint8_t EEStringLength(const unsigned char *String)
{
  uint8_t           Length = 0;         /* return value */
  unsigned char     Char;               /* character */

  /* read characters until we get the terminating 0 */
  while ((Char = DATA_read_byte(String)))
  {
    Length++;                           /* got another character */
    String++;                           /* next one */
  }

  return Length;
}

#endif



/* ************************************************************************
 *   user input 
 *   - test push button
 *   - rotary encoder
 *   - increase/decrease push buttons
 *   - touch screen
 * ************************************************************************ */


#ifdef HW_ENCODER 

/*
 *  read rotary encoder
 *  - adds delay of 0.5ms
 *  - A & B have pull-up resistors
 *  - encoder might be in parallel with LCD signal lines
 *
 *  returns user action:
 *  - KEY_NONE for no turn or invalid signal
 *  - KEY_RIGHT for right/clockwise turn
 *  - KEY_LEFT for left/counter-clockwise turn
 */

uint8_t ReadEncoder(void)
{
  uint8_t           Key = KEY_NONE;     /* return value */
  uint8_t           Temp;               /* temporary value */
  uint8_t           AB = 0;             /* new AB state */
  uint8_t           Old_AB;             /* old AB state */

  /* set encoder's A & B pins to input */
  Old_AB = ENCODER_DDR;                 /* save current settings */
  ENCODER_DDR &= ~((1 << ENCODER_A) | (1 << ENCODER_B));
  wait500us();                          /* settle time */

  /* get A & B signals (1 = open / 0 = closed) */
  Temp = ENCODER_PIN;
  if (Temp & (1 << ENCODER_A)) AB = 0b00000010;
  if (Temp & (1 << ENCODER_B)) AB |= 0b00000001;

  /* restore port/pin settings */
  ENCODER_DDR = Old_AB;                 /* restore old settings */

  /* update state history */
  if (UI.EncDir == DIR_RESET)           /* first scan */
  {
    UI.EncState = AB;                   /* set as last state */
    UI.EncDir = DIR_NONE;               /* reset direction */
    UI.EncTicks = 0;                    /* reset time counter */
  }

  /* time counter */
  if (UI.EncTicks > 0)        /* after first pulse */
  {
    if (UI.EncTicks < 250)    /* prevent overflow */
    {
      UI.EncTicks++;          /* increase counter */
    }
  }

  Old_AB = UI.EncState;                 /* save last state */
  UI.EncState = AB;                     /* and save new state */

  /* process signals */
  if (Old_AB != AB)        /* signals changed */
  {
    /* check if only one bit has changed (Gray code) */
    Temp = AB ^ Old_AB;                 /* get bit difference */
    if (!(Temp & 0b00000001))           /* B hasn't changed */
    {
      Temp >>= 1;                       /* shift right to get A */
    }

    if (Temp == 1)                      /* valid change */
    {
      /*
       *  detect direction
       *  - check sequence: next AB for AB=11, AB=10, AB=01, AB=00
       *  - right turn (CW):
       *    Gray code for AB: 00-10-11-01 -> check sequence 01110010
       *  - left turn (CCW):
       *    Gray code for AB: 00-01-11-10 -> check sequence 10001101
       */

      Temp = 0b01110010;                /* expected values for a right turn */
      Temp >>= (Old_AB * 2);            /* get expected value by shifting */
      Temp &= 0b00000011;               /* select value */
      if (Temp == AB)                   /* value matches */
        Temp = KEY_RIGHT;               /* turn to the right */
      else                              /* value mismatches */
        Temp = KEY_LEFT;                /* turn to the left */

      /* step/detent logic */
      UI.EncPulses++;                   /* got a new pulse */

      if (Temp != UI.EncDir)            /* direction has changed */
      {     
        UI.EncPulses = 1;               /* first pulse for new direction */
        UI.EncTicks = 1;                /* enable time counter */
      }

      UI.EncDir = Temp;                 /* update direction */

      if (UI.EncPulses >= ENCODER_PULSES)    /* reached step */
      {
        UI.EncPulses = 0;               /* reset pulses */
        Key = Temp;                     /* signal valid step */
      }
    }
    else                                /* invalid change */
    {
      UI.EncDir = DIR_RESET;            /* reset direction state */
    }
  }

  return Key;
}

#endif



#ifdef HW_INCDEC_KEYS

/*
 *  check increase/decrease push buttons
 *  - adds delay of 0.5ms
 *  - buttons might be in parallel with LCD signal lines
 *  - speed-up functionality (similar to rotary encoder's turning velocity)
 *
 *  returns:
 *  - KEY_NONE for no button press
 *  - KEY_RIGHT for increase
 *  - KEY_LEFT for decrease
 *  - KEY_INCDEC for increase and decrease (both at the same time)
 */

uint8_t ReadIncDecKeys(void)
{
  uint8_t           Key = KEY_NONE;          /* return value */
  uint8_t           RegState;                /* register state */
  uint8_t           Run = 1;                 /* loop control */
  uint8_t           Temp;                    /* temporary value */
  uint8_t           TicksInc = 0;            /* time counter */
  uint8_t           TicksDec = 0;            /* time counter */

  /* set pins to input */
  RegState = KEY_DDR;              /* save current settings */
  KEY_DDR &= ~((1 << KEY_INC) | (1 << KEY_DEC));
  wait500us();                     /* settle time */

  /* processing loop */
  while (Run == 1)
  {
    /* check push button state (low active) */
    Temp = KEY_PIN;                     /* get current state */
    Temp = ~Temp;                       /* invert bits */
    Temp &= (1 << KEY_INC) | (1 << KEY_DEC); /* filter buttons */

    if (Temp == 0)                 /* not button pressed */
    {
      Run = 0;                     /* end loop */
    }
    else                           /* button pressed */
    {    
      if (Temp & (1 << KEY_INC))   /* "increase" button */
      {
        TicksInc++;                /* increase counter */
      }

      if (Temp & (1 << KEY_DEC))   /* "decrease" button */
      {
        TicksDec++;                /* increase counter */
      }

      Temp = TicksInc + TicksDec;
      if (Temp >= 10)              /* long key press (300ms) */
      {
        Run = 2;                   /* end loop & signal long key press */
      }
      else
      {
        MilliSleep(30);            /* time to debounce and delay */
      }
    }
  }

  /* process counters */
  if (TicksInc > 0)                /* "increase" button pressed */
  {
    Key = KEY_RIGHT;
  }

  if (TicksDec > 0)                /* "decrease" button pressed */
  {
    if (Key == KEY_RIGHT)          /* also "increase" button */
    {
      Key = KEY_INCDEC;            /* both buttons pressed */
    }
    else                           /* just "decrease */
    {
      Key = KEY_LEFT;
    }
  }

  /* speed-up functionality */
  if (Key != KEY_NONE)             /* button pressed */
  {
    Temp = 1;                      /* default step size */

    if (Key == UI.KeyOld)          /* same key as before */
    {
      if (Run == 2)                /* long button press */
      {
        Temp = UI.KeyStepOld;      /* get former step size */

        if (Temp <= 6)             /* limit step size to 7 */
        {
          Temp++;                  /* increase step size */
        }
      }
    }

    UI.KeyStepOld = Temp;          /* update former step size */
    UI.KeyStep = Temp;             /* set new step size */
  }

  /* restore port/pin settings */
  KEY_DDR = RegState;              /* restore old settings */

  return Key;
}

#endif



#ifdef HW_TOUCH

/*
 *  check touch screen
 *
 *  requires:
 *  - Mode (bitfield):
 *    CHECK_KEY_TWICE   check for two short test key presses
 *    any other flags   ignore
 *
 *  returns:
 *  - KEY_NONE for no touch event
 *  - KEY_SHORT for short press of center area
 *  - KEY_LONG for long press of center area
 *  - KEY_RIGHT for increase (right & bottom bar)
 *  - KEY_LEFT for decrease (left & top bar)
 */

uint8_t ReadTouchScreen(uint8_t Mode)
{
  uint8_t           Key = KEY_NONE;          /* return value */
  uint8_t           OldKey = KEY_NONE;       /* former key action */
  uint8_t           Run = 1;                 /* loop control */
  uint8_t           Flag;                    /* control flag */
  uint8_t           n = 0;                   /* counter */
  uint8_t           X, Y;                    /* char pos */
  uint8_t           X_Max, Y_Max;            /* char pos limits */

  X_Max = UI.CharMax_X;       /* get limits */
  Y_Max = UI.CharMax_Y;

  /* processing loop */
  while (Run == 1)
  {
    Flag = Touch_Check();     /* check touch controller */

    if (Flag)                 /* touch event */
    {
      X = UI.TouchPos_X;      /* get char pos */
      Y = UI.TouchPos_Y;

      /* derive "button" */
      if (X <= 3)                  /* left touch bar (3 columns) */
      {
        Key = KEY_LEFT;
      }
      else if (X >= (X_Max - 2))   /* right touch bar (3 columns) */
      {
        Key = KEY_RIGHT;
      }
      else if (Y <= 2)             /* top touch bar (2 rows) */
      {
        Key = KEY_LEFT;
      }
      else if (Y >= (Y_Max - 1))   /* bottom touch bar (2 rows) */
      {
        Key = KEY_RIGHT;
      }
      else                         /* center area */
      {
        Key = KEY_SHORT;
      }

      /* control logic */
      if (Key == KEY_NONE)         /* no valid Key */
      {
        Run = 0;                   /* end loop */
      }
      else                         /* valid key */
      {
        if (OldKey == KEY_NONE)    /* first run */
        {
          OldKey = Key;            /* set former key */
        }

        if (OldKey != Key)         /* key has changed */
        {
          Key = KEY_NONE;          /* reset key */
          Run = 0;                 /* end loop */
        }
      }

      n++;                    /* increase counter */

      if (n >= 10)            /* long "button" press (300ms) */
      {
        Run = 2;              /* end loop & signal long press */
      }
      else                    /* less than 300ms */
      {
        MilliSleep(30);       /* time to debounce and delay */
      }
    }
    else                      /* no touch event */
    {
      Run = 0;                /* end loop */
    }
  }


  /* speed-up functionality */
  if (Key != KEY_NONE)             /* "button" pressed */
  {
    n = 1;                         /* default step size */

    if (Key == KEY_SHORT)          /* special case: center area */
    {
      if (Run == 2)                /* long key press */
      {
        Key = KEY_LONG;            /* change to long key press */
      }
    }
    else if (Key == UI.KeyOld)     /* same key as before */
    {
      if (Run == 2)                /* long button press */
      {
        n = UI.KeyStepOld;         /* get former step size */

        if (n <= 6)                /* limit step size to 7 */
        {
          n++;                     /* increase step size */
        }
      }
    }

    UI.KeyStepOld = n;             /* update former step size */
    UI.KeyStep = n;                /* set new step size */
  }

  /* check for second key press (center area) if requested */
  if ((Mode & CHECK_KEY_TWICE) && (Key == KEY_SHORT))
  {
    MilliSleep(30);                /* delay for checking key again */
    Run = 20;                      /* timeout of 200ms */
 
    while (Run > 0)                /* timeout loop */
    {
      Flag = Touch_Check();        /* check touch controller */

      if (Flag)                    /* touch event */
      {
        X = UI.TouchPos_X;         /* get char pos */
        Y = UI.TouchPos_Y;

        /* check for center area */
        if ((X > 3) && (X < (X_Max - 2)) &&
            (Y > 2) && (Y < (Y_Max - 1)))
        {
          Key = KEY_TWICE;         /* signal two key presses */
          MilliSleep(200);         /* smooth UI */
        }

        /* either way (center area or some other bar) */
        Run = 0;                  /* end loop */
      }
      else
      {
        Run--;                     /* decrease timeout */
        MilliSleep(10);            /* wait 10ms */
      }
    }
  }

  return Key;
}

#endif



/*
 *  get user feedback
 *  - test push button
 *  - optional rotary encoder incl. detection of turning velocity
 *  - optional increase/decrease push buttons
 *  - optional touch screen
 *  - processing can be terminated by signaling OP_BREAK_KEY via Cfg.OP_Control
 *    (e.g. set by an interrupt handler to deal with an infinite timeout)
 *  - optional auto-power-off for auto-hold mode (signaled by OP_PWR_TIMEOUT
 *    via Cfg.OP_Control) 
 *
 *  requires:
 *  - Timeout in ms 
 *    0 = no timeout, wait for key press or any other feedback
 *  - Mode (bitfield):
 *    CURSOR_NONE      no cursor
 *    CURSOR_STEADY    steady cursor
 *    CURSOR_BLINK     blinking cursor
 *    CHECK_OP_MODE    consider tester operation mode (Cfg.OP_Mode)
 *    CHECK_KEY_TWICE  check for two short test key presses
 *    CHECK_BAT        check battery (and power off on low battery)
 *    CURSOR_TEXT      show text instead of cursor (UI.KeyHint)
 *
 *  returns:
 *  - KEY_TIMEOUT     reached timeout (no key press)
 *  - KEY_SHORT       short press of test key
 *  - KEY_LONG        long press of test key
 *  - KEY_TWICE       two short presses of test key 
 *  - KEY_RIGHT       right key (e.g. rotary encoder turned right)
 *  - KEY_LEFT        left key (e.g. rotary encoder turned left)
 *  - KEY_INCDEC      increase and decrease keys both pressed
 *  - KEY_POWER_OFF   power off requested
 *  The turning velocity (speed-up) is returned via UI.KeyStep (1-7).
 */

uint8_t TestKey(uint16_t Timeout, uint8_t Mode)
{
  uint8_t           Key = KEY_TIMEOUT;  /* return value */
  uint8_t           Run = 1;            /* loop control */
  uint8_t           Ticks = 0;          /* time counter */
  uint8_t           Test;               /* temp. value */
  #ifdef HW_ENCODER
  uint8_t           Timeout2;           /* step timeout */
  uint8_t           Steps = 0;          /* step counter */
  uint8_t           MinSteps = 2;       /* required steps */
  uint16_t          Temp;               /* temp value */
  #endif
  #ifdef POWER_OFF_TIMEOUT
  uint16_t          PwrTimeout = 0;     /* timeout for auto power-off (auto-hold mode) */
  #endif
  #ifdef UI_KEY_HINTS
  uint8_t           Pos = 0;            /* X position for text */
  #endif
  #ifdef UI_COLORED_CURSOR
  uint16_t          Color;              /* pen color */
  #endif


  /*
   *  init
   */

  #ifdef HW_ENCODER
    /* rotary encoder: sample each 2.5ms (1ms would be ideal) */
    #define DELAY_TICK   2         /* 2ms + 0.5ms for ReadEncoder() */
    #define DELAY_500    200       /* ticks for 500ms */
    #define DELAY_100    40        /* ticks for 100ms */
  #else
    /* just the test key */
    #define DELAY_TICK   5         /* 5ms */
    #define DELAY_500    100       /* ticks for 500ms */
    #define DELAY_100    20        /* ticks for 100ms */
  #endif

  #ifdef HW_ENCODER
  /* init variables for rotary encoder */
  UI.EncDir = DIR_RESET;      /* resets also UI.EncState and .EncTicks */
  UI.EncPulses = 0;
  Timeout2 = 50;
  #endif

  #ifdef HW_KEYS
  /* init variables for additional keys */ 
  UI.KeyStep = 1;             /* default level #1 */
  #endif

  #ifdef POWER_OFF_TIMEOUT
  /* init power-off timeout */
  if (Cfg.OP_Control & OP_PWR_TIMEOUT)  /* power-off timeout enabled */
  {
    PwrTimeout = POWER_OFF_TIMEOUT * 2;      /* in 500ms */
  }
  #endif

  if (Mode & CHECK_OP_MODE)        /* consider operation mode */
  {
    if (Cfg.OP_Mode & OP_AUTOHOLD)      /* in auto-hold mode */
    {
      Timeout = 0;                 /* disable timeout */
                                   /* and keep cursor mode */
    }
    else                                /* in continuous mode */
    {
      Mode &= ~(CURSOR_STEADY | CURSOR_BLINK);    /* clear all cursor flags */
                                   /* and keep timeout */
    }
  }

  #ifdef UI_COLORED_CURSOR
  Color = UI.PenColor;             /* save current pen color */
  UI.PenColor = COLOR_CURSOR;      /* set cursor color */
  #endif

  #ifdef UI_KEY_HINTS
  if (Mode & CURSOR_TEXT)          /* show key hint */
  {
    Test = EEStringLength(UI.KeyHint);       /* length of string */
    Test--;                                  /* adjust to X addressing */

    if (UI.CharMax_X > Test)                 /* sanity check */
    {
      Pos = UI.CharMax_X - Test;             /* text position */

      /* disable cursor */
      Mode &= ~(CURSOR_STEADY | CURSOR_BLINK);    /* clear all cursor flags */

      /* display key hint at bottom right */
      LCD_CharPos(Pos, UI.CharMax_Y);        /* set start position */
      Display_EEString(UI.KeyHint);          /* display hint */
    }
  }
  #endif

  if (Mode & (CURSOR_STEADY | CURSOR_BLINK))  /* cursor enabled */
  {
    LCD_Cursor(1);            /* enable cursor on display */
  }


  /*
   *  wait for user feedback or timeout
   */
 
  while (Run)
  {
    /* take care about feedback timeout */
    if (Timeout > 0)               /* timeout enabled */
    {
      if (Timeout > DELAY_TICK)    /* some time left */
      {
        Timeout -= DELAY_TICK;     /* decrease timeout */
      }
      else                         /* timeout */
      {
        Run = 0;                   /* end loop */
      }
    }


    /*
     *  check for test push button
     *  - push button is low active
     */

    Test = BUTTON_PIN & (1 << TEST_BUTTON);  /* get button status */

    if (Test == 0)            /* test button pressed */
    {
      Ticks = 0;              /* reset counter */
      MilliSleep(30);         /* time to debounce */

      while (Run)             /* detect how long key is pressed */
      {
        Test = BUTTON_PIN & (1 << TEST_BUTTON);   /* get button status */

        if (Test == 0)        /* key still pressed */
        {
          Ticks++;                      /* increase counter */
          if (Ticks > 26) Run = 0;      /* end loop if 300ms are reached */
          else MilliSleep(10);          /* otherwise wait 10ms */
        }
        else                            /* key released */
        {
          Run = 0;                      /* end loop */
        }
      }

      /* determine key press type */
      if (Ticks > 26)                   /* long (>= 300ms) */
      {
        Key = KEY_LONG;                 /* signal long key press */
      }
      else                              /* short (< 300ms) */
      {
        Key = KEY_SHORT;                /* signal short key press */

        /* check for second key press if requested */
        if (Mode & CHECK_KEY_TWICE)
        {
          MilliSleep(50);               /* delay for checking key again */
          Ticks = 20;                   /* timeout of 200ms */
 
          while (Ticks > 0)             /* timeout loop */
          {
            Test = BUTTON_PIN & (1 << TEST_BUTTON);    /* get button status */         

            if (Test == 0)                             /* test button pressed */
            {
              MilliSleep(30);                          /* time to debounce */
              Test = BUTTON_PIN & (1 << TEST_BUTTON);  /* get button status */

              if (Test == 0)                           /* test button still pressed */
              {
                Ticks = 1;                             /* end loop */
                Key = KEY_TWICE;                       /* signal two key presses */
                MilliSleep(200);                       /* smooth UI */
              }
            }

            Ticks--;                                   /* decrease timeout */
            MilliSleep(10);                            /* wait 10ms */
          }
        }
      }
    }
    else                      /* no key press */
    {
      /*
       *  touch screen
       */

      #ifdef HW_TOUCH
      Test = ReadTouchScreen(Mode);

      if (Test)               /* got user input */
      {
        Key = Test;                /* save key */
        break;                     /* exit loop */
      }
      #endif

      /*
       *  increase/decrease push buttons
       */

      #ifdef HW_INCDEC_KEYS
      Test = ReadIncDecKeys();

      if (Test)               /* got user input */
      {
        Key = Test;                /* save key */
        break;                     /* exit loop */
      }
      #endif

      /*
       *  rotary encoder
       */

      #ifdef HW_ENCODER
      Test = ReadEncoder();        /* read rotary encoder */

      if (Test)                    /* got user input */
      {
        if (Steps == 0)            /* first step */
        {
          Key = Test;              /* save direction */
        }

        if (Test == Key)           /* step in same direction */
        {
          Steps++;                 /* increase counter */

          /* calculate turning velocity */
          Test = UI.EncTicks / Steps;        /* ticks per step */
          Timeout2 += Test;                  /* add to timeout */
          Timeout2 += 3 * ENCODER_PULSES;    /* add some buffer */
          /* adjustment for steps/360°: *(steps/16) */
          Temp = Test * ENCODER_STEPS;
          Temp /= 16;
          Test = (uint8_t)Temp;
          /* velocity levels: 0 (fast) - 5 (slow) */
          if (Test > 40) Test = 40;          /* limit ticks */
          Test /= 8;                         /* get velocity level (0-5) */
          /* require 3 steps for high velocities */
          if (Test <= 2) MinSteps = 3;

          if (Steps == MinSteps)        /* got required steps */
          {
            /* reverse velocity level: 2 (slow) - 7 (fast) */
            UI.KeyStep = 7 - Test;      /* velocity level (2-7) */
            break;                      /* exit loop */
          }
        }
        else                       /* changed direction */
        {
          /* keep last direction and velocity level #1 */
          break;                   /* exit loop */
        }
      }

      if (Steps)                   /* after first step */
      {
        if (UI.EncTicks >= Timeout2)    /* timeout for velocity detection */
        {
          /* keep velocity level #1 */
          break;                        /* exit loop */
        }
      }
      #endif

      /*
       *  TTL serial
       */

      #ifdef SERIAL_RW
      if (Cfg.OP_Control & OP_RX_LOCKED)     /* buffer locked */
      {
        /* we received a command via the serial interface */
        Key = KEY_COMMAND;         /* remote command */
        break;                     /* exit loop */
      }
      #endif

      /*
       *  timing
       */

      /* delay for next loop run */
      MilliSleep(DELAY_TICK);           /* wait a little bit */

      Ticks++;                          /* increase counter */

      #ifndef BAT_NONE
      /*
       *  100ms timer
       *  - for battery monitoring
       */

      if (Ticks % DELAY_100 == 0)       /* every 100ms */
      {
        if (Cfg.BatTimer > 1)           /* timeout not zero yet */
        {
          Cfg.BatTimer--;               /* decrease timeout counter */
        }
        else                            /* timeout triggered */
        {
          if (Mode & CHECK_BAT)         /* battery check requested */
          {
            CheckBattery();             /* check battery */
                                        /* also powers off on low battery */
          }
        }
      } 
      #endif

      /*
       *  500ms timer
       *  - for blinking cursor
       *    HD44780's built-in blinking cursor is ugly anyway :)
       *  - also for optional auto power-off
       */

      if (Ticks == DELAY_500)           /* every 500ms */
      {
        Ticks = 0;                      /* reset counter */

        /* blinking cursor */
        if (Mode & CURSOR_BLINK)        /* blinking cursor enabled */
        {
          /* we misuse Run as toggle switch */
          if (Run == 1)                 /* turn off */
          {
            LCD_Cursor(0);              /* disable cursor */
            Run = 2;                    /* toggle flag */
          }
          else                          /* turn on */
          {
            LCD_Cursor(1);              /* enable cursor */
            Run = 1;                    /* toggle flag */
          }
        }

        #ifdef POWER_OFF_TIMEOUT
        /* automatic power-off */
        if (PwrTimeout > 0)             /* power-off timeout enabled */
        {
          if (PwrTimeout > 1)           /* some time left */
          {
            PwrTimeout--;               /* decrease counter */
          }
          else                          /* timeout */
          {
            Key = KEY_POWER_OFF;        /* signal power-off */
            Run = 0;                    /* exit loop */
          }
        }
        #endif
      }
    }

    /* check if we should exit anyway */
    if (Cfg.OP_Control & OP_BREAK_KEY)  /* got break signal */
    {
      Cfg.OP_Control &= ~OP_BREAK_KEY;  /* clear break signal */
      break;                            /* exit loop */
    }
  }


  /*
   *  clean up
   */

  #ifdef UI_COLORED_CURSOR
  UI.PenColor = Color;        /* restore pen color */
  #endif

  if (Mode & (CURSOR_STEADY | CURSOR_BLINK))  /* cursor enabled */
  {
    LCD_Cursor(0);            /* disable cursor on display */
  }

  #ifdef UI_KEY_HINTS
  if (Pos)                    /* show key hint */
  {
    /* clear key hint */
    LCD_CharPos(Pos, UI.CharMax_Y);     /* set start position */
    LCD_ClearLine(0);                   /* clear remaining space */
  }
  #endif

  #ifdef HW_KEYS
  UI.KeyOld = Key;            /* update former key */
  #endif

  #ifdef POWER_OFF_TIMEOUT
  /* automatic power-off */
  if (Key == KEY_POWER_OFF)   /* power-off triggered */
  {
    PowerOff();               /* power off */
  }
  #endif

  #undef DELAY_500
  #undef DELAY_TICK

  return Key;
}



/*
 *  convenience version of TestKey() to wait for any user input or timeout
 *  based on the tester's operation mode
 */

void WaitKey(void)
{
  /* wait for key press or 3s timeout */
  TestKey(3000, CURSOR_STEADY | CHECK_OP_MODE | CHECK_BAT);
}



#ifdef FUNC_SMOOTHLONGKEYPRESS

/*
 *  smooth UI for a long key press to prevent a second long-key-press
 *  action
 */

void SmoothLongKeyPress(void)
{
  /* wait until button is released */
  while (!(BUTTON_PIN & (1 << TEST_BUTTON)))  /* as long as key is pressed */
  {
    wait10ms();               /* wait a little bit */
    wdt_reset();              /* reset watchdog */
  }

  MilliSleep(500);            /* smooth UI */
}

#endif



/* ************************************************************************
 *   various support functions
 * ************************************************************************ */


/*
 *  Tell user to create or remove short-circuit of all three probes 
 *  and wait until he really does. But allow user to abort creating
 *  a short circuit by a key press.
 *
 *  requires:
 *  - mode:
 *    0 = remove short-circuit
 *    1 = create short circuit
 *
 *  returns:
 *  - 0 on any problem
 *  - 1 on success
 */

uint8_t ShortCircuit(uint8_t Mode)
{
  uint8_t           Flag = 2;           /* return value / control flag */
  uint8_t           Test;               /* test feedback */
  uint8_t           Comp;               /* expected result */
  unsigned char     *String;            /* display string pointer */

  /* init */
  if (Mode == 0)         /* remove short */
  {
    String = (unsigned char *)Remove_str;    /* "remove" */
    Comp = 0;                                /* no shorted probe pairs */
  }
  else                   /* create short */
  {
    String = (unsigned char *)Create_str;    /* "create" */
    Comp = 3;                                /* 3 shorted probe pairs */
  } 

  /* check if already done */
  Test = ShortedProbes();               /* get current status */

  if (Test == Comp)                     /* already done */
  {
    Flag = 1;                           /* skip loop */
  }
  else                                  /* not done yet */
  {
    /* tell user what to do */
    LCD_Clear();
    Display_EEString(String);             /* display: Remove/Create */
    Display_NextLine();
    Display_EEString(ShortCircuit_str);   /* display: short circuit! */
  }  

  /* wait until all probes are dis/connected */
  while (Flag == 2)
  {
    Test = ShortedProbes();        /* check for shorted probes */

    if (Test == Comp)              /* job done */
    {
       Flag = 1;                   /* end loop */
       MilliSleep(200);            /* delay to smooth UI */
    }
    else                           /* job not done yet */
    {
      /* wait 100ms or for any key press */
      Test = TestKey(100, CHECK_BAT);
      if (Mode == 1)                         /* short circuit expected */
      {
        if (Test > KEY_TIMEOUT) Flag = 0;    /* abort on key press */
      }
    }
  }

  return Flag;
}



/* ************************************************************************
 *   menu management
 * ************************************************************************ */


/*
 *  mark selected item
 *
 *  requires:
 *  - item: current item number (0-255)
 *  - selected: selected item number (0-255)
 */

void MarkItem(uint8_t Item, uint8_t Selected)
{
  if (Selected == Item)       /* current item selected */
  {
    #ifdef LCD_COLOR
    UI.PenColor = COLOR_MARKER;    /* change color */
    #endif
    Display_Char('*');             /* display asterisk */
    #ifdef LCD_COLOR
    UI.PenColor = COLOR_PEN;       /* reset color */
    #endif
  }
  else                        /* current item not selected */
  {
    Display_Space();               /* display space */
  }
}



/*
 *  menu tool
 *  - expects title in first line
 *  - outputs menu starting in second line
 *
 *  requires:
 *  - Items: number of menu items
 *  - Type: type of menu items
 *      1  pointer (in RAM) to fixed string stored in EEPROM
 *      2  uint16_t stored in EEPROM
 *  - Menu: address of array with menu items
 *  - Unit: optional fixed string stored in EEPROM
 *
 *  returns:
 *  - ID of selected item
 */

uint8_t MenuTool(uint8_t Items, uint8_t Type, void *Menu[], unsigned char *Unit)
{
  uint8_t           Selected = 0;       /* return value / ID of selected item */
  uint8_t           First = 0;          /* first item listed */
  uint8_t           Run = 2;            /* loop control flag */
  uint8_t           Lines;              /* line number */
  uint8_t           n;                  /* temp value */
  void              *Address;           /* address of menu element */
  uint16_t          Value;              /* temp. value */

  /* init */
  Items--;                    /* to match array counter */
  Lines = UI.CharMax_Y;       /* max. number of lines */
  Lines--;                    /* adjust to match item counter */

  /* add ":" to title in line #1 */
  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();    /* use title color */
  #endif
  Display_Colon();            /* display: <whatever>: */
  #ifdef UI_COLORED_TITLES
  Display_UsePenColor();      /* use pen color */
  #endif


  /*
   *  processing loop
   */

  while (Run)
  {
    if (Lines == 1)           /* 2-line display */
    {
      First = Selected;       /* just one line for items */
      Run++;                  /* set flag for changed list */
    }


    /*
     *  display item(s)
     *  - starting in line #2
     */

    Address = &Menu[First];        /* get address of first item */
    n = 0;                         /* reset counter */

    while (n < Lines)
    {
      LCD_CharPos(1, n + 2);       /* move to start of line */

      /* display indicator for multiline displays */
      if (Lines > 1)
      {
        MarkItem(First + n, Selected);
      }

      if (Run > 1)            /* list changed */
      {
        /* display item or value */
        if (Type == 1)                  /* fixed string */
        {
          Display_EEString(*(unsigned char **)Address);
        }
        else                            /* uint16_t in EEPROM */
        {
          Value = DATA_read_word(Address);   /* read value at eeprom address */
          Display_Value(Value, 0, 0);
        }     

        /* display optional fixed string */
        if (Unit)
        {
          Display_EEString(Unit);
        }  

        LCD_ClearLine(0);     /* clear rest of this line */
      }

      Address += 2;                /* next address (2 byte steps) */
      n++;                         /* next item */

      if (n > Items) n = Lines;    /* end loop for a short list */
    }

    Run = 1;             /* reset loop flag (changed list) */

    /* show navigation help for 2-line displays */
    if (Lines == 1)
    {
      LCD_CharPos(UI.CharMax_X, UI.CharMax_Y);    /* set position to bottom right */
      if (Selected < Items)        /* another item follows */
      {
        n = '>';                   /* '>' for more */
      }
      else n = '<';                /* last item */
      {
        n = '<';                   /* '<' for less */
      }
      Display_Char(n);             /* display char */
    }

    #ifndef HW_KEYS
      MilliSleep(100);        /* smooth UI */
    #endif


    /*
     *  process user feedback
     */
 
    n = TestKey(0, CHECK_BAT);     /* wait for key */

    #ifdef HW_KEYS
    /* processing for rotary encoder etc. */
    if (n == KEY_SHORT)            /* short key press: select item */
    {
      n = KEY_LONG;                     /* trigger item selection */
    }
    else if (n == KEY_RIGHT)       /* rotary encoder: right turn */
    {
      n = KEY_SHORT;                    /* trigger next item */
    }
    else if (n == KEY_LEFT)        /* rotary encoder: left turn */
    {
      if (Selected == 0)                /* first item */
      {
        Selected = Items;               /* roll over to last item */

        if (Items >= Lines)             /* large list */
        {
          First = Items - Lines + 1;    /* update first item listed */
          Run++;                        /* set flag for changed list */
        }
      }
      else                              /* not first item */
      {
        Selected--;                     /* move to previous item */

        if (Selected == First)          /* item would be the first one listed */
        {
          if (Selected > 0)             /* more items in list */
          {
            First--;                    /* scroll one item down */
            Run++;                      /* set flag for changed list */
          }
        }
      }
    }
    #endif

    /* processing for test key */
    if (n == KEY_SHORT)            /* short key press: move to next item */
    {
      if (Selected == Items)       /* last item */
      {
        Selected = 0;              /* roll over to first one */
        First = 0;                 /* also reset first item listed */

        if (Items >= Lines)        /* large list */
        {
          Run++;                   /* set flag for changed list */
        }
      }
      else                         /* more items follow */
      {
        Selected++;                /* move to next item */        

        n = First + Lines - 1;     /* last item on screen */
        if (Selected == n)         /* item would be the last one listed */
        {
          if (Items > Selected)    /* more items follow in list */
          {
            First++;               /* scroll one item up */
            Run++;                 /* set flag for changed list */
          }
        }
      }
    }
    else if (n == KEY_LONG)        /* long key press: select current item */
    {
      Run = 0;                     /* end loop */
    }
  }

  LCD_Clear();                /* feedback for user */
  MilliSleep(500);            /* smooth UI */

  return Selected;
}



/* ************************************************************************
 *   menus
 * ************************************************************************ */


/*
 *  adjustment menu
 *  - select profile to load or save
 *
 *  requires:
 *  - Mode: storage mode
 *    STORAGE_SAVE      - save profile 
 *    STORAGE_LOAD      - load profile
 *    STORAGE_SHORT     - short menu
 */

void AdjustmentMenu(uint8_t Mode)
{
  #define MENU_ITEMS  3       /* number of menu items */

  uint8_t           n = 0;                   /* item index number / counter */
  void              *Item_Str[MENU_ITEMS];   /* menu item strings */
  uint8_t           Item_ID[MENU_ITEMS];     /* menu item IDs */
  uint8_t           ID;                      /* ID of selected item */

  /*
   *  set up menu
   *  - items are added in display order
   */

  /* profile #1 */
  Item_Str[n] = (void *)Profile1_str;
  Item_ID[n] = 1;
  n++;  

  /* profile #2 */
  Item_Str[n] = (void *)Profile2_str;
  Item_ID[n] = 2;
  n++;

  #ifdef UI_CHOOSE_PROFILE
  if (Mode & STORAGE_SHORT)             /* short menu */
  {
    /* no "exit menu" */
    Mode &= ~STORAGE_SHORT;             /* clear flag */
  }
  else                                  /* normal menu */
  {
  #endif

    /* exit menu */
    Item_Str[n] = (void *)Exit_str;
    Item_ID[n] = 0;
    n++;                                /* add 1 for item #0 */

  #ifdef UI_CHOOSE_PROFILE
  }
  #endif

  /* display storage mode */
  LCD_Clear();                     /* clear display */
  if (Mode == STORAGE_SAVE)        /* write mode */
  {
    #ifdef UI_COLORED_TITLES
      Display_ColoredEEString(Save_str, COLOR_TITLE);  /* display: Save */
    #else
      Display_EEString(Save_str);                      /* display: Save */
    #endif
  }
  else                             /* read mode */
  {
    #ifdef UI_COLORED_TITLES
      Display_ColoredEEString(Load_str, COLOR_TITLE);  /* display: Load */
    #else
      Display_EEString(Load_str);                      /* display: Load */
    #endif
  }

  /* run menu */
  ID = MenuTool(n, 1, Item_Str, NULL);       /* menu dialog */
  ID = Item_ID[ID];                          /* get item ID */

  if (ID > 0)                 /* valid profile ID */
  {
    /* load/save selected profile */
    ManageAdjustmentStorage(Mode, ID);
  }

  #undef MENU_ITEMS
}


/*
 *  local constants for main menu
 */

/* menu item IDs */
#define MENUITEM_EXIT              0
#define MENUITEM_SELFTEST          1
#define MENUITEM_ADJUSTMENT        2
#define MENUITEM_SAVE              3
#define MENUITEM_LOAD              4
#define MENUITEM_SHOW              5
#define MENUITEM_PWM_TOOL          6
#define MENUITEM_SQUAREWAVE        7
#define MENUITEM_ZENER             8
#define MENUITEM_ESR               9
#define MENUITEM_FREQ_COUNTER     10
#define MENUITEM_ENCODER          11
#define MENUITEM_CONTRAST         12
#define MENUITEM_IR_RECEIVER      13
#define MENUITEM_OPTO_COUPLER     14
#define MENUITEM_SERVO            15
#define MENUITEM_TOUCH            16
#define MENUITEM_IR_TRANSMITTER   17
#define MENUITEM_DS18B20          18
#define MENUITEM_CAP_LEAKAGE      19
#define MENUITEM_POWER_OFF        20
#define MENUITEM_EVENT_COUNTER    21
#define MENUITEM_MONITOR_R        22
#define MENUITEM_MONITOR_C        23
#define MENUITEM_DHTXX            24
#define MENUITEM_ONEWIRE_SCAN     25
#define MENUITEM_FONT_TEST        26
#define MENUITEM_MONITOR_L        27
#define MENUITEM_MONITOR_RCL      28
#define MENUITEM_MONITOR_RL       29
#define MENUITEM_LC_METER         30


/*
 *  create main menu and return ID of selected item
 *
 *  returns:
 *  - ID of selected menu item
 */

uint8_t PresentMainMenu(void)
{
  /* 
   *  local constants for calculating the number of menu items
   */

  #define ITEMS_0        6         /* basic items */

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    #define ITEM_01      1
  #else
    #define ITEM_01      0
  #endif

  #ifdef SW_SQUAREWAVE
    #define ITEM_02      1
  #else
    #define ITEM_02      0
  #endif

  #ifdef HW_ZENER
    #define ITEM_03      1
  #else
    #define ITEM_03      0
  #endif

  #ifdef SW_ESR_TOOL
    #define ITEM_04      1
  #else
    #define ITEM_04      0
  #endif

  #ifdef HW_FREQ_COUNTER
    #define ITEM_05      1
  #else
    #define ITEM_05      0
  #endif

  #ifdef SW_ENCODER
    #define ITEM_06      1
  #else
    #define ITEM_06      0
  #endif

  #ifdef SW_CONTRAST
    #define ITEM_07      1
  #else
    #define ITEM_07      0
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    #define ITEM_08      1
  #else
    #define ITEM_08      0
  #endif

  #ifdef SW_OPTO_COUPLER
    #define ITEM_09      1
  #else
    #define ITEM_09      0
  #endif

  #ifdef SW_SERVO
    #define ITEM_10      1
  #else
    #define ITEM_10      0
  #endif

  #ifdef HW_TOUCH
    #define ITEM_11      1
  #else
    #define ITEM_11      0
  #endif

  #ifdef SW_IR_TRANSMITTER
    #define ITEM_12      1
  #else
    #define ITEM_12      0
  #endif

  #ifdef SW_DS18B20
    #define ITEM_13      1
  #else
    #define ITEM_13      0
  #endif

  #ifdef SW_CAP_LEAKAGE
    #define ITEM_14      1
  #else
    #define ITEM_14      0
  #endif

  #ifdef SW_POWER_OFF
    #define ITEM_15      1
  #else
    #define ITEM_15      0
  #endif

  #ifdef HW_EVENT_COUNTER
    #define ITEM_16      1
  #else
    #define ITEM_16      0
  #endif

  #ifdef SW_MONITOR_R
    #define ITEM_17      1
  #else
    #define ITEM_17      0
  #endif

  #ifdef SW_MONITOR_C
    #define ITEM_18      1
  #else
    #define ITEM_18      0
  #endif

  #ifdef SW_DHTXX
    #define ITEM_19      1
  #else
    #define ITEM_19      0
  #endif

  #ifdef SW_ONEWIRE_SCAN
    #define ITEM_20      1
  #else
    #define ITEM_20      0
  #endif

  #ifdef SW_FONT_TEST
    #define ITEM_21      1
  #else
    #define ITEM_21      0
  #endif

  #ifdef SW_MONITOR_L
    #define ITEM_22      1
  #else
    #define ITEM_22      0
  #endif

  #ifdef SW_MONITOR_RCL
    #define ITEM_23      1
  #else
    #define ITEM_23      0
  #endif 

  #ifdef SW_MONITOR_RL
    #define ITEM_24      1
  #else
    #define ITEM_24      0
  #endif

  #ifdef HW_LC_METER
    #define ITEM_25      1
  #else
    #define ITEM_25      0
  #endif

  #define ITEMS_1        (ITEM_01 + ITEM_02 + ITEM_03 + ITEM_04 + ITEM_05 + ITEM_06 + ITEM_07 + ITEM_08 + ITEM_09 + ITEM_10)
  #define ITEMS_2        (ITEM_11 + ITEM_12 + ITEM_13 + ITEM_14 + ITEM_15 + ITEM_16 + ITEM_17 + ITEM_18 + ITEM_19 + ITEM_20)
  #define ITEMS_3        (ITEM_21 + ITEM_22 + ITEM_23 + ITEM_24 + ITEM_25)

  /* number of menu items */
  #define MENU_ITEMS     (ITEMS_0 + ITEMS_1 + ITEMS_2 + ITEMS_3)


  /*
   *  local variables
   */

  uint8_t           n = 0;                   /* item index number / counter */
  uint8_t           ID;                      /* ID of selected item */
  void              *Item_Str[MENU_ITEMS];   /* menu item strings */
  uint8_t           Item_ID[MENU_ITEMS];     /* menu item IDs */


  /*
   *  set up menu
   *  - items are added in display order
   */

  /*
   *  test/check/signal features
   */

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
  /* PWM tool */
  Item_Str[n] = (void *)PWM_str;
  Item_ID[n] = MENUITEM_PWM_TOOL;
  n++;
  #endif

  #ifdef SW_SQUAREWAVE
  /* Square Wave Signal Generator */
  Item_Str[n] = (void *)SquareWave_str;
  Item_ID[n] = MENUITEM_SQUAREWAVE;
  n++;
  #endif

  #ifdef HW_ZENER
  /* Zener tool */
  Item_Str[n] = (void *)Zener_str;
  Item_ID[n] = MENUITEM_ZENER;  
  n++;
  #endif

  #ifdef SW_ESR_TOOL
  /* in-circuit ESR */
  Item_Str[n] = (void *)ESR_str;
  Item_ID[n] = MENUITEM_ESR;
  n++;
  #endif

  #ifdef SW_CAP_LEAKAGE
  /* cap leakage */
  Item_Str[n] = (void *)CapLeak_str;
  Item_ID[n] = MENUITEM_CAP_LEAKAGE;
  n++;
  #endif

  #ifdef SW_MONITOR_R
  /* monitor R */
  Item_Str[n] = (void *)Monitor_R_str;
  Item_ID[n] = MENUITEM_MONITOR_R;
  n++;
  #endif

  #ifdef SW_MONITOR_C
  /* monitor C */
  Item_Str[n] = (void *)Monitor_C_str;
  Item_ID[n] = MENUITEM_MONITOR_C;
  n++;
  #endif

  #ifdef SW_MONITOR_L
  /* monitor L */
  Item_Str[n] = (void *)Monitor_L_str;
  Item_ID[n] = MENUITEM_MONITOR_L;
  n++;
  #endif

  #ifdef SW_MONITOR_RCL
  /* monitor R/C/L */
  Item_Str[n] = (void *)Monitor_RCL_str;
  Item_ID[n] = MENUITEM_MONITOR_RCL;
  n++;
  #endif

  #ifdef SW_MONITOR_RL
  /* monitor R/L */
  Item_Str[n] = (void *)Monitor_RL_str;
  Item_ID[n] = MENUITEM_MONITOR_RL;
  n++;
  #endif

  #ifdef HW_LC_METER
  /* LC meter */
  Item_Str[n] = (void *)LC_Meter_str;
  Item_ID[n] = MENUITEM_LC_METER;
  n++;
  #endif

  #ifdef HW_FREQ_COUNTER
  /* frequency counter */
  Item_Str[n] = (void *)FreqCounter_str;
  Item_ID[n] = MENUITEM_FREQ_COUNTER;
  n++;
  #endif

  #ifdef HW_EVENT_COUNTER
  /* event counter */
  Item_Str[n] = (void *)EventCounter_str;
  Item_ID[n] = MENUITEM_EVENT_COUNTER;
  n++;
  #endif

  #ifdef SW_ENCODER
  /* rotary encoder check */
  Item_Str[n] = (void *)Encoder_str;
  Item_ID[n] = MENUITEM_ENCODER;
  n++;
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
  /* IR RC detection */
  Item_Str[n] = (void *)IR_Detector_str;
  Item_ID[n] = MENUITEM_IR_RECEIVER;
  n++;
  #endif

  #ifdef SW_IR_TRANSMITTER
  /* IR RC transmitter */
  Item_Str[n] = (void *)IR_Transmitter_str;
  Item_ID[n] = MENUITEM_IR_TRANSMITTER;
  n++;
  #endif

  #ifdef SW_OPTO_COUPLER
  /* opto coupler tool */
  Item_Str[n] = (void *)OptoCoupler_str;
  Item_ID[n] = MENUITEM_OPTO_COUPLER;
  n++;
  #endif

  #ifdef SW_SERVO
  /* servo check */
  Item_Str[n] = (void *)Servo_str;
  Item_ID[n] = MENUITEM_SERVO;
  n++;
  #endif

  #ifdef SW_ONEWIRE_SCAN
  /* OneWire scan tool */
  Item_Str[n] = (void *)OneWire_Scan_str;
  Item_ID[n] = MENUITEM_ONEWIRE_SCAN;
  n++;
  #endif

  #ifdef SW_DS18B20
  /* DS18B20 sensor */
  Item_Str[n] = (void *)DS18B20_str;
  Item_ID[n] = MENUITEM_DS18B20;
  n++;
  #endif

  #ifdef SW_DHTXX
  /* DHT11/DHT22 sensor */
  Item_Str[n] = (void *)DHTxx_str;
  Item_ID[n] = MENUITEM_DHTXX;
  n++;
  #endif

  /*
   *  tester management and settings
   */

  /* selftest */
  Item_Str[n] = (void *)Selftest_str;
  Item_ID[n] = MENUITEM_SELFTEST;
  n++;

  /* self-adjustment */
  Item_Str[n] = (void *)Adjustment_str;
  Item_ID[n] = MENUITEM_ADJUSTMENT;
  n++;

  #ifdef SW_CONTRAST
  /* LCD contrast */
  Item_Str[n] = (void *)Contrast_str;
  Item_ID[n] = MENUITEM_CONTRAST;
  n++;
  #endif

  #ifdef HW_TOUCH
  /* touch screen adjustment */
  Item_Str[n] = (void *)TouchSetup_str;
  Item_ID[n] = MENUITEM_TOUCH;
  n++;
  #endif

  /* save self-adjustment values */
  Item_Str[n] = (void *)Save_str;
  Item_ID[n] = MENUITEM_SAVE;
  n++;

  /* load self-adjustment values */
  Item_Str[n] = (void *)Load_str;
  Item_ID[n] = MENUITEM_LOAD;
  n++;

  /* show self-adjustment values */
  Item_Str[n] = (void *)Show_str;
  Item_ID[n] = MENUITEM_SHOW;
  n++;

  #ifdef SW_FONT_TEST
  /* font test */
  Item_Str[n] = (void *)FontTest_str;
  Item_ID[n] = MENUITEM_FONT_TEST;
  n++;
  #endif

  #ifdef SW_POWER_OFF
  /* power off tester */
  Item_Str[n] = (void *)PowerOff_str;
  Item_ID[n] = MENUITEM_POWER_OFF;
  n++;
  #endif

  /* exit menu */
  Item_Str[n] = (void *)Exit_str;
  Item_ID[n] = MENUITEM_EXIT;
  n++;                                  /* add 1 for item #0 */


  /*
   *  run menu
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display "Select" */
    Display_ColoredEEString(Select_str, COLOR_TITLE);
  #else
    Display_EEString(Select_str);       /* display "Select" */
  #endif

  ID = MenuTool(n, 1, Item_Str, NULL);  /* menu dialog */
  ID = Item_ID[ID];                     /* get item ID */

  /* clean up */
  #undef MENU_ITEMS
  #undef ITEMS_0
  #undef ITEMS_1
  #undef ITEMS_2
  #undef ITEMS_3

  #undef ITEM_01
  #undef ITEM_02
  #undef ITEM_03
  #undef ITEM_04
  #undef ITEM_05
  #undef ITEM_06
  #undef ITEM_07
  #undef ITEM_08
  #undef ITEM_09
  #undef ITEM_10
  #undef ITEM_11
  #undef ITEM_12
  #undef ITEM_13
  #undef ITEM_14
  #undef ITEM_15
  #undef ITEM_16
  #undef ITEM_17
  #undef ITEM_18
  #undef ITEM_19
  #undef ITEM_20
  #undef ITEM_21
  #undef ITEM_22
  #undef ITEM_23
  #undef ITEM_24
  #undef ITEM_25

  return(ID);                 /* return item ID */
}



/*
 *  main menu
 */

void MainMenu(void)
{
  uint8_t           ID;                 /* ID of selected item */
  uint8_t           Flag = 1;           /* feedback flag */
  #ifdef SW_PWM_SIMPLE
  uint16_t          Frequency;          /* PWM frequency */  
  #endif

  ID = PresentMainMenu();     /* create menu and get user feedback */

  /* run selected item */
  switch (ID)
  {
    /* exit menu */
    /* case MENUITEM_EXIT: */

    /* self-test */
    case MENUITEM_SELFTEST:
      Flag = SelfTest();
      break;

    /* self-adjustment */
    case MENUITEM_ADJUSTMENT:
      Flag = SelfAdjustment();
      break;

    /* save adjustment values */
    case MENUITEM_SAVE:
      AdjustmentMenu(STORAGE_SAVE);
      break;

    /* load adjustment values */
    case MENUITEM_LOAD:
      AdjustmentMenu(STORAGE_LOAD);
      break;

    /* show basic adjustment values */
    case MENUITEM_SHOW:
      ShowAdjustmentValues();
      break;

    #ifdef SW_PWM_SIMPLE
    /* PWM tool with simple UI */
    case MENUITEM_PWM_TOOL:
      /* run PWM menu */
      LCD_Clear();
      #ifdef UI_COLORED_TITLES
        /* display: PWM */
        Display_ColoredEEString(PWM_str, COLOR_TITLE);
      #else
        Display_EEString(PWM_str);           /* display: PWM */
      #endif
      ID = MenuTool(NUM_PWM_FREQ, 2, (void *)PWM_Freq_table, (unsigned char *)Hertz_str);
      Frequency = DATA_read_word(&PWM_Freq_table[ID]);      /* get selected frequency */
      PWM_Tool(Frequency);                                  /* and run PWM tool */
      break;
    #endif

    #ifdef SW_PWM_PLUS
    /* PWM tool with improved UI */
    case MENUITEM_PWM_TOOL:
      PWM_Tool();
      break;
    #endif

    #ifdef SW_SQUAREWAVE
    /* square wave signal generator */
    case MENUITEM_SQUAREWAVE:
      SquareWave_SignalGenerator();
      break;
    #endif

    #ifdef HW_ZENER
    /* Zener tool */
    case MENUITEM_ZENER:
      Zener_Tool();
      break;
    #endif

    #ifdef SW_ESR_TOOL
    /* ESR tool */
    case MENUITEM_ESR:
      ESR_Tool();
      break;
    #endif

    #ifdef HW_FREQ_COUNTER
    /* frequency counter */
    case MENUITEM_FREQ_COUNTER:
      FrequencyCounter();
      break;
    #endif

    #ifdef SW_ENCODER
    /* rotary encoder check */
    case MENUITEM_ENCODER:
      Encoder_Tool();
      break;
    #endif

    #ifdef SW_CONTRAST
    /* change contrast */
    case MENUITEM_CONTRAST:
      ChangeContrast();
      break;
    #endif

    #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    /* IR RC detector/decoder */
    case MENUITEM_IR_RECEIVER:
      IR_Detector();
      break;
    #endif

    #ifdef SW_OPTO_COUPLER
    /* opto coupler tool */
    case MENUITEM_OPTO_COUPLER:
      OptoCoupler_Tool();
      break;
    #endif

    #ifdef SW_SERVO
    /* servo check */
    case MENUITEM_SERVO:
      Servo_Check();
      break;
    #endif

    #ifdef HW_TOUCH
    /* touch screen adjustment */
    case MENUITEM_TOUCH:
      Flag = Touch_Adjust();
      break;
    #endif

    #ifdef SW_IR_TRANSMITTER
    /* IR RC transmitter */
    case MENUITEM_IR_TRANSMITTER:
      IR_RemoteControl();
      break;
    #endif

    #ifdef SW_DS18B20
    /* DS18B20 sensor */
    case MENUITEM_DS18B20:
      Flag = DS18B20_Tool();
      break;
    #endif

    #ifdef SW_CAP_LEAKAGE
    /* cap leakage */
    case MENUITEM_CAP_LEAKAGE:
      Cap_Leakage();
      break;
    #endif

    #ifdef SW_POWER_OFF
    /* power off */
    case MENUITEM_POWER_OFF:
      PowerOff();
      break;
    #endif

    #ifdef HW_EVENT_COUNTER
    /* event counter */
    case MENUITEM_EVENT_COUNTER:
      EventCounter();
      break;
    #endif

    #ifdef SW_MONITOR_R
    /* monitor R */
    case MENUITEM_MONITOR_R:
      Monitor_R();
      break;
    #endif

    #ifdef SW_MONITOR_C
    /* monitor C */
    case MENUITEM_MONITOR_C:
      Monitor_C();
      break;
    #endif

    #ifdef SW_DHTXX
    /* DHT11/DHT22 sensor */
    case MENUITEM_DHTXX:
      Flag = DHTxx_Tool();
      break;
    #endif

    #ifdef SW_ONEWIRE_SCAN
    /* OneWire scan tool */
    case MENUITEM_ONEWIRE_SCAN:
      Flag = OneWire_Scan_Tool();
      break;
    #endif

    #ifdef SW_FONT_TEST
    /* font test */
    case MENUITEM_FONT_TEST:
      FontTest();
      break;
    #endif

    #ifdef SW_MONITOR_L
    /* monitor L */
    case MENUITEM_MONITOR_L:
      Monitor_L();
      break;
    #endif

    #ifdef SW_MONITOR_RCL
    /* monitor R/C/L */
    case MENUITEM_MONITOR_RCL:
      Monitor_RCL();
      break;
    #endif

    #ifdef SW_MONITOR_RL
    /* monitor R/L */
    case MENUITEM_MONITOR_RL:
      Monitor_RL();
      break;
    #endif

    #ifdef HW_LC_METER
    /* LC meter */
    case MENUITEM_LC_METER:
      Flag = LC_Meter();
      break;
    #endif
  }

  /* display result */
  LCD_Clear();                     /* clear display */
  if (Flag == 0)
    Display_EEString(Error_str);   /* display: error! */
  else
    Display_EEString(Done_str);    /* display: done! */
}


/*
 *  clean up local constants for main menu
 */

/* menu item IDs */
#undef MENUITEM_EXIT
#undef MENUITEM_SELFTEST
#undef MENUITEM_ADJUSTMENT
#undef MENUITEM_SAVE
#undef MENUITEM_LOAD
#undef MENUITEM_SHOW
#undef MENUITEM_PWM_TOOL
#undef MENUITEM_SQUAREWAVE
#undef MENUITEM_ZENER
#undef MENUITEM_ESR
#undef MENUITEM_FREQ_COUNTER
#undef MENUITEM_ENCODER
#undef MENUITEM_CONTRAST
#undef MENUITEM_IR_RECEIVER
#undef MENUITEM_OPTO_COUPLER
#undef MENUITEM_SERVO
#undef MENUITEM_TOUCH
#undef MENUITEM_IR_TRANSMITTER
#undef MENUITEM_DS18B20
#undef MENUITEM_CAP_LEAKAGE
#undef MENUITEM_POWER_OFF
#undef MENUITEM_EVENT_COUNTER
#undef MENUITEM_MONITOR_R
#undef MENUITEM_MONITOR_C
#undef MENUITEM_DHTXX
#undef MENUITEM_ONEWIRE_SCAN
#undef MENUITEM_FONT_TEST
#undef MENUITEM_MONITOR_L
#undef MENUITEM_MONITOR_RCL
#undef MENUITEM_LC_METER



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef USER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

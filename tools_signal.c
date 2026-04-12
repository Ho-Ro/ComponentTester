/* ************************************************************************
 *
 *   signal tools (hardware and software options)
 *
 *   (c) 2012-2024 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define TOOLS_SIGNAL_C



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
 *  local variables
 */

/* sweep function for servo tester */
#ifdef SW_SERVO
uint8_t                  SweepStep;     /* sweep step */
volatile uint8_t         SweepDir;      /* sweep direction */
#endif



/* ************************************************************************
 *   PWM: simple PWM generator
 * ************************************************************************ */


#ifdef SW_PWM_SIMPLE

/*
 *  PWM generator with simple UI
 *  - uses probe #2 (OC1B) as PWM output
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - max. reasonable PWM frequency for 8MHz MCU clock is 40kHz
 *  - requires idle sleep mode to keep timer running when MCU is sleeping
 *
 *  requires:
 *  - Freqency in Hz
 */

void PWM_Tool(uint16_t Frequency)
{
  uint8_t           Test = 1;           /* loop control and user feedback */
  uint8_t           Ratio;              /* PWM ratio (in %) */
  uint8_t           Bits;               /* bits for timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Toggle;             /* counter value to toggle output */
  uint32_t          Value;              /* temporary value */
  #ifdef PWM_SHOW_DURATION
  uint16_t          Time;               /* duration/resolution of timer step */
  #endif

  /*
   *  Timer1:
   *  - phase correct PWM:    f_PWM = f_MCU / (2 * prescaler * top)
   *  - available prescalers: 1, 8, 64, 256, 1024
   *  - range of top:         (2^2 - 1) up to (2^16 - 1)
   *
   *  - ranges for a 8MHz MCU clock:
   *    prescaler  /2pre     top 2^16   top 2^2    top 100
   *    1          4MHz      61Hz       1MHz       40kHz
   *    8          500kHz    7.6Hz      125kHz     5kHz
   *    64         62.5kHz   0.95Hz     15.625kHz  625Hz
   *    256        15625Hz   0.24Hz     3906.25Hz  156.25Hz
   *  - to support a PWM ratio of 1% top should be at least 100
   */

  ShortCircuit(0);                      /* make sure probes are not shorted */

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: PWM */
    Display_ColoredEEString_Space(PWM_str, COLOR_TITLE);
  #else
    Display_EEString_Space(PWM_str);    /* display: PWM */
  #endif
  Display_Value2(Frequency);            /* display frequency */
  Display_EEString(Hertz_str);          /* display: Hz */
  #ifndef HW_FIXED_SIGNAL_OUTPUT
  ProbePinout(PROBES_PWM);              /* show probes used */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* set up probes: #1 and #3 are signal ground, #2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* dedicated output via OC1B */
  SIGNAL_PORT &= ~(1 << SIGNAL_OUT);    /* low by default */
  SIGNAL_DDR |= (1 << SIGNAL_OUT);      /* enable output */
  #endif

  /*
   *  calculate required prescaler and top value based on MCU clock
   *  - top = f_MCU / (2 * prescaler * f_PWM)
   */

  Value = CPU_FREQ / 2;                 /* /2 */
  Value /= Frequency;                   /* /f_PWM */

  if (Value > 2000000)        /* low frequency (<20Hz @8MHz) */
  {
    #ifdef PWM_SHOW_DURATION
      Toggle = 256;                     /* prescaler 256 */
    #else
      Value /= 256;                     /* /prescaler */
    #endif
    Bits = (1 << CS12);                 /* prescaler bits for 256:1 */
  }
  else if (Value > 16000)     /* mid-range frequency (<250Hz @8MHz) */
  {
    #ifdef PWM_SHOW_DURATION
      Toggle = 64;                      /* prescaler 64 */
    #else
      Value /= 64;                      /* /prescaler */
    #endif
    Bits = (1 << CS11) | (1 << CS10);   /* prescaler bits for 64:1 */
  }
  else                        /* high frequency */
  {
    #ifdef PWM_SHOW_DURATION
      Toggle = 1;                       /* prescaler 1 */
    #endif
    Bits = (1 << CS10);                 /* prescaler bits for 1:1 */
  }

  #ifdef PWM_SHOW_DURATION
  Value /= Toggle;                 /* /prescaler */
  #endif
  Top = (uint16_t)Value;           /* keep lower 16 bits */

  #ifdef PWM_SHOW_DURATION
  /* calculate duration of timer step */
  /* t = (1 / f_MCU) * 2 * prescaler = 2 * prescaler / f_MCU */
  Value = 2000000000 / CPU_FREQ;        /* 2/f_MCU in ns */
  Value *= Toggle;                      /* * prescaler */
  Time = (uint16_t)Value;               /* keep lower 16 bits */
  #endif

  /* set start values */
  Ratio = 50;                 /* default ratio is 50% */
  #if 0
  /* calculate toggle value: top * (ratio / 100) */
  Value = (uint32_t)Top * Ratio;
  Value /= 100;
  Toggle = (uint16_t)Value;
  #endif
  Toggle = Top / 2;           /* compare value for 50% */


  /*
   *  set up Timer1 for PWM
   *  - phase correct PWM
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  TCCR1B = 0;                                /* stop timer */
  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);
  TCNT1 = 0;                                 /* set counter to 0 */
  OCR1A = Top;                               /* set top value (-1) */
  OCR1B = Toggle;                            /* set value to compare with */

  /* start counter by setting clock prescaler */
  TCCR1B = (1 << WGM13) | Bits;


  /*
   *  ratio control
   */

  while (Test > 0)
  {
    /* show PWM ratio in line #2 */
    LCD_ClearLine2();
    Display_Value(Ratio, 0, '%');       /* show ratio in % */
    #ifdef PWM_SHOW_DURATION
    /* and also pulse duration */
    Display_Space();
    /* pulse duration = duration of timer step * toggle value */ 
    Value = (uint32_t)Time * OCR1B;          /* in ns */
    Display_Value(Value, -9, 's');
    #endif

    #ifdef HW_KEYS
    if (Test <= KEY_LONG)               /* just for test button usage */
    #endif
    MilliSleep(500);                    /* smooth UI */

    /*
     *  user interface
     *  - short key press -> increase ratio
     *    long key press -> decrease ratio
     *    two short key presses -> exit tool
     */

    /* wait for user feedback */
    Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_SHORT)              /* short key press */
    {
      if (Ratio <= 95) Ratio += 5;      /* +5% and limit to 100% */
    }
    else if (Test == KEY_TWICE)         /* two short key presses */
    {
      Test = 0;                         /* end loop */
    }
    #ifdef HW_KEYS
    else if (Test == KEY_RIGHT)         /* right key */
    {
      if (Ratio <= 99) Ratio += 1;      /* +1% and limit to 100% */
    }
    else if (Test == KEY_LEFT)          /* left key */
    {
      if (Ratio >= 1) Ratio -= 1;       /* -1% and limit to 0% */
    }
    #endif
    else                                /* long key press */
    {
      if (Ratio >= 5) Ratio -= 5;       /* -5% and limit to 0% */
    }

    /* calculate toggle value: top * (ratio / 100) */
    Value = (uint32_t)Top * Ratio;
    Value /= 100;
    OCR1B = (uint16_t)Value;            /* update compare value */
  }


  /*
   *  clean up
   */

  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  R_DDR = 0;                            /* set HiZ mode */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  SIGNAL_DDR &= ~(1 << SIGNAL_OUT);     /* set HiZ mode */
  #endif
}

#endif



/* ************************************************************************
 *   PWM: extended PWM generator
 * ************************************************************************ */


#ifdef SW_PWM_PLUS

/*
 *  PWM generator with improved UI
 *  - uses probe #2 (OC1B) as PWM output
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - max. reasonable PWM frequency for 8MHz MCU clock is 40kHz
 *  - requires additional keys (e.g. rotary encoder) and
 *    display with more than 2 text lines
 *  - requires idle sleep mode to keep timer running when MCU is sleeping
 */

void PWM_Tool(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Mode;               /* UI */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Step;               /* step size */
  uint8_t           Ratio;              /* PWM ratio (in %) */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bits = 0;           /* prescaler register bits */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Step2;              /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */
  #ifdef PWM_SHOW_DURATION
  uint16_t          TimeValue = 0;      /* duration/resolution of timer step */
  int8_t            TimeScale = 0;      /* scale of duration */
  #endif

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG       0b00000001     /* run / otherwise end */
  #define CHANGE_FREQ    0b00000010     /* change frequency */
  #define CHANGE_RATIO   0b00000100     /* change ratio */
  #define DISPLAY_FREQ   0b00001000     /* display frequency */
  #define DISPLAY_RATIO  0b00010000     /* display ratio */

  /* local constants for Mode */
  #define MODE_FREQ               1     /* frequency mode */
  #define MODE_RATIO              2     /* ratio mode */


  /*
   *  Timer1:
   *  - phase & frequency correct PWM:  f_PWM = f_MCU / (2 * prescaler * top)
   *  - available prescalers:           1, 8, 64, 256, 1024
   *  - range of top:                   (2^2 - 1) up to (2^16 - 1)
   *  - ranges for a 8MHz MCU clock:
   *    prescaler  /2pre       top 2^16   top 2^2    top 100
   *    1          4MHz        61Hz       1MHz       40kHz
   *    8          500kHz      7.6Hz      125kHz     5kHz
   *    64         62.5kHz     0.95Hz     15.625kHz  625Hz
   *    256        15625Hz     0.24Hz     3906.25Hz  156.25Hz
   *    1024       3906.25Hz   0.06Hz     976.5Hz    39Hz
   *  - to support a PWM ratio of 1% top should be at least 100
   */

  ShortCircuit(0);                    /* make sure probes are not shorted */

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: PWM */
    Display_ColoredEEString_Space(PWM_str, COLOR_TITLE);
  #else
    Display_EEString_Space(PWM_str);    /* display: PWM */
  #endif
  #ifndef HW_FIXED_SIGNAL_OUTPUT
  ProbePinout(PROBES_PWM);              /* show probes used */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* set up probes: #1 and #3 are signal ground, #2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* dedicated output via OC1B */
  SIGNAL_PORT &= ~(1 << SIGNAL_OUT);    /* low by default */
  SIGNAL_DDR |= (1 << SIGNAL_OUT);      /* enable output */
  #endif


  /*
   *  set up Timer1 for PWM
   *  - phase and frequency correct PWM
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  TCNT1 = 0;                            /* set counter to 0 */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);


  /*
   *  processing loop
   */

  /* start values */
  /* top = f_MCU / (2 * prescaler * f_PWM) */
  Top = (CPU_FREQ / 2000);         /* 1kHz */
  Ratio = 50;                      /* 50% PWM ratio */
  Prescaler = 1;                   /* prescaler 1:1 */
  Index = 0;                       /* first index of prescaler tables */
  Bits = (1 << CS10);              /* register bits for prescaler 1:1 */
  Flag = RUN_FLAG | CHANGE_FREQ | CHANGE_RATIO | DISPLAY_FREQ | DISPLAY_RATIO;
  Mode = MODE_FREQ;                /* frequency mode */

  while (Flag > 0)       /* processing loop */
  {
    /*
     *  change timer settings
     */

    if (Flag & CHANGE_FREQ)        /* change frequency */
    {
      /*
       *  auto-ranging
       */

      Step = Index;                /* save old index */

      /* check if we should change the range */
      if (Top > 0x7FFF)            /* more than 15 bits */
      {
        if (Index < (NUM_TIMER1 - 1))   /* don't exceed upper prescaler limit */
        {
          Index++;                 /* increase prescaler */
        }
      }
      else if (Top < 0x0FFF)       /* less than 12 bits */
      {
        if (Index > 0)             /* don't exceed lower prescaler limit */
        {
          Index--;                 /* decrease prescaler */
        }
      }

      /* process changed range */
      if (Index != Step)           /* range has changed */
      {
        Step2 = Prescaler;         /* save old value */

        /* read new prescaler and register bits from table */
        Prescaler = DATA_read_word(&T1_Prescaler_table[Index]);
        Bits = DATA_read_byte(&T1_RegBits_table[Index]);

        if (Index > Step)          /* larger prescaler */
        {
          /* decrease top value by same factor as the prescaler increased */
          Temp = Prescaler / Step2;
          Top /= Temp;
        }
        else                       /* smaller prescaler */
        {
          /* increase top value by same factor as the prescaler decreased */
          Temp = Step2 / Prescaler;
          Top *= Temp;
        }
      }

      /* set frequency */
      OCR1A = Top;                      /* set top value */
      TCCR1B = (1 << WGM13) | Bits;     /* (re)start timer */

      #ifdef PWM_SHOW_DURATION
      /* calculate duration of timer step */
      TimeScale = -9;                        /* ns */
      /* t = (1 / f_MCU) * 2 * prescaler = 2 * prescaler / f_MCU */
      Value = 2000000000 / CPU_FREQ;         /* 2/f_MCU in ns */
      Value *= Prescaler;                    /* * prescaler */
      while (Value > UINT16_MAX)             /* rescale */
      {
        Value /= 1000;                       /* /1000 */
        TimeScale += 3;                      /* 10^3 */
      } 
      TimeValue = (uint16_t)Value;           /* keep lower 16 bits */
      #endif

      Flag &= ~CHANGE_FREQ;             /* clear flag */
      /* a frequency change implies a ratio change */
    }


    if (Flag & CHANGE_RATIO)       /* change ratio */
    {
      /* toggle = top * (ratio / 100) */
      Value = (uint32_t)Top * Ratio;
      Value /= 100;
      OCR1B = (uint16_t)Value;          /* set compare/toggle value */

      #ifdef PWM_SHOW_DURATION
      /* also update the display of the pulse duration */
      Flag |= DISPLAY_RATIO;            /* display ratio */
      #endif

      Flag &= ~CHANGE_RATIO;            /* clear flag */
    }


    /*
     *  update display
     */

    if (Flag & DISPLAY_FREQ)       /* display frequency in line #2 */
    {
      LCD_ClearLine2();
      MarkItem(MODE_FREQ, Mode);        /* mark mode if selected */

      /* f_PWM = f_MCU / (2 * prescaler * top) */
      Value = CPU_FREQ * 50;            /* scale to 0.01Hz and /2 */
      Value /= Prescaler;               /* /prescaler */
      Step = 2;                         /* 2 decimal places */

      /*
       *  optimize resolution of frequency without causing an overflow
       *  prescaler       :  1  8  64  256  1024
       *  decimal places  :  2  3   4    4     5
       */

      Temp = Prescaler;
      while (Temp >= 8)         /* loop through prescaler steps */
      {
        Value *= 10;            /* scale by factor 0.1 */
        Step++;                 /* one decimal place more */
        Temp /= 8;              /* next lower prescaler */
      }

      Value /= Top;             /* /top */

      Display_FullValue(Value, Step, 0);  /* display frequency */
      Display_EEString(Hertz_str);        /* display: Hz */

      Flag &= ~DISPLAY_FREQ;            /* clear flag */
    }


    if (Flag & DISPLAY_RATIO)      /* display ratio in line #3 */
    {
      LCD_ClearLine3();                 /* clear line #3 */
      MarkItem(MODE_RATIO, Mode);       /* mark mode if selected */

      Display_Value(Ratio, 0, '%');     /* show ratio in % */

      #ifdef PWM_SHOW_DURATION
      /* and also pulse duration */
      Display_Space();
      /* pulse duration = duration of timer step * toggle value */ 
      Value = (uint32_t)TimeValue * OCR1B;
      Display_Value(Value, TimeScale, 's');
      #endif

      Flag &= ~DISPLAY_RATIO;           /* clear flag */
    }

    /* smooth UI after long key press */
    if (Test == KEY_LONG)          /* long key press */
    {
      SmoothLongKeyPress();             /* delay next key press */
    }


    /*
     *  user feedback
     */

    /* wait for user feedback */
    Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

    /* consider rotary encoder's turning velocity */
    Step = UI.KeyStep;             /* get velocity (1-7) */
    Step2 = Step;
    if (Step > 1)                  /* larger step */
    {
      /* increase step size based on turning velocity */
      if (Mode == MODE_FREQ)       /* frequency mode */
      {
        /*
         *  value ranges for each prescaler:
         *  -    1:  100 -> 32767 /  100 <- 32760
         *  -    8: 4095 -> 32767 / 4095 <- 32760
         *  -   64: 4095 -> 32767 / 4095 <- 16380
         *  -  256: 8191 -> 32767 / 4095 <- 16380
         *  - 1024: 8191 -> 65635 / 4095 <- 65635
         */

        /* step^4: 16 81 256 625 1296 2401 */
        Step2 *= Step;             /* ^2 */
        Step2 *= Step2;            /* ^2 */
      }
      else                         /* ratio mode */
      {
        /* 0-100% */
        Step *= 100 / 32;
      }
    }

    /* process user input */
    if (Test == KEY_SHORT)              /* short key press */
    {
      /* toggle frequency/ratio mode */
      if (Mode == MODE_FREQ)       /* frequency mode */
      {
        Mode = MODE_RATIO;         /* change to ratio mode */
      }
      else                         /* ratio mode */
      {
        Mode = MODE_FREQ;          /* change to frequency mode */
      }

      Flag |= DISPLAY_FREQ | DISPLAY_RATIO;  /* update display */
    }
    else if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    else if (Test == KEY_LONG)          /* long key press */
    {
      if (Mode == MODE_FREQ)       /* frequency mode */
      {
        /* set 1kHz */
        Prescaler = 1;
        Index = 0;
        Bits = (1 << CS10);        /* prescaler register bits for 1 */
        Top = (CPU_FREQ / 2000);   /* 1kHz */
        Flag |= CHANGE_FREQ | DISPLAY_FREQ | CHANGE_RATIO;   /* set flags */
      }
      else                         /* ratio mode */
      {
        /* set 50% */
        Ratio = 50;
        Flag |= CHANGE_RATIO | DISPLAY_RATIO;     /* set flags */
      }
    }
    else if (Test == KEY_RIGHT)    /* right key */
    {
      if (Mode == MODE_FREQ)       /* frequency mode */
      {
        /* increase frequency -> decrease top */
        Temp = Top - Step2;        /* take advantage of underflow */
        if ((Temp > Top) || (Temp < 0x0064))      /* underflow */
        {
          Temp = 0x0064;           /* lower limit */
        }
        Top = Temp;                /* set new value */

        Flag |= CHANGE_FREQ | DISPLAY_FREQ | CHANGE_RATIO;  /* set flags */
      }
      else                         /* ratio mode */
      {
        /* increase ratio */
        Ratio += Step;             /* add (max 200) */
        if (Ratio > 100)           /* limit exceeded */
        {
          Ratio = 100;             /* max. is 100 */
        }

        Flag |= CHANGE_RATIO | DISPLAY_RATIO;     /* set flags */
      }
    }
    else if (Test == KEY_LEFT)     /* left key */
    {
      if (Mode == MODE_FREQ)       /* frequency mode */
      {
        /* decrease frequency -> increase top */
        Temp = Top + Step2;        /* take advantage of overflow */
        if (Temp < Top)            /* overflow */
        {
          Temp = 0xFFFF;           /* upper limit */
        }
        Top = Temp;                /* set new value */

        Flag |= CHANGE_FREQ | DISPLAY_FREQ | CHANGE_RATIO;  /* set flags */
      }
      else                         /* ratio mode */
      {
        /* decrease ratio */
        if (Ratio > Step)          /* no underflow */
        {
          Ratio -= Step;           /* new ratio */
        }
        else                       /* underflow */
        {
          Ratio = 0;               /* lower limit is 0 */
        }

        Flag |= CHANGE_RATIO | DISPLAY_RATIO;     /* set flags */
      }
    }
  }


  /*
   *  clean up
   */

  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  R_DDR = 0;                            /* set HiZ mode */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  SIGNAL_DDR &= ~(1 << SIGNAL_OUT);     /* set HiZ mode */
  #endif

  /* local constants for Mode */
  #undef MODE_RATIO
  #undef MODE_FREQ

  /* local constants for Flag */
  #undef DISPLAY_RATIO
  #undef DISPLAY_FREQ
  #undef CHANGE_RATIO
  #undef CHANGE_FREQ
  #undef RUN_FLAG
}

#endif



/* ************************************************************************
 *   PWM: servo check
 * ************************************************************************ */


#ifdef SW_SERVO

/*
 *  Servo Check, PWM generator for testing servos
 *  - uses probe #2 (OC1B) as PWM output
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - requires additional keys (e.g. rotary encoder) and
 *    display with more than 2 lines
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 */

void Servo_Check(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Mode;               /* UI mode */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Index;              /* PWM index */
  uint8_t           Period[4] = {200, 80, 40, 30};  /* in 0.1ms */
  uint16_t          Toggle;             /* toggle value */
  uint16_t          Step;               /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG       0b00000001     /* run / otherwise end */
  #define SWEEP_MODE     0b00000010     /* sweep mode */
  #define CHANGE_PULSE   0b00000100     /* change pulse width */
  #define CHANGE_FREQ    0b00001000     /* change frequency */
  #define DISPLAY_PULSE  0b00010000     /* display pulse width */
  #define DISPLAY_FREQ   0b00100000     /* display frequency */
  #define TOGGLE_SWEEP   0b01000000     /* enter/leave sweep operation */

  /* local constants for Mode */
  #define MODE_PULSE              1     /* pulse width mode */
  #define MODE_FREQ               2     /* frequency mode */


  /*
   *  MCU clock specific value
   *  - step size for a resolution of about 0.01ms
   *  - 8MHz: 5, 16MHz: 10, 20MHz: 13
   */

  #if (CPU_FREQ >= 16000000)
    #define PULSE_STEP        10
  #elif (CPU_FREQ >= 8000000)
    #define PULSE_STEP        5
  #else
    #define PULSE_STEP        1
  #endif

  /*
   *  PWM for servos:
   *  - frequency
   *    50Hz / 20ms  analog servo
   *    125Hz / 8ms  digital servo
   *    250Hz / 4ms  high speed digital servo
   *    333Hz / 3ms  high speed digital servo
   *  - pulse 1 - 2ms (allow 0.5 - 2.5ms)
   *    left   1.0ms
   *    mid    1.5ms
   *    right  2.0ms
   *  - typical rotation is 90-120� & 180�
   *  - typical speed is 30-500ms/60�
   */

  ShortCircuit(0);                 /* make sure probes are not shorted */

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Servo */
    Display_ColoredEEString_Space(Servo_str, COLOR_TITLE);
  #else
    Display_EEString_Space(Servo_str);  /* display: Servo */
  #endif
  #ifndef HW_FIXED_SIGNAL_OUTPUT
  ProbePinout(PROBES_PWM);              /* show probes used */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* set up probes: #1 and #3 are signal ground, #2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* dedicated output via OC1B */
  SIGNAL_PORT &= ~(1 << SIGNAL_OUT);    /* low by default */
  SIGNAL_DDR |= (1 << SIGNAL_OUT);      /* enable output */
  #endif


  /*
   *  calculate required values for PWM based on MCU clock
   *  - Timer1, phase correct PWM
   *  - top = f_MCU / (2 * prescaler * f_PWM)
   *        = (f_MCU * t_PWM) / (2 * prescaler)
   *  - toggle = (f_MCU * t_pulse) / (2 * prescaler)
   *  - use prescaler 1:8 for best resolution across all MCU clocks
   *    with t_pulse in 0.1ms
   *    = ((f_MCU / 10000) * t_pulse) / 16
   */

  /* PWM toggle values (t_pulse) */
  #define SERVO_LEFT_MAX      (((CPU_FREQ / 10000) * 5) / 16)    /* 0.5ms */
  #define SERVO_LEFT_NORM     (((CPU_FREQ / 10000) * 10) / 16)   /* 1.0ms */
  #define SERVO_MID           (((CPU_FREQ / 10000) * 15) / 16)   /* 1.5ms */
  #define SERVO_RIGHT_NORM    (((CPU_FREQ / 10000) * 20) / 16)   /* 2.0ms */  
  #define SERVO_RIGHT_MAX     (((CPU_FREQ / 10000) * 25) / 16)   /* 2.5ms */

  /* sweep control */
  #define SERVO_STEP_MAX      (SERVO_LEFT_NORM / 10)   /* toggle_1ms / 10 */

  /*
   *  calculate required values for sweep timer
   *  - Timer0, CTC mode
   *  - t_step = 3ms -> f_step = 333Hz
   *  - prescaler = 1024
   *  - top = (f_MCU / (f_step * prescaler)) - 1
   *        = (t_step / (t_MCU_cycle * prescaler)) - 1
   *  - t_step = t_MCU_cycle * prescaler * (top + 1)
   *  - SERVO_STEP_TIME in 탎
   */

  #define SERVO_SWEEP_TOP     (((CPU_FREQ / 333) / 1024) - 1)
  #define SERVO_STEP_TIME     ((MCU_CYCLE_TIME * 1024 * (SERVO_SWEEP_TOP + 1)) / 10000)


  /*
   *  set up Timer0 for sweeping
   *  - CTC mode
   *  - top value by OCR0A (double buffered)
   *  - fixed prescaler 1:1024
   */

  TCCR0B = 0;                      /* disable Timer0 */
  TCNT0 = 0;                       /* reset counter */
  OCR0A = SERVO_SWEEP_TOP;         /* set compare value */
  TCCR0A = (1 << WGM01);           /* set timer mode */
  TIMSK0 = (1 << OCIE0A);          /* enable output compare match A interrupt */


  /*
   *  set up Timer1 for PWM
   *  - phase correct PWM
   *  - top value by OCR1A (buffered)
   *  - OC1B non-inverted output
   *  - fixed prescaler 1:8
   */

  TCNT1 = 0;                       /* reset counter to 0 */
  TIMSK1 = 0;                      /* disable all interrupts for Timer1 */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13) | (1 << CS11);  /* start Timer1 by setting prescaler */

  /* set start values */
  Toggle = SERVO_MID;              /* toggle value (1.5ms) */
  Index = 0;                       /* #0 (20.0ms) */
  SweepStep = 0;                   /* no step */
  SweepDir = 0;                    /* no direction */
  Mode = MODE_PULSE;               /* pulse width mode */
  Flag = RUN_FLAG | MODE_PULSE | CHANGE_PULSE | CHANGE_FREQ | DISPLAY_PULSE | DISPLAY_FREQ;

  /*
   *  todo:
   *  - since the pulse length is displayed with a resolution of 0.01ms
   *    a visible change might need several steps
   *  - improve UI to give visual feedback for each step
   */

  while (Flag > 0)       /* processing loop */
  {
    /*
     *  change timer settings
     */

    /* change pulse width */
    if (Flag & CHANGE_PULSE)
    {
      OCR1B = Toggle;                   /* set toggle value */

      Flag &= ~CHANGE_PULSE;            /* clear flag */
    }

    /* change frequency */
    if (Flag & CHANGE_FREQ)
    {
      /* top = ((f_MCU / 10000) * t_pulse) / 16 */
      Test = Period[Index];             /* get period */
      Value = (CPU_FREQ / 10000);       /* MCU clock in 10kHz */
      Value *= Test;
      Value /= 16; 
      OCR1A = (uint16_t)Value;          /* set top value */

      Flag &= ~CHANGE_FREQ;             /* clear flag */
    }


    /*
     *  update display
     */

    /* display pulse duration / sweep period */
    if (Flag & DISPLAY_PULSE)
    {
      LCD_ClearLine2();                 /* line #2 */
      MarkItem(MODE_PULSE, Mode);       /* mark mode if selected */

      if (Flag & SWEEP_MODE)            /* sweep mode */
      {
        /*
         *  calculate sweep time
         *  - t_sweep = t_step * (toggle_1ms / step)
         */

        Value = SERVO_STEP_TIME;        /* step time in 탎 (around 3000) */
        Value *= SERVO_LEFT_NORM;       /* * toggle value for 1ms */
        Value /= SweepStep;             /* / step size (in 탎) */
      }
      else                              /* normal mode */
      {
        /*
         *  calculate pulse length
         *  - t = (toggle * 2 * prescaler) / f_MCU
         */

        Value = (uint32_t)Toggle;
        Value *= 16000;                 /* * (2 * prescaler) (in 0.001) */
        Value /= (CPU_FREQ / 1000);     /* / f_MCU (in 1탎) */
      }

      /* display value */
      Display_FullValue(Value, 3, 'm');
      Display_Char('s');

      Flag &= ~DISPLAY_PULSE;           /* clear flag */
    }

    /* display PWM frequency/period */
    if (Flag & DISPLAY_FREQ)
    {
      LCD_ClearLine3();                 /* clear line #3 */
      MarkItem(MODE_FREQ, Mode);        /* mark mode if selected */

      Test = Period[Index];             /* get period */
      Value = 10000 / Test;             /* calculate frequency */
      Display_Value2(Value);            /* display frequency */
      Display_EEString(Hertz_str);      /* display: Hz */

      if (Flag & SWEEP_MODE)            /* in sweep mode */
      {
        Display_Space();
        Display_EEString(Sweep_str);    /* display: sweep */
      }

      Flag &= ~DISPLAY_FREQ;            /* clear flag */
    }

    /* smooth UI after long key press */
    if (Test == KEY_LONG)          /* long key press */
    {
      SmoothLongKeyPress();             /* delay next key press */
    }


    /*
     *  user feedback
     */

    /* wait for user feedback */
    Test = TestKey(0, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    /* consider rotary encoder's turning velocity (1-7) */
    Step = UI.KeyStep;             /* get velocity */
    if (Step > 1)                  /* larger step */
    {
      /* increase step size based on turning velocity */
      Step--;

      if (Flag & SWEEP_MODE)       /* in sweep mode */
      {
        /*
         *  MCU clock specific value range
         *  - 8MHz: 1-50, 16MHz: 1-100, 20MHz: 1-125
         */

        Step *= (SERVO_STEP_MAX / 32) + 1;
      }
      else                         /* in normal mode */
      {
        /*
         *  MCU clock specific value range
         *  - 8MHz: 250-1250, 16MHz: 500-2500, 20MHz: 625-3125
         *  - use multiples of 0.01ms step size
         */

        Step *= PULSE_STEP * ((SERVO_RIGHT_MAX - SERVO_LEFT_MAX) / 500);
      }
    }
    else                           /* single step */
    {
      if (! (Flag & SWEEP_MODE))   /* in normal mode */
      {
        /*
         *  MCU clock specific value
         *  - change step size for a resolution of about 0.01ms
         */

        Step = PULSE_STEP;
      }
    }

    /* process user input */
    if (Test == KEY_SHORT)              /* short key press */
    {
      /* toggle pulse/frequency mode */
      if (Mode == MODE_PULSE)           /* pulse width mode */
      {
        Mode = MODE_FREQ;               /* change to frequency mode */
      }
      else                              /* frequency mode */
      {
        Mode = MODE_PULSE;              /* change to pulse width mode */
      }

      Flag |= DISPLAY_PULSE | DISPLAY_FREQ;     /* update display */
    }
    else if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    else if (Test == KEY_LONG)          /* long key press */
    {
      if (Mode == MODE_PULSE)           /* pulse width mode */
      {
        if (Flag & SWEEP_MODE)          /* in sweep mode */
        {
          /* return to slowest sweep speed */
          SweepStep = 1;                /* smallest step */
          Flag |= DISPLAY_PULSE;        /* set flag */
        }
        else                            /* in normal mode */
        {
          /* return to middle position (1.5ms) */
          Toggle = SERVO_MID;           /* set mid */
          Flag |= CHANGE_PULSE | DISPLAY_PULSE;   /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        if (Flag & SWEEP_MODE)          /* in sweep mode */
        {
          /* leave sweep mode */
          Flag &= ~SWEEP_MODE;          /* clear flag */
        }
        else                            /* in normal mode */
        {
          /* enter sweep mode */
          Flag |= SWEEP_MODE;           /* set flag */
        }

        Flag |= DISPLAY_PULSE | DISPLAY_FREQ | TOGGLE_SWEEP;  /* set flags */
      }
    }
    else if (Test == KEY_RIGHT)         /* right key */
    {
      if (Mode == MODE_PULSE)           /* pulse width mode */
      {
        if (Flag & SWEEP_MODE)          /* in sweep mode */
        {
          /* increase sweep speed -> increase sweep step */
          Temp = SweepStep + Step;
          if (Temp > SERVO_STEP_MAX) Temp = SERVO_STEP_MAX;
          SweepStep = (uint8_t)Temp;
          Flag |= DISPLAY_PULSE;        /* set flag */
        }
        else                            /* in normal mode */
        {
          /* increase pulse length -> increase toggle value */
          Temp = Toggle + Step;         /* take advantage of overflow */
          if ((Temp < Toggle) || (Temp > SERVO_RIGHT_MAX))  /* overflow */
          {
            Temp = SERVO_RIGHT_MAX;     /* upper limit */
          }
          Toggle = Temp;                /* set new value */
          Flag |= CHANGE_PULSE | DISPLAY_PULSE;   /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        /* next PWN frequency -> increase index */
        if (Index < 3)                  /* upper limit is 3 */
        {
          Index++;                      /* next one */
          Flag |= DISPLAY_FREQ | CHANGE_FREQ;     /* set flags */
        }       
      }
    }
    else if (Test == KEY_LEFT)          /* left key */
    {
      if (Mode == MODE_PULSE)           /* pulse width mode */
      {
        if (Flag & SWEEP_MODE)          /* in sweep mode */
        {
          /* decrease sweep speed -> decrease sweep step */
          Temp = SweepStep - Step;
          if (Step >= SweepStep) Temp = 1;
          SweepStep = (uint8_t)Temp;
          Flag |= DISPLAY_PULSE;        /* set flag */
        }
        else                            /* in normal mode */
        {
          /* decrease pulse length -> decrease toggle value */
          Temp = Toggle - Step;         /* take advantage of underflow */
          if ((Temp > Toggle) || (Temp < SERVO_LEFT_MAX))   /* underflow */
          {
            Temp = SERVO_LEFT_MAX;      /* lower limit */
          }
          Toggle = Temp;                /* set new value */
          Flag |= CHANGE_PULSE | DISPLAY_PULSE;   /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        /* previous PWM frequency -> decrease index */
        if (Index > 0)                  /* lower limit is 0 */
        {
          Index--;                      /* previous one */
          Flag |= DISPLAY_FREQ | CHANGE_FREQ;     /* set flags */
        }
      }
    }


    /*
     *  enter/leave sweep operation
     *  - use Timer0 as sweep timer
     */

    if (Flag & TOGGLE_SWEEP)
    {
      if (Flag & SWEEP_MODE)            /* enter sweeping */
      {
        /* set start values */
        SweepStep = 1;                  /* forward */
        SweepDir = 1;                   /* lowest speed */

        /* start sweep timer */
        TCNT0 = 0;                      /* reset counter */
        TCCR0B = (1 << CS02) | (1 << CS00);  /* enable timer by setting prescaler */  
      }
      else                              /* exit sweeping */
      {
        /* stop sweep timer */
        TCCR0B = 0;                     /* disable Timer0 */
      }

      Flag &= ~TOGGLE_SWEEP;            /* clear flag */
    }
  }


  /*
   *  clean up
   */

  TCCR0B = 0;                 /* disable Timer0 */
  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TCCR1B = 0;                 /* disable Timer1 */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  R_DDR = 0;                            /* set HiZ mode */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  SIGNAL_DDR &= ~(1 << SIGNAL_OUT);     /* set HiZ mode */
  #endif

  /* local constants for sweeping */
  #undef SERVO_STEP_TIME
  #undef SERVO_SWEEP_TOP
  #undef SERVO_STEP_MAX
  #undef SERVO_RIGHT_MAX
  #undef SERVO_RIGHT_NORM
  #undef SERVO_MID
  #undef SERVO_LEFT_MAX
  #undef SERVO_LEFT_NORM

  /* local constant for step size */
  #undef PULSE_STEP

  /* local constants for Mode */
  #undef MODE_FREQ
  #undef MODE_PULSE

  /* local constants for Flag */
  #undef TOGGLE_SWEEP
  #undef DISPLAY_FREQ
  #undef DISPLAY_PULSE
  #undef CHANGE_FREQ
  #undef CHANGE_PULSE
  #undef SWEEP_MODE
  #undef RUN_FLAG
}



/*
 *  ISR for match of Timer0's OCR0A (Output Compare Register A)
 *  - sweep timer for servo check  
 */

ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
  uint16_t          Temp;     /* temp. value */

  /*
   *  hints:
   *  - the OCF0A interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   *    (no nested interrupts)
   */

  /* toggle values for PWM */
  #define SERVO_LEFT_NORM     (((CPU_FREQ / 10000) * 10) / 16)   /* 1.0ms */
  #define SERVO_RIGHT_NORM    (((CPU_FREQ / 10000) * 20) / 16)   /* 2.0ms */


  /*
   *  update PWM pulse length (Timer1)
   */

  Temp = OCR1B;               /* get current compare value */

  if (SweepDir == 1)          /* forward */
  {
    Temp += SweepStep;        /* add step */

    if (Temp >= SERVO_RIGHT_NORM)  /* exceeded upper limit */
    {
      Temp = SERVO_RIGHT_NORM;     /* limit */
      SweepDir = 2;                /* change direction */
    }
  }
  else                        /* backward */
  {
    Temp -= SweepStep;        /* substract step */

    if (Temp <= SERVO_LEFT_NORM)   /* exceeded lower limit */
    {
      Temp = SERVO_LEFT_NORM;      /* limit */
      SweepDir = 1;                /* change direction */
    }
  }

  OCR1B = Temp;               /* set new compare value */

  #undef SERVO_LEFT_NORM
  #undef SERVO_RIGHT_NORM
}

#endif



/* ************************************************************************
 *   Signal Generator (just squarewave)
 * ************************************************************************ */


#ifdef SW_SQUAREWAVE

/*
 *  create square wave signal with variable frequency
 *  - uses probe #2 (OC1B) as output
 *    and probe #1 & probe #3 as ground
 *  - alternative: dedicated signal output via OC1B
 *  - requires additional keys (e.g. rotary encoder)
 *  - requires idle sleep mode to keep timer running when MCU is sleeping
 */

void SquareWave_SignalGenerator(void)
{
  uint8_t           Flag = 1;           /* loop control */
  uint8_t           Test;               /* user feedback */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bits = 0;           /* prescaler register bits */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* counter's top value */
  uint16_t          Step;               /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */

  /*
      fast PWM:             f_PWM = f_MCU / (prescaler * (1 + top))
      available prescalers: 1, 8, 64, 256, 1024
      top:                  (2^2 - 1) up to (2^16 - 1)

      ranges for a 8MHz MCU clock:
      prescaler  /pre       top 2^16     top 2^2
      1          8MHz       122Hz        2MHz
      8          1MHz       15.26Hz      250kHz
      64         125kHz     1.9Hz        31.25kHz
      256        31.25kHz   0.5Hz        7812.5Hz
      1024       7812.5Hz   0.12Hz       1953.125Hz 
  */

  ShortCircuit(0);                      /* make sure probes are not shorted */

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Square Wave */
    Display_ColoredEEString_Space(SquareWave_str, COLOR_TITLE);
  #else
    Display_EEString_Space(SquareWave_str);  /* display: Square Wave */
  #endif
  #ifndef HW_FIXED_SIGNAL_OUTPUT
  ProbePinout(PROBES_PWM);              /* show probes used */
  #endif

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  /* set up probes: #1 and #3 are signal ground, #2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  /* dedicated output via OC1B */
  SIGNAL_PORT &= ~(1 << SIGNAL_OUT);    /* low by default */
  SIGNAL_DDR |= (1 << SIGNAL_OUT);      /* enable output */
  #endif


  /*
   *  set up Timer1 for PWM with 50% duty cycle 
   *  - fast PWM mode 
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1) | (1 << COM1B0);
  TCCR1B = (1 << WGM13) | (1 << WGM12);


  /*
   *  processing loop
   */

  /* set values for default frequency: 1kHz */
  Index = 0;                       /* prescaler 1/1 */
  Prescaler = 1;                   /* prescaler 1/1 */
  Bits = (1 << CS10);              /* register bits for prescaler 1 */
  Top = (CPU_FREQ / 1000) - 1;     /* top = f_MCU / (prescaler * f_PWM) - 1 */

  while (Flag > 0)
  {
    /*
     *  auto-ranging
     */

    Test = Index;

    /* check if we should change the range */
    if (Top > 0x7FFF)              /* more than 15 bits */
    {
      if (Index < (NUM_TIMER1 - 1))     /* don't exceed upper prescaler limit */
      {
        Index++;                   /* increase prescaler */
      }
    }
    else if (Top < 0x0FFF)         /* less than 12 bits */
    {
      if (Index > 0)               /* don't exceed lower prescaler limit */
      {
        Index--;                   /* decrease prescaler */
      }
    }

    /* process changed range */
    if (Index != Test)             /* range has changed */
    {
      Step = Prescaler;            /* save old value */

      /* read new prescaler and register bits from table */
      Prescaler = DATA_read_word(&T1_Prescaler_table[Index]);
      Bits = DATA_read_byte(&T1_RegBits_table[Index]);

      /* adjust top value for changed prescaler */
      if (Index > Test)            /* larger prescaler */
      {
        /* decrease top value by same factor as the prescaler increased */
        Temp = Prescaler / Step;
        Top /= Temp;
      }
      else                         /* smaller prescaler */
      {
        /* increase top value by same factor as the prescaler decreased */
        Temp = Step / Prescaler;
        Top *= Temp;  
      }
    }


    /*
     *  change timer settings
     */

    /* update timer */
    TCCR1B = (1 << WGM13) | (1 << WGM12);    /* stop timer */
    TCNT1 = 0;                               /* reset counter */
    OCR1B = Top / 2;                         /* 50% duty cycle */
    OCR1A = Top;                             /* top value for frequency */
    TCCR1B = (1 << WGM13) | (1 << WGM12) | Bits;    /* (re)start timer */


    /*
     *  display frequency
     *  - f_PWM = f_MCU / (prescaler * (1 + top))
     */

    Value = CPU_FREQ * 100;        /* scale to 0.01Hz */
    Value /= Prescaler;            /* / prescaler */
    Test = 2;                      /* 2 decimal places */

    /*
     *  optimize resolution of frequency without causing an overflow
     *  prescaler       :  1  8  64  256  1024
     *  decimal places  :  2  3   4    4     5
     */

    Temp = Prescaler;
    while (Temp >= 8)         /* loop through prescaler steps */
    {
      Value *= 10;            /* scale by factor 0.1 */
      Test++;                 /* one decimal place more */
      Temp /= 8;              /* next lower prescaler */
    }

    Value /= Top + 1;                   /* / (1 + top) */
    LCD_ClearLine2();
    Display_FullValue(Value, Test, 0);  /* display frequency */
    Display_EEString(Hertz_str);        /* display: Hz */


    /*
     *  user feedback
     */

    /* wait for user feedback */
    Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

    /* consider rotary encoder's turning velocity */
    Step = UI.KeyStep;             /* get velocity (1-7) */

    if (Step > 1)                  /* larger step */
    {
      /* increase step size based on turning velocity */

      /* step^4: 16 81 256 625 1296 2401 */
      Step *= Step;                /* ^2 */
      Step *= Step;                /* ^2 */
    }

    /* process user input */
    if (Test == KEY_RIGHT)         /* encoder: right turn */
    {
      /* increase frequency -> decrease top value */
      Temp = Top - Step;                /* take advantage of underflow */
      if ((Temp > Top) || (Temp < 0x003))    /* underflow */
      {
        Temp = 0x0003;                  /* lower limit */
      }
      Top = Temp;                       /* set new value */
    }
    else if (Test == KEY_LEFT)     /* encoder: left turn */
    {
      /* decrease frequency -> increase top value */
      Temp = Top + Step;                /* take advantage of overflow */
      if ((Temp < Top)  || (Temp == 0xFFFF))      /* overflow */
      {
        Temp = 0xFFFE;                  /* upper limit */
      }
      Top = Temp;                       /* set new value */
    }
    else if (Test == KEY_TWICE)    /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    else if (Test == KEY_LONG)     /* long key press */
    {
      /* set default frequency: 1kHz */
      Index = 0;                        /* prescaler 1/1 */
      Prescaler = 1;                    /* prescaler 1/1 */
      Bits = (1 << CS10);               /* register bits for prescaler 1 */
      Top = (CPU_FREQ / 1000) - 1;      /* top = f_MCU / (prescaler * f) - 1 */
    }
  }


  /*
   *  clean up
   */

  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */

  #ifndef HW_FIXED_SIGNAL_OUTPUT
  R_DDR = 0;                  /* set HiZ mode */
  #endif

  #ifdef HW_FIXED_SIGNAL_OUTPUT
  SIGNAL_DDR &= ~(1 << SIGNAL_OUT);     /* set HiZ mode */
  #endif
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef TOOLS_SIGNAL_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

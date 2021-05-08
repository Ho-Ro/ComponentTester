/* ************************************************************************
 *
 *   extras / additional features
 *
 *   (c) 2012-2017 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define EXTRAS_C

/* probes */
#define PROBES_PWM       0
#define PROBES_ESR       1



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

#ifdef HW_FREQ_COUNTER
FreqCounter_Type       Freq;            /* frequency counter */
#endif

#ifdef SW_SERVO
uint8_t                SweepStep;       /* sweep step */
uint8_t                SweepDir;        /* sweep direction */
#endif



/* ************************************************************************
 *   support functions
 * ************************************************************************ */


#if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS) || defined (SW_SQUAREWAVE) || defined (SW_SERVO) || defined (SW_ESR)

/*
 *  display probe pinout
 *
 *  required:
 *  - Mode
 */

void ProbePinout(uint8_t Mode)
{
  uint8_t           ID_1, ID_2, ID_3;   /* characters */

  LCD_ClearLine2();                /* info goes to line #2 */

  if (Mode == PROBES_PWM)     /* PWM */
  {
    /* 1: Gnd / 2: signal / 3: Gnd */
    ID_1 = '-';
    ID_2 = 's';
    ID_3 = '-';
  }
  else                        /* ESR */
  {
    /* 1: + / 3: - */
    ID_1 = '+';
    ID_2 = 0;
    ID_3 = '-';
  }

  Show_SimplePinout(ID_1, ID_2, ID_3);  /* display pinout */

  TestKey(5000, 0);           /* wait for any key press or 5s */  
  LCD_ClearLine2();           /* clear line #2 */
}

#endif



/* ************************************************************************
 *   PWM
 * ************************************************************************ */


#ifdef SW_PWM_SIMPLE

/*
 *  PWM generator with simple UI
 *  - use probe #2 (OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *  - max. reasonable PWM frequency for 8MHz MCU clock is 40kHz
 *
 *  requires:
 *  - Freqency in Hz
 */

void PWM_Tool(uint16_t Frequency)
{
  uint8_t           Test = 1;           /* loop control and user feedback */
  uint8_t           Ratio;              /* PWM ratio (in %) */
  uint8_t           Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Toggle;             /* counter value to toggle output */
  uint32_t          Value;              /* temporary value */

  /*
   *  - phase correct PWM:    f = f_MCU / (2 * prescaler * top)
   *  - available prescalers: 1, 8, 64, 256, 1024
   *  - range of top:  (2^2 - 1) up to (2^16 - 1)
   *
   *  - ranges for a 8MHz MCU clock:
   *    prescaler  /2pre     top 2^16   top 2^2    top 100
   *    1          4MHz      61Hz       1MHz       40kHz
   *    8          500kHz    7.6Hz      125kHz     5kHz
   *    64         62.5kHz   0.95Hz     15.625kHz  625Hz
   *    256        15625Hz   0.24Hz     3906.25Hz  156.25Hz
   *  - to support a PWM ratio of 1% top should be 100 at least
   */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString_Space(PWM_str);        /* display: PWM */
  DisplayValue(Frequency, 0, 'H');    /* display frequency */
  LCD_Char('z');                      /* make it Hz :-) */
  ProbePinout(PROBES_PWM);            /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  calculate required prescaler and top value based on MCU clock
   *  - top = f_MCU / (2 * prescaler * f_PWM)
   */

  Value = CPU_FREQ / 2;
  Value /= Frequency;

  if (Value > 2000000)        /* low frequency (<20Hz @8MHz) */
  {
    Value /= 256;
    Prescaler = (1 << CS12);                 /* 256 */
  }
  else if (Value > 16000)     /* mid-range frequency (<250Hz @8MHz) */
  {
    Value /= 64;
    Prescaler = (1 << CS11) | (1 << CS10);   /* 64 */
  }
  else                        /* high frequency */
  {
    Prescaler = (1 << CS10);                 /* 1 */
  }

  Top = (uint16_t)Value;      /* keep lower 16 bits */

  /* set start values */
  Ratio = 50;                 /* default ratio is 50% */
  Toggle = Top / 2;           /* compare value for 50% */


  /*
   *  set up Timer1 for PWM
   *  - phase correct PWM
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  TCCR1B = 0;                                /* disable timer */
  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);
  TCNT1 = 0;                                 /* set counter to 0 */
  OCR1A = Top;                               /* set top value (-1) */
  OCR1B = Toggle;                            /* set value to compare with */

  /* enable counter by setting clock prescaler */
  TCCR1B = (1 << WGM13) | Prescaler;


  /*
   *  ratio control
   */

  while (Test > 0)
  {
    /* show current ratio */
    LCD_ClearLine2();
    DisplayValue(Ratio, 0, '%');        /* show ratio in % */
    #ifdef HW_ENCODER
    if (Test <= KEY_LONG)               /* just for test button usage */
    #endif
    MilliSleep(500);                    /* smooth UI */

    /*
     *  user interface
     *  - short key press -> increase ratio
     *    long key press -> decrease ratio
     *    two short key presses -> exit tool
     */

    Test = TestKey(0, 0);               /* wait for user feedback */
    if (Test == KEY_SHORT)              /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Prescaler = TestKey(200, 0);      /* check for second key press */
      if (Prescaler > 0)                /* second key press */
      {
        Test = 0;                         /* end loop */
      }
      else                              /* single key press */
      {
        if (Ratio <= 95) Ratio += 5;      /* +5% and limit to 100% */
      }
    }
    #ifdef HW_ENCODER
    else if (Test == KEY_TURN_RIGHT)    /* rotary encoder: right turn */
    {
      if (Ratio <= 99) Ratio += 1;      /* +1% and limit to 100% */
    }
    else if (Test == KEY_TURN_LEFT)     /* rotary encoder: left turn */
    {
      if (Ratio >= 1) Ratio -= 1;         /* -1% and limit to 0% */
    }
    #endif
    else                                /* long key press */
    {
      if (Ratio >= 5) Ratio -= 5;         /* -5% and limit to 0% */
    }

    /* calculate toggle value: top * (ratio / 100) */
    Value = (uint32_t)Top * Ratio;
    Value /= 100;
    OCR1B = (uint16_t)Value;            /* update compare value */
  }

  /* clean up */
  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */
  R_DDR = 0;                  /* set HiZ mode */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */
}

#endif



#ifdef SW_PWM_PLUS

/*
 *  PWM generator with improved UI
 *  - use probe #2 (OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *  - max. reasonable PWM frequency for 8MHz MCU clock is 40kHz
 *  - requires rotary encoder and display with more than 2 text lines
 */

void PWM_Tool(void)
{
  #define FLAG_RUN       0b00000001     /* run / otherwise end */
  #define MODE_FREQ      0b00000010     /* frequency mode / otherwise ratio mode */
  #define CHANGE_FREQ    0b00000100     /* change frequency */
  #define CHANGE_RATIO   0b00001000     /* change ratio */
  #define UPDATE_FREQ    0b00010000     /* update display of frequency */
  #define UPDATE_RATIO   0b00100000     /* update display of ratio */

  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* user feedback */
  uint8_t           Step;               /* step size */
  uint8_t           Ratio;              /* PWM ratio (in %) */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bitmask = 0;        /* prescaler bitmask */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Step2;              /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */

  /*
   *  - phase correct PWM:    f = f_MCU / (2 * prescaler * top)
   *  - available prescalers: 1, 8, 64, 256, 1024
   *  - range of top:  (2^2 - 1) up to (2^16 - 1)
   *  - ranges for a 8MHz MCU clock:
   *    prescaler  /2pre       top 2^16   top 2^2    top 100
   *    1          4MHz        61Hz       1MHz       40kHz
   *    8          500kHz      7.6Hz      125kHz     5kHz
   *    64         62.5kHz     0.95Hz     15.625kHz  625Hz
   *    256        15625Hz     0.24Hz     3906.25Hz  156.25Hz
   *    1024       3906.25Hz   0.06Hz     976.5Hz    39Hz
   *  - to support a PWM ratio of 1% top should be 100 at least
   */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString_Space(PWM_str);        /* display: PWM */
  ProbePinout(PROBES_PWM);            /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  set up Timer1 for PWM
   *  - phase and frequency correct PWM
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  TCNT1 = 0;                                 /* set counter to 0 */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);


  /*
   *  processing loop
   */

  /* start values */
  Flag = FLAG_RUN | CHANGE_FREQ | CHANGE_RATIO | UPDATE_FREQ | UPDATE_RATIO;
  Ratio = 50;                      /* 50% PWM ratio */
  Prescaler = 1;
  Index = 0;
  Bitmask = (1 << CS10);           /* prescaler bitmask for 1 */
  Top = (CPU_FREQ / 2000);         /* 1kHz */

  while (Flag > 0)       /* processing loop */
  {
    /*
     *  change timer settings
     */

    /* change frequency */
    if (Flag & CHANGE_FREQ)
    {
      /*
       *  auto-ranging
       */

      Test = Index;

      /* check if we should change the range */
      if (Top > 0x7FFF)            /* more than 15 bits */
      {
        if (Index < 4)             /* don't exceed upper prescaler limit */
        {
          Index++;                 /* increase prescaler */
        }
      }
      else if (Top < 0x03FF)       /* less than 10 bits */
      {
        if (Index > 0)             /* don't exceed lower prescaler limit */
        {
          Index--;                 /* decrease prescaler */
        }
      }

      /* process changed range */
      if (Index != Test)           /* range has changed */
      {
        Step2 = Prescaler;         /* save old value */

        /* read new prescaler and bitmask from table */
        Prescaler = eeprom_read_word(&T1_Prescaler_table[Index]);
        Bitmask = eeprom_read_byte(&T1_Bitmask_table[Index]);

        if (Index > Test)          /* larger prescaler */
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
      TCCR1B = (1 << WGM13) | Bitmask;  /* (re)start timer */
      Flag &= ~CHANGE_FREQ;             /* clear flag */
      /* a frequency change implies a ratio change */
    }

    /* change ratio */
    if (Flag & CHANGE_RATIO)
    {
      /* toggle = top * (ratio / 100) */
      Value = (uint32_t)Top * Ratio;
      Value /= 100;
      OCR1B = (uint16_t)Value;          /* set compare/toggle value */
      Flag &= ~CHANGE_RATIO;            /* clear flag */
    }


    /*
     *  update display
     */

    /* frequency */
    if (Flag & UPDATE_FREQ)
    {
      LCD_ClearLine2();
      if (Flag & MODE_FREQ) LCD_Char('*');   /* mode selected */
      else LCD_Space();

      /* f = f_MCU / (2 * prescaler * top) */
      Value = CPU_FREQ * 50;            /* scale to 0.01Hz and /2 */
      Value /= Prescaler;
      Test = 2;                         /* 2 decimal places */

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

      Value /= Top;

      DisplayFullValue(Value, Test, 'H');
      LCD_Char('z');                    /* add z for Hz */

      Flag &= ~UPDATE_FREQ;             /* clear flag */
    }

    /* ratio */
    if (Flag & UPDATE_RATIO)
    {
      LCD_ClearLine(3);
      LCD_CharPos(1, 3);
      if (Flag & MODE_FREQ) LCD_Space();
      else LCD_Char('*');               /* mode selected */

      DisplayValue(Ratio, 0, '%');      /* show ratio in % */

      Flag &= ~UPDATE_RATIO;            /* clear flag */
    }


    /*
     *  user feedback
     */

    Test = TestKey(0, 0);          /* wait for key / rotary encoder */

    /* consider rotary encoder's turning velocity */
    Step = Enc.Velocity;           /* get velocity (1-7) */
    Step2 = 1;
    if (Step > 1)                  /* larger step */
    {
      if (Flag & MODE_FREQ)        /* step for frequency */
      {
        if (Step <= 4)             /* L step */
        {
          /* increase step size via binary shifting */
          Step2 <<= Step;          /* 2^Step */
        }
        else                       /* XXL step */
        {
          /* step size based on top */
          Step2 = Top / 8;
          Step2++;                      /* prevent zero */
        }
      }
      else                         /* step for ratio */
      {
        Step <<= 2;                       /* increase step size */
        if (Step > 100) Step = 100;       /* limit maximum */
      }
    }

    /* process user input */
    if (Test == KEY_SHORT)              /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, 0);           /* check for second key press */

      if (Test > KEY_TIMEOUT)           /* second key press */
      {
        Flag = 0;                       /* end loop */
      }
      else                              /* single key press */
      {
        /* toggle frequency/ratio mode */
        if (Flag & MODE_FREQ)           /* frequency mode */
        {
          Flag &= ~MODE_FREQ;           /* clear frequency flag */
        }
        else                            /* ratio mode */
        {
          Flag |= MODE_FREQ;            /* set frequency flag */
        }

        Flag |= UPDATE_FREQ | UPDATE_RATIO;  /* update display */
      }
    }
    else if (Test == KEY_TURN_RIGHT)    /* rotary encoder: right turn */
    {
      if (Flag & MODE_FREQ)        /* frequency mode */
      {
        /* increase frequency -> decrease top */
        Temp = Top - Step2;        /* take advantage of underflow */
        if ((Temp > Top) || (Temp < 0x0064))      /* underflow */
        {
          Temp = 0x0064;           /* lower limit */
        }
        Top = Temp;                /* set new value */

        Flag |= CHANGE_FREQ | UPDATE_FREQ | CHANGE_RATIO;   /* set flags */
      }
      else                         /* ratio mode */
      {
        /* increase ratio */
        Ratio += Step;             /* add (max 200) */
        if (Ratio > 100)           /* limit exceeded */
        {
          Ratio = 100;             /* max. is 100 */
        }

        Flag |= CHANGE_RATIO | UPDATE_RATIO;      /* set flags */
      }
    }
    else if (Test == KEY_TURN_LEFT)     /* rotary encoder: left turn */
    {
      if (Flag & MODE_FREQ)        /* frequency mode */
      {
        /* decrease frequency -> increase top */
        Temp = Top + Step2;        /* take advantage of overflow */
        if (Temp < Top)            /* overflow */
        {
          Temp = 0xFFFF;           /* upper limit */
        }
        Top = Temp;                /* set new value */

        Flag |= CHANGE_FREQ | UPDATE_FREQ | CHANGE_RATIO;   /* set flags */
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

        Flag |= CHANGE_RATIO | UPDATE_RATIO;      /* set flags */
      }
    }
  }

  /* clean up */
  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */
  R_DDR = 0;                  /* set HiZ mode */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */

  #undef UPDATE_RATIO
  #undef UPDATE_FREQ
  #undef CHANGE_RATIO
  #undef CHANGE_FREQ
  #undef MODE_FREQ
  #undef FLAG_RUN
}

#endif



#ifdef SW_SERVO

/*
 *  Server Check, PWM generator for testing servos
 *  - use probe #2 (OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *  - requires rotary encoder and display with more than 2 lines
 */

void Servo_Check(void)
{
  #define FLAG_RUN       0b00000001     /* run / otherwise end */
  #define MODE_PULSE     0b00000010     /* pulse width mode / otherwise frequency  */
  #define MODE_SWEEP     0b00000100     /* sweep mode */
  #define CHANGE_PULSE   0b00001000     /* change pulse width */
  #define CHANGE_FREQ    0b00010000     /* change frequency */
  #define UPDATE_PULSE   0b00100000     /* update display of pulse width */
  #define UPDATE_FREQ    0b01000000     /* update display of frequency */
  #define TOGGLE_SWEEP   0b10000000     /* enter/leave sweep operation */

  uint8_t           Flag = 1;           /* loop control */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Index;              /* PWM index */
  uint8_t           Period[4] = {200, 80, 40, 30};  /* in 0.1ms */
  uint16_t          Toggle;             /* toggle value */
  uint16_t          Step;               /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */

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
   *  - typical rotation is 90-120° & 180°
   *  - typical speed is 30-500ms/60°
   */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString_Space(Servo_str);      /* display: Servo */
  ProbePinout(PROBES_PWM);            /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  calculate required values for PWM based on MCU clock
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
   *  - t_step = 3ms -> f_step = 333Hz
   *  - prescaler = 1024
   *  - top = (f_MCU / (f_step * prescaler)) - 1
   *        = (t_step / (t_MCU_cycle * prescaler)) - 1
   *  - t_step = t_MCU_cycle * prescaler * (top + 1)
   *  - SERVO_STEP_TIME in µs
   */

  #define SERVO_SWEEP_TOP     (((CPU_FREQ / 333) / 1024) - 1)
  #define SERVO_STEP_TIME     ((MCU_CYCLE_TIME * 1024 * (SERVO_SWEEP_TOP + 1)) / 10000)


  /*
   *  set up Timer0 for sweeping
   *  - CTC mode
   *  - top value by OCR0A (unbuffered)
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

  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;   /* change sleep mode to Idle */

  TCNT1 = 0;                       /* reset counter to 0 */
  TIMSK1 = 0;                      /* disable all interrupts for Timer1 */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13) | (1 << CS11);  /* enable timer1 by setting prescaler */

  sei();                           /* enable interrupts */


  /* set start values */
  Toggle = SERVO_MID;              /* toggle value (1.5ms) */
  Index = 0;                       /* #0 (20.0ms) */
  SweepStep = 0;
  SweepDir = 0;
  Flag = FLAG_RUN | MODE_PULSE | CHANGE_PULSE | CHANGE_FREQ | UPDATE_FREQ | UPDATE_PULSE | UPDATE_FREQ;

  while (Flag > 0)       /* processing loop */
  {
    /*
     *  change timer settings
     */

    /* pulse width */
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

    /* pulse duration */
    if (Flag & UPDATE_PULSE)
    {
      LCD_ClearLine2();

      if (Flag & MODE_PULSE) LCD_Char('*');  /* mode selected */
      else LCD_Space();                      /* not selected */

      if (Flag & MODE_SWEEP)            /* sweep mode */
      {
        /*
         *  show sweep time
         *  - t_sweep = t_step * (toggle_1ms / step)
         */

        Value = SERVO_STEP_TIME;        /* step time in µs (around 3000) */
        Value *= SERVO_LEFT_NORM;       /* * toggle value for 1ms */
        Value /= SweepStep;             /* / step size */

        DisplayValue(Value, -6, 's');   /* display time */
      }
      else                              /* normal mode */
      {
        /*
         *  show pulse length
         *  - t = (toggle * 2 * prescaler) / f_MCU
         */

        Value = (uint32_t)Toggle;
        Value *= 1600;                  /* * (2 * prescaler) (in 0.01) */
        Value /= (CPU_FREQ / 1000);     /* / f_MCU (in 0.01ms) */

        DisplayValue(Value, -5, 's');   /* display duration */
      }

      Flag &= ~UPDATE_PULSE;            /* clear flag */
    }

    /* PWM frequency/period */
    if (Flag & UPDATE_FREQ)
    {
      LCD_ClearLine(3);
      LCD_CharPos(1, 3);

      if (Flag & MODE_PULSE) LCD_Space();    /* not selected */
      else LCD_Char('*');                    /* mode selected */

      Test = Period[Index];             /* get period */
      Value = 10000 / Test;             /* calculate frequency */
      DisplayValue(Value, 0, 'H');      /* display frequency */
      LCD_Char('z');

      if (Flag & MODE_SWEEP)            /* in sweep mode */
      {
        LCD_Space();
        LCD_EEString(Sweep_str);        /* display: sweep */
      }

      Flag &= ~UPDATE_FREQ;             /* clear flag */
    }

    /* smooth UI after long key press */
    if (Test == KEY_LONG)          /* long key press */
    {
      /* wait until button is released to prevent further actions */
      while (!(CONTROL_PIN & (1 << TEST_BUTTON))) /* as long as key is pressed */
      {
        wait10ms();           /* simply wait */
      }

      MilliSleep(500);        /* smooth UI */
    }


    /*
     *  user feedback
     */

    Test = TestKey(0, 2);          /* wait for key / rotary encoder */

    /* consider rotary encoder's turning velocity */
    Step = 1;
    if (Enc.Velocity > 1)          /* larger step */
    {
      Step <<= Enc.Velocity;       /* 2^velocity */
    }

    if (Test == KEY_SHORT)              /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, 0);           /* check for second key press */

      if (Test > KEY_TIMEOUT)           /* second key press */
      {
        Flag = 0;                       /* end loop */
      }
      else                              /* single key press */
      {
        /* toggle pulse/frequency mode */
        if (Flag & MODE_PULSE)          /* pulse width mode */
        {
          Flag &= ~MODE_PULSE;          /* clear pulse flag */
        }
        else                            /* frequency mode */
        {
          Flag |= MODE_PULSE;           /* set pulse flag */
        }

        Flag |= UPDATE_PULSE | UPDATE_FREQ;  /* update display */
      }
    }
    else if (Test == KEY_LONG)          /* long key press */
    {
      if (Flag & MODE_PULSE)            /* pulse width mode */
      {
        if (Flag & MODE_SWEEP)          /* in sweep mode */
        {
          /* return to slowest sweep speed */
          SweepStep = 1;                /* smallest step */
          Flag |= UPDATE_PULSE;         /* set flag */
        }
        else                            /* in normal mode */
        {
          /* return to middle position (1.5ms) */
          Toggle = SERVO_MID;           /* set mid */
          Flag |= CHANGE_PULSE | UPDATE_PULSE;    /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        if (Flag & MODE_SWEEP)          /* in sweep mode */
        {
          /* leave sweep mode */
          Flag &= ~MODE_SWEEP;          /* clear flag */
        }
        else                            /* in normal mode */
        {
          /* enter sweep mode */
          Flag |= MODE_SWEEP;           /* set flag */
        }

        Flag |= UPDATE_PULSE | UPDATE_FREQ | TOGGLE_SWEEP;  /* set flags */
      }
    }
    else if (Test == KEY_TURN_RIGHT)    /* rotary encoder: right turn */
    {
      if (Flag & MODE_PULSE)            /* pulse width mode */
      {
        if (Flag & MODE_SWEEP)          /* in sweep mode */
        {
          /* increase sweep speed -> increase sweep step */
          Temp = SweepStep + Step;
          if (Temp > SERVO_STEP_MAX) Temp = SERVO_STEP_MAX;
          SweepStep = (uint8_t)Temp;
          Flag |= UPDATE_PULSE;         /* set flag */
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
          Flag |= CHANGE_PULSE | UPDATE_PULSE;    /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        /* next PWN frequency -> increase index */
        if (Index < 3)                  /* upper limit is 3 */
        {
          Index++;                      /* next one */
          Flag |= UPDATE_FREQ | CHANGE_FREQ;      /* set flags */
        }       
      }
    }
    else if (Test == KEY_TURN_LEFT)     /* rotary encoder: left turn */
    {
      if (Flag & MODE_PULSE)            /* pulse width mode */
      {
        if (Flag & MODE_SWEEP)          /* in sweep mode */
        {
          /* decrease sweep speed -> decrease sweep step */
          Temp = SweepStep - Step;
          if (Step >= SweepStep) Temp = 1;
          SweepStep = (uint8_t)Temp;
          Flag |= UPDATE_PULSE;         /* set flag */
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
          Flag |= CHANGE_PULSE | UPDATE_PULSE;    /* set flags */
        }
      }
      else                              /* frequency mode */
      {
        /* previous PWM frequency -> decrease index */
        if (Index > 0)                  /* lower limit is 0 */
        {
          Index--;                      /* previous one */
          Flag |= UPDATE_FREQ | CHANGE_FREQ;      /* set flags */
        }
      }
    }


    /*
     *  enter/leave sweep operation
     *  - use Timer0 as sweep timer
     */

    if (Flag & TOGGLE_SWEEP)
    {
      if (Flag & MODE_SWEEP)            /* enter sweeping */
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

  /* clean up */
  TCCR0B = 0;                 /* disable Timer0 */
  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TCCR1B = 0;                 /* disable timer #1 */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */
  cli();                      /* disable interrupts */
  R_DDR = 0;                  /* set HiZ mode */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */

  #undef SERVO_STEP_TIME
  #undef SERVO_SWEEP_TOP
  #undef SERVO_STEP_MAX
  #undef SERVO_RIGHT_MAX
  #undef SERVO_RIGHT_NORM
  #undef SERVO_MID
  #undef SERVO_LEFT_MAX
  #undef SERVO_LEFT_NORM

  #undef TOGGLE_SWEEP
  #undef UPDATE_FREQ
  #undef UPDATE_PULSE
  #undef CHANGE_FREQ
  #undef CHANGE_PULSE
  #undef MODE_SWEEP
  #undef MODE_PULSE
  #undef FLAG_RUN
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
   *  - the OCF0A interrupt flag is  cleared automatically
   *  - interrupt processing is disabled while this ISR runs
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
 *  - use probe #2 (OC1B) as output
 *    and probe #1 + probe #3 as ground
 */

void SquareWave_SignalGenerator(void)
{
  uint8_t           Flag = 1;           /* loop control */
  uint8_t           Test;               /* user feedback */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bitmask = 0;        /* prescaler bitmask */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* counter's top value */
  uint16_t          Step;               /* step size */
  uint16_t          Temp;               /* temporary value */
  uint32_t          Value;              /* temporary value */

  /*
      fast PWM:             f = f_MCU / (prescaler * (1 + top))
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
  LCD_Clear();
  LCD_EEString_Space(SquareWave_str);   /* display: Square Wave */
  ProbePinout(PROBES_PWM);              /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << R_RL_2);                /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  set up Timer1 for PWM with 50% duty cycle 
   *  - fast PWM mode 
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1) | (1 << COM1B0);
  TCCR1B = (1 << WGM13);


  /*
   *  processing loop
   */

  /* start values for 1kHz */
  Index = 0;                       /* prescaler 1/1 */
  Prescaler = 1;                   /* prescaler 1/1 */
  Bitmask = (1 << CS10);           /* bitmask for prescaler 1 */
  Top = (CPU_FREQ / 1000) - 1;     /* top = f_MCU / (prescaler * f) - 1 */

  while (Flag > 0)
  {
    /*
     *  auto-ranging
     */

    Test = Index;

    /* check if we should change the range */
    if (Top > 0x7FFF)              /* more than 15 bits */
    {
      if (Index < 4)               /* don't exceed upper prescaler limit */
      {
        Index++;                   /* increase prescaler */
      }
    }
    else if (Top < 0x03FF)         /* less than 10 bits */
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

      /* read new prescaler and bitmask from table */
      Prescaler = eeprom_read_word(&T1_Prescaler_table[Index]);
      Bitmask = eeprom_read_byte(&T1_Bitmask_table[Index]);

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
    TCCR1B = (1 << WGM13) | (1 << WGM12) | Bitmask;    /* (re)start timer */


    /*
     *  display frequency
     *  - f = f_MCU / (prescaler * (1 + top))
     */

    Value = CPU_FREQ * 100;        /* scale to 0.01Hz */
    Value /= Prescaler;
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

    Value /= Top + 1;
    LCD_ClearLine2();
    DisplayFullValue(Value, Test, 'H');
    LCD_Char('z');                 /* add z for Hz */


    /*
     *  user feedback
     */

    Test = TestKey(0, 0);          /* wait for key / rotary encoder */

    /* consider rotary encoder's turning velocity */
    Step = 1;
    if (Enc.Velocity > 1)          /* larger step */
    {
      if (Enc.Velocity <= 4)            /* L step */
      {
        /* increase step size via binary shifting */
        Step <<= Enc.Velocity;          /* 2^Step */
      }
      else                              /* XXL step */
      {
        /* step size based on top */
        Step = Top / 8;
        Step++;                         /* prevent zero */
      }
    }

    /* process user input */
    if (Test == KEY_TURN_RIGHT)    /* encoder: right turn */
    {
      /* increase frequency -> decrease top value */
      Temp = Top - Step;           /* take advantage of underflow */
      if ((Temp > Top) || (Temp < 0x003))    /* underflow */
      {
        Temp = 0x0003;           /* lower limit */
      }
      Top = Temp;                /* set new value */
    }
    else if (Test == KEY_TURN_LEFT)  /* encoder: left turn */
    {
      /* decrease frequency -> increase top value */
      Temp = Top + Step;           /* take advantage of overflow */
      if ((Temp < Top)  || (Temp == 0xFFFF))      /* overflow */
      {
        Temp = 0xFFFE;             /* upper limit */
      }
      Top = Temp;                  /* set new value */
    }
    else if (Test > KEY_TIMEOUT)   /* any other key press */
    {
      /* todo: single press: default f / double press: end */
      Flag = 0;               /* end loop */
    }
  }


  /* clean up */
  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */
  R_DDR = 0;                  /* set HiZ mode */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */
}

#endif



/* ************************************************************************
 *   ESR tool
 * ************************************************************************ */


#ifdef SW_ESR

/*
 *  ESR tool
 */

void ESR_Tool(void)
{
  uint8_t           Run = 1;       /* control flag */
  uint8_t           Test;          /* temp. value */
  Capacitor_Type    *Cap;          /* pointer to cap */
  uint16_t          ESR;           /* ESR (in 0.01 Ohms) */

  Check.Diodes = 0;                /* disable diode check in cap measurement */
  Cap = &Caps[0];                  /* pointer to first cap */

  #ifdef HW_DISCHARGE_RELAY
  ADC_DDR = (1 << TP_REF);         /* short circuit probes */
  #endif

  /* show tool info */
  LCD_Clear();
  LCD_EEString(ESR_str);           /* display: ESR */
  ProbePinout(PROBES_ESR);         /* show probes used */
  LCD_Char('-');                   /* display "no value" */

  while (Run > 0)
  {
    /*
     *  short or long key press -> measure
     *  two short key presses -> exit tool
     */

    Test = TestKey(0, 2);               /* wait for user feedback */
    if (Test == KEY_SHORT)              /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, 0);           /* check for second key press */
      if (Test > KEY_TIMEOUT)           /* second key press */
      {
        Run = 0;                        /* end loop */
      }
    }

    /* measure cap */
    if (Run > 0)                        /* key pressed */
    {
      #ifdef HW_DISCHARGE_RELAY
      ADC_DDR = 0;                      /* remove short circuit */
      #endif

      LCD_ClearLine2();                 /* update line #2 */
      LCD_EEString(Running_str);        /* display: probing... */
      MeasureCap(PROBE_1, PROBE_3, 0);  /* probe-1 = Vcc, probe-3 = Gnd */
      LCD_ClearLine2();                 /* update line #2 */
      
      if (Check.Found == COMP_CAPACITOR)     /* found capacitor */
      {
        /* show capacitance */
        DisplayValue(Cap->Value, Cap->Scale, 'F');

        /* show ESR */
        LCD_Space();
        ESR = MeasureESR(Cap);
        if (ESR > 0)                    /* got valid ESR */
        {
          DisplayValue(ESR, -2, LCD_CHAR_OMEGA);
        }
        else                            /* no ESR */
        {
          LCD_Char('-');
        }
      }
      else                                   /* no capacitor */
      {
        LCD_Char('-');
      }

      #ifdef HW_DISCHARGE_RELAY
      ADC_DDR = (1<<TP_REF);            /* short circuit probes */
      #endif
    }
  }

  #ifdef HW_DISCHARGE_RELAY
  ADC_DDR = 0;                     /* remove short circuit */
  #endif
}

#endif



/* ************************************************************************
 *   Zener tool
 * ************************************************************************ */


#ifdef HW_ZENER

/*
 *  Zener tool:
 *  - Zener voltage measurement hardware option
 */

void Zener_Tool(void)
{
  uint8_t                Run = 1;            /* control flag */
  uint8_t                Counter;            /* length of key press */
  uint8_t                Counter2 = 0;       /* time between two key presses */
  uint16_t               Value;              /* current value */
  uint16_t               Min;                /* minimal value */

  /* show info */
  LCD_Clear();
  LCD_EEString(Zener_str);         /* display: Zener */
  LCD_NextLine();
  LCD_Char('-');                   /* display "no value" */

  while (Run > 0)             /* processing loop */
  {
    Counter = 0;              /* reset key press time */
    MilliSleep(30);           /* delay */
    Counter2++;               /* increase delay time */

    /*
     *  key press triggers measurement
     *  - also enables boost converter via hardware
     *  two short key presses exit tool
     */

    while (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* as long as key is pressed */
    {
      /* get voltage (10:1 voltage divider) */
      Value = ReadU(TP_ZENER);     /* special probe pin */
      Value /= 10;                 /* scale to 0.1V */

      /* display voltage */
      if (Counter % 8 == 0)        /* every 8 loop runs (240ms) */
      {
        LCD_ClearLine2();        
        DisplayValue(Value, -1, 'V');
      }

      /* data hold */
      if (Counter == 0)            /* first loop run */
      {
        Min = UINT16_MAX;          /* reset to default */
      }
      else if (Counter >= 10)      /* ensure stable DC */
      {
        if (Value < Min) Min = Value;   /* update minimum */
      }

      /* timer */
      MilliSleep(30);                        /* delay next run / also debounce */
      Counter++;                             /* increaye key press time */
      if (Counter > 240) Counter = 201;      /* prevent overflow */
    }


    /*
     *  user interface logic
     */

    if (Counter > 0)                         /* key was pressed */
    {
      /* detect two quick key presses */
      if (Run == 2)                          /* flag for short key press set */
      {
        if (Counter2 <= 8)                   /* short delay between key presses <= 250ms */
        {
          Run = 0;                           /* end loop */
        }
        else                                 /* long delay between key presses */
        {
          Run = 1;                           /* reset flag */
        }
      }
      else                                   /* flag not set */
      {
        if (Counter <= 10)                   /* short key press <= 300ms */
        {
          Run = 2;                           /* set flag */
        }
      }

      /* display hold value */
      LCD_ClearLine2();

      if (Min != UINT16_MAX)       /* got updated value */
      {
        DisplayValue(Min, -1, 'V');     /* display minimum */
        LCD_Space();
        LCD_EEString(Min_str);          /* display: Min */
      }
      else                         /* unchanged default */
      {
        LCD_Char('-');                  /* display "no value" */
      }

      Counter2 = 0;           /* reset delay time */
    }
  }
}

#endif



/* ************************************************************************
 *   Frequency counter
 * ************************************************************************ */


#ifdef HW_FREQ_COUNTER

/*
 *  frequency counter
 *  - frequency input: T0
 */

void FrequencyCounter(void)
{
  uint8_t           Flag = 1;           /* loop control flag */
  uint8_t           Old_DDR;            /* old DDR state */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bitmask;            /* prescaler bitmask */
  uint16_t          GateTime;           /* gate time in ms */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value for timer */
  uint32_t          Value;

  /* show info */
  LCD_Clear();
  LCD_EEString(FreqCounter_str);   /* display: Freq. Counter */
  LCD_NextLine();
  LCD_Char('-');                   /* display "no value" */
  

  /*
   *  We use Timer1 for the gate time and Timer0 to count pulses of the
   *  unknown signal.
   */


  /*
      counter limit for Timer1
      - gate time in µs
      - MCU cycles per µs
      - top = gatetime * MCU_cycles / prescaler 

      auto ranging

      range         gate time  prescaler  pulses
      -------------------------------------------------
      -10kHz           1000ms        256  -10000
      10kHz-100kHz      100ms         64  1000-10000
      100kHz-1MHz        10ms          8  1000-10000
      1MHz-               1ms          1  1000-
   */


  /* power save mode would disable timer0 and timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  /* start values for autoranging (assuming high frequency) */
  GateTime = 1;                    /* gate time 1ms */
  Index = 0;                       /* prescaler table index (prescaler 1/1) */

  /* set up Timer0 */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */ 


  /* measurement loop */
  while (Flag > 0)
  {
    /* set up T0 as input */
    Old_DDR = COUNTER_DDR;              /* save current settings */
    COUNTER_DDR &= ~(1 << COUNTER_IN);  /* signal input */
    wait500us();                        /* settle time */

    /* update prescaler */
    Prescaler = eeprom_read_word(&T1_Prescaler_table[Index]);
    Bitmask = eeprom_read_byte(&T1_Bitmask_table[Index]);

    /* calculate compare value for Timer1 */
    Value = MCU_CYCLES_PER_US;          /* clock based MCU cycles per µs */
    Value *= GateTime;                  /* gatetime (in ms) */
    Value *= 1000;                      /* scale to µs */
    Value /= Prescaler;                 /* divide by prescaler */
    Top = (uint16_t)Value;              /* use lower 16 bit */

    /* start timers */
    Freq.Pulses = 0;                    /* reset pulse counter */
    Flag = 2;                           /* enter waiting loop */
    TCNT0 = 0;                          /* Timer0: set counter to 0 */
    TCNT1 = 0;                          /* Timer1: set counter to 0 */
    OCR1A = Top;                        /* Timer1: set value to compare with */
    sei();                              /* enable interrupts */
    TCCR1B = Bitmask;                   /* start Timer1, prescaler */
    TCCR0B = (1 << CS02) | (1 << CS01); /* start Timer0, clock source: T0 falling edge */

    /* wait for timer1 or key press */
    while (Flag == 2)
    {
      if (TCCR1B == 0)                  /* Timer1 stopped by ISR */
      {
        Flag = 1;                       /* end waiting loop and signal Timer1 event */
      }
      else                              /* Timer1 still running */
      {
        /* check for key press */
        while (!(CONTROL_PIN & (1 << TEST_BUTTON)))
        {
          MilliSleep(50);          /* take a nap */
          Flag = 0;                /* end all loops */
        }

        if (Flag > 0) MilliSleep(100);    /* sleep for 100ms */

        /* I'd like to use TestKey() but ReadEncoder() produces some glitch
           which causes TCNT0 to be increased by 1 most times. */
      }
    }

    cli();                              /* disable interrupts */

    /* process measurement */
    COUNTER_DDR = Old_DDR;              /* restore old settings */

    if (Flag == 1)                      /* got valid measurement */
    {
      /* calculate frequency: f = pulses / gatetime */
      Freq.Pulses += TCNT0;             /* add counter of Timer0 to global counter */
      Value = Freq.Pulses;              /* number of pulses */
      Value *= 1000;                    /* scale gatetime to µs */
      Value /= GateTime;                /* divide by gatetime (in ms) */

      /* display frequency */
      LCD_ClearLine2();
      LCD_Char('f');                    /* display: f */
      LCD_Space();
      DisplayValue(Value, 0, 'H');      /* display frequency */
      LCD_Char('z');                    /* append "z" for Hz */

      /* autorange */
      if (Freq.Pulses > 10000)          /* range overrun */
      {
        if (GateTime > 1)               /* upper range limit not reached yet */
        {
          GateTime /= 10;               /* 100ms -> 10ms -> 1ms */
          Index--;                      /* one prescaler step down */
        }
      }
      else if (Freq.Pulses < 1000)      /* range underrun */
      {
        if (GateTime < 1000)            /* lower range limit not reached yet */
        {
          GateTime *= 10;               /* 1ms -> 10ms -> 100ms -> 1000ms */
          Index++;                      /* one prescaler step up */
        }
      }
    }
  }

  /* clean up */
  TIMSK0 = 0;                           /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                           /* disable all interrupts for Timer1 */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */
}



/*
 *  ISR for overflow of Timer0
 *  - catch overflows of pulse counter 
 */

ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
  /*
   *  hints:
   *  - the TOV0 interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   */

  Freq.Pulses += 256;         /* add overflow to global counter */
}



/*
 *  ISR for match of Timer1's OCR1A (Output Compare Register A)
 *  - for frequency counter gate time
 */

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
  /*
   *  hints:
   *  - the OCF1A interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   */

  /* gate time has passed */
  TCCR1B = 0;                 /* disable Timer1 */
  TCCR0B = 0;                 /* disable Timer0 */
}


#endif



/* ************************************************************************
 *   rotary encoder check
 * ************************************************************************ */


#ifdef SW_ENCODER

/* local constants */
#define DIR_NONE         0b00000000     /* no turn or error */
#define DIR_RIGHT        0b00000001     /* turned to the right */
#define DIR_LEFT         0b00000010     /* turned to the left */


/*
 *  check rotary encoder
 *
 *  requires:
 *  - pointer to encoder history
 */

uint8_t CheckEncoder(uint8_t *History)
{
  uint8_t           Action = DIR_NONE;       /* return value */
  uint8_t           Old_AB;                  /* old AB state */
  uint8_t           AB = 0;                  /* new AB state */
  uint8_t           Dir;                     /* turning direction */
  uint8_t           Steps;                   /* encoder steps */
  uint8_t           Temp;                    /* temporary value */

  /* we assume: probe-1 = A / probe-2 = B / probe-3 = Common */
  /* set up probes: probe-1 -- Rl -- Vcc / probe-2 -- Rl -- Vcc / Gnd -- probe-3 */
  R_PORT = Probes.Rl_1 | Probes.Rl_2;   /* pullup via Rl */
  R_DDR =  Probes.Rl_1 | Probes.Rl_2;   /* enable pull-up resistors */
  ADC_PORT = 0;                         /* pull down directly */
  ADC_DDR = Probes.Pin_3;               /* enable Gnd for probe-3 */
  wait500us();                          /* settle time */

  /* get A & B signals */
  Temp = ADC_PIN;
  if (Temp & Probes.Pin_1) AB = 0b00000010;
  if (Temp & Probes.Pin_2) AB |= 0b00000001;

  R_DDR = 0;                  /* reset probes */
  ADC_DDR = 0;

  /* unpack history */
  Temp = *History;
  Old_AB = Temp & 0b00000011;      /* old AB state, first 2 bits */
  Temp >>=2 ;                      /* move 2 bits */
  Dir = Temp & 0b00000011;         /* direction, next 2 bits */
  Temp >>= 2;                      /* move 2 bits */
  Steps = Temp;                    /* steps, remaining 4 bits */

  /* update state history */
  if (Dir == (DIR_RIGHT | DIR_LEFT))    /* first scan */
  {
    Old_AB = AB;              /* set as last state */
    Dir = DIR_NONE;           /* reset direction */
  }

  /* process signals */
  if (Old_AB != AB)           /* signals changed */
  {
    /* check if only one bit has changed (Gray code) */
    Temp = AB ^ Old_AB;                 /* get bit difference */
    if (!(Temp & 0b00000001)) Temp >>= 1;
    if (Temp == 1)                      /* valid change */
    {
      /* determine direction */
      /* Gray code: 00 01 11 10 */
      Temp = 0b10001101;                /* expected values for a right turn */
      Temp >>= (Old_AB * 2);            /* get expected value by shifting */
      Temp &= 0b00000011;               /* select value */
      if (Temp == AB)                   /* value matches */
        Temp = DIR_RIGHT;               /* turn to the right */
      else                              /* value mismatches */
        Temp = DIR_LEFT;                /* turn to the left */

      /* detection logic */
      if (Temp == Dir)                  /* turn in same direction */
      {
        Steps++;                        /* got another step */

        /* for proper detection we need 4 Gray code steps */
        if (Steps == 4)                 /* got 4 steps */
        {
          LCD_ClearLine2();

          /*
           *  The turning direction determines A and B:
           *  - right: A = Probe #1 / B = Probe #2
           *  - left:  A = Probe #2 / B = Probe #1
           */

          if (Dir == DIR_RIGHT)         /* right */
          {
            Semi.A = Probes.ID_1;
            Semi.B = Probes.ID_2;
          }
          else                          /* left */
          {
            Semi.A = Probes.ID_2;
            Semi.B = Probes.ID_1;
          }

          Semi.C = Probes.ID_3;         /* Common */

          /* display pinout */
          Show_SemiPinout('A', 'B', 'C');

          Steps = 0;                      /* reset steps */
          Action = Temp;                  /* signal valid step */
        }
      }
      else                         /* turn has changed direction */
      {
        Steps = 1;                 /* first step for new direction */
      }

      Dir = Temp;                  /* update direction */
    }
    else                                /* invalid change */
    {
      Dir = DIR_RIGHT | DIR_LEFT;       /* trigger reset of history */
    }
  }

  /* pack new history */
  Temp = AB;             /* AB state, first 2 bits */
  Dir <<= 2;             /* direction, next 2 bits */
  Temp |= Dir;
  Steps <<= 4;           /* steps, remaining 4 bits */
  Temp |= Steps;
  *History = Temp;       /* save new history */

  return Action;
}


/*
 *  rotary encoder check
 */

void Encoder_Tool(void)
{
  uint8_t      Flag;          /* flag/counter */
  uint8_t      History[3];    /* encoder history */

  /*
   *  History:
   *  - 000000xx AB state
   *  - 0000xx00 turning direction
   *  - xxxx0000 steps               
   */

  /* show info */
  LCD_Clear();
  LCD_EEString(Encoder_str);       /* display: Rotary Encoder */

  /* init array */
  for (Flag = 0; Flag <= 2; Flag++)
  {
    History[Flag] = (DIR_RIGHT | DIR_LEFT) << 2;
  }

  /* processing loop */
  Flag = 5;
  while (Flag < 10)
  {
    wdt_reset();

    if (Flag == 5)                 /* ask user to turn */
    {
      LCD_ClearLine2();
      LCD_EEString(TurnRight_str);     /* display: Turn right! */
      Flag = 0;                        /* reset flag */
    }

    UpdateProbes(PROBE_1, PROBE_2, PROBE_3);      /* check first pinout */
    Flag = CheckEncoder(&History[0]);

    if (Flag == 0)
    {
      UpdateProbes(PROBE_1, PROBE_3, PROBE_2);    /* check second pinout */
      Flag = CheckEncoder(&History[1]);
    }

    if (Flag == 0)
    {    
      UpdateProbes(PROBE_2, PROBE_3, PROBE_1);    /* check third pinout */
      Flag = CheckEncoder(&History[2]);
    }

    if (Flag > 0)             /* detected encoder */
    {
      TestKey(3000, 11);           /* let the user read */
      Flag = 5;                    /* reset flag */
    }
    else                      /* nothing found yet */
    {
      if (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* if key is pressed */
      {
        MilliSleep(100);           /* smooth UI */
        Flag = 10;                 /* end loop */
      }
    }
  }
}

#endif



/* ************************************************************************
 *   opto couplers
 * ************************************************************************ */


#ifdef SW_OPTO_COUPLER

/*
 *  check for LED
 *  - simple wrapper for CheckDiode()
 *
 *  requires:
 *  - Probe1: ID of positive probe (anode)
 *  - Probe2: ID of negative probe (cathode)
 */

void Check_LED(uint8_t Probe1, uint8_t Probe2)
{
  uint8_t           Probe3;             /* ID of probe #3 */
  uint16_t          U1;                 /* voltage */

  /* update all three probes */
  Probe3 = GetThirdProbe(Probe1, Probe2);    /* get third one */
  UpdateProbes(Probe1, Probe2, Probe3);      /* update probes */

  /* we assume: probe-1 = A / probe2 = C */
  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_PORT = 0;                      /* set resistor port to Gnd */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */
  ADC_DDR = Probes.Pin_1;          /* set probe-1 to output */
  ADC_PORT = Probes.Pin_1;         /* pull-up probe-1 directly */

  U1 = ReadU_5ms(Probes.ADC_2);    /* voltage at Rl (cathode) */

  if (U1 >= 977)         /*  not just a leakage current (> 1.4mA) */
  {
    CheckDiode();        /* run standard diode check */
  }
}



/*
 *  check opto couplers
 *  - pins which have to be connected (common Gnd):
 *    - LED's cathode and BJT's emitter 
 *    - LED's cathode and TRIAC's MT2
 *  - supports:
 *    - BJT
 *    - Triac (with and without zero crossing circuit)
 */

void OptoCoupler_Tool(void)
{
  uint8_t           Run = 1;            /* loop control */
  uint8_t           Test;               /* user input */
  uint16_t          U1, U2;             /* voltages */
  uint16_t          U3, U4;             /* voltages */
  uint32_t          CTR = 0;            /* CTR in % */

  /* init */
  LCD_NextLine_Mode(MODE_KEEP | MODE_KEY);   /* set line mode */

  /* display info */
  LCD_Clear();
  LCD_EEString(OptoCoupler_str);        /* display: Opto Coupler */
  LCD_NextLine_EEString(Start_str);     /* display: Start */

  while (Run)
  {
    /* user input */
    Test = TestKey(0, 2);          /* get user input */

    if (Test == KEY_SHORT)         /* short key press */
    {
      /* a second key press ends tool */
      MilliSleep(50);
      Test = TestKey(300, 0);
      if (Test > KEY_TIMEOUT) Run = 0;  /* end loop */
    }

    if (Run)                       /* check opto coupler */
    {
      LCD_Clear();
      LCD_EEString(OptoCoupler_str);    /* display: Opto Coupler */
      LCD_NextLine();
      Test = 0;

      /*
       *  scan for LED
       */

      Check.Found = COMP_NONE;          /* reset component search */
      Check.Diodes = 0;                 /* reset number of diodes */

      /* check all possible probe combinations */
      Check_LED(PROBE_1, PROBE_2);
      Check_LED(PROBE_2, PROBE_1);
      Check_LED(PROBE_1, PROBE_3);
      Check_LED(PROBE_3, PROBE_1);
      Check_LED(PROBE_2, PROBE_3);
      Check_LED(PROBE_3, PROBE_2);

      if (Check.Diodes == 1)       /* got one */
      {
        /* update all three probes for remaining checks */
        Test = GetThirdProbe(Diodes[0].A, Diodes[0].C);  /* get third probe */
        UpdateProbes(Diodes[0].A, Diodes[0].C, Test);    /* update probes */

        Test = 50;                      /* proceed with other checks */
      }


      /*
       *  we assume:
       *  probe-1 = LED's anode
       *  probe-2 = LED's cathode & BJT's emitter or TRIAC's MT2
       *  probe-3 = BJT's collector or TRIAC's MT1
       */


      /*
       *  check for BJT and TRIAC
       *  - BJT conducts only while LED is lit.
       *  - TRIAC keeps conducting as long as load current flows.
       *    Some types with zero crossing circuit got an inhibit voltage
       *    of about 5V.
       */

      if (Test == 50)
      {
        /* set probes: probe-2 -- Gnd / probe-3 -- Rl -- Vcc */
        ADC_DDR = Probes.Pin_2;              /* set probe-2 to output */
        ADC_PORT = 0;                        /* pull down probe-2 directly */
        R_DDR = Probes.Rl_1 | Probes.Rl_3;   /* select Rl for probe-1 & Rl for probe-3 */
        R_PORT = Probes.Rl_3;                /* pull up collector via Rl */
        U1 = ReadU_5ms(Probes.ADC_3);        /* voltage at collector when LED is off */

        /* make sure we have no conduction without the LED lit */
        if (U1 > 4000)        /* allow a leakage current of 1.5mA */
        {
          /* simulate zero crossing in case of a TRIAC with zero crossing circuit */
          R_PORT = Probes.Rl_1;                /* turn on LED */
          wait1ms();                           /* wait a tad */
          R_PORT = Probes.Rl_1 | Probes.Rl_3;  /* also pull up collector via Rl */
          U1 = ReadU_5ms(Probes.ADC_3);        /* voltage at collector when LED is on */

          R_PORT = Probes.Rl_3;                /* turn off LED */
          U2 = ReadU_5ms(Probes.ADC_3);        /* voltage at collector when LED is off */

          /* we should have conduction when the LED is lit */
          if (U1 <= 4000)          /* more than 1.5mA */
          {
            if (U2 >= 4000)        /* no conduction, allow some leakage current */
            {
              Test = 100;          /* BJT type */
            }
            else                   /* conduction */
            {
              /* check if both voltages are about the same */
              U3 = U1;
              U3 /= 8;             /* 12.5% */
              U4 = U1 - U3;        /* lower threshold */
              U3 += U1;            /* upper threshold */
              if ((U2 > U4) && (U2 < U3))
              {
                Test = 101;        /* TRIAC type */
              }
            }
          }
        }

        R_DDR = Probes.Rl_1;                 /* set probe-3 to HiZ */
      }


      /*
       *  measure CRT for BJT type
       */

      if (Test == 100)          /* got BJT type */
      {
        /* change probes: probe-3 -- Vcc */
        ADC_DDR = Probes.Pin_2 | Probes.Pin_3;    /* set probe-3 to output */
        ADC_PORT = Probes.Pin_3;                  /* pull up probe-3 directly */

        /* get voltages at current shunts */
        Config.Samples = 10;            /* just a few samples for 1ms runtime */
        R_PORT = Probes.Rl_1;           /* turn LED on */
        wait1ms();                      /* time for propagation delay */
        U1 = ReadU(Probes.ADC_1);       /* voltage at LED's anode (Rl) */
        U2 = ReadU(Probes.ADC_2);       /* voltage at emitter (RiL) */
        R_PORT = 0;                     /* turn LED off */
        Config.Samples = ADC_SAMPLES;   /* reset samples to default */

        /* calculate LED's If */
        /* If = (Vcc - U1) / (RiH + Rl) */
        U3 = Config.Vcc - U1;           /* Vcc - U1 (mV) */
        CTR = (uint32_t)U3;
        CTR *= 10000;                   /* scale to 0.0001 mV */
        U4 = NV.RiH + (R_LOW * 10);     /* RiH + Rl (0.1 Ohms) */
        CTR /= U4;                      /* If = U/R in µA */
        U3 = (uint16_t)CTR;             /* If in µA */

        /* calculate BJT's Ie */
        /* Ie = I_total - If = (U2 / RiL) - If */
        CTR = (uint32_t)U2;             /* U2 (mV) */
        CTR *= 10000;                   /* scale to 0.0001 mV */
        CTR /= NV.RiL;                  /* /RiL in 0.1 Ohms -> I_total (µA) */ 
        CTR -= U3;                      /* Ie = I_total - If (µA) */

        /* calculate CTR */
        /* CTR = Ie / If */
        CTR *= 100;                     /* scale up to % */
        CTR /= U3;                      /* Ie / If (%) */
      }


      /*
       *  Measure turn-on and turn-off times
       *  - Unfortunately we can't use the analog comparator in conjunction
       *    with Timer1, because the 1.1V bandgap reference would limit the
       *    time measurement to opto couplers with a CTR > 200%.
       */

      if (Test == 100)
      {
        U1 = UINT16_MAX;           /* reset value */
        U2 = UINT16_MAX;

        ADC_DDR = Probes.Pin_2;              /* set probe-2 to output */
        ADC_PORT = 0;                        /* pull down probe-2 directly */
        R_DDR = Probes.Rl_1 | Probes.Rl_3;   /* select Rl for probe-1 & Rl for probe-3 */
        R_PORT = Probes.Rl_3;                /* pull up collector via Rl */

        U1 = ReadU_5ms(Probes.ADC_3);        /* voltage at collector when LED is off */

        /* make sure we have no conduction without the LED lit */
        if (U1 > 4000)        /* allow a leakage current of 1.5mA */
        {
          Test = Probes.Pin_3;     /* port pin mask for probe-3 */

          /*
           *  turn-on delay
           */

          Run = 0;                                /* zero counter */
          R_PORT = Probes.Rl_1 | Probes.Rl_3;     /* turn on LED */

          /*
           *  wait for logic low level (<2.0V)
           *  - MCU cycles for full loop run: 7
           */

          while (ADC_PIN & Test)
          {
            Run++;                      /* increase counter */
            if (Run > 250) break;       /* check for overflow */
          }

          if (Run <= 250)          /* no overrun */
          {
            U1 = Run * 70;                   /* delay (0.1 MCU cycles) */
            U1 /= MCU_CYCLES_PER_US;         /* delay (0.1 µs) */
          }


          /*
           *  turn-off delay
           */

          Run = 0;                                /* zero counter */
          R_PORT = Probes.Rl_3;                   /* turn off LED */

          /*
           *  wait for logic high level (>2.5V)
           *  - MCU cycles for full loop run: 7
           */

          while (!(ADC_PIN & Test))
          {
            Run++;                      /* increase counter */
            if (Run > 250) break;       /* check for overflow */
          }

          if (Run <= 250)          /* no overrun */
          {
            U2 = Run * 70;                   /* delay (0.1 MCU cycles) */
            U2 /= MCU_CYCLES_PER_US;         /* delay (0.1 µs) */
          }

          Run = 1;            /* reset value */
          Test = 100;         /* reset value */
        }
      }


      /*
       *  display result
       */

      if (Test == 100)          /* got BJT type */
      {
        LCD_EEString(BJT_str);          /* display: BJT */

        LCD_NextLine_EEString_Space(CTR_str);     /* display: CTR */
        DisplayValue(CTR, 0, '%');                /* display CTR */

        LCD_NextLine_EEString_Space(If_str);      /* display: If */
        DisplayValue(U3, -6, 'A');                /* display If */

        if (U1 < UINT16_MAX)       /* valid t_on */
        {
          LCD_NextLine_EEString_Space(t_on_str);   /* display: t_on */
          if (U1 < 10)        /* < 1µs */
          {
            LCD_Char('<');
            U1 = 10;          /* 1µs */
          }
          DisplayValue(U1, -7, 's');
        }

        if (U2 < UINT16_MAX)       /* valid t_off */
        {
          LCD_NextLine_EEString_Space(t_off_str);  /* display: t_off */
          if (U2 < 10)        /* < 1µs */
          {
            LCD_Char('<');
            U2 = 10;          /* 1µs */
          }
          DisplayValue(U2, -7, 's');
        }

        LCD_NextLine_EEString_Space(Vf_str);      /* display: Vf */
        DisplayValue(Diodes[0].V_f, -3, 'V');     /* display Vf */
      }
      else if (Test == 101)     /* got TRIAC type */
      {
        LCD_EEString(Triac_str);        /* display: TRIAC */

        LCD_NextLine_EEString_Space(Vf_str);      /* display: Vf */
        DisplayValue(Diodes[0].V_f, -3, 'V');     /* display Vf */
      }
      else                      /* none found */
      {
        LCD_EEString(None_str);         /* display: None */
      }
    }
  }
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* probes */
#undef PROBES_ESR
#undef PROBES_PWM

/* source management */
#undef EXTRAS_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

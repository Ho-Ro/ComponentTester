/* ************************************************************************
 *
 *   L/C meter (hardware option)
 *
 *   (c) 2020-2021 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define TOOLS_LC_METER_C



/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */

/* global includes */
#include <stdfix.h>           /* fixed-point math */



/*
 *  local constants
 */

/* calculation modi */
#define CALC_CI          0         /* calculate C_i */
#define CALC_CX          1         /* calculate C_x */

/* math */
#define PI               3.14159265358979323846   /* pi */

/* limits for f_i (base frequency, around 595 kHz) */
#define FI_MIN           550000    /* 550 kHz */
#define FI_MAX           650000    /* 650 kHz */

/* limits for f_p (frequency with C_p, around 423 kHz) */
#define FP_MIN           400000    /* 400 kHz */
#define FP_MAX           440000    /* 440 kHz */



/*
 *  local variables
 */

#ifdef HW_LC_METER

  /* pulse counter (from tools_signal.c) */
  extern volatile uint32_t    Pulses;        /* number of pulses */

  /* frequencies */
  uint32_t                    LC_Freq;       /* measured frequency (in Hz) */
  uint32_t                    f_i;           /* base frequency (in Hz) */
  uint32_t                    f_x;           /* frequency with C_x/L_x (in Hz) */

  /* capacitances */
  uint32_t                    C_i;           /* C_i (in 0.1 pF) */

#endif



/* ************************************************************************
 *   L/C meter hardware option
 *   - uses shared ISRs in tools_signal.c for frequency counter
 *   - circuit based on AADE L/C meter from Neil Hecht
 *   - It's basically an LC oscillator (parallel mode) with an additional
 *     known reference cap which can be enabled via a relay. A second relay
 *     selects the DUT (L or C). C_x is connected in parallel to the LC
 *     tank circuit, and L_x in series with L from the LC tank circuit.
 *   - max. frequency of LC oscillator (L_i 82µH / C_i 1nF): about 600kHz
 * ************************************************************************ */


/*
 *  hints:
 *  - pin assigment:
 *    f_out     COUNTER_IN      T0
 *    C_p       LC_CTRL_CP      reference cap
 *    L/C       LC_CTRL_LC      L/C mode selection
 *  - control lines:
 *    - C_p (parallel reference cap)
 *      low      - enable C_p
 *      high/HiZ - disable C_p
 *    - L/C (L/C mode selection)
 *      low      - C
 *      high     - L
 *  - reference cap: LC_TINY_C_REF (in 0.1 pF)
 */


/*
Measuring C
- Parallel resonance:
  f_i = 1 / (2pi * sqrt(L_i * C_i))
- Adding an unknown capacitor C_x in parallel:
  f_x = 1 / (2pi * sqrt(L_i * (C_i + C_x)))
- With f_i/f_x we get:
  f_i/f_x = (2pi * sqrt(L_i * (C_i + C_x))) / (2pi * sqrt (L_i * C_i)))
  (f_i/f_x)^2 = (L_i * (C_i + C_x)) / (L_i * C_i)
              = (C_i + C_x) / C_i 
  C_x = C_i * ((f_i/f_x)^2 - 1)

Measurement of C_i by using a known reference cap C_p
- same as above, just with C_p instead of C_x:
  f_p = 1 / (2pi * sqrt(L_i * (C_i + C_p)))
- C_i = C_p / ((f_i/f_p)^2 - 1)

Measuring L
- Parallel resonance:
  f_i = 1 / (2pi * sqrt(L_i * C_i))
- resolve for L_i:
  L_i = 1 / (C_i * (2pi * f_i)^2)
- add unknown inductance L_x in series with L_i:
  f_s = 1 / (2pi * sqrt(L_s * C_i))
  L_s = 1 / (C_i * (2pi * f_s)^2)
- with L_s = L_i + L_x
  L_x = L_s - L_i
      = (1 / (C_i * (2pi * f_s)^2)) - (1 / (C_i * (2pi * f_i)^2))
      = (1 / (C_i * (2pi)^2)) * ((1 / f_s^2) - (1 / f_i^2))

Measurement ranges:
- L_i 82µH / C_i 1nF (base frequency around 595 kHz)
  firmware enforces a lower frequency limit of 10 kHz

  Mode  feasible         theoretically   PIC LC Meter
  ------------------------------------------------------
  L     1nH - 150mH      0.2nH - 250mH   10nH - 100mH
  C     10fF - 33nF      3.3fF - 3.5µF   0.1pF - 900nF
             - 120nF with signal clean-up
*/


#ifdef HW_LC_METER

/*
 *  measure frequency of LC oscillator
 *  - frequency input: T0
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 *  - max. frequency is 1/4 of MCU clock
 *  - stores frequency in global variable xxx
 *
 *  returns:
 *  - 0 for measurement done
 *  - key code >0 in case of any user feedback
 */

uint8_t Get_LC_Frequency(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Old_DDR;            /* old DDR state */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bits;               /* prescaler register bits */
  uint16_t          GateTime;           /* gate time in ms */
  uint16_t          Top;                /* top value for timer */
  uint32_t          Value;              /* temporary value */

  /* control flags */
  #define RUN_FLAG       1         /* run flag */
  #define WAIT_FLAG      2         /* enter/run waiting loop */
  #define GATE_FLAG      3         /* gatetime flag */

  /*
   *  We use Timer1 for the gate time and Timer0 to count pulses of the
   *  unknown signal. Max. frequency for Timer0 is 1/4 of the MCU clock.
   */

  LC_Freq = 0;                /* reset frequency value */
  Flag = RUN_FLAG;            /* enter measurement loop */


  /*
      auto ranging

      Timer1 top value (gate time)
      - top = gatetime * MCU_cycles / prescaler 
      - gate time in µs
      - MCU cycles per µs
      - top max. 2^16 - 1

      Frequency
      - f = pulses / gatetime
      - pulses = f * gatetime

      range         gate time  prescaler  MCU clock  remark
      ----------------------------------------------------------
      >= 400kHz        1000ms       1024  > 16MHz    < 1nF
                       1000ms        256  <= 16MHz
      < 400kHz          100ms         64  all        > 1nF
   */

  /* start values for autoranging (assuming high frequency) */
  GateTime = 100;                  /* gate time 100ms */
  Index = 2;                       /* prescaler table index (prescaler 64:1) */

  /* set up Timer0 (pulse counter) */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 (gate time) */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */

  /* set up T0 as input (pin might be shared with display) */
  Old_DDR = COUNTER_DDR;                /* save current settings */
  COUNTER_DDR &= ~(1 << COUNTER_IN);    /* signal input */
  wait500us();                          /* settle time */


  /*
   *  measurement loop
   */

  while (Flag)
  {
    /* update prescaler */
    Top = DATA_read_word(&T1_Prescaler_table[Index]);  /* prescaler value */
    Bits = DATA_read_byte(&T1_RegBits_table[Index]);   /* prescaler bits */

    /* calculate compare value for Timer1 (gate time) */
    /* top = gatetime * MCU_cycles / timer prescaler */
    Value = GateTime;                   /* gatetime (in ms) */
    /* * MCU cycles per µs and scale gatetime to µs */
    Value *= (MCU_CYCLES_PER_US * 1000);
    Value /= Top;                       /* divide by timer prescaler */
    Top = (uint16_t)Value;              /* use lower 16 bit */

    /* start timers */
    Pulses = 0;                         /* reset pulse counter */
    Flag = WAIT_FLAG;                   /* enter waiting loop */
    TCNT0 = 0;                          /* Timer0: reset pulse counter */
    TCNT1 = 0;                          /* Timer1: reset gate time counter */
    OCR1A = Top;                        /* Timer1: set gate time */
    TCCR1B = Bits;                      /* start Timer1: prescaler */
    TCCR0B = (1 << CS02) | (1 << CS01); /* start Timer0: clock source T0 - falling edge */
                                        /* for rising edge: | (1 << CS00) */

    /* wait for timer1 or key press */
    while (Flag == WAIT_FLAG)
    {
      if (TCCR1B == 0)                  /* Timer1 stopped by ISR */
      {
        Flag = GATE_FLAG;               /* end loop and signal Timer1 event */
      }
      else                              /* Timer1 still running */
      {
        /* wait for user feedback */
        Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

        if (Test)                       /* anything but timeout */
        {
          Flag = 0;                     /* end processing loop */
        }
      }
    }

    Cfg.OP_Control &= ~OP_BREAK_KEY;    /* clear break signal (just in case) */


    /*
     *  process measurement
     */

    if (Flag == GATE_FLAG)              /* got measurement */
    {
      /* total sum of pulses during gate period */
      Pulses += TCNT0;                  /* add counter of Timer0 */

      /*
       *  calculate frequency
       *  - f = pulses / gatetime
       *  - 20MHz MCU: 5M pulses per second at maximum
       */

      Value = Pulses;                   /* number of pulses */
      Value *= 1000;                    /* scale to ms */
      Value /= GateTime;                /* divide by gatetime (in ms) */
      Flag = 0;                         /* end loop */

      /* autoranging */
      if (Value < 400000UL)             /* range overrun */
      {
        if (GateTime == 1000)           /* in bottom range */
        {
          GateTime = 100;               /* change to top range */
          Index = 2;                    /* prescaler 64:1 */
          Flag = RUN_FLAG;              /* keep going */
        }
        /* else: already in top range */
      }
      else                              /* range underrun */
      {
        if (GateTime == 100)            /* in top range */
        {
          GateTime = 1000;              /* change to bottom range */
          #if CPU_FREQ > 16000000
            Index = 4;                  /* prescaler 1024:1 */
          #else
            Index = 3;                  /* prescaler 256:1 */
          #endif
          Flag = RUN_FLAG;              /* keep going */
        }
        /* else: already in bottom range */
      }

      if (Flag == 0)                    /* no change of range */
      {
        LC_Freq = Value;                /* save frequency */
      }
    }
  }


  /*
   *  clean up
   */

  /* T0 pin might be shared with display */
  COUNTER_DDR = Old_DDR;      /* restore old settings */

  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                 /* disable all interrupts for Timer1 */

  /* local constants */
  #undef RUN_FLAG
  #undef WAIT_FLAG
  #undef GATE_FLAG

  return Test;
}



/*
 *  calculate C_x or C_i
 *  - C_x = C_i * ((f_i/f_x)^2 - 1)
 *  - C_i = C_p / ((f_i/f_p)^2 - 1)
 *  - expects f_i >= f_x or f_i > f_p
 *
 *  requires:
 *  - Mode
 *    CALC_CI: calculate C_i
 *    CALC_CX: calculate C_x
 */

void LC_Calc_C(uint8_t Mode)
{
  unsigned long accum    t_r;      /* frequency term */
  unsigned long accum    C_x;      /* capacitance */

  /*
   *  Simplifications:
   *  - max. C_x is about 900nF -> f_x = 20kHz
   *    so we assume lowest f_x to be 10kHz
   *  - highest f_x is f_i (around 600kHz)
   *  - f_x: 10kHz up to 600kHz
   *    f_i/f_x: 60 down to 1
   *    (f_i/f_x)^2: 3600 down to 1
   */

  /* frequency term t_r = (f_i/f_x)^2 - 1 */
  t_r = (unsigned long accum)f_i;       /* base frequency (C_i/L_i) */
  t_r /= (unsigned long accum)f_x;      /* frequency with C_x */
  t_r *= t_r;                           /* ^2 */
  t_r -= 1;                             /* -1 */

  /* capacitance value */
  if (Mode == CALC_CI)        /* calculate C_i */
  {
    /* C_i = C_p / ((f_i/f_p)^2 - 1) = C_p / t_r */
    /* C_p in 0.1 pF */
    C_x = (unsigned long accum)LC_METER_C_REF / t_r;   /* in 0.1 pF */

    /* get integer part incl. rounding */
    C_i = bitsulk(C_x + 0.5ULK) >> ULACCUM_FBIT;       /* in 0.1 pF */
  }
  else                        /* calculate C_x */
  {
    /* C_x = C_i * ((f_i/f_x)^2 - 1) = C_i * t_r */
    /* C_i in 0.1 pF */
    C_x = (unsigned long accum)C_i * t_r;              /* in 0.1 pF */

    /* additional scaling for low values */
    if (C_x < 10000ULK)            /* < 1 nF */
    {
      /* rescale to 10 fF */
      C_x *= 10;                   /* rescale to 10^-14 */
      Caps[0].Scale = -14;         /* for 10 fF */
    }
    else                           /* >= 1 nF */
    {
      /* keep 0.1 pF scale */
      Caps[0].Scale = -13;         /* for 0.1 pF */
    }

    /* get integer part incl. rounding */
    Caps[0].Value = bitsulk(C_x + 0.5ULK) >> ULACCUM_FBIT;
  }
}



/*
 *  calculate L_x
 *  - L_x = (1 / (C_i * (2pi)^2)) * ((1 / f_s^2) - (1 / f_i^2))
 *  - expects f_i >= f_s
 */

void LC_Calc_L()
{
  unsigned long accum    t_i;      /* frequency term #1 */
  unsigned long accum    t_s;      /* frequency term #2 */

  /*
   *  Simplifications:
   *  - max. L_x is about 100mH -> f_s = 17kHz
   *    so we assume lowest f_s to be 10kHz
   *  - highest f_s is f_i (around 600kHz)
   *  - f_s: 10kHz up to 600kHz
   *    f_i^2: 3.6 * 10^11
   *    1 / f_i^2: 2.77 * 10^-12
   *    f_s^2: 1 * 10^8 up to 3.6 * 10^11
   *    1 / f_s^2: 1 * 10^-8 down to 2.77 * 10^-12
   *  - 1 / (C_i * (2pi)^2): ?
   */

  /* frequency term t_s = 1 / f_s^2 */
  t_s = (unsigned long accum)f_x;       /* frequency with L_x */
  t_s /= 10000;                         /* rescale to 10k */
  t_s *= t_s;                           /* ^2 */
  t_s = 1 / t_s;                        /* in 10^-8 */

  /* frequency term t_i = 1 / f_i^2 */
  t_i = (unsigned long accum)f_i;       /* base frequency (C_i/L_i) */
  t_i /= 10000;                         /* rescale to 10^4 */
  t_i *= t_i;                           /* ^2 */
  t_i = 1 / t_i;                        /* in 10^-8 */

  /* (1 / f_s^2) - (1 / f_i^2) = t_s - t_i */
  t_s -= t_i;                           /* in 10^-8 */

  /* reuse t_i for capacitance term: 1 / (C_i * (2pi)^2) */
  /* C_i in 0.1pF (10^-13) */
  t_i = (unsigned long accum)(PI * PI * 4);  /* (2pi)^2) */
  t_i *= C_i;                             /* * C_i */
  t_i /= 100000;                          /* rescale to 10^-8 */
  t_i = 1 / t_i;                          /* in 10^8 */
  t_i *= 1000000;                         /* rescale to 10^-2 */

  /* L_x = <capacitance term> * <frequency term> */
  t_i *= t_s;                 /* in µH (10^-6) */

  /* additional scaling for low values */
  if (t_i < 1000ULK)          /* < 1 mH */
  {
    /* rescale to nH */
    t_i *= 1000;              /* rescale to 10^-9 */
    Inductor.Scale = -9;      /* for nH */
  }
  else                        /* >= 1 nH */
  {
    /* keep µH scale */
    Inductor.Scale = -6;      /* for µH */
  }

  /* get integer part incl. rounding */
  Inductor.Value = bitsulk(t_i + 0.5ULK) >> ULACCUM_FBIT;
}



/*
 *  L/C meter self-adjustment
 *  - measure base frequency f_i (C_i/L_i)
 *  - measure C_i with help of C_p
 *
 *  returns:
 *  - 0 on any problem
 *  - 1 on success
 */

uint8_t LC_SelfAdjust(void)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           Test;               /* user feedback */
  uint8_t           OldState;           /* old state of L/C selection */

  /* inform user */
  LCD_ClearLine2();                     /* clear line #2 */
  SmoothLongKeyPress();                 /* smooth UI after long key press */
  Display_EEString(Adjusting_str);      /* display: adjusting */

  /* get current state of L/C selection */
  OldState = LC_CTRL_PORT & (1 << LC_CTRL_LC);    /* filter bit for L/C */

  /* select C mode: set control line for L/C selection low */
  LC_CTRL_PORT &= ~(1 << LC_CTRL_LC);   /* clear bit */
  MilliSleep(100);                      /* settling time */

  /* measure base frequency f_i */
  Test = Get_LC_Frequency();

  if (Test == 0)              /* got frequency */
  {
    /* check if f_i is fine (roughly around 600 kHz */
    if ((LC_Freq >= FI_MIN) && (LC_Freq <= FI_MAX))
    {
      f_i = LC_Freq;                         /* save f_i */

      /* enable reference cap C_p */
      LC_CTRL_DDR |= (1 << LC_CTRL_CP);      /* enable low output */
      MilliSleep(100);                       /* settling time */

      /* measure f_p */
      Test = Get_LC_Frequency();

      if (Test == 0)                         /* got frequency */
      {
        /* check if f_p is fine (roughly around 423 kHz) */
        if ((LC_Freq >= FP_MIN) && (LC_Freq <= FP_MAX))
        {
          f_x = LC_Freq;                     /* update f_p */
          LC_Calc_C(CALC_CI);                /* calculate C_i */

          Flag = 1;                          /* signal success */
        }
        /* else: invalid f_p */
      }
      /* else: aborted by user feedback */      

      /* disable reference cap C_p */
      LC_CTRL_DDR &= ~(1 << LC_CTRL_CP);     /* set C_p to HiZ mode */  
    }
    /* else: f_i out of range */
  }
  /* else: aborted by user feedback */

  /* restore old state of L/C selection */
  LC_CTRL_PORT |= OldState;             /* set bit (when set in OldState) */

  return Flag;
}



/*
 *  L/C meter
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any error
 */

uint8_t LC_Meter(void)
{
  uint8_t           Flag = 1;           /* return value */
  uint8_t           Run = 0;            /* loop control flag */
  uint8_t           Test;               /* user feedback */
  uint8_t           CtrlDir;            /* control DDR state */
  uint8_t           Mode;               /* measurement mode (L/C) */
  uint8_t           Delay;              /* delay flag */

  /* control flags */
  #define RUN_FLAG            0b00000001     /* run flag */
  #define UPDATE_MODE         0b00000010     /* update mode (L/C) */
  #define SHOW_VALUE          0b00000100     /* show value */
  #define NO_VALUE            0b00001000     /* no value available */

  /* measurement mode */
  #define MODE_C              0              /* C */
  #define MODE_L              1              /* L */

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: LC Meter */
    Display_ColoredEEString(LC_Meter_str, COLOR_TITLE);
  #else
    Display_EEString(LC_Meter_str);     /* display: LC Meter */
  #endif

  /*
   *  init
   */

  /* set up control lines */
  CtrlDir = LC_CTRL_DDR;                /* get current direction */
  LC_CTRL_DDR &= ~(1 << LC_CTRL_CP);    /* set C_p to HiZ mode (disable C_p) */
  LC_CTRL_DDR |= (1 << LC_CTRL_LC);     /* set L/C selection to output mode */
  LC_CTRL_PORT &= ~(1 << LC_CTRL_CP);   /* set C_p low (default) */
  /* to enable C_p we simply switch to output mode */

  /* initial self-adjustment */
  Test = LC_SelfAdjust();          /* run adjustment */

  if (Test)                        /* adjustment done */
  {
    /* set start values */
    Mode = MODE_C;                      /* start with C */
    Run = RUN_FLAG | UPDATE_MODE;       /* set control flags */
  }
  else                             /* adjustment error */
  {
    Flag = 0;                           /* signal error */
  }


  /*
   *  processing loop
   */

  while (Run)
  {
    /*
     *  set measurement mode
     */

    if (Run & UPDATE_MODE)
    {
      /* select mode */
      if (Mode == MODE_C)          /* measure C */
      {
        /* set control line for L/C selection low */
        LC_CTRL_PORT &= ~(1 << LC_CTRL_LC);       /* clear bit */
      }
      else                         /* measue L */
      {
        /* set control line for L/C selection high */
        LC_CTRL_PORT |= (1 << LC_CTRL_LC);        /* set bit */
      }

      /* trigger output of "no value" */
      Run |= SHOW_VALUE | NO_VALUE;

      Run &= ~UPDATE_MODE;         /* clear flag */
    }


    /*
     *  manage measurement
     */

    Test = 0;                      /* reset value */

    if (! (Run & NO_VALUE))
    {
      Test = Get_LC_Frequency();   /* measure f_x/f_s */

      if (Test == 0)               /* got frequency */
      {
        f_x = LC_Freq;             /* save f_x/f_s */

        /* f_x must be lower than f_i */
        if (f_i >= f_x)            /* f_x lower than f_i */
        {
          Delay = 0;               /* reset flag */

          if (f_x < 400000UL)      /* < 400 kHz / 100 ms gate time */
          {
            Delay = 1;             /* set flag for short gate time */
          }

          if (Mode == MODE_C)      /* C */
          {
            /* check for minimum frequency */
            if (f_x >= 10000UL)    /* >= 10 kHz */
            {
              /* frequency is fine */
              LC_Calc_C(CALC_CX);  /* calculate C_x */
            }
            else                   /* < 10 kHz */
            {
              /* frequency too low */
              Run |= NO_VALUE;     /* show "no value" */
            }
          }
          else                     /* L */
          {
            /* missing L_x causes open LC tank with 0 Hz */
            if (f_x <= 10)         /* allow some slack */
            {
              /* we like to see 0.0nH */
              f_x = f_i;           /* simply take f_i as f_x */
            }

            /* check for minimum frequency */
            if (f_x >= 10000UL)    /* >= 10 kHz */
            {
              /* frequency is fine */
              LC_Calc_L();         /* calculate L_x */
            }
            else                   /* < 10 kHz */
            {
              /* frequency too low */
              Run |= NO_VALUE;     /* show "no value" */
            }
          }

          if (Delay)               /* short gate time */
          {
            /* delay the measurement update */
            Test = TestKey(500, CHECK_KEY_TWICE | CHECK_BAT);
          }
        }
        else                       /* f_x higher than f_i */
        {
          /* f_i has drifted (increased) */
          Run |= NO_VALUE;         /* show "no value" */
          /* todo: update f_i (auto-adjust)? */
        }

        #ifdef LC_METER_SHOW_FREQ
        /* display frequency of LC oscillator in line #3 */
        LCD_ClearLine(3);
        LCD_CharPos(1, 3);
        Display_Char('f');                    /* display: f */
        Display_Colon();                      /* display: : */
        Display_Space();
        Display_FullValue(LC_Freq, 0, 0);     /* display frequency */
        Display_EEString(Hertz_str);          /* display: Hz */
        #endif

        Run |= SHOW_VALUE;         /* show value */
      }
    }


    /*
     *  process user feedback
     */

    if (Test)                 /* any user feedback */
    {
      #ifdef HW_KEYS
      /* short key press, right key or left key */
      if ((Test == KEY_SHORT) || (Test == KEY_RIGHT) || (Test == KEY_LEFT))
      #else
      if (Test == KEY_SHORT)            /* short key press */
      #endif
      {
        /* change mode */
        if (Mode == MODE_C)             /* from C */
        {
          Mode = MODE_L;                /* to L */
        }
        else                            /* from L */
        {
          Mode = MODE_C;                /* to C */
        }

        Run |= UPDATE_MODE;             /* update mode */
      }
      else if (Test == KEY_LONG)        /* long key press */
      {
        /* repeat self-ajustment */
        Test = LC_SelfAdjust();         /* run self-ajustment */

        if (Test)                       /* adjustment done */
        {
          Run |= SHOW_VALUE | NO_VALUE;      /* display "no value" */
        }
        else                            /* adjustment error */
        {
          Run = 0;                           /* end processing loop */
          Flag = 0;                          /* signal error */
        }
      }
      else if (Test == KEY_TWICE)       /* two short key presses */
      {
        /* end tool */
        Run = 0;                        /* end processing loop */
      }
    }


    /*
     *  display measurement value in line #2
     */

    if (Run & SHOW_VALUE)
    {
      /* display mode */
      LCD_ClearLine2();            /* clear line */

      if (Mode == MODE_C)          /* C */
      {
        Test = 'C';
      }
      else                         /* L */
      {
        Test = 'L';
      }

      Display_Char(Test);          /* display L/C */
      Display_Colon();

      /* display value */
      Display_Space();

      if (Run & NO_VALUE)          /* no value */
      {
        Display_Minus();           /* display: -*/
      }
      else                         /* value */
      {
        if (Mode == MODE_C)        /* C */
        {
          /* display capacitance */
          Display_Value(Caps[0].Value, Caps[0].Scale, 'F');
        }
        else                       /* L */
        {
          /* display inductance */
          Display_Value(Inductor.Value, Inductor.Scale, 'H');
        }
      }

      Run &= ~(SHOW_VALUE | NO_VALUE);  /* clear flags */      
    }
  }


  /*
   *  clean up
   */

  /* filter control lines which were in input mode */ 
  CtrlDir ^= (1 << LC_CTRL_CP) | (1 << LC_CTRL_LC);
  CtrlDir &= (1 << LC_CTRL_CP) | (1 << LC_CTRL_LC);
  LC_CTRL_DDR &= ~CtrlDir;         /* set former direction */

  /* local constants */
  #undef RUN_FLAG
  #undef UPDATE_MODE
  #undef SHOW_VALUE
  #undef NO_VALUE

  #undef MODE_C
  #undef MODE_L

  return Flag;
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* misc constants */
#undef CALC_CI
#undef CALC_CX

#undef PI

#undef FI_MIN
#undef FI_MAX

#undef FP_MIN
#undef FP_MAX


/* source management */
#undef TOOLS_LC_METER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

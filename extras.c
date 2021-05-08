/* ************************************************************************
 *
 *   extras / additional features
 *
 *   (c) 2012-2014 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define EXTRAS_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   PWM tool
 * ************************************************************************ */


#ifdef EXTRA

/*
 *  PWM tool
 *  - use probe #2 (PB2, OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *
 *  requires:
 *  - Freqency in Hz
 */

void PWM_Tool(uint16_t Frequency)
{
  uint8_t           Test = 1;           /* loop control and user feedback */
  uint8_t           Ratio;              /* PWM ratio */
  uint8_t           Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Toggle;             /* counter value to toggle output */
  uint32_t          Value;              /* temporary value */

  /*
      fast PWM:             f = f_MCU / (prescaler * depth)
      phase correct PWM:    f = f_MCU / (2 * prescaler * depth)
      available prescalers: 1, 8, 64, 256, 1024
      depth:                2^x (x is the bit depth)
  */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString2(PWM_str);             /* display: PWM */
  DisplayValue(Frequency, 0, 'H');    /* display frequency */
  LCD_Data('z');                      /* make it Hz :-) */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << (TP2 * 2));             /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  calculate required prescaler and top value based on MCU clock
   *
   *    depth = f_MCU / (2 * prescaler * f_PWM)
   */

  Value = CPU_FREQ / 2;
  Value /= Frequency;

  if (Value > 2000000)        /* low frequency */
  {
    Value /= 256;
    Prescaler = (1 << CS12);                 /* 256 */
  }
  else if (Value > 16000)     /* mid-range frequency */
  {
    Value /= 64;
    Prescaler = (1 << CS11) | (1 << CS10);   /* 64 */    
  }
  else                        /* high frequency */
  {
    Prescaler = (1 << CS10);                 /* 1 */    
  }

  Top = (uint16_t)Value;


  /*
   *  setup timer1 for PWM
   *  - PWM, phase correct, top value by OCR1A
   */

  Ratio = 50;                                /* default ratio is 50% */
  Toggle = (Top / 2) - 1;                    /* compare value for 50% */
  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  TCCR1B = 0;                                /* disable timer */
  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);
  TCNT1 = 0;                                 /* set counter to 0 */
  OCR1A = Top - 1;                           /* set top value (-1) */
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
    MilliSleep(500);                    /* smooth UI */

    /*
        short key press -> increase ratio
        long key press -> decrease ratio
        two short key presses -> exit tool
     */

    Test = TestKey(0, 0);               /* wait for user feedback */
    if (Test == 1)                      /* short key press */
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
    else                                /* long key press */
    {
      if (Ratio >= 5) Ratio -= 5;         /* -5% and limit to 0% */
    }

    /* calculate toggle value: (depth * (ratio / 100)) - 1 */
    Value = (uint32_t)Top * Ratio;
    Value /= 100;
    Toggle = (uint16_t)Value;
    Toggle--;

    OCR1B = Toggle;                     /* update compare value */
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


#ifdef EXTRA

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

  #ifdef HW_RELAY
  ADC_DDR = (1<<TP_REF);           /* short circuit probes */
  #endif

  /* show tool info */
  LCD_Clear();
  LCD_EEString(ESR_str);           /* display: ESR */
  LCD_Line2();
  LCD_Data('-');                   /* display "no value" */

  while (Run > 0)
  {
    /*
     *  short or long key press -> measure
     *  two short key presses -> exit tool
     */

    Test = TestKey(0, 0);               /* wait for user feedback */
    if (Test == 1)                      /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, 0);           /* check for second key press */
      if (Test > 0)                     /* second key press */
      {
        Run = 0;                        /* end loop */
      }
    }

    /* measure cap */
    if (Run > 0)                       /* key pressed */
    {
      #ifdef HW_RELAY
      ADC_DDR = 0;                     /* remove short circuit */
      #endif

      LCD_ClearLine2();                /* update line #2 */
      LCD_EEString(Running_str);       /* display: probing... */
      MeasureCap(TP2, TP1, 0);         /* probe 2 = Gnd, probe 1 = Vcc */
      LCD_ClearLine2();                /* update line #2 */
      
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
          LCD_Data('-');
        }
      }
      else                                   /* no capacitor */
      {
        LCD_Data('-');
      }

      #ifdef HW_RELAY
      ADC_DDR = (1<<TP_REF);            /* short circuit probes */
      #endif
    }
  }

  #ifdef HW_RELAY
  ADC_DDR = 0;                     /* remove short circuit */
  #endif
}

#endif



/* ************************************************************************
 *   Zener tool
 * ************************************************************************ */


#ifdef EXTRA
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
  LCD_Line2();
  LCD_Data('-');                   /* display "no value" */

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
      Value = ReadU(TP_ZENER);
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
        LCD_Data('-');                  /* display "no value" */
      }

      Counter2 = 0;           /* reset delay time */
    }
  }
}

  #endif
#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef EXTRAS_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

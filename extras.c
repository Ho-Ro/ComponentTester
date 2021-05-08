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



/*
 *  local variables
 */

#ifdef HW_FREQ_COUNTER
FreqCounter_Type       Freq;            /* frequency counter */
#endif



/* ************************************************************************
 *   support functions
 * ************************************************************************ */


#if defined (SW_PWM) || defined (SW_ESR)

/*
 *  display probe pins used
 */

void ToolInfo(const unsigned char *String)
{
  uint8_t           n = 0;
  uint8_t           Key = 0;

  LCD_ClearLine2();                /* info goes to line #2 */

  /* blink text up to three times */
  while (n <= 2)
  {
    LCD_EEString2(Probes_str);     /* show text */
    LCD_EEString(String);
    Key = TestKey(700, 0);         /* wait 700ms */

    LCD_ClearLine2();              /* clear line #2 */
    if (Key == 0) Key = TestKey(300, 0);      /* wait 300ms */

    if (Key > 0) n = 3;            /* on key press end loop */
    n++;                           /* next run */
  }

  MilliSleep(250);                 /* smooth UI */
}

#endif



/* ************************************************************************
 *   PWM tool
 * ************************************************************************ */


#ifdef SW_PWM

/*
 *  PWM tool
 *  - use probe #2 (PB2, OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *  - max. reasonable PWM frequency for 8MHz MCU clock is 40kHz
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
      phase correct PWM:    f = f_MCU / (2 * prescaler * top)
      available prescalers: 1, 8, 64, 256, 1024
      top:                  (2^2 - 1) up to (2^16 - 1)

      ranges for a 8MHz MCU clock:
      prescaler  /2pre     top 2^16     top 2^2
      1          4MHz      61Hz         1MHz
      8          500kHz    7.6Hz        125kHz
      64         62.5kHz   0.95Hz       15625Hz
      256        15625Hz   0.24Hz       3906.25Hz
  */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString2(PWM_str);             /* display: PWM */
  DisplayValue(Frequency, 0, 'H');    /* display frequency */
  LCD_Data('z');                      /* make it Hz :-) */
  ToolInfo(PWM_Probes_str);           /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << (TP2 * 2));             /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  calculate required prescaler and top value based on MCU clock
   *
   *    top = f_MCU / (2 * prescaler * f_PWM)
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


  /*
   *  setup Timer1 for PWM
   *  - phase correct PWM
   *  - top value by OCR1A
   *  - OC1B non-inverted output
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
    #ifdef HW_ENCODER
    if (Test < 3)                       /* just for test button usage */
    #endif
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
    #ifdef HW_ENCODER
    else if (Test == 3)                 /* rotary encoder: right turn */
    {
      if (Ratio <= 99) Ratio += 1;      /* +1% and limit to 100% */
    }
    else if (Test == 4)                 /* rotary encoder: left turn */
    {
      if (Ratio >= 1) Ratio -= 1;         /* -1% and limit to 0% */
    }
    #endif
    else                                /* long key press */
    {
      if (Ratio >= 5) Ratio -= 5;         /* -5% and limit to 0% */
    }

    /* calculate toggle value: (top * (ratio / 100)) - 1 */
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
 *   Frequency (squarewave) Generator
 * ************************************************************************ */


#ifdef SW_FREQ_GEN

/*
 *  create squarewave signal with variable frequency
 *  - use probe #2 (PB2, OC1B) as output
 *    and probe #1 + probe #3 as ground
 */

void FrequencyGenerator(void)
{
  uint8_t           Flag = 2;           /* loop control */
  uint8_t           Test;
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bitmask = 0;        /* prescaler bitmask */
  uint16_t          Prescaler;          /* timer prescaler */
  uint16_t          OldPrescaler;       /* old timer prescaler */
  uint16_t          Top;                /* counter's top value */
  uint16_t          Temp;
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

  ShortCircuit(0);                    /* make sure probes are not shorted */
  LCD_Clear();
  LCD_EEString2(FreqGen_str);         /* display: f Gen. */
  ToolInfo(PWM_Probes_str);           /* show probes used */

  /* probes 1 and 3 are signal ground, probe 2 is signal output */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe 1 & 3 */
  R_DDR = (1 << (TP2 * 2));             /* enable Rl for probe 2 */
  R_PORT = 0;                           /* pull down probe 2 initially */


  /*
   *  setup Timer1 for PWM with 50% duty cycle 
   *  - fast PWM mode 
   *  - top value by OCR1A
   *  - OC1B non-inverted output
   */

  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1) | (1 << COM1B0);
  TCCR1B = (1 << WGM13); // | (1 << WGM12);


  /*
   *  processing loop
   */

  /* start values for 1kHz */
  Index = 0;                       /* prescaler 1/1 */
  Prescaler = 1;                   /* prescaler 1/1 */
  Top = (CPU_FREQ / 1000) - 1;     /* top = f_MCU / (prescaler * f) - 1 */

  while (Flag > 0)
  {
    /* update prescaler */
    if (Flag >= 2)
    {
      OldPrescaler = Prescaler;         /* save old value */

      /* read new prescaler and bitmask from table */
      Prescaler = MEM_read_word(&T1_Prescaler_table[Index]);
      Bitmask = MEM_read_byte(&T1_Bitmask_table[Index]);

      /* auto-ranging: adjust top value for changed prescaler */
      if (Flag == 2)          /* lower prescaler / higher frequency */
      {
        /* increase top value by same factor as the prescaler decreased */
        Temp = OldPrescaler / Prescaler;
        Top *= Temp;  
      }
      else                    /* higher prescaler / lower frequency */
      {
        /* decrease top value by same factor as the prescaler increased */
        Temp = Prescaler / OldPrescaler;
        Top /= Temp;
      }

      Flag = 1;                         /* reset flag */
    }

    /* display frequency: f = f_MCU / (prescaler * (1 + top)) */
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
    LCD_Data('z');                 /* add z for Hz */

    /* update timer */
    TCCR1B = (1 << WGM13) | (1 << WGM12);    /* stop timer */
    TCNT1 = 0;                               /* reset counter */
    OCR1B = Top / 2;                         /* 50% duty cycle */
    OCR1A = Top;                             /* top value for frequency */
    TCCR1B = (1 << WGM13) | (1 << WGM12) | Bitmask;     /* start timer */

    /* user feedback */
    Test = TestKey(0, 0);          /* wait for key / rotary encoder */
    Temp = Enc.Velocity;           /* take turning velocity into account */

    if (Enc.Velocity > 1)          /* adjust steps based on frequency */
    {
      if (Index >= 1)              /* low frequencies */
      {
        Temp *= 10;                /* increase steps even more */
      }
      else if ((Index == 0) && (Top < 1000))  /* high frequencies */
      {
        Temp = 10;                 /* limit steps to 10 */
      }
      else                         /* default */
      {
        Temp *= 5;                 /* increase steps */
      }
    }

    if (Test == 3)            /* encoder right turn */
    {
      /* increase frequency / decrease top value */
      if (Top >= Temp)             /* no underflow */
      {
        Top -= Temp;                 /* decrease top value */
      }
      else                         /* underflow */
      {
        Top = 0;                     /* can't go below zero */
      }
      if (Top < 3) Top = 3;        /* enforce lower limit of top value */

      /* auto-ranging */
      if (Top < 0x03FF)            /* less than 10 bits */
      {
        if (Index > 0)             /* don't exceed lower prescaler limit */
        {
          Index--;                 /* decrease prescaler */
          Flag = 2;                /* signal decreased prescaler */
        }
      }
    }
    else if (Test == 4)       /* encoder left turn */
    {
      /* decrease frequency / increase top value */
      Value = (uint32_t)Top + Temp;
      if (Value <= 0x0000FFFE)     /* no overflow */
      {
        Top += Temp;                 /* increase top value */
      }
      else                         /* overflow */
      {
        Top = 0xFFFE;                /* can't go beyond 0xFFFE */
      }

      /* auto-ranging */
      if (Top > 0x7FFF)            /* more than 15 bits */
      {
        if (Index < 4)             /* don't exceed upper prescaler limit */
        {
          Index++;                 /* increase prescaler */
          Flag = 3;                /* signal increased prescaler */
        }
      }
    }
    else if (Test > 0)        /* key press */
    {
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

  #ifdef HW_RELAY
  ADC_DDR = (1<<TP_REF);           /* short circuit probes */
  #endif

  /* show tool info */
  LCD_Clear();
  LCD_EEString(ESR_str);           /* display: ESR */
  ToolInfo(ESR_Probes_str);        /* show probes used */
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
        LCD_Data('-');                  /* display "no value" */
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
 *  - frequency input PD4/T0
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
  LCD_Line2();
  LCD_Data('-');                   /* display "no value" */
  

  /*
   *  We use Timer1 for the gate time and Timer0 to count pulses of the
   *  unknown signal.
   */


  /*
      counter limit for Timer1
      - gate time in 탎
      - MCU cycles per 탎
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

  /* setup Timer0 */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* setup Timer1 */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */ 


  /* measurement loop */
  while (Flag > 0)
  {
    /* setup PD4 as input */
    Old_DDR = CONTROL_DDR;                /* save current settings */
    CONTROL_DDR &= ~(1 << PD4);           /* signal input */
    wait500us();                          /* settle time */

    /* update prescaler */
    Prescaler = MEM_read_word(&T1_Prescaler_table[Index]);
    Bitmask = MEM_read_byte(&T1_Bitmask_table[Index]);

    /* calculate compare value for Timer1 */
    Value = (CPU_FREQ / 1000000);         /* clock based MCU cycles per 탎 */
    Value *= GateTime;                    /* gatetime (in ms) */
    Value *= 1000;                        /* scale to 탎 */
    Value /= Prescaler;                   /* divide by prescaler */
    Top = (uint16_t)Value;                /* use lower 16 bit */

    /* start timers */
    Freq.Pulses = 0;                      /* reset pulse counter */
    Flag = 2;                             /* enter waiting loop */
    TCNT0 = 0;                            /* Timer0: set counter to 0 */
    TCNT1 = 0;                            /* Timer1: set counter to 0 */
    OCR1A = Top;                          /* Timer1: set value to compare with */
    sei();                                /* enable interrupts */
    TCCR1B = Bitmask;                     /* start Timer1, prescaler */
    TCCR0B = (1 << CS02) | (1 << CS01);   /* start Timer0, clock source: T0 falling edge */

    /* wait for timer1 or key press */
    while (Flag == 2)
    {
      if (TCCR1B == 0)                    /* Timer1 stopped by ISR */
      {
        Flag = 1;                         /* end waiting loop and signal Timer1 event */
      }
      else                                /* Timer1 still running */
      {
        /* check for key press */
        while (!(CONTROL_PIN & (1 << TEST_BUTTON)))
        {
          MilliSleep(50);                 /* take a nap */
          Flag = 0;                       /* end all loops */
        }

        if (Flag > 0) MilliSleep(100);    /* sleep for 100ms */

        /* I'd like to use TestKey() but ReadEncoder() produces some glitch
           which causes TCNT0 to be increased by 1 most times. */
      }
    }

    cli();                                /* disable interrupts */

    /* process measurement */
    CONTROL_DDR = Old_DDR;                /* restore old settings */

    if (Flag == 1)                        /* got valid measurement */
    {
      /* calculate frequency: f = pulses / gatetime */
      Freq.Pulses += TCNT0;             /* add counter of Timer0 to global counter */
      Value = Freq.Pulses;              /* number of pulses */
      Value *= 1000;                    /* scale gatetime to 탎 */
      Value /= GateTime;                /* divide by gatetime (in ms) */

      /* display frequency */
      LCD_ClearLine2();
      LCD_Data('f');                    /* display: f */
      LCD_Space();
      DisplayValue(Value, 0, 'H');      /* display frequency */
      LCD_Data('z');                    /* append "z" for Hz */

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
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */
}



/*
 *  ISR for overflow of Timer0
 */

ISR(TIMER0_OVF_vect, ISR_BLOCK) {
  /* this automatically clears ... */

  sei();                      /* allow nested interrupts */
  Freq.Pulses += 256;         /* add overflow to global counter */
}



/*
 *  ISR for a match of TCNT1 (Timer1) and OCR1A (Output Compare Register A)
 */

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
  /* this automatically clears the OCF1A flag in the Interrupt Flag Register */

  TCCR1B = 0;                 /* disable Timer1 */
  TCCR0B = 0;                 /* disable Timer0 */
}


#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef EXTRAS_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

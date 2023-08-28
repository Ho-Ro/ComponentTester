/* ************************************************************************
 *
 *   counter tools (hardware and software options)
 *
 *   (c) 2012-2023 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define TOOLS_COUNTER_C



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

/* pulse counter */
#if defined (HW_FREQ_COUNTER_EXT) || defined (HW_EVENT_COUNTER) || defined (HW_LC_METER)
  volatile uint32_t      Pulses;        /* number of pulses */
#elif defined (HW_FREQ_COUNTER_BASIC) || defined (HW_RING_TESTER)
  volatile uint16_t      Pulses;        /* number of pulses */
#endif

/* time counter */
#ifdef HW_EVENT_COUNTER
volatile uint8_t         TimeTicks;     /* tick counter */
volatile uint16_t        TimeCounter;   /* time counter */
#endif



/* ************************************************************************
 *   shared ISRs for counter tools
 *   - also used by Get_LC_Frequency() in tools_LC_Meter.c
 * ************************************************************************ */


#if defined (HW_FREQ_COUNTER_BASIC) || defined (HW_FREQ_COUNTER_EXT) || defined (HW_EVENT_COUNTER) || defined (HW_LC_METER) || defined (HW_RING_TESTER)

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
   *    (no nested interrupts)
   */

  Pulses += 256;              /* add overflow to global counter */
}

#endif



#if defined (HW_FREQ_COUNTER_BASIC) || defined (HW_FREQ_COUNTER_EXT) || defined (HW_LC_METER) || defined (HW_RING_TESTER)

/*
 *  ISR for match of Timer1's OCR1A (Output Compare Register A)
 *  - for gate time of frequency counter
 */

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
  /*
   *  hints:
   *  - the OCF1A interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   *    (no nested interrupts)
   */

  /* gate time has passed */
  TCCR1B = 0;                 /* disable Timer1 */
  TCCR0B = 0;                 /* disable Timer0 */

  /* break TestKey() processing */
  Cfg.OP_Control |= OP_BREAK_KEY;       /* set break signal */
}

#endif



/* ************************************************************************
 *   simple frequency counter
 * ************************************************************************ */


#ifdef HW_FREQ_COUNTER_BASIC

/*
 *  basic frequency counter
 *  - frequency input: T0
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 */

void FrequencyCounter(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test;               /* user feedback */
  uint8_t           Old_DDR;            /* old DDR state */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bits;               /* prescaler register bits */
  uint16_t          GateTime;           /* gate time in ms */
  uint16_t          Top;                /* top value for timer */
  uint32_t          Value;              /* temporary value */

  /* local constants for Flag */
  #define RUN_FLAG       1         /* run flag */
  #define WAIT_FLAG      2         /* enter/run waiting loop */
  #define GATE_FLAG      3         /* gatetime flag */
  #define SHOW_FREQ      4         /* display frequency */

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: Freq. Counter */
    Display_ColoredEEString(FreqCounter_str, COLOR_TITLE);
  #else
    Display_EEString(FreqCounter_str);  /* display: Freq. Counter */
  #endif


  /*
   *  We use Timer1 for the gate time and Timer0 to count pulses of the
   *  unknown signal. Max. frequency for Timer0 is 1/4 of the MCU clock.
   */

  Flag = RUN_FLAG;            /* enter measurement loop */

  /*
      auto ranging

      Timer1 top value (gate time)
      - top = gatetime * MCU_cycles / prescaler 
      - gate time in 탎
      - MCU cycles per 탎
      - top max. 2^16 - 1

      Frequency
      - f = pulses / gatetime
      - pulses = f * gatetime

      range         gate time  prescaler  MCU clock  pulses
      ----------------------------------------------------------
      <10kHz           1000ms       1024  > 16MHz    <10k
                       1000ms        256  <= 16MHz   <10k      
      10kHz-100kHz      100ms         64  all        1k-10k
      >100kHz            10ms          8  all        >1k (<50k)
   */

  /* start values for autoranging (assuming high frequency) */
  GateTime = 10;                   /* gate time 10ms */
  Index = 1;                       /* prescaler table index (prescaler 8:1) */

  /* set up Timer0 (pulse counter) */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 (gate time) */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */


  /*
   *  measurement loop
   */

  while (Flag > 0)
  {
    /* set up T0 as input (pin might be shared with display) */
    Old_DDR = COUNTER_DDR;              /* save current settings */
    COUNTER_DDR &= ~(1 << COUNTER_IN);  /* signal input */
    wait500us();                        /* settle time */

    /* update prescaler */
    Top = DATA_read_word(&T1_Prescaler_table[Index]);  /* prescaler value */
    Bits = DATA_read_byte(&T1_RegBits_table[Index]);   /* prescaler bits */

    /* calculate compare value for Timer1 (gate time) */
    /* top = gatetime * MCU_cycles / timer prescaler */
    Value = GateTime;                   /* gatetime (in ms) */
    /* * MCU cycles per 탎 and scale gatetime to 탎 */
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
    TCCR0B = (1 << CS02) | (1 << CS01); /* start Timer0: clock source T0 on falling edge */

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

        if (Test == KEY_TWICE)          /* two short key presses */
        {
          Flag = 0;                     /* end processing loop */
        }
      }
    }

    /* T0 pin might be shared with display */
    COUNTER_DDR = Old_DDR;              /* restore old settings */

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
       *    with 10ms gate time max. 50k pulses
       */

      Value = Pulses;                   /* number of pulses */
      Value *= 1000;                    /* scale to ms */
      Value /= GateTime;                /* divide by gatetime (in ms) */
      Flag = SHOW_FREQ;                 /* display frequency */

      /* autoranging */
      if (Pulses > 10000)               /* range overrun */
      {
        if (GateTime > 10)              /* upper range limit not reached yet */
        {
          GateTime /= 10;               /* 1000ms -> 100ms -> 10ms */
          Index--;                      /* one prescaler step down */
          #if CPU_FREQ > 16000000 
          if (Index == 3) Index--;      /* skip 256, use 64 */
          #endif
          Flag = RUN_FLAG;              /* don't display frequency */
        }
      }
      else if (Pulses < 1000)           /* range underrun */
      {
        if (GateTime < 1000)            /* lower range limit not reached yet */
        {
          GateTime *= 10;               /* 1ms -> 10ms -> 100ms -> 1000ms */
          Index++;                      /* one prescaler step up */
          #if CPU_FREQ > 16000000 
          if (Index == 3) Index++;      /* skip 256, use 1024 */
          #endif
          Flag = RUN_FLAG;              /* don't display frequency */
        }
      }

      /* prevent display of "0 Hz" */
      if (Pulses == 0)                  /* no signal or f too low */
      {
        Flag = RUN_FLAG;                /* don't display frequency */
      }
    }


    /*
     *  display frequency (in line #2)
     */

    LCD_ClearLine2();                   /* clear line #2 */
    Display_Char('f');                  /* display: f */
    Display_Space();

    if (Flag == SHOW_FREQ)              /* valid frequency */
    {
      Display_Value(Value, 0, 0);       /* display frequency */
      Display_EEString(Hertz_str);      /* display: Hz */
      Flag = RUN_FLAG;                  /* clear flag */
    }
    else                                /* invalid frequency */
    {
      Display_Minus();                  /* display: no value */
    }
  }


  /*
   *  clean up
   */

  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                 /* disable all interrupts for Timer1 */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef WAIT_FLAG
  #undef GATE_FLAG
  #undef SHOW_FREQ
}

#endif



/* ************************************************************************
 *   extended frequency counter
 * ************************************************************************ */


#ifdef HW_FREQ_COUNTER_EXT

/*
 *  extended frequency counter
 *  - uses frontend with buffer, prescaler and crystal oscillators
 *  - frequency input: T0 
 *  - control signals
 *    prescaler       - COUNTER_CTRL_DIV
 *    channel addr #0 - COUNTER_CTRL_CH0
 *    channel addr #1 - COUNTER_CTRL_CH1
 *  - prescaler
 *    0 - 1:1
 *    1 - 16:1 (or 32:1)
 *  - source channel address
 *    00 - buffered frequency input
 *    01 - unused
 *    10 - HF crystal oscillator
 *    11 - LF crystal oscillator
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 */

void FrequencyCounter(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           InDir;              /* input DDR state */
  uint8_t           CtrlDir;            /* control DDR state */
  uint8_t           Channel;            /* input channel */
  uint8_t           Range;              /* range ID */
  uint8_t           Div = 0;            /* frequency prescaler */
  uint8_t           Index;              /* prescaler table index */
  uint8_t           Bits = 0;           /* prescaler register bits */
  uint16_t          GateTime = 0;       /* gate time in ms */
  uint16_t          Top = 0;            /* top value for timer */
  unsigned char     *String = NULL;     /* string pointer (EEPROM) */
  uint32_t          MinPulses = 0;      /* minimim pulses for range */
//  uint32_t          MaxPulses = 0;      /* maximum pulses for range */
  uint32_t          Value;              /* temporary value */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run flag */
  #define WAIT_FLAG           0b00000010     /* wait flag */
  #define GATE_FLAG           0b00000100     /* gatetime flag */
  #define UPDATE_CHANNEL      0b00001000     /* update source channel */
  #define UPDATE_RANGE        0b00010000     /* update measurement range */
  #define SHOW_FREQ           0b00100000     /* display frequency */
  #define RESCAN_FLAG         0b01000000     /* rescan */
  #define SKIP_FREQ           0b10000000     /* skip display of f */

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: Freq. Counter */
    Display_ColoredEEString(FreqCounter_str, COLOR_TITLE);
  #else
    Display_EEString(FreqCounter_str);  /* display: Freq. Counter */
  #endif


  /*
   *  We use Timer1 for the gate time and Timer0 to count pulses of the
   *  unknown signal. Max. frequency for Timer0 is 1/4 of the MCU clock.
   */

  /*
      auto ranging

      Timer1 top value (gate time)
      - top = gatetime * MCU_cycles / timer prescaler
      - gate time in 탎
      - MCU cycles per 탎
      - top max. 2^16 - 1

      Frequency
      - f = f-prescaler * pulses / gatetime
      - pulses = f * gatetime / f-prescaler

                    gate    timer      MCU       frequency
      range         time    prescaler  clock     prescaler  pulses
      ----------------------------------------------------------------
      n/a           3000ms       1024  all
      <100kHz       1000ms       1024  > 16MHz         1:1  <100k
                    1000ms        256  <= 16MHz        1:1  <100k
      100kHz-1MHz    100ms         64  all             1:1  10k-100k
      >1MHz          100ms         64  all            16:1  >6250 (<500k)
                     100ms         64  all            32:1  >3125 (<500k)
   */

  /* set up control lines */
  CtrlDir = COUNTER_CTRL_DDR;      /* get current direction */
  /* set to output mode */
  COUNTER_CTRL_DDR |= (1 << COUNTER_CTRL_DIV) | (1 << COUNTER_CTRL_CH0) | (1 << COUNTER_CTRL_CH1);

  /* set up Timer0 (pulse counter) */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 (gate time) */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */

  /* set start values */
  Channel = 0;                     /* source channel: ext. frequency */
  Range = 2;                       /* start with highest range */
  Flag = RUN_FLAG | UPDATE_CHANNEL | UPDATE_RANGE;     /* set control flags */
  #ifdef UI_QUARTZ_CRYSTAL
  Check.Symbol = SYMBOL_CRYSTAL;   /* set symbol ID */
  #endif


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  set and display source channel
     */

    if (Flag & UPDATE_CHANNEL)          /* update channel settings */
    {
      switch (Channel)             /* get channel specifics */
      {
        case 0:     /* buffered frequency input */
          String = (unsigned char *)FreqInput_str;
          Test = 0;
          break;

        case 1:     /* high frequency crystal oscillator */
          String = (unsigned char *)HF_Crystal_str;
          Test = (1 << COUNTER_CTRL_CH1);
          break;

        case 2:     /* low frequency crystal oscillator */
          String = (unsigned char *)LF_Crystal_str;
          Test = (1 << COUNTER_CTRL_CH1) | (1 << COUNTER_CTRL_CH0);
          break;
      }

      /* set source channel */
      InDir = COUNTER_CTRL_PORT;        /* get current state */
      InDir &= ~((1 << COUNTER_CTRL_CH1) | (1 << COUNTER_CTRL_CH0));  /* clear channel lines */
      InDir |= Test;                    /* set channel lines */
      COUNTER_CTRL_PORT = InDir;        /* update port */

      /* display source channel (in line #3) */ 
      LCD_ClearLine3();                      /* clear line #3 */
      Display_EEString(CounterChannel_str);  /* display: "Ch" */
      Display_Space();
      Display_EEString(String);              /* display channel name */

      #ifdef UI_QUARTZ_CRYSTAL
      if (Channel == 0)                 /* buffered frequency input */
      {
        Clear_Symbol(4);                /* clear symbol in line #4 */
      }
      else                              /* HF or LF crystal oscillator */
      {
        Display_FancySemiPinout(4);     /* display crystal symbol in line #4 */
      }
      #endif

      /* restart scan in top range */
      Range = 2;                             /* select top range */
      Flag |= UPDATE_RANGE;                  /* update range */
      Flag &= ~(RESCAN_FLAG | SKIP_FREQ);    /* reset rescan */

      Flag &= ~UPDATE_CHANNEL;          /* clear flag */
    }


    /*
     *  settings for ranges
     */

    if (Flag & UPDATE_RANGE)       /* update range settings */
    {
      switch (Range)               /* get range specifics */
      {
        case 0:     /* <100kHz */
          Div = 1;                 /* frequency prescaler 1:1 */
          #if CPU_FREQ <= 16000000
            Index = 3;             /* table index 3: 256:1 */
          #else
            Index = 4;             /* table index 4: 1024:1 */
          #endif
          GateTime = 1000;         /* gate time: 1000ms */
          MinPulses = 0;           /* lower limit: none */
//          MaxPulses = 100000;      /* upper limit: 100k */
          break;

        case 1:     /* 100kHz-1MHz */
          Div = 1;                 /* frequency prescaler 1:1 */
          Index = 2;               /* table index 2: 64:1 */
          GateTime = 100;          /* gate time: 100ms */
          MinPulses = 10000;       /* lower limit: 10k */
//          MaxPulses = 100000;      /* upper limit: 100k */
          break;

        case 2:     /* >1MHz */
          Div = FREQ_COUNTER_PRESCALER; /* frequency prescaler 16:1 or 32:1 */
          Index = 2;               /* table index 2: 64:1 */
          GateTime = 100;          /* gate time: 100ms */
          #if FREQ_COUNTER_PRESCALER == 16
            MinPulses = 6250;      /* lower limit: 6250 */
          #elif FREQ_COUNTER_PRESCALER == 32
            MinPulses = 3125;      /* lower limit: 3125 */
          #endif
//          MaxPulses = 0;           /* upper limit: none */
          break;
      }

      /* update Timer1 prescaler */
      Top = DATA_read_word(&T1_Prescaler_table[Index]);     /* prescaler value */
      Bits = DATA_read_byte(&T1_RegBits_table[Index]);      /* prescaler bits */


      /* calculate compare value for Timer1 (gate time) */
      /* top = gatetime * MCU_cycles / timer prescaler */
      Value = GateTime;                 /* gatetime (in ms) */
      /* * MCU cycles per 탎 and scale gatetime to 탎 */
      Value *= (MCU_CYCLES_PER_US * 1000);
      Value /= Top;                     /* divide by timer prescaler */
      Top = (uint16_t)Value;            /* use lower 16 bit */


      /* update frequency counter prescaler */
      if (Div == FREQ_COUNTER_PRESCALER)     /* 16:1 / 32:1 */
      {
        /* enable frequency prescaler */
        COUNTER_CTRL_PORT |= (1 << COUNTER_CTRL_DIV);       /* set pin high */
      }
      else                                   /* 1:1 */
      {
        /* disable frequency prescaler */
        COUNTER_CTRL_PORT &= ~(1 << COUNTER_CTRL_DIV);      /* set pin low */
      }

      Flag &= ~UPDATE_RANGE;            /* clear flag */
    }  


    /* set up T0 as input */
    InDir = COUNTER_DDR & (1 << COUNTER_IN);      /* get current direction */
    COUNTER_DDR &= ~(1 << COUNTER_IN);  /* set to input mode */
    wait500us();                        /* settle time */


    /* start timers */
    Flag |= WAIT_FLAG;                  /* enter waiting loop */
    Pulses = 0;                         /* reset pulse counter */
    TCNT0 = 0;                          /* Timer0: reset pulse counter */
    TCNT1 = 0;                          /* Timer1: reset gate time counter */
    OCR1A = Top;                        /* Timer1: set gate time */
    TCCR1B = Bits;                      /* start Timer1: prescaler */
    TCCR0B = (1 << CS02) | (1 << CS01); /* start Timer0: clock source T0 on falling edge */


    /*
     *  wait for timer1 or user feedback
     */

    while (Flag & WAIT_FLAG)
    {
      if (TCCR1B == 0)                  /* Timer1 stopped by ISR */
      {
        Flag |= GATE_FLAG;                   /* signal Timer1 event */
        Flag &= ~WAIT_FLAG;                  /* end waiting loop */
      }
      else                              /* Timer1 still running */
      {
        /* wait for user feedback */
        Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

        if (Test == KEY_SHORT)          /* short key press */
        {
          /* select next source channel */
          if (Channel < 2) Channel++;        /* next channel */
          else Channel = 0;                  /* overrun */

          Flag |= UPDATE_CHANNEL;            /* update channel */
          Flag &= ~WAIT_FLAG;                /* end waiting loop */
        }
        else if (Test == KEY_TWICE)     /* two short key presses */
        {
          Flag = 0;                          /* end processing loop */
        }
        #ifdef HW_KEYS
        else if (Test == KEY_RIGHT)     /* right key */
        {
          if (Channel < 2) Channel++;        /* next channel */
          else Channel = 0;                  /* overrun */

          Flag |= UPDATE_CHANNEL;            /* update channel */
          Flag &= ~WAIT_FLAG;                /* end waiting loop */
        }
        else if (Test == KEY_LEFT)      /* left key */
        {
          if (Channel > 0) Channel--;        /* previous channel */
          else Channel = 2;                  /* underrun */

          Flag |= UPDATE_CHANNEL;            /* update channel */
          Flag &= ~WAIT_FLAG;                /* end waiting loop */
        }
        #endif
      }
    }

    if (InDir)                     /* restore old setting for T0 */
    {
      COUNTER_DDR |= (1 << COUNTER_IN);      /* set to output mode */
    }

    Cfg.OP_Control &= ~OP_BREAK_KEY;    /* clear break signal (just in case) */


    /*
     *  process measurement
     */

    if (Flag & GATE_FLAG)               /* got measurement */
    {
      /* total sum of pulses during gate period */
      Pulses += TCNT0;                  /* add counter of Timer0 */

      /*
       *  autoranging
       */

      if (Pulses < MinPulses)           /* range underrun */
      {
        if (Range > 0)                  /* not lowest range yet */
        {
          Range--;                      /* change to lower range */
          Flag |= UPDATE_RANGE;         /* update range */
        }
      }
      #if 0
      else if (Pulses > MaxPulses)      /* range overrun */
      {
        if (Range < 2)                  /* not highest range yet */
        {
          Range++;                      /* change to higher range */
          Flag |= UPDATE_RANGE;         /* update range */
        }
      }
      #endif

      /* show frequency only when not switching ranges */
      if (! (Flag & UPDATE_RANGE))      /* no change of range */
      {
        /* prevent display of "0 Hz" (no signal or f too low) */
        if (Pulses)                     /* got signal */
        {
          Flag |= SHOW_FREQ;            /* show frequency */
        }

        /* manage rescan */
        Flag &= ~(RESCAN_FLAG | SKIP_FREQ);       /* reset flags */

        if (Range < 2)                            /* not top range */
        {
          Range = 2;                              /* change to top range */
          Flag |= (UPDATE_RANGE | RESCAN_FLAG);   /* update range and rescan */
        }
      }


      /*
       *  calculate frequency
       *  - f = pulses * f-prescaler / gatetime
       *  - 20MHz MCU: 5M pulses per second at maximum
       *    with 100ms gate time max. 500k pulses
       */

      if (Div == 1)                     /* f-prescaler 1:1 */
      {
        /* no overflow possible */
        Pulses *= 1000;                 /* scale to ms */
        Pulses /= GateTime;             /* divide by gatetime (in ms) */
      }
      else                              /* f-prescaler 16:1 or 32:1 */
      {
        /* prevent overflow */
        Pulses *= 100;                  /* scale to 10ms */
        Pulses *= Div;                  /* * f-prescaler */
        Pulses /= (GateTime / 10);      /* divide by gatetime (in 10ms) */
      }

      Flag &= ~GATE_FLAG;          /* clear flag */
    }


    /*
     *  display frequency (in line #2)
     */

    if (! (Flag & SKIP_FREQ))      /* update frequency display */
    {
      LCD_ClearLine2();            /* clear line #2 */
      Display_Char('f');           /* display: f */
      Display_Space();

      if (Flag & SHOW_FREQ)        /* valid frequency */
      {
        /* determine prefix */
        Test = 0;                  /* dot position */
        Index = 0;                 /* unit char */

        if (Pulses >= 1000000)     /* f >= 1MHz */
        {
          Test = 6;                     /* 10^6 */
          Index = 'M';                  /* M for mega */
        }
        else if (Pulses >= 1000)   /* f >= 1kHz */
        {
          Test = 3;                     /* 10^3 */
          Index = 'k';                  /* k for kilo */
        }

        /* display frequency */
        Display_FullValue(Pulses, Test, Index);
        Display_EEString(Hertz_str);    /* display: "Hz" */

        Flag &= ~SHOW_FREQ;             /* clear flag */
      }
      else                         /* invalid frequency */
      {
        Display_Minus();           /* display: no value */
      }

      /* manage rescan */
      if (Flag & RESCAN_FLAG)      /* in rescan mode */
      {
        /* prevent any updates while in rescan mode */
        Flag |= SKIP_FREQ;         /* skip frequency display */
      }
    }
  }


  /*
   *  clean up
   */

  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                 /* disable all interrupts for Timer1 */

  /* filter control lines which were in input mode */ 
  CtrlDir ^= (1 << COUNTER_CTRL_DIV) | (1 << COUNTER_CTRL_CH0) | (1 << COUNTER_CTRL_CH1);
  CtrlDir &= (1 << COUNTER_CTRL_DIV) | (1 << COUNTER_CTRL_CH0) | (1 << COUNTER_CTRL_CH1);
  COUNTER_CTRL_DDR &= ~CtrlDir;         /* set former direction */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef WAIT_FLAG
  #undef GATE_FLAG
  #undef UPDATE_CHANNEL
  #undef UPDATE_RANGE
  #undef SHOW_FREQ
  #undef RESCAN_FLAG
  #undef SKIP_FREQ
}

#endif



/* ************************************************************************
 *   ring tester for high Q chokes and transformers
 * ************************************************************************ */


#ifdef HW_RING_TESTER

/*
 *  ring tester (LOPT/FBT tester)
 *  - uses frontend for ring detection
 *  - counter input: T0
 *  - control via probes (RING_TESTER_PROBES)
 *    probe #1: +5V
 *    probe #2: pulse output
 *    probe #3: Gnd
 *  - control via dedicated pin (RING_TESTER_PIN)
 *    RINGTESTER_OUT: pulse output
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 */

void RingTester(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test;               /* user feedback */
  uint8_t           Old_DDR;            /* old DDR state */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG       1         /* run flag */
  #define WAIT_FLAG      2         /* enter/run waiting loop */
  #define GATE_FLAG      3         /* gatetime flag */
  #define SHOW_RINGS     4         /* display rings */

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: Ring Tester */
    Display_ColoredEEString(RingTester_str, COLOR_TITLE);
  #else
    Display_EEString(RingTester_str);   /* display: Ring Tester */
  #endif

  #ifdef RING_TESTER_PROBES
  ProbePinout(PROBES_RINGTESTER);       /* show probes used */
  #endif

  /*
   *  working priciple:
   *  - a shorted or bad inductor has a low Q value
   *  - a trigger pulse causes a damped oscillation in a tank circuit
   *  - the higher the inductor's Q the longer the oscillation lasts
   *  - so we simply count the number of rings until the oscillation
   *    fades away
   */

  /*
   *  set up pulse output
   */

  #ifdef RING_TESTER_PIN
  /* use dedicated pin */
  RINGTESTER_PORT &= ~(1 << RINGTESTER_OUT);      /* low by default */
  RINGTESTER_DDR |= (1 << RINGTESTER_OUT);        /* enable output */
  #endif

  #ifdef RING_TESTER_PROBES
  /* set probes: probe #1 - Vcc / probe #2 - Rl - pulse out / probe #3 - Gnd */
  R_PORT = 0;                                /* pull down probe #2 */
  R_DDR = (1 << R_RL_2);                     /* enable Rl for probe #2 */
  ADC_PORT = (1 << TP1);                     /* pull up #1, pull down #3 */
  ADC_DDR = (1 << TP1) | (1 << TP3);         /* enable direct pull for #1 and #3 */
  #endif


  /*
   *  We use Timer1 for the gate time and Timer0 to count rings.
   *  Max. frequency for Timer0 is 1/4 of the MCU clock.
   */

  Flag = RUN_FLAG;            /* enter measurement loop */

  /*
      Fixed gate time of 10ms.

      Timer1 top value (gate time)
      - top = gatetime * MCU_cycles / prescaler 
      - gate time in 탎
      - MCU cycles per 탎
      - top max. 2^16 - 1

      Using prescaler 8:1 - register bits: (1 << CS11)
   */

  /* set up Timer0 (ring counter) */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 (gate time) */
  TCCR1A = 0;                      /* normal mode (count up) */
  TIFR1 = (1 << OCF1A);            /* clear output compare A match flag */
  TIMSK1 = (1 << OCIE1A);          /* enable output compare A match interrupt */
  /* top = gatetime * MCU_cycles / timer prescaler */
  OCR1A = (uint16_t)((10000UL * MCU_CYCLES_PER_US) / 8);


  /*
   *  measurement loop
   */

  while (Flag > 0)
  {
    /* set up T0 as input (pin might be shared with display) */
    Old_DDR = COUNTER_DDR;              /* save current settings */
    COUNTER_DDR &= ~(1 << COUNTER_IN);  /* signal input */
    wait500us();                        /* settle time */

    /* start timers */
    Pulses = 0;                         /* reset pulse counter */
    Flag = WAIT_FLAG;                   /* enter waiting loop */
    TCNT0 = 0;                          /* Timer0: reset ring counter */
    TCNT1 = 0;                          /* Timer1: reset gate time counter */
    TCCR1B = (1 << CS11);               /* start Timer1: prescaler 8:1 */
    TCCR0B = (1 << CS02) | (1 << CS01); /* start Timer0: clock source T0 on falling edge */


    /*
     *  create trigger pulse (2 ms)
     *  - will create one pseudo ring since timers are already running
     */

    #ifdef RING_TESTER_PIN
    RINGTESTER_PORT |= (1 << RINGTESTER_OUT);     /* set pin high */
    wait2ms();                                    /* wait 2 ms */
    RINGTESTER_PORT &= ~(1 << RINGTESTER_OUT);    /* set pin low */
    #endif

    #ifdef RING_TESTER_PROBES
    R_PORT = (1 << R_RL_2);             /* pull up probe #2 via Rl */
    wait2ms();                          /* wait 2 ms */
    R_PORT = 0;                         /* pull down probe #2 */
    #endif

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

        if (Test == KEY_TWICE)          /* two short key presses */
        {
          Flag = 0;                     /* end processing loop */
        }
      }
    }

    /* T0 pin might be shared with display */
    COUNTER_DDR = Old_DDR;              /* restore old settings */

    Cfg.OP_Control &= ~OP_BREAK_KEY;    /* clear break signal (just in case) */


    /*
     *  process measurement
     */

    if (Flag == GATE_FLAG)              /* got measurement */
    {
      /* total sum of rings during gate period */
      Pulses += TCNT0;                  /* add counter of Timer0 */

      /* consider first/pseudo ring created by trigger pulse */
      if (Pulses > 0)                   /* sanity check */
      {
        Pulses--;                       /* subtract one ring */
      }

      Flag = SHOW_RINGS;                /* display rings */
    }


    /*
     *  display number of rings (in line #2)
     */

    LCD_ClearLine2();                   /* clear line #2 */
    Display_Char('n');                  /* display: n */
    Display_Space();

    if (Flag == SHOW_RINGS)             /* valid number of rings */
    {
      Display_Value(Pulses, 0, 0);      /* display rings */
      Flag = RUN_FLAG;                  /* clear flag */
    }
    else                                /* invalid number of rings */
    {
      Display_Minus();                  /* display: no value */
    }


    /*
     *  add some delay to slow down the update rate
     *  and to smooth the UI
     */

    /* check test button using a timeout of 400 ms */
    Test = TestKey(400, CHECK_KEY_TWICE | CHECK_BAT);

    /* catch double press for exit */
    if (Test == KEY_TWICE)              /* two short key presses */
    {
      Flag = 0;                         /* end processing loop */
    }
  }


  /*
   *  clean up
   */

  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                 /* disable all interrupts for Timer1 */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef WAIT_FLAG
  #undef GATE_FLAG
  #undef SHOW_RINGS
}

#endif



/* ************************************************************************
 *   counter: event counter
 * ************************************************************************ */


#ifdef HW_EVENT_COUNTER


/*
 *  ISR for match of Timer1's OCR1B (Output Compare Register B)
 *  - for time ticks of event counter
 */

ISR(TIMER1_COMPB_vect, ISR_BLOCK)
{
  /*
   *  hints:
   *  - the OCF1B interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   *    (no nested interrupts)
   */

  /* time ticks */
  TimeTicks++;                     /* got another tick */
  if (TimeTicks >= 5)              /* 5 ticks = 1 second */
  {
    TimeTicks = 0;                 /* reset tick counter */
    TimeCounter++;                 /* got another second */
  }

  TIFR1 = (1 << OCF1A);                 /* clear output compare A match flag */

  /* break TestKey() processing */
  Cfg.OP_Control |= OP_BREAK_KEY;       /* set break signal */
}



/*
 *  event counter
 *  - counter input: T0
 *    needs to be dedicated pin (not in parallel with display!)
 *  - requires additional keys (e.g. rotary encoder) and
 *    display with more than 5 lines
 *  - requires idle sleep mode to keep timers running when MCU is sleeping
 *  - requires MCU clock of 8, 16 or 20MHz
 */

void EventCounter(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test;               /* user feedback */
  uint8_t           CounterMode;        /* counter mode */
  uint8_t           Item;               /* UI item */
  uint8_t           Show;               /* display control */
  uint8_t           Temp;               /* temp. value */
  uint16_t          Step;               /* step size */
  unsigned char     *String = NULL;     /* string pointer (EEPROM) */
  uint16_t          TimeTrigger;        /* time limit/trigger */
  uint32_t          EventsTrigger;      /* events limit/trigger */
  uint32_t          Events;             /* events */


  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run flag */
  #define WAIT_FLAG           0b00000010     /* wait flag */
  #define IDLE_FLAG           0b00000100     /* idle flag (not counting) */
  #define DELAY_FLAG          0b00001000     /* delay flag */
  #define START_COUNTING      0b00010000     /* start counting */
  #define MANAGE_COUNTING     0b00100000     /* manage counting */
  #define STOP_COUNTING       0b01000000     /* stop counting */

  /* local constants for CounterMode */
  #define MODE_COUNT          1         /* count events and time (start/stop) */
  #define MODE_TIME           2         /* count events during given time period */
  #define MODE_EVENTS         3         /* count time for given number of events */

  /* local constants for Item */
  #define UI_COUNTERMODE      1         /* counter mode */
  #define UI_EVENTS           2         /* events */
  #define UI_TIME             3         /* time */
  #define UI_STARTSTOP        4         /* start/stop */

  /* local constants for Show (follows UI items, bitfield) */
  #define SHOW_MODE           0b00000001     /* show mode */
  #define SHOW_EVENTS         0b00000010     /* show events */
  #define SHOW_TIME           0b00000100     /* show time */
  #define SHOW_STARTSTOP      0b00001000     /* show start/stop */

  /* local constants for defaults and maximums */
  #define DEFAULT_TIME        60             /* one minute */
  #define DEFAULT_EVENTS      100            /* ? */
  #define MAX_TIME            43200          /* 12h (in seconds) */
  #define MAX_EVENTS          4000000000     /* ? */

  /* show flags based on item number */
  uint8_t UI_Index[4] = { SHOW_MODE, SHOW_EVENTS, SHOW_TIME, SHOW_STARTSTOP };

  #ifdef EVENT_COUNTER_TRIGGER_OUT
  /*
   *  init probe pins:
   *  - probe #1: Gnd
   *  - probe #2: trigger output (default: low)
   *  - probe #3: Gnd
   */

  /* set probes: probe-1 -- Gnd / probe-2 -- Rl -- Gnd / probe-3 -- Gnd */
  ADC_PORT = 0;                         /* pull down directly: */
  ADC_DDR = (1 << TP1) | (1 << TP3);    /* probe #1 and probe #3 */
  R_PORT = 0;                           /* pull down: */
  R_DDR = (1 << R_RL_2);                /* probe #2 via Rl */
  #endif

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: Event Counter */
    Display_ColoredEEString(EventCounter_str, COLOR_TITLE);
  #else
    Display_EEString(EventCounter_str); /* display: Event Counter */
  #endif


  /*
   *  We use Timer1 for the time period and Timer0 to count the events.
   *  Max. event frequency for Timer0 is 1/4 of the MCU clock.
   */

  /*
   *  MCU     pre-     top     top
   *  Clock   scaler   1s      0.2s
   *   1MHz   1:64     15625    3125  (not supported)
   *   8MHz   1:256    31250    6250
   *  16MHz   1:256    62500   12500
   *  20MHz   1:256    78125   15625
   *
   *  - top = (f_MCU / (f_tick * prescaler)) - 1
   *        = (f_MCU * t_tick / prescaler) - 1
   *  - t_tick = 0.2s
   */

  #define TOP       (CPU_FREQ / (5 * 256)) - 1

  /* set up Timer0 (event counter) */
  TCCR0A = 0;                      /* normal mode (count up) */
  TIFR0 = (1 << TOV0);             /* clear overflow flag */
  TIMSK0 = (1 << TOIE0);           /* enable overflow interrupt */

  /* set up Timer1 (time ticks) */
  TCCR1A = 0;                      /* CTC mode */
  TIFR1 = (1 << OCF1A) | (1 << OCF1B);  /* clear output compare A & B match flag */
  TIMSK1 = (1 << OCIE1B);          /* enable output compare B match interrupt */
  OCR1B = TOP;                     /* set top value for time tick */
  OCR1A = TOP;                     /* same for CTC */

  /* set up T0 as input (just in case) */
  COUNTER_DDR &= ~(1 << COUNTER_IN);    /* set to input mode */
  wait500us();                          /* settle time */

  /* set start values */
  Events = 0;                      /* make compiler happy */
  EventsTrigger = DEFAULT_EVENTS;  /* set default value */
  TimeTrigger = DEFAULT_TIME;      /* set default value */
  Step = 0;                        /* make compiler happy */
  CounterMode = MODE_COUNT;        /* default mode: count */
  Item = UI_COUNTERMODE;           /* select start item */
  Flag = RUN_FLAG | IDLE_FLAG;     /* set control flags */
  /* display everything at startup */
  Show = SHOW_MODE | SHOW_EVENTS | SHOW_TIME | SHOW_STARTSTOP;


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  start counting
     */

    if (Flag & START_COUNTING)
    {
      /* reset counters */
      Pulses = 0;                  /* pulse counter (ISR) */
      Events = 0;                  /* total value for events */
      TimeTicks = 0;               /* counter for ticks */
      TimeCounter = 0;             /* total value for time (s) */
      TCNT0 = 0;                   /* Timer0: reset event/pulse counter */
      TCNT1 = 0;                   /* Timer1: reset time counter */      

      /* start counters */
      /* start Timer1: prescaler 1:256, CTC mode */
      TCCR1B = (1 << CS12) | (1 << WGM12);
      /* start Timer0: clock source T0 on rising edge */
      TCCR0B = (1 << CS02) | (1 << CS01) | (1 << CS00);

      #ifdef EVENT_COUNTER_TRIGGER_OUT
      /* set trigger output high */
      R_PORT = (1 << R_RL_2);      /* pull up probe #2 via Rl */
      #endif

      Flag &= ~(START_COUNTING | IDLE_FLAG);      /* clear flags */

      /* update display of events and time (clear trigger values) */
      Show |= SHOW_EVENTS | SHOW_TIME;
    }


    /*
     *  manage counting
     *  - run for each time tick (Timer1)
     */

    if (Flag & MANAGE_COUNTING)
    {
      /* time counters are managed by ISR */

      /* events: get current value */
      Events = Pulses;                  /* get pulses */
      Events += TCNT0;                  /* add counter */

      /* prevent overflow */
      if ((TimeCounter >= MAX_TIME) ||
          (Events >= MAX_EVENTS))
      {
        /* reached maximum */
        Flag |= STOP_COUNTING;          /* stop counting */
      }

      /* manage trigger */
      if (CounterMode == MODE_TIME)          /* time mode */
      {
        if (TimeCounter >= TimeTrigger)      /* limit exeeded */
        {
          Flag |= STOP_COUNTING;        /* stop counting */
        }
      }
      else if (CounterMode == MODE_EVENTS)   /* events mode */
      {
        if (Events >= EventsTrigger)    /* limit exeeded */
        {
          Flag |= STOP_COUNTING;        /* stop counting */
        }
      }

      Flag &= ~MANAGE_COUNTING;         /* clear flag */

      /* each second */
      if (TimeTicks == 0)               /* full second */
      { 
        Show |= SHOW_EVENTS | SHOW_TIME;     /* show events and time */
      }
    }


    /*
     *  stop counting (part 1)
     */

    if (Flag & STOP_COUNTING)
    {
      /* stop counters */
      TCCR1B = 0;                  /* disable Timer1 */
      TCCR0B = 0;                  /* disable Timer0 */

      #ifdef EVENT_COUNTER_TRIGGER_OUT
      /* set trigger output low */
      R_PORT = 0;                  /* pull down probe #2 via Rl */
      #endif

      /* flags are reset later on to allow output of results */

      /* display current values for events and time */
      Show |= SHOW_EVENTS | SHOW_TIME | SHOW_STARTSTOP;
    }


    /*
     *  display counter mode
     */

    if (Show & SHOW_MODE)          /* display mode */
    {
      switch (CounterMode)              /* get mode specifics */
      {
        case MODE_COUNT:                /* count time and events */
          String = (unsigned char *)Count_str;
          break;

        case MODE_TIME:                 /* given time period */
          String = (unsigned char *)Time_str;
          break;

        case MODE_EVENTS:               /* given number of events */
          String = (unsigned char *)Events_str;
          break;
      }

      /* display mode (in line #2) */
      LCD_ClearLine2();                      /* clear line #2 */
      MarkItem(UI_COUNTERMODE, Item);        /* mark item if selected */
      Display_EEString(String);              /* display mode name */

      Show |= SHOW_TIME | SHOW_EVENTS;       /* update display of trigger values */
    }


    /*
     *  display events
     */

    if (Show & SHOW_EVENTS)        /* display events */
    {
      LCD_ClearLine3();                 /* clear line #3 */
      MarkItem(UI_EVENTS, Item);        /* mark item if selected */
      Display_Char('n');                /* display: n */
      Display_Space();

      if (Flag & IDLE_FLAG)        /* not counting */
      {
        if (CounterMode == MODE_EVENTS)      /* events mode */
        {
          /* display trigger value */
          Display_FullValue(EventsTrigger, 0, 0);
        }
      }
      else                         /* counting */
      {
        /* display events counter */
        Display_FullValue(Events, 0, 0);
      }
    }


    /*
     *  display time
     */

    if (Show & SHOW_TIME)          /* display time */
    {
      LCD_ClearLine(4);                      /* clear line #4 */
      LCD_CharPos(1, 4);                     /* go to start of line #4 */
      MarkItem(UI_TIME, Item);               /* mark item if selected */
      Display_Char('t');                     /* display: t */
      Display_Space();

      if (Flag & IDLE_FLAG)        /* not counting */
      {
        if (CounterMode == MODE_TIME)        /* time mode */
        {
          /* display trigger value */
          Display_FullValue(TimeTrigger, 0, 's');
        }
      }
      else                         /* counting */
      {
        /* display time elapsed */
        Display_FullValue(TimeCounter, 0, 's');
      }
    }


    /*
     *  stop counting (part 2)
     */

    if (Flag & STOP_COUNTING)
    {
      /* reset flags */
      Flag &= ~STOP_COUNTING;           /* clear flag */
      Flag |= IDLE_FLAG;                /* set idle flag */
    }


    /*
     *  display start/stop
     */

    if (Show & SHOW_STARTSTOP)     /* display start/stop */
    {
      if (Flag & IDLE_FLAG)        /* display: start */
      {
        String = (unsigned char *)Start_str;
      }
      else                         /* display: stop */
      {
        String = (unsigned char *)Stop_str;
      }

      LCD_ClearLine(5);                      /* clear line #5 */
      LCD_CharPos(1, 5);                     /* go to start of line #5 */
      MarkItem(UI_STARTSTOP, Item);          /* mark item if selected */
      Display_EEString(String);              /* display start/stop */
    }


    /* smooth UI after long key press */
    if (Flag & DELAY_FLAG)
    {
      SmoothLongKeyPress();        /* delay next key press */

      Flag &= ~DELAY_FLAG;         /* clear flag */
    }

    /* update display control flag */
    if (Flag & IDLE_FLAG)               /* not counting */
    {
      /* set display control flag based on currently selected item */
      Show = UI_Index[Item - 1];
    }
    else                                /* counting */
    {
      /* reset display control */
      Show = 0;
    }


    /*
     *  wait for user feedback or Timer1 (via OP_BREAK_KEY)
     */

    Flag |= WAIT_FLAG;             /* enter waiting loop */

    while (Flag & WAIT_FLAG)
    {
      /* wait for user feedback */
      Test = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

      /* consider rotary encoder's turning velocity (1-7) */
      Temp = UI.KeyStep;                /* get velocity */
      Step = Temp;
      if (Temp > 1)                     /* larger step */
      {
        /* increase step size based on turning velocity */
        if (Item == UI_TIME)            /* time */
        {
          /* 16 bit value */
          if (Temp <= 3) Step = 10;
          else if (Temp <= 5) Step = 120;
          else Step = 1800;
        }
        else if (Item == UI_EVENTS)     /* events */
        {
          /* 32 bit value - simply too large */
          if (Temp <= 3) Step = 10;
          else if (Temp <= 5) Step = 100;
          else Step = 1000;
        }
      }

      if (Test == KEY_TIMEOUT)          /* timeout by OP_BREAK_KEY */
      {
        Flag |= MANAGE_COUNTING;        /* manage counting */
        Flag &= ~WAIT_FLAG;             /* end waiting loop */
      }
      else if (Test == KEY_SHORT)       /* short key press */
      {
        /* switch to next item */
        if (Flag & IDLE_FLAG)           /* when not counting */
        {
          if (Item < UI_STARTSTOP)      /* not last item */
          {
            /* go to next item */
            Item++;                     /* next one */

            /* special rules */
            if (CounterMode == MODE_COUNT)        /* counter mode */
            {
              /* skip events and time */
              if (Item < UI_STARTSTOP) Item = UI_STARTSTOP;
            }
            else if (CounterMode == MODE_EVENTS)  /* events mode */
            {
              /* skip time */
              if (Item == UI_TIME) Item = UI_STARTSTOP;
            }
            else                                  /* time mode */
            {
              /* skip events */
              if (Item == UI_EVENTS) Item = UI_TIME;
            }
          }
          else                          /* last item */
          {
            /* go to first item */
            Item = UI_COUNTERMODE;
          }

          /* update display flags (old and new item) */
          Test = UI_Index[Item - 1];    /* get new display flag */
          Show |= Test;                 /* add new display flag */

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
      }
      else if (Test == KEY_LONG)        /* long key press */
      {
        if (Item == UI_STARTSTOP)       /* start/stop selected */
        {
          if (Flag & IDLE_FLAG)         /* not counting */
          {
            /* start counting */
            Flag |= START_COUNTING | DELAY_FLAG;
          }
          else                          /* counting */
          {
            /* stop counting */
            Flag |= STOP_COUNTING | DELAY_FLAG;
          }

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_EVENTS)     /* events selected */
        {
          /* reset to default value */
          EventsTrigger = DEFAULT_EVENTS;

          Flag |= DELAY_FLAG;           /* set delay flag */
          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_TIME)       /* time selected */
        {
          /* reset to default value */
          TimeTrigger = DEFAULT_TIME;

          Flag |= DELAY_FLAG;           /* set delay flag */
          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
      }
      else if (Test == KEY_TWICE)       /* two short key presses */
      {
        /* exit tool */
        Flag = 0;                       /* end processing loop */
      }
      else if (Test == KEY_RIGHT)       /* right key */
      {
        if (Item == UI_COUNTERMODE)     /* counter mode selected */
        {
          /* change to next mode */
          CounterMode++;
          /* overrun to first mode */
          if (CounterMode > MODE_EVENTS) CounterMode = MODE_COUNT;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_EVENTS)     /* events selected */
        {
          /* increase event trigger */
          EventsTrigger += Step;
          /* limit overflow to max. value */
          if (EventsTrigger > MAX_EVENTS) EventsTrigger = MAX_EVENTS;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_TIME)       /* time selected */
        {
          /* increase time trigger */
          TimeTrigger += Step;
          /* limit overflow to max. value */
          if (TimeTrigger > MAX_TIME) TimeTrigger = MAX_TIME;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
      }
      else if (Test == KEY_LEFT)        /* left key */
      {
        if (Item == UI_COUNTERMODE)     /* counter mode selected */
        {
          /* change to previous mode */
          CounterMode--;
          /* underrun to last mode */
          if (CounterMode == 0) CounterMode = MODE_EVENTS;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_EVENTS)     /* events selected */
        {
          /* decrease event trigger */
          EventsTrigger -= Step;
          /* limit underflow to zero */
          if (EventsTrigger > MAX_EVENTS) EventsTrigger = 0;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
        else if (Item == UI_TIME)       /* time selected */
        {
          /* decrease time trigger */
          TimeTrigger -= Step;
          /* limit underflow to zero */
          if (TimeTrigger > MAX_TIME) TimeTrigger = 0;

          Flag &= ~WAIT_FLAG;           /* end waiting loop */
        }
      }
    }
  }


  /*
   *  clean up
   */

  /* timers */
  TIMSK0 = 0;                 /* disable all interrupts for Timer0 */
  TIMSK1 = 0;                 /* disable all interrupts for Timer1 */

  /* local constant for timer1 */
  #undef TOP

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef WAIT_FLAG
  #undef IDLE_FLAG
  #undef DELAY_FLAG
  #undef START_COUNTING
  #undef MANAGE_COUNTING
  #undef STOP_COUNTING

  /* local constants for CounterMode */
  #undef MODE_COUNT
  #undef MODE_TIME
  #undef MODE_EVENTS

  /* local constants for Item */
  #undef UI_COUNTERMODE
  #undef UI_EVENTS
  #undef UI_TIME
  #undef UI_STARTSTOP

  /* local constants for Show */
  #undef SHOW_MODE
  #undef SHOW_EVENTS
  #undef SHOW_TIME
  #undef SHOW_STARTSTOP

  /* local constants for defaults and maximums */
  #undef DEFAULT_TIME
  #undef DEFAULT_EVENTS
  #undef MAX_TIME
  #undef MAX_EVENTS
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef TOOLS_COUNTER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

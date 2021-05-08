/* ************************************************************************
 *
 *   pause functions
 *
 *   (c) 2012-2017 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define PAUSE_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   sleep functions
 * ************************************************************************ */


/*
 *  enter MCU sleep mode for a specific time in ms
 *  - valid time 0 - 65535ms
 *  - uses timer2
 *  - don't use this function for time critical stuff!
 */

void MilliSleep(uint16_t Time)
{
  uint32_t               Cycles;        /* timer cycles */
  uint8_t                Timeout;       /* compare value */
  uint8_t                Mode;          /* sleep mode */
  uint8_t                Flag = 0;      /* interrupt flag */

  /*
   *  calculate stuff
   */

  /*
      Using timer prescaler of 1024 (maximum):
        MCU frequency  1MHz    8MHz   16MHz  20MHz
        timer cycle    1024탎  128탎  64탎   51.2탎

      We don't compensate the binary to decimal offset and also not the time
      required for the processing loop, because it would make things much more
      complicated and we don't need exact timing here.
  */

  Mode = Cfg.SleepMode;                 /* get requested sleep mode */

  /* calculate required timer cycles (prescaler 1024) */
  Cycles = Time;
  Cycles *= MCU_CYCLES_PER_US;          /* timer cycles based on MCU frequency */

  if (Mode == SLEEP_MODE_PWR_SAVE)      /* power save mode */
  {
    uint32_t        Value;

    /*
     *  Based on the datasheet the main clock source is disabled in power save
     *  mode and timer2 needs a watch crystal as clock source (asychronous
     *  clock). Fortunately the main clock keeps running in power save mode and
     *  will clock timer2 (synchronous clock). Undocumented feature?
     */

    /*
     *  After returning from the power down or power save sleep modes the
     *  main oscillator needs following start-up times to stabilize:
     *  - crystal oscillator:  16k cycles
     *  - ceramic resonator:   1k or 256 cycles
     *  - internal RC osc.:    6 cycles
     */

    /* compensate oscillator start-up */
    Value = Cycles / 256;               /* calculate loop runs */
    Value++;                            /* fix offset by division */
    /* multiply with startup cycles equivalent to timer cycles */
    Value *= (OSC_STARTUP / 1024);      /* overhead cycles */

    if (Cycles > Value)            /* we are able to compensate */
    {
      Cycles -= Value;                  /* subtract overhead cycles */
    }
    else                           /* no way to compensate */
    {
      /* idle mode doesn't require oscillator start-up after wake-up */ 
      Mode = SLEEP_MODE_IDLE;           /* change sleep mode to Idle */
    }
  }


  /*
   *  set up timer
   */

  TCCR2B = 0;                      /* disable timer */
  TCNT2 = 0;                       /* set counter to 0 */
  TCCR2A = (1 << WGM21);           /* set CTC mode */
  TIMSK2 = (1 << OCIE2A);          /* enable interrupt for OCR2A match */

  set_sleep_mode(Mode);            /* set sleep mode */

  if (SREG & (1 << SREG_I))        /* if interrupts are already enabled */
  {
    Flag = 1;                      /* keep that in mind */
  }
  else                             /* otherwise */
  {
    sei();                         /* enable interrupts */
  }


  /*
   *  processing loop
   *  - sleep for several intervals until requested time is reached
   */

  while (Cycles > 0)
  {
    wdt_reset();              /* reset watchdog */

    /*
     *  manage timeout
     */

    if (Cycles > 255)         /* prevent 8 bit overflow */
    {
      Timeout = 255;          /* use maximum */
    }
    else                      /* within 8 bit range */
    {
      Timeout = (uint8_t)Cycles;
    }

    Cycles -= Timeout;        /* update counter */
    Timeout--;                /* interrupt is triggered by cycle after match */
    /* todo: what happens if Timeout is 0? */

    /* update timer */
    OCR2A = Timeout;               /* set compare value (timeout) */

    /*
     *  sleep
     *  - since any interrupt would wake up the MCU we have to make sure
     *    that we track the right interrupt
     */

    /* enable timer by setting clock prescaler to 1024 */
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

    while (TCCR2B != 0)       /* as long as Timer2 is running */
    {
      sleep_mode();                     /* sleep (again) */    
      /* woken up */
    }
  }

  if (Flag == 0)              /* we should disable interrupts */
  {
    cli();                    /* disable interrupts */
  }
}



/*
 *  ISR for match of Timer2's OCR2A (Output Compare Register A)
 */

ISR(TIMER2_COMPA_vect, ISR_BLOCK)
{
  /*
   *  hints:
   *  - the OCF2A interrupt flag is cleared automatically
   *  - interrupt processing is disabled while this ISR runs
   */

  TCCR2B = 0;                 /* disable Timer2 */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef PAUSE_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

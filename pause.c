/* ************************************************************************
 *
 *   pause functions
 *
 *   (c) 2012-2013 by Markus Reschke
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
#include "LCD.h"              /* LCD module */
#include "functions.h"        /* external functions */


/* ************************************************************************
 *   Interrupt Service Routines
 * ************************************************************************ */


/*
 *  ISR for a match of TCNT2 (Timer2) and OCR2A (Output Compare Register A)
 */

ISR (TIMER2_COMPA_vect)
{
  /* this automatically clears the OCF2A flag in the Interrupt Flag Register */

  TCCR2B = 0;                    /* disable Timer2 */
}



/* ************************************************************************
 *   sleep functions
 * ************************************************************************ */


/*
 *  enter MCU sleep mode for a specific time in ms
 *  - valid time 0 - 65535ms
 *  - don't use this function for time critical stuff!
 */

void MilliSleep(uint16_t Time)
{
  uint32_t               Cycles;        /* timer cycles */
  uint8_t                Timeout;       /* compare value */
  uint8_t                Mode;          /* sleep mode */

  /*
   *  calculate stuff
   */

  /*
      Using timer prescaler of 1024 (maximum):
        MCU frequency  1MHz    8MHz   16MHz
        timer cycle    1024µs  128µs  64µs

      We don't compensate the binary to decimal offset and also not the time
      required for the processing loop, because it would make things much more
      complicated and we don't need exact timing here.
  */

  Mode = Config.SleepMode;              /* get requested sleep mode */

  /* calculate required timer cycles (prescaler 1024) */
  Cycles = Time * (CPU_FREQ / 1000000);    /* timer cycles based on MCU frequency */

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
   *  setup timer
   */

  TCCR2B = 0;                      /* disable timer */
  TCCR2A = (1 << WGM21);           /* set CTC mode */
  TIMSK2 = (1 << OCIE2A);          /* enable interrupt for OCR0A match */

  set_sleep_mode(Mode);            /* set sleep mode */


  /*
   *  processing loop
   *  - sleep for several intervals until requested time is reached
   */

  while (Cycles > 0)
  {
    wdt_reset();              /* reset watchdog */

    /* get timeout */
    if (Cycles > 255) Timeout = 255;
    else Timeout = Cycles;
    Cycles -= Timeout;
    Timeout--;                     /* interrupt is triggered by cycle after match */
    /* todo: what happens if Timeout is 0? */

    /* update timer */
    TCNT2 = 0;                     /* set counter to 0 */
    OCR2A = Timeout;               /* set value to compare with (timeout) */

    /* sleep */
    /* enable timer by setting clock prescaler to 1024 */
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
    sei();                                   /* enable interrupts */
    sleep_mode();                            /* and sleep */    

    /* after wakeup */
    cli();                         /* disable interrupts */
  }
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef PAUSE_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

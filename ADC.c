/* ************************************************************************
 *
 *   ADC functions
 *
 *   (c) 2012-2020 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define ADC_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/* ************************************************************************
 *   ADC
 * ************************************************************************ */


/*
 *  read ADC channel and return voltage in mV
 *  - use Vcc as reference by default
 *  - switch to bandgap reference for low voltages (< 1.0V) to improve
 *    ADC resolution
 *  - with a 125kHz ADC clock a single conversion needs about 0.1ms
 *    with 25 samples we end up with about 2.6ms
 *
 *  requires:
 *  - Channel: ADC MUX input channel
 *    - ATmega328: register bits corresponding with MUX0-3
 *    - ATmega324/644/1284: register bits corresponding with MUX0-4
 *    - ATmega640/1280/2560: register bits corresponding with MUX0-4
 *      (todo: add MUX5 to support also ADC8-15)
 */

uint16_t ReadU(uint8_t Channel)
{
  uint16_t          U;             /* return value (mV) */
  uint8_t           Counter;       /* loop counter */
  uint8_t           Ref;           /* voltage reference register bits */
  uint32_t          Value;         /* ADC value */

  /* AREF pin is connected to external buffer cap (1nF) */

  #if 0
  /* manage channels ADC8-15 (ATmega640/1280/2560) */
  if (Channel & 0b00100000)        /* bit 6 set: ADC8-15 */
  {
    ADCSRB |= (1 << MUX5);         /* set MUX5 */
  }
  else                             /* bit 6 not set: ADC0-7 */
  {
    ADCSRB &= ~(1 << MUX5);        /* clear MUX5 */
  }
  #endif

  /* prepare bitfield for register: start with AVcc as voltage reference */
  Channel &= ADC_CHAN_MASK;        /* filter reg bits for MUX channel */
  Channel |= ADC_REF_VCC;          /* add bits for voltage reference: AVcc */

sample:

  ADMUX = Channel;                 /* set input channel and U reference */

  /*
   *  change of voltage reference
   *  - voltage needs some time to stabilize at buffer cap 
   *  - run a dummy conversion after change (recommended by datasheet)
   *  - It seems that we have to run a dummy conversion also after the
   *    ADC hasn't run for a while. So let's do one anyway.
   */

  Ref = Channel & ADC_REF_MASK;    /* get register bits for voltage reference */
  if (Ref != Cfg.Ref)              /* reference source has changed */
  {
    /* wait some time for voltage stabilization */
    #ifndef ADC_LARGE_BUFFER_CAP
      /* buffer cap: 1nF or none at all */
      wait100us();                   /* 100µs */
    #else
      /* buffer cap: 100nF */
      wait10ms();                    /* 10ms */
    #endif

    #if 0
    /* dummy conversion */
    ADCSRA |= (1 << ADSC);         /* start conversion */
    while (ADCSRA & (1 << ADSC));  /* wait until conversion is done */
    #endif

    Cfg.Ref = Ref;                 /* update reference source */
  }

  /* perform dummy conversion anyway */
  ADCSRA |= (1 << ADSC);         /* start conversion */
  while (ADCSRA & (1 << ADSC));  /* wait until conversion is done */


  /*
   *  sample ADC readings
   */

  Value = 0UL;                     /* reset sampling variable */
  Counter = 0;                     /* reset counter */

  while (Counter < Cfg.Samples)    /* take samples */
  {
    ADCSRA |= (1 << ADSC);         /* start conversion */
    while (ADCSRA & (1 << ADSC));  /* wait until conversion is done */

    Value += ADCW;                 /* add ADC reading */

    /* auto-switch voltage reference for low readings */
    if (Counter == 4)                   /* 5 samples */
    {
      if ((uint16_t)Value < 1024)       /* < 1V (5V / 5 samples) */
      {
        if (Ref != ADC_REF_BANDGAP)     /* bandgap ref not selected */
        {
          if (Cfg.AutoScale == 1)       /* autoscaling enabled */
          {
            Channel &= ~ADC_REF_MASK;     /* clear reference bits */
            Channel |= ADC_REF_BANDGAP;   /* select bandgap reference */

            goto sample;                /* re-run sampling */
          }
        }
      }
    }

    Counter++;                     /* another sample done */
  }


  /*
   *  convert ADC reading to voltage
   *  - single sample: U = ADC reading * U_ref / 1024
   */

  /* get voltage of reference used */
  if (Ref == ADC_REF_BANDGAP)      /* bandgap reference */
  {
    U = Cfg.Bandgap;                 /* voltage of bandgap reference */
  }
  else                             /* Vcc as reference */
  {
    U = Cfg.Vcc;                     /* voltage of Vcc */   
  }

  /* convert to voltage; */
  Value *= U;                      /* ADC readings * U_ref */
//  Value += 511 * Cfg.Samples;      /* automagic rounding */
  Value /= 1024;                   /* / 1024 for 10bit ADC */

  /* de-sample to get average voltage */
  Value /= Cfg.Samples;
  U = (uint16_t)Value;

  return U; 
}



/* ************************************************************************
 *   convenience functions
 * ************************************************************************ */


/*
 *  wait 5ms and then read ADC
 *  - same as ReadU()
 */

uint16_t ReadU_5ms(uint8_t Channel)
{
   wait5ms();       /* wait 5ms */

   return (ReadU(Channel));
}



/*
 *  wait 20ms and then read ADC
 *  - same as ReadU()
 */

uint16_t ReadU_20ms(uint8_t Channel)
{
  wait20ms();       /* wait 20ms */

  return (ReadU(Channel));
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef ADC_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

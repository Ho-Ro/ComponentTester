/* ************************************************************************
 *
 *   ADC functions
 *
 *   (c) 2012-2020 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz K�bbeler
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
 *  - Channel: ADC MUX input channel (register bits corresponding with MUX0-4)
 *             (or MUX0-5 when support for pins ADC8-15 is enabled)
 */

uint16_t ReadU(uint8_t Channel)
{
  uint16_t          U;             /* return value (mV) */
  uint8_t           Counter;       /* loop counter */
  uint8_t           Ref;           /* voltage reference register bits */
  uint32_t          Value;         /* ADC value */

  /* AREF pin is connected to external buffer cap (1nF) */

  #if 0
  /* manage channels ADC8-15 */
  if (Channel & 0b00100000)        /* bit 6 set: ADC8-15 */
  {
    ADCSRB |= (1 << MUX5);         /* set MUX5 */
  }
  else                             /* bit 6 not set: ADC0-7 */
  {
    ADCSRB &= ~(1 << MUX5);        /* clear MUX5 */
  }

  Channel &= 0b00011111;           /* filter bits 0-4 (MUX0-4) */
  #endif

  Channel |= ADC_REF_VCC;          /* add voltage reference: AVcc */

sample:

  ADMUX = Channel;                 /* set input channel and U reference */

  /* 
   *  dummy conversion
   *  - if voltage reference has changed run a dummy conversion
   *  - recommended by datasheet
   */

  Ref = Channel & ADC_REF_MASK;    /* get register bits for voltage reference */
  if (Ref != Cfg.RefFlag)          /* reference has changed */
  {
    /* wait some time for voltage stabilization */
    #ifndef ADC_LARGE_BUFFER_CAP
      /* buffer cap: 1nF or none at all */
      wait100us();                   /* 100�s */
    #else
      /* buffer cap: 100nF */
      wait10ms();                    /* 10ms */
    #endif

    ADCSRA |= (1 << ADSC);         /* start conversion */
    while (ADCSRA & (1 << ADSC));  /* wait until conversion is done */

    Cfg.RefFlag = Ref;             /* update voltage reference */
  }


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

    Counter++;                     /* one less to do */
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

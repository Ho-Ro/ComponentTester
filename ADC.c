/* ************************************************************************
 *
 *   ADC functions
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
#include "LCD.h"              /* LCD module */
#include "functions.h"        /* external functions */


/* ************************************************************************
 *   ADC
 * ************************************************************************ */


/*
 *  read ADC and return voltage in mV
 *  - Use Vcc as reference by default.
 *  - Switch to bandgap reference for low voltages (< 1.0V) to improve
 *    ADC resolution.
 *
 *  requires:
 *  - Probe: input channel of ADC MUX (lower 4 bits)
 *           may also include setting of voltage reference  
 *
 */

unsigned int ReadU(uint8_t Probe)
{
  unsigned int      U;             /* return value (mV) */
  uint8_t           Counter;       /* loop counter */
  unsigned long     Value;         /* ADC value */

  Probe |= (1 << REFS0);           /* use internal reference anyway */

sample:

  ADMUX = Probe;                   /* set input channel and U reference */

  /* if voltage reference changed run a dummy conversion */
  /* (recommended by datasheet) */
  Counter = Probe & (1 << REFS1);    /* get REFS1 bit flag */
  if (Counter != Config.RefFlag)
  {
    wait100us();                     /* time for voltage stabilization */

    ADCSRA |= (1 << ADSC);           /* start conversion */
    while (ADCSRA & (1 << ADSC));    /* wait until conversion is done */

    Config.RefFlag = Counter;        /* update flag */
  }


  /*
   *  sample ADC readings
   */

  Value = 0UL;                     /* reset sampling variable */
  Counter = 0;                     /* reset counter */
  while (Counter < Config.Samples) /* take samples */
  {
    ADCSRA |= (1 << ADSC);         /* start conversion */
    while (ADCSRA & (1 << ADSC));  /* wait until conversion is done */

    Value += ADCW;                 /* add ADC reading */

    /* auto-switch voltage reference for low readings */
    if ((Counter == 4) &&
        ((uint16_t)Value < 1024) &&
        !(Probe & (1 << REFS1)))
    {
      Probe |= (1 << REFS1);       /* select internal bandgap reference */
      goto sample;                 /* re-run sampling */
    }

    Counter++;                     /* one less to do */
  }


  /*
   *  convert ADC reading to voltage
   *  - single sample: U = ADC reading * U_ref / 1024
   */

  /* get voltage of reference used */
  if (Probe & (1 << REFS1)) U = Config.U_Bandgap;   /* bandgap reference */
  else U = UREF_VCC;                                /* Vcc reference */   

  /* convert to voltage; */
  Value *= U;                      /* ADC readings * U_ref */
//  Value += 511 * Config.Samples;   /* automagic rounding */
  Value /= 1024;                   /* / 1024 for 10bit ADC */

  /* de-sample to get average voltage */
  Value /= Config.Samples;
  U = (unsigned int)Value;

  return U; 
}



/* ************************************************************************
 *   convenience functions
 * ************************************************************************ */


/*
 *  wait 5ms and then read ADC
 *  - same as ReadU()
 */

unsigned int ReadU_5ms(uint8_t Probe)
{
   wait5ms();       /* wait 5ms */

   return (ReadU(Probe));
}



/*
 *  wait 20ms and then read ADC
 *  - same as ReadU()
 */

unsigned int ReadU_20ms(uint8_t Probe)
{
  wait20ms();       /* wait 20ms */

  return (ReadU(Probe));
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef ADC_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

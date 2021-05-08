/* ************************************************************************
 *
 *   inductor measurements
 *
 *   (c) 2012-2014 by Markus Reschke
 *   based on code from Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define INDUCTOR_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  local defines
 */

/* mode bitmask */
#define MODE_LOW_CURRENT      0b00000001     /* low test current */
#define MODE_HIGH_CURRENT     0b00000010     /* high test current */
#define MODE_DELAYED_START    0b00000100     /* delayed start */



/* ************************************************************************
 *   inductance measurements
 * ************************************************************************ */


/*

Current of an inductor when switching on power:

  i_L(t) = I_0 * (1 - e^(-t R_total / L))

  I_0: end current 

  i_L(t) / I_0 = 1 - e^(-t R_total / L)

  e^(-t R_total / L) = 1 - (i_L(t) / I_0))

With ln(e^x) = x we get:

  -t R_total / L = ln(1 - (i_L(t) / I_0))

  L = -t R_total / ln(1 - (i_L(t) / I_0))

So we can measure the current at a specific time after switching on to
get L.

  R_total = Ri_H + R_L + Rl + Ri_L

  I_0 = 5V / R_total = 5V / (Ri_H + R_L + Rl + Ri_L) 

We get i_L(t) by measuring the voltage accross a shunt resistor. For a proper
time measurement we'll use the integrated analog comparator and a timer, i.e.
we'll wait until the voltage across the shunt resistor reaches the voltage
of the bandgap reference.

  R_shunt = Rl + Ri_L

  i_L(t_stop) = U_ref / R_shunt

  L = -t_stop * R_total / ln(1 - ((U_ref / R_shunt) / (5V / R_total)))

    = -t_stop * R_total / ln(1 - (U_ref * R_total) / (5V * R_shunt))

Instead of calculating L directly we'll use a table with pre-calculated values
to speed up things and keep the firmware small. The table is based on the
the ratio:
  a = (U_ref * R_total) / (5V * R_shunt)

Estimates for minimal and maximal values (R_L max. 2k Ohm):
- min: 1.0V * 720 Ohm / 5V * 700 Ohm = 0.206
- max: 1.2V * 2770 Ohm / 5V * 700 Ohm = 0.950
  999 is maximum due to ln(1-a/1000)
  Hence the maximum R_L supported is 2k Ohms.

Table:
- ratio a = ((U_ref * R_total) / (5V * R_shunt)) * 10^3
  estimated range: 206 - 977
- values are: (-1 / ln(1 - (a * 10^-3))) * 10^3
  - internal scale factor 10^3 
- bc:
  - options: -i -l
  - define x (a) { return (-1 / l(1 - a/1000)) * 1000; }

For a small inductance we have to use a higher test current, i.e. using Ri_L as
current shunt (Rl = 0).

Estimates for minimal and maximal values (R_L max. 40 Ohm):
- min: 1.0V * 40 Ohm / 5V * 20 Ohm = 0,4
- max: 1.2V * 80 Ohm / 5V * 20 Ohm = 0.960
  999 is maximum due to ln(1-a/1000)
  Hence the maximum R_L supported is 40 Ohms.

Since the range overlaps with the low test current we may use a single table.

*/



#ifdef SW_INDUCTOR


/*
 *  measure inductance between two probe pins
 *
 *  requires:
 *  - pointer to time variable (ns)
 *  - measurement mode (low/high current, delayed start)
 *
 *  returns:
 *  - 3 on success
 *  - 2 if inductance is too low
 *  - 1 if inductance is too high
 *  - 0 on any problem
 */

uint8_t MeasureInductance(uint32_t *Time, uint8_t Mode)
{
  uint8_t           Flag = 3;           /* return value */
  uint8_t           Test;               /* test flag */
  int8_t            Offset;             /* counter offet */
  unsigned int      Ticks_L;            /* timer counter */
  unsigned int      Ticks_H;            /* timer overflow counter */
  unsigned long     Counter;            /* counter */

  /* sanity check */
  if (Time == NULL) return 0;

  DischargeProbes();                    /* try to discharge probes */
  if (Check.Found == COMP_ERROR) return 0;


  /*
   *  measurement modes:
   *  - low current: Gnd -- Rl -- probe-2 / probe-1 -- Vcc
   *  - high current: Gnd -- probe-2 / probe-1 -- Vcc
   */


  /*
   *  init hardware
   */

  /* set probes: Gnd -- probe-1 / Gnd -- Rl -- probe-2 */
  R_PORT = 0;                           /* set resistor port to low */
  ADC_PORT = 0;                         /* set ADC port to low */

  if (Mode & MODE_LOW_CURRENT)     /* low current */
  {
    R_DDR = Probes.Rl_2;                /* pull down probe-2 via Rl */
    ADC_DDR = Probes.ADC_1;             /* pull down probe-1 directly */
  }
  else                             /* high current */
  {
    R_DDR = 0;                          /* disable probe resistors */
    /* pull down probe-1 and probe-2 directly */
    ADC_DDR = Probes.ADC_1 | Probes.ADC_2;
  }

  /* setup analog comparator */
  ADCSRB = (1 << ACME);                 /* use ADC multiplexer as negative input */
  ACSR =  (1 << ACBG) | (1 << ACIC);    /* use bandgap as positive input, trigger timer1 */
  ADMUX = (1 << REFS0) | Probes.Pin_2;  /* switch ADC multiplexer to probe-2 */
                                        /* and set AREF to Vcc */
  ADCSRA = ADC_CLOCK_DIV;               /* disable ADC, but keep clock dividers */
  wait200us();                          /* allow bandgap reference to settle */


  /*
   *  setup timer
   */

  Ticks_H = 0;                          /* reset timer overflow counter */
  TCCR1A = 0;                           /* set default mode */
  TCCR1B = 0;                           /* set more timer modes */
  /* timer stopped, falling edge detection, noise canceler disabled */
  TCNT1 = 0;                            /* set Counter1 to 0 */
  /* clear all flags (input capture, compare A & B, overflow */
  TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);

  if (Mode & MODE_DELAYED_START)        /* delayed start */
  {
    Test = (CPU_FREQ / 1000000);        /* MCU cycles per µs */

    /* change probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    ADC_PORT = Probes.ADC_1;            /* pull up probe-1 directly */

    /*
     *  delay timer by about 3-4µs to skip capacitive effects of large inductors
     *  - a loop run needs 4 cycles, the last loop run just 3
     *  - cycles burnt: <MCU cycles per µs> * 4 - 1
     *    time delay: 4µs - 1 MCU cycle
     */

    while (Test > 0)
    {
      Test--;
      asm volatile("nop\n\t"::);
    }

    TCCR1B |= (1 << CS10);              /* start timer (1/1 clock divider) */
  }
  else                                  /* immediate start */
  {
    TCCR1B |= (1 << CS10);              /* start timer (1/1 clock divider) */
    /* change probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    ADC_PORT = Probes.ADC_1;            /* pull up probe-1 directly */
  }


  /*
   *  timer loop
   *  - run until voltage threshold is reached
   *  - detect timer overflows
   */

  while (1)
  {
    Test = TIFR1;                       /* get timer1 flags */

    /* end loop if input capture flag is set (= same voltage) */
    if (Test & (1 << ICF1)) break;

    /* detect timer overflow by checking the overflow flag */
    if (Test & (1 << TOV1))
    {
      /* happens at 65.536ms for 1MHz or 8.192ms for 8MHz */
      TIFR1 = (1 << TOV1);              /* reset flag */
      wdt_reset();                      /* reset watchdog */
      Ticks_H++;                        /* increase overflow counter */

      /* if it takes too long (0.26s) */
      if (Ticks_H == (CPU_FREQ / 250000))
      {
        Flag = 0;             /* signal timeout */
        break;                /* end loop */
      }
    }
  }

  /* stop counter */
  TCCR1B = 0;                           /* stop timer */
  TIFR1 = (1 << ICF1);                  /* reset Input Capture flag */

  Ticks_L = ICR1;                       /* get counter value */

  /* prepare cut off: Gnd -- Rl -- probe-2 / probe-1 -- Rl -- Gnd */
  R_DDR = Probes.Rl_2 | Probes.Rl_1;  

  /* stop current flow */
  ADC_DDR = 0;

  /* catch missed timer overflow */
  if ((TCNT1 > Ticks_L) && (Test & (1 << TOV1)))
  {
    TIFR1 = (1 << TOV1);                /* reset overflow flag */
    Ticks_H++;                          /* increase overflow counter */
  }

  /* enable ADC again */
  ADCSRA = (1 << ADEN) | (1 << ADIF) | ADC_CLOCK_DIV;


  /*
   *  process counters
   */

  /* combine both counter values */
  Counter = (unsigned long)Ticks_L;          /* lower 16 bits */
  Counter |= (unsigned long)Ticks_H << 16;   /* upper 16 bits */

  Offset = -4;                /* subtract processing overhead */

  if (Mode & MODE_DELAYED_START)             /* delayed start */
  {
    /* add MCU cycles for delayed start */
    Offset += ((CPU_FREQ / 1000000) * 4) - 1;
  }
  else                                       /* immediate start */
  {
    Offset -= 1;              /* timer started one cycle too early */
  }

  if (Offset >= 0)            /* positive offet */
  {
    Counter += Offset;
  }
  else                        /* negative offset */
  {
    Offset *= -1;                            /* make it positive */
    if (Counter < Offset) Counter = 0;       /* prevent underflow */
    else Counter -= Offset;                  /* subtract offset */
  }

  /* convert counter (MCU cycles) to time (in ns) */
  if (Counter > 0)
  {
    Counter += (CPU_FREQ / 2000000);         /* add half of cycles for rounding */
    Counter *= (1000000000 / CPU_FREQ);      /* divide by frequeny and scale to ns */
  }

  if (Counter <= 500) Flag = 2;         /* signal "inductance too low" */
  *Time = Counter;                      /* save time */

  return Flag;
}



/*
 *  measure inductance between two probe pins of a resistor
 *
 *  requires:
 *  - pointer the resistor data structure
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any error
 */

uint8_t MeasureInductor(Resistor_Type *Resistor)
{
  uint8_t           Test = 0;           /* return value / measurement result */
  uint8_t           Mode;               /* measurement mode */
  uint8_t           Scale;              /* scale of value */
  unsigned int      R_total;            /* total resistance */
  unsigned int      Factor;             /* factor */
  unsigned long     Value;              /* value */
  unsigned long     Time1;              /* time #1 */
  unsigned long     Time2;              /* time #2 */

  /* reset data */
  Inductor.Scale = 0;
  Inductor.Value = 0;

  /* sanity check */
  if (Resistor == NULL) return Test;

  /* limit resistor to 2k (feasibilty & prevent variable overflow) */ 
  if (CmpValue(Resistor->Value, Resistor->Scale, 2000, 0) >= 0) return Test;


  /*
   *  Manage measurements:
   *  - run in immediate and delayed mode to deal with capacitive effects
   *    of large inductors and keep smaller time
   *  - in case of a small inductance run in high current mode (implies
   *    immediate mode only)
   */

  UpdateProbes(Resistor->A, Resistor->B, 0);      /* update probes */

  Mode = MODE_LOW_CURRENT;
  Test = MeasureInductance(&Time1, Mode);
 
  if (Test == 2)                   /* inductance too low */
  {
    /* if resistance < 40 Ohms we may run the high current test */
    if (CmpValue(Resistor->Value, Resistor->Scale, 40, 0) < 0)
    {
      Mode = MODE_HIGH_CURRENT;
      Test = MeasureInductance(&Time1, Mode);
    }
  }
  else if (Test == 3)              /* valid time */
  {
    /* let's run the delayed mode */
    Mode = MODE_LOW_CURRENT | MODE_DELAYED_START;
    Test = MeasureInductance(&Time2, Mode);
    if (Time1 > Time2) Time1 = Time2;        /* lower value wins */
  }  

  if (Test != 3) Test = 0;         /* measurements failed */


  /*
   *  calculate inductance
   */

  if (Test == 3)
  {
    /*
     *  resistances
     */

    /* total resistance (in 0.1 Ohms) */
    R_total = RescaleValue(Resistor->Value, Resistor->Scale, -1);  /* R_L */
    R_total += Config.RiH + Config.RiL;

    /* shunt resistance (in 0.1 Ohms) */
    Factor = Config.RiL;

    if (Mode & MODE_LOW_CURRENT)        /* low current measurement mode */
    {
      /* add R_l */
      R_total += (R_LOW * 10);
      Factor += (R_LOW * 10);
    }


    /*
     *  ratio and factor
     *  - ratio = ((U_ref * R_total) / (5V * R_shunt)) * 10^3
     */

    /* calculate ratio */
    Value = Config.Bandgap + Config.CompOffset;   /* = U_ref (in mV) */
    Value *= R_total;                             /* * R_total (in 0.1 Ohms) */
    Value /= Factor;                              /* / R_shunt (in 0.1 Ohms) */
    Value /= 5;                                   /* / 5000mV, * 10^3 */

    /* get ratio based factor */
    Factor = GetFactor((unsigned int)Value, TABLE_INDUCTOR);


   /*
    *  calculate inductance
    *  L = t_stop * R_total * factor
    */

    Scale = -9;               /* nH by default */
    Value = Time1;            /* t_stop (in ns) */

    while (Value > 100000)    /* re-scale to prevent overflow */
    {
      Value += 5;             /* for automagic rounding */
      Value /= 10;            /* scale down by 10^1 */
      Scale++;                /* increase exponent by 1 */
    }

    Value *= Factor;          /* * factor (in 10^-3) */

    while (Value > 100000)    /* re-scale to prevent overflow */
    {
      Value += 5;             /* for automagic rounding */
      Value /= 10;            /* scale down by 10^1 */
      Scale++;                /* increase exponent by 1 */
    }

    Value *= R_total;         /* * R_total (in 0.1 Ohms) */
    Value /= 10000;           /* /1o for 1 Ohms, /1000 for factor */

    /* update data */
    Inductor.Scale = Scale;
    Inductor.Value = Value;
    Test = 1;                 /* signal success */
  }

  return Test;
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local defines */
#undef MODE_LOW_CURRENT
#undef MODE_HIGH_CURRENT
#undef MODE_DELAYED_START


/* source management */
#undef INDUCTOR_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

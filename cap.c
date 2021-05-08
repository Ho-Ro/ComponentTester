/* ************************************************************************
 *
 *   capacitor measurements
 *
 *   (c) 2012-2013 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define CAP_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/* ************************************************************************
 *   ESR measurement
 * ************************************************************************ */


#ifdef EXTRA

#if 0
/*
 *  measure ESR
 *
 *  returns:
 *  - ESR in 0.01 Ohm
 */

unsigned int MeasureESR(void)
{
  unsigned int           ESR = 0;

  /* setup probes: Gnd -- probe 1 / probe 2 -- Rl -- Vcc */

  return ESR;
}
#endif

#endif



/* ************************************************************************
 *   capacitance measurements
 * ************************************************************************ */


/*

We measure the capacitance by measuring the time needed to charge up the DUT
to a specific voltage using a constant voltage source:

  U_c(t) = U_in * (1 - e^(-t/RC))

With ln(e^x) = x we get

  C = -t / (R * ln(1 - U_c/U_in) 

for an ideal capacitor whithout parallel resistive losses by leakage.

Instead of calculating C directly we'll use pre calculated tables to speed
up things and keep the firmware small. The tables hold the pre-calculated
values of -1/(R * ln(1 - U_c/U_in) for a specific range of U_c, so we just
have to multiply the time with that stored factor to get C.

Large caps:
- R = 680 + 22 (22 is the internal resistance of the µC for pull-up)
- U_in = 5V
- values are: (-1 / (R * ln(1 - U_c/U_in))) * 10^9n * 10^-2s * 10^-1
  - 10^9n for nF scale
  - 10^-2s for charge pulses of 10ms each
  - 10^-1 internal scale factor (make values fit in unsigned int)
- bc:
  - options: -i -l
  - define x (u) { return (-1000000 / (702 * l(1 - u/5000))); }

Small caps:
- R = 470k (neglect internal resistance of uC)
- U_in = 5V
- values are: (-1 / (R * ln(1 - U_c/U_in))) * 10^12p * 10^-4
  - 10^12p for pF scale
  - 10^-4 internal scale factor (make values fit in unsigned int)
- bc:
  - options: -i -l
  - define x (u) { return (-100000000 / (470000 * l(1 - u/5000))); }
- We could use 10^-3 as internal scale factor to maximize resolution.

*/


/*
 *  measure cap >4.7µF between two probe pins
 *
 *  requires:
 *  - Cap: pointer to capacitor data structure 
 *
 *  returns:
 *  - 3 on success
 *  - 2 if capacitance is too low
 *  - 1 if capacitance is too high
 *  - 0 on any problem
 */

uint8_t LargeCap(Capacitor_Type *Cap)
{
  uint8_t           Flag = 3;           /* return value */
  uint8_t           TempByte;           /* temp. value */
  uint8_t           Mode;               /* measurement mode */
  int8_t            Scale;              /* capacitance scale */
  unsigned int      TempInt;            /* temp. value */
  unsigned int      Pulses;             /* number of charging pulses */
  unsigned int      U_Zero;             /* voltage before charging */
  unsigned int      U_Cap;              /* voltage of DUT */
  unsigned int      U_Drop = 0;         /* voltage drop */
  unsigned long     Raw;                /* raw capacitance value */
  unsigned long     Value;              /* corrected capacitance value */

  /* setup mode */
  Mode = FLAG_10MS | FLAG_PULLUP;       /* start with large caps */


  /*
   *  We charge the DUT with up to 500 pulses each 10ms long until the
   *  DUT reaches 300mV. The charging is done via Rl. This method is
   *  suitable for large capacitances from 47uF up to 100mF. If we find a 
   *  lower capacitance we'll switch to 1ms charging pulses and try again
   *  (4.7µF up to 47µF).
   *
   *  Problem:
   *  ReadADC() needs about 5ms (44 runs). We charge the DUT for 10ms and
   *  measure for 5ms. During that time the voltage will drop due to
   *  resistive losses of the DUT and the measurement itself. So the DUT
   *  seems to need more time to reach 300mV causing a higher capacitance
   *  be calculated.
   *
   *  Remark:
   *  The Analog Input Resistance of the ADC is 100MOhm typically.
   */

large_cap:

  /* prepare probes */
  DischargeProbes();                    /* try to discharge probes */
  if (Check.Found == COMP_ERROR) return 0;     /* skip on error */

  /* setup probes: Gnd -- probe 1 / probe 2 -- Rl -- Vcc */
  ADC_PORT = 0;                    /* set ADC port to low */
  ADC_DDR = Probes.ADC_2;          /* pull-down probe 2 directly */
  R_PORT = 0;                      /* set resistor port to low */
  R_DDR = 0;                       /* set resistor port to HiZ */
  U_Zero = ReadU(Probes.Pin_1);    /* get zero voltage (noise) */

  /* charge DUT with up to 500 pulses until it reaches 300mV */
  Pulses = 0;
  TempByte = 1;
  while (TempByte)
  {
    Pulses++;
    PullProbe(Probes.Rl_1, Mode);       /* charging pulse */
    U_Cap = ReadU(Probes.Pin_1);        /* get voltage */
    U_Cap -= U_Zero;                    /* zero offset */

    /* end loop if charging is too slow */
    if ((Pulses == 126) && (U_Cap < 75)) TempByte = 0;
    
    /* end loop if 300mV are reached */
    if (U_Cap >= 300) TempByte = 0;

    /* end loop if maximum pulses are reached */
    if (Pulses == 500) TempByte = 0;

    wdt_reset();                        /* reset watchdog */
  }

  /* if 300mV are not reached DUT isn't a cap or much too large (>100mF) */
  /* we can ignore that for mid-sized caps */
  if (U_Cap < 300)
  {
    Flag = 1;
  }

  /* if 1300mV are reached with one pulse we got a small cap */
  if ((Pulses == 1) && (U_Cap > 1300))
  {
    if (Mode & FLAG_10MS)                    /* <47µF */
    {
      Mode = FLAG_1MS | FLAG_PULLUP;         /* set mode (1ms charging pulses) */
      goto large_cap;                        /* and re-run */
    }
    else                                     /* <4.7µF */
    {
      Flag = 2;
    }
  }


  /*
   *  check if DUT sustains the charge and get the voltage drop
   *  - run the same time as before minus the 10ms charging time
   *  - this gives us the approximation of the self-discharging
   */

  if (Flag == 3)
  {
    /* check self-discharging */
    TempInt = Pulses;
    while (TempInt > 0)
    {
      TempInt--;                        /* descrease timeout */
      U_Drop = ReadU(Probes.Pin_1);     /* get voltage */
      U_Drop -= U_Zero;                 /* zero offset */
      wdt_reset();                      /* reset watchdog */
    }

    /* calculate voltage drop */
    if (U_Cap > U_Drop) U_Drop = U_Cap - U_Drop;
    else U_Drop = 0;

    /* if voltage drop is too large consider DUT not to be a cap */
    if (U_Drop > 100) Flag = 0;
  }


  /*
   *  calculate capacitance
   *  - use factor from pre-calculated LargeCap_table
   *  - ignore Config.CapZero since it's in the pF range
   */

  if (Flag == 3)
  {
    Scale = -9;                           /* factor is scaled to nF */
    /* get interpolated factor from table */
    Raw = GetFactor(U_Cap + U_Drop, TABLE_LARGE_CAP);
    Raw *= Pulses;                        /* C = pulses * factor */
    if (Mode & FLAG_10MS) Raw *= 10;      /* *10 for 10ms charging pulses */

    if (Raw > UINT32_MAX / 1000)          /* scale down if C >4.3mF */
    {
      Raw /= 1000;                        /* scale down by 10^3 */
      Scale += 3;                         /* add 3 to the exponent */
    }

    Value = Raw;                          /* copy raw value */

    /* it seems that we got a systematic error */
    Value *= 100;
    if (Mode & FLAG_10MS) Value /= 109;   /* -9% for large cap */
    else Value /= 104;                    /* -4% for mid cap */

    /* copy data */
    Cap->A = Probes.Pin_2;    /* pull-down probe pin */
    Cap->B = Probes.Pin_1;    /* pull-up probe pin */
    Cap->Scale = Scale;       /* -9 or -6 */
    Cap->Raw = Raw;
    Cap->Value = Value;       /* max. 4.3*10^6nF or 100*10^3µF */ 
  }

  return Flag;
}



/*
 *  measure cap <4.7µF between two probe pins
 *
 *  requires:
 *  - Cap: pointer to capacitor data structure 
 *
 *  returns:
 *  - 3 on success
 *  - 2 if capacitance is too low
 *  - 1 if capacitance is too high
 *  - 0 on any problem
 */

uint8_t SmallCap(Capacitor_Type *Cap)
{
  uint8_t           Flag = 3;           /* return value */
  uint8_t           TempByte;           /* temp. value */
  int8_t            Scale;              /* capacitance scale */
  unsigned int      Ticks;              /* timer counter */
  unsigned int      Ticks2;             /* timer overflow counter */
  unsigned int      U_c;                /* voltage of capacitor */
  unsigned long     Raw;                /* raw capacitance value */
  unsigned long     Value;              /* corrected capacitance value */


  /*
   *  Measurement method used for small caps < 50uF:
   *  We need a much better resolution for the time measurement. Therefore we
   *  use the µCs internal 16-bit counter and analog comparator. The counter
   *  inceases until the comparator detects that the voltage of the DUT is as
   *  high as the internal bandgap reference. To support the higher time
   *  resolution we use the Rh probe resistor for charging.
   *
   *  Remark:
   *  The analog comparator has an Input Leakage Current of -50nA up to 50nA 
   *  at Vcc/2. The Input Offset is <10mV at Vcc/2.
   */

  Ticks2 = 0;                           /* reset timer overflow counter */

  /*
   *  init hardware
   */

  /* prepare probes */
  DischargeProbes();                    /* try to discharge probes */
  if (Check.Found == COMP_ERROR) return 0;     /* skip on error */

  /* set probes: Gnd -- all probes / Gnd -- Rh -- probe-1 */
  R_PORT = 0;                           /* set resistor port to low */
  /* set ADC probe pins to output mode */
  ADC_DDR = (1 << TP1) | (1 << TP2) | (1 << TP3);
  ADC_PORT = 0;                         /* set ADC port to low */
  R_DDR = Probes.Rh_1;                  /* pull-down probe-1 via Rh */

  /* setup analog comparator */
  ADCSRB = (1 << ACME);                 /* use ADC multiplexer as negative input */
  ACSR =  (1 << ACBG) | (1 << ACIC);    /* use bandgap as positive input, trigger timer1 */
  ADMUX = (1 << REFS0) | Probes.Pin_1;  /* switch ADC multiplexer to probe 1 */
                                        /* and set AREF to Vcc */
  ADCSRA = ADC_CLOCK_DIV;               /* disable ADC, but keep clock dividers */
  wait200us();

  /* setup timer */
  TCCR1A = 0;                           /* set default mode */
  TCCR1B = 0;                           /* set more timer modes */
  /* timer stopped, falling edge detection, noise canceler disabled */
  TCNT1 = 0;                            /* set Counter1 to 0 */
  /* clear all flags (input capture, compare A & B, overflow */
  TIFR1 = (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A) | (1 << TOV1);
  R_PORT = Probes.Rh_1;                 /* pull-up probe-1 via Rh */  
                                        
  /* enable timer */
  if (Check.Found == COMP_FET)
  {
    /* keep all probe pins pulled down but probe-1 */
    TempByte = (((1 << TP1) | (1 << TP2) | (1 << TP3)) & ~(1 << Probes.Pin_1));    
  }
  else
  {
    TempByte = Probes.ADC_2;            /* keep just probe-1 pulled down */
  }

  /* start timer by setting clock prescaler (1/1 clock divider) */
  TCCR1B = (1 << CS10);
  ADC_DDR = TempByte;                   /* start charging DUT */


  /*
   *  timer loop
   *  - run until voltage is reached
   *  - detect timer overflows
   */

   while (1)
   {
     TempByte = TIFR1;                  /* get timer1 flags */

     /* end loop if input capture flag is set (= same voltage) */
     if (TempByte & (1 << ICF1)) break;

     /* detect timer overflow by checking the overflow flag */
     if (TempByte & (1 << TOV1))
     {
       /* happens at 65.536ms for 1MHz or 8.192ms for 8MHz */
       TIFR1 = (1 << TOV1);             /* reset flag */
       wdt_reset();                     /* reset watchdog */
       Ticks2++;                        /* increase overflow counter */

       /* end loop if charging takes too long (13.1s) */
       if (Ticks2 == (CPU_FREQ / 5000)) break;
     }
   }

  /* stop counter */
  TCCR1B = 0;                           /* stop timer */
  TIFR1 = (1 << ICF1);                  /* reset Input Capture flag */

  Ticks = ICR1;                         /* get counter value */

  /* disable charging */
  R_DDR = 0;                  /* set resistor port to HiZ mode */

  /* catch missed timer overflow */
  if ((TCNT1 > Ticks) && (TempByte & (1 << TOV1)))
  {
    TIFR1 = (1 << TOV1);                /* reset overflow flag */
    Ticks2++;                           /* increase overflow counter */
  }

  /* enable ADC again */
  ADCSRA = (1 << ADEN) | (1 << ADIF) | ADC_CLOCK_DIV;

  /* get voltage of DUT */
  U_c = ReadU(Probes.Pin_1);       /* get voltage of cap */

  /* start discharging DUT */
  R_PORT = 0;                      /* pull down probe-2 via Rh */
  R_DDR = Probes.Rh_1;             /* enable Rh for probe-1 again */

  /* skip measurement if charging took too long */
  if (Ticks2 >= (CPU_FREQ / 5000)) Flag = 1;


  /*
   *  calculate capacitance (<50uF)
   *  - use factor from pre-calculated SmallCap_table
   */

  if (Flag == 3)
  {
    /*  combine both counter values */
    Raw = (unsigned long)Ticks;           /* set lower 16 bits */
    Raw |= (unsigned long)Ticks2 << 16;   /* set upper 16 bits */
    if (Raw > 2) Raw -= 2;                /* subtract processing time overhead */

    Scale = -12;                          /* default factor is for pF scale */
    if (Raw > (UINT32_MAX / 1000))        /* prevent overflow (4.3*10^6) */
    {
      Raw /= 1000;                        /* scale down by 10^3 */
      Scale += 3;                         /* add 3 to the exponent (nF) */
    }

    /* multiply with factor from table */
    Raw *= GetFactor(Config.Bandgap + Config.CompOffset, TABLE_SMALL_CAP);

    /* divide by CPU frequency to get the time and multiply with table scale */
    Raw /= (CPU_FREQ / 10000);
    Value = Raw;                          /* take raw value */

    /* take care about zero offset if feasable */
    if (Scale == -12)                     /* pF scale */
    {
      if (Value >= Config.CapZero)        /* if value is larger than offset */
      {
        Value -= Config.CapZero;          /* substract offset */
      }
      else                                /* if value is smaller than offset */
      {
        /* we have to prevent a negative value */
        Value = 0;                        /* set value to 0 */
      }
    }

    /* copy data */
    Cap->A = Probes.Pin_2;    /* pull-down probe pin */
    Cap->B = Probes.Pin_1;    /* pull-up probe pin */
    Cap->Scale = Scale;       /* -12 or -9 */
    Cap->Raw = Raw;
    Cap->Value = Value;       /* max. 5.1*10^6pF or 125*10^3nF */


    /*
     *  Self-adjust the voltage offset of the analog comparator and internal
     *  bandgap reference if C is 100nF up to 20µF. The minimum of 100nF
     *  should keep the voltage stable long enough for the measurements. 
     *  Changed offsets will be used in next test run.
     */

    if (((Scale == -12) && (Value >= 100000)) ||
        ((Scale == -9) && (Value <= 20000)))
    {
      signed int         Offset;
      signed long        TempLong;

      /*
       *  We can self-adjust the offset of the internal bandgap reference
       *  by measuring a voltage lower than the bandgap reference, one time
       *  with the bandgap as reference and a second time with Vcc as
       *  reference. The common voltage source is the cap we just measured.
       */

       while (ReadU(Probes.Pin_1) > 980)
       {
         /* keep discharging */
       }

       R_DDR = 0;                       /* stop discharging */

       Config.AutoScale = 0;            /* disable auto scaling */
       Ticks = ReadU(Probes.Pin_1);     /* U_c with Vcc reference */
       Config.AutoScale = 1;            /* enable auto scaling again */
       Ticks2 = ReadU(Probes.Pin_1);    /* U_c with bandgap reference */

       R_DDR = Probes.Rh_1;             /* resume discharging */

       Offset = Ticks - Ticks2;
       /* allow some offset caused by the different voltage resolutions
          (4.88 vs. 1.07) */
       if ((Offset < -4) || (Offset > 4))    /* offset too large */
       {
         /*
          *  Calculate total offset:
          *  - first get offset per mV: Offset / U_c
          *  - total offset for U_ref: (Offset / U_c) * U_ref
          */

         TempLong = Offset;
         TempLong *= Config.Bandgap;         /* * U_ref */
         TempLong /= Ticks2;                 /* / U_c */

         Config.RefOffset = (int8_t)TempLong;
       }


      /*
       *  In the cap measurement above the analog comparator compared
       *  the voltages of the cap and the bandgap reference. Since the µC
       *  has an internal voltage drop for the bandgap reference the
       *  µC used actually U_bandgap - U_offset. We get that offset by
       *  comparing the bandgap reference with the voltage of the cap:
       *  U_c = U_bandgap - U_offset -> U_offset = U_c - U_bandgap
       */

      Offset = U_c - Config.Bandgap;

      /* limit offset to a valid range of -50mV - 50mV */
      if ((Offset > -50) && (Offset < 50)) Config.CompOffset = Offset;
    }
  }

  return Flag;
}



/*
 *  measure capacitance between two probe pins
 *
 *  requires:
 *  - Probe1: ID of probe to be pulled up [0-2]
 *  - Probe2: ID of probe to be pulled down [0-2]
 *  - ID: capacitor ID [0-2]
 */

void MeasureCap(uint8_t Probe1, uint8_t Probe2, uint8_t ID)
{
  uint8_t           TempByte;           /* temp. value */
  Capacitor_Type    *Cap;               /* pointer to cap data structure */
  Diode_Type        *Diode;             /* pointer to diode data structure */
  Resistor_Type     *Resistor;          /* pointer to resistor data structure */


  /*
   *  init
   */

  /* reset cap data */
  Cap = &Caps[ID];
  Cap->A = 0;
  Cap->B = 0;
  Cap->Scale = -12;           /* pF by default */
  Cap->Raw = 0;
  Cap->Value = 0;

  if (Check.Found == COMP_ERROR) return;    /* skip check on any error */


  /*
   *  Skip resistors
   *  - But check for a resistor < 10 Ohm. Might be a large cap.
   */

  if (Check.Found == COMP_RESISTOR)
  {
    Resistor = &Resistors[0];         /* pointer to first resistor */
    TempByte = 0;

    while (TempByte < Check.Resistors)
    {
      /* got matching pins */
      if (((Resistor->A == Probe1) && (Resistor->B == Probe2)) ||
          ((Resistor->A == Probe2) && (Resistor->B == Probe1)))
      {
        /* check for low value */
        if (CmpValue(Resistor->Value, Resistor->Scale, 10UL, 0) == -1)
          TempByte = 99;                /* signal low resistance and end loop */
      }

      TempByte++;      /* next one */
      Resistor++;      /* next one */  
    }

    /* we got a valid resistor */ 
    if (TempByte != 100) return;        /* skip this one */
  }


  /*
   *  Skip measurement for "dangerous" diodes
   *  - when Vf collides with the voltage of the capacitance measurement
   */

  Diode = &Diodes[0];         /* pointer to first diode */

  for (TempByte = 0; TempByte < Check.Diodes; TempByte++)
  {
    /* got matching pins and low threshold voltage */
    if ((Diode->C == Probe2) &&
        (Diode->A == Probe1) &&
        (Diode->V_f < 1500))
    {
      return;
    }

    Diode++;                  /* next one */
  }


  /*
   *  run measurements
   */

  UpdateProbes(Probe1, Probe2, 0);           /* update bitmasks and probes */

  /* first run measurement for large caps */ 
  TempByte = LargeCap(Cap);

  /* if cap is too small run measurement for small caps */
  if (TempByte == 2)
  {
    TempByte = SmallCap(Cap);
  }


  /*
   *  check for plausibility
   */

  /* if there aren't any diodes in reverse direction which could be
     detected as capacitors by mistake */
  if (Check.Diodes == 0)
  {
    /* low resistance might be a large cap */
    if (Check.Found == COMP_RESISTOR)
    {
      /* report capacitor for large C (> 4.3µF) */
      if (Cap->Scale >= -6) Check.Found = COMP_CAPACITOR;
    }

    /* we consider values below 5pF being just ghosts */
    else if ((Cap->Scale > -12) || (Cap->Value >= 5UL))
    {
      Check.Found = COMP_CAPACITOR;     /* report capacitor */
    }
  }


  /*
   *  clean up
   */

  DischargeProbes();               /* discharge DUT */

  /* reset all ports and pins */
  ADC_DDR = 0;                     /* set ADC port to input */
  ADC_PORT = 0;                    /* set ADC port low */
  R_DDR = 0;                       /* set resistor port to input */
  R_PORT = 0;                      /* set resistor port low */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef CAP_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

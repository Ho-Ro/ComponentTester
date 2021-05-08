/* ************************************************************************
 *
 *   semiconductor tests and measurements
 *
 *   (c) 2012-2014 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define SEMI_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   support functions
 * ************************************************************************ */


/*
 *  measure hFE of BJT in common collector circuit (emitter follower)
 *
 *  requires:
 *  - Type: NPN or PNP
 *
 *  returns:
 *  - hFE
 */

unsigned long Get_hFE_C(uint8_t Type)
{
  unsigned long     hFE;           /* return value */
  unsigned int      U_R_e;         /* voltage across emitter resistor */
  unsigned int      U_R_b;         /* voltage across base resistor */
  unsigned int      Ri;            /* internal resistance of MCU */


  /*
   *  measure hFE for a BJT in common collector circuit
   *  (emitter follower):
   *  - hFE = (I_e - I_b) / I_b
   *  - measure the voltages across the resistors and calculate the currents
   *    (resistor values are well known)
   *  - hFE = ((U_R_e / R_e) - (U_R_b / R_b)) / (U_R_b / R_b)
   */

  /*
   *  setup probes and get voltages
   */

  if (Type == TYPE_NPN)            /* NPN */
  {
    /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
    /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    ADC_DDR = Probes.ADC_1;             /* set probe 1 to output */
    ADC_PORT = Probes.ADC_1;            /* pull up collector directly */
    R_DDR = Probes.Rl_2 | Probes.Rl_3;  /* select Rl for probe-2 & Rl for probe-3 */
    R_PORT = Probes.Rl_3;               /* pull up base via Rl */

    U_R_e = ReadU_5ms(Probes.Pin_2);              /* U_R_e = U_e */
    U_R_b = Config.Vcc - ReadU(Probes.Pin_3);     /* U_R_b = Vcc - U_b */
  }
  else                             /* PNP */
  {
    /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
    /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
    ADC_PORT = 0;                       /* set ADC port low */
    ADC_DDR = Probes.ADC_2;             /* pull down collector directly */
    R_PORT = Probes.Rl_1;               /* pull up emitter via Rl */
    R_DDR = Probes.Rl_1 | Probes.Rl_3;  /* pull down base via Rl */

    U_R_e = Config.Vcc - ReadU_5ms(Probes.Pin_1); /* U_R_e = Vcc - U_e */
    U_R_b = ReadU(Probes.Pin_3);                  /* U_R_b = U_b */
  }

  if (U_R_b < 10)             /* I_b < 14µA -> Darlington */
  {
    /* change base resistor from Rl to Rh and measure again */
    if (Type == TYPE_NPN)            /* NPN */
    {    
      R_DDR = Probes.Rl_2 | Probes.Rh_3;     /* select Rl for probe-2 & Rh for probe-3 */
      R_PORT = Probes.Rh_3;                  /* pull up base via Rh */

      U_R_e = ReadU_5ms(Probes.Pin_2);            /* U_R_e = U_e */
      U_R_b = Config.Vcc - ReadU(Probes.Pin_3);   /* U_R_b = Vcc - U_b */

      Ri = Config.RiL;                       /* get internal resistor */
    }
    else                             /* PNP */
    {
      R_DDR = Probes.Rl_1 | Probes.Rh_3;     /* pull down base via Rh */

      U_R_e = Config.Vcc - ReadU_5ms(Probes.Pin_1);    /* U_R_e = Vcc - U_e */
      U_R_b = ReadU(Probes.Pin_3);                     /* U_R_b = U_b */

      Ri = Config.RiH;                       /* get internal resistor */
    }

    /*
     *  Since I_b is so small vs. I_e we'll neglect it and use
     *  hFE = I_e / I_b
     *      = (U_R_e / R_e) / (U_R_b / R_b)
     *      = (U_R_e * R_b) / (U_R_b * R_e)
     */

    if (U_R_b < 1) U_R_b = 1;                /* prevent division by zero */
    hFE =  U_R_e * R_HIGH;                   /* U_R_e * R_b */
    hFE /= U_R_b;                            /* / U_R_b */
    hFE *= 10;                               /* upscale to 0.1 */
    hFE /= (R_LOW * 10) + Ri;                /* / R_e in 0.1 Ohm */
  }
  else                        /* I_b > 14µA -> standard */
  {
    /*
     *  Both resistors are the same (R_e = R_b): 
     *  - hFE = ((U_R_e / R_e) - (U_R_b / R_b)) / (U_R_b / R_b)
     *  -     = (U_R_e - U_R_b) / U_R_b 
     */

    hFE = (unsigned long)((U_R_e - U_R_b) / U_R_b);
  }

  return hFE;
}



/*
 *  measure the gate threshold voltage of a depletion-mode MOSFET
 *
 *  requires:
 *  - Type: n-channel or p-channel
 */

void GetGateThreshold(uint8_t Type)
{
  signed long       Ugs = 0;       /* gate threshold voltage / Vth */
  uint8_t           Drain_Rl;      /* Rl bitmask for drain */
  uint8_t           Drain_ADC;     /* ADC bitmask for drain */
  uint8_t           PullMode;
  uint8_t           Counter;       /* loop counter */

  /*
   *  init variables
   */

  if (Type & TYPE_N_CHANNEL)       /* n-channel */
  {
    /* we assume: probe-1 = D / probe-2 = S / probe-3 = G */
    /* probe-2 is still pulled down directly */
    /* probe-1 is still pulled up via Rl */

    Drain_Rl =  Probes.Rl_1;
    Drain_ADC = Probes.ADC_1;
    PullMode = FLAG_10MS | FLAG_PULLDOWN;
  }
  else                             /* p-channel */
  {
    /* we assume: probe-1 = S / probe-2 = D / probe-3 = G */
    /* probe-2 is still pulled down via Rl */
    /* probe-1 is still pulled up directly */

    Drain_Rl =  Probes.Rl_2;
    Drain_ADC = Probes.ADC_2;
    PullMode = FLAG_10MS | FLAG_PULLUP;
  }


  /*
   *  For low reaction times we use the ADC directly.
   */

  /* sanitize bit mask for drain to prevent a never-ending loop */ 
  Drain_ADC &= 0b00000111;              /* drain */
  ADMUX = Probes.Pin_3 | (1 << REFS0);  /* select probe-3 for ADC input */

  /* sample 10 times */
  for (Counter = 0; Counter < 10; Counter++) 
  {
    wdt_reset();                         /* reset watchdog */

    /* discharge gate via Rl for 10 ms */
    PullProbe(Probes.Rl_3, PullMode);

    /* pull up/down gate via Rh to slowly charge gate */
    R_DDR = Drain_Rl | Probes.Rh_3;

    /* wait until FET conducts */
    if (Type & TYPE_N_CHANNEL)          /* n-channel */
    {
      /* FET conducts when the voltage at drain reaches low level */
      while (ADC_PIN & Drain_ADC);
    }
    else                                /* p-channel */
    {
      /* FET conducts when the voltage at drain reaches high level */
      while (!(ADC_PIN & Drain_ADC));             
    }

    R_DDR = Drain_Rl;                   /* set probe-3 to HiZ mode */

    /* get voltage of gate */
    ADCSRA |= (1 << ADSC);              /* start ADC conversion */
    while (ADCSRA & (1 << ADSC));       /* wait until conversion is done */

    /* add ADC reading */
    if (Type & TYPE_N_CHANNEL)          /* n-channel */
    {
      Ugs += ADCW;                        /* Ugs = U_g */
    }
    else                                /* p-channel */
    {
      Ugs -= (1023 - ADCW);               /* Ugs = - (Vcc - U_g) */
    }
  }

  /* calculate V_th */
  Ugs /= 10;                     /* average of 10 samples */
  Ugs *= Config.Vcc;             /* convert to voltage */
  Ugs /= 1024;                   /* using 10 bit resolution */

  /* save data */
  Semi.U_2 = (int)Ugs;           /* gate threshold voltage (in mV) */
}



/*
 *  measure leakage current
 *  - current through a semiconducter in non-conducting mode
 *
 *  returns:
 *  - leakage current in µA
 */

uint16_t GetLeakageCurrent(void)
{
  uint16_t               I_leak = 0;    /* return value */
  uint16_t               U_Rl;          /* voltage at Rl */
  uint16_t               R_Shunt;       /* shunt resistor */
  uint32_t               Value;

  /*
   *  setup probes:
   *  - use Rl as current shunt
   *  - probe-1 = pos / probe-2 = neg / probe-3 = HiZ
   *    Diode:    probe-1 = cathode /  probe-2 = anode
   *    NPN BJT:  probe-1 = collector / probe-2 = emitter
   *    PNP BJT:  probe-1 = emitter / probe-2 = collector
   */

  R_PORT = 0;                      /* set resistor port to Gnd */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */
  ADC_DDR = Probes.ADC_1;          /* set probe-1 to output */
  ADC_PORT = Probes.ADC_1;         /* pull-up probe-1 directly */

  U_Rl = ReadU_5ms(Probes.Pin_2);  /* get voltage at Rl */

  /* calculate current */
  R_Shunt = Config.RiL + (R_LOW * 10);  /* consider internal resistance of MCU (0.1 Ohms) */ 
  R_Shunt += 5;                    /* for rounding */
  R_Shunt /= 10;                   /* scale to Ohms */
  Value = U_Rl * 100000;           /* scale to 10nV */
  Value /= R_Shunt;                /* in 10nA */
  Value += 55;                     /* for rounding */
  Value /= 100;                    /* scale to µA */
  I_leak = Value;

  /* clean up */
  ADC_DDR = 0;           /* set ADC port to HiZ mode */
  ADC_PORT = 0;          /* set ADC port low */
  R_DDR = 0;             /* set resistor port to HiZ mode */
  R_PORT = 0;            /* set resistor port low */

  return I_leak;
}



/* ************************************************************************
 *   diodes
 * ************************************************************************ */


/*
 *  check for diode
 */

void CheckDiode(void)
{
  Diode_Type        *Diode;             /* pointer to diode */
  unsigned int      U1_Rl;              /* Vf #1 with Rl pull-up */
  unsigned int      U1_Rh;              /* Vf #1 with Rh pull-up */
  unsigned int      U1_Zero;            /* Vf #1 zero */
  unsigned int      U2_Rl;              /* Vf #2 with Rl pull-down */
  unsigned int      U2_Rh;              /* Vf #2 with Rh pull-down */
  unsigned int      U2_Zero;            /* Vf #2 zero */
  unsigned int      U_Diff;             /* Vf difference */

  wdt_reset();                          /* reset watchdog */

  DischargeProbes();                    /* try to discharge probes */
  if (Check.Found == COMP_ERROR) return;     /* skip on error */

  /*
   *  DUT could be:
   *  - simple diode
   *  - protection diode of a MOSFET or another device
   *  - intrinsic diode junction of a BJT
   *  - small resistor (< 3k)
   *  - capacitor (> around 22µF)
   *
   *  Solution:
   *  - Vf of a diode rises with the current within some limits (about twice
   *    for Si and Schottky). Ge, Z-diodes and LEDs are hard to determine.
   *    So it might be better to filter out other components.
   *  - For a MOSFET pretection diode we have to make sure that the MOSFET
   *    is not conducting, to be able to get Vf of the protection diode.
   *    So we discharge the gate and run the measurements twice for assumed
   *    p and n channel FETs.
   *  - Take care about the internal voltage drop of the MCU at the cathode
   *    for high test currents (Rl).
   *  - Filter out resistors by the used voltage divider:
   *    k = Rl + Ri_H + Ri_L
   *    U_Rh = U_Rl / (k - (k - 1) U_Rl / 5V)
   *    U_Rl = k U_Rh / (1 + (k - 1) U_Rh / 5V) 
   *  - Filter out caps by checking the voltage before and after measurement
   *    with Rh. In 15ms a 22µF cap would be charged from 0 to 7mV, a larger
   *    cap would have a lower voltage. We have to consider that caps also
   *    might be charged by EMI.
   *
   *  Hints:
   *  - Rl drives a current of about 7mA. That's not the best current for
   *    measuring Vf. The current for Rh is about 10.6µA.
   *    Most DMMs use 1mA.
   */


  /*
   *  Vf #1, supporting a possible p-channel MOSFET
   */

  /* we assume: probe-1 = A / probe2 = C */
  /* set probes: Gnd -- probe-2 / probe-1 -- HiZ */
  ADC_PORT = 0;
  ADC_DDR = Probes.ADC_2;               /* pull down cathode directly */
  /* R_DDR is set to HiZ by DischargeProbes() */
  U1_Zero = ReadU(Probes.Pin_1);        /* get voltage at anode */

  /* measure voltage across DUT (Vf) with Rh */
  /* set probes: Gnd -- probe-2 / probe-1 -- Rh -- Vcc */
  R_DDR = Probes.Rh_1;                  /* enable Rh for probe-1 */
  R_PORT = Probes.Rh_1;                 /* pull up anode via Rh */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLUP);     /* discharge gate */
  U1_Rh = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
                                        /* neglect voltage at cathode */

  /* measure voltage across DUT (Vf) with Rl */
  /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
  R_DDR = Probes.Rl_1;                  /* enable Rl for probe-1 */
  R_PORT = Probes.Rl_1;                 /* pull up anode via Rl */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLUP);     /* discharge gate */
  U1_Rl = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
  U1_Rl -= ReadU(Probes.Pin_2);         /* substract voltage at cathode */


  DischargeProbes();                    /* try to discharge probes */
  if (Check.Found == COMP_ERROR) return;     /* skip on error */  


  /*
   *  Vf #2, supporting a possible n-channel MOSFET
   */

  /* we assume: probe-1 = A / probe2 = C */
  /* set probes: Gnd -- probe-2 / probe-1 -- HiZ */
  ADC_PORT = 0;
  ADC_DDR = Probes.ADC_2;               /* pull down cathode directly */
  /* R_DDR is set to HiZ by DischargeProbes() */
  U2_Zero = ReadU(Probes.Pin_1);        /* get voltage at anode */

  /* set probes: Gnd -- Rh -- probe-2 / probe-1 -- Vcc */
  ADC_DDR = 0;                          /* set to HiZ to prepare change */
  ADC_PORT = Probes.ADC_1;              /* pull up anode directly */
  ADC_DDR = Probes.ADC_1;               /* enable output */
  R_PORT = 0;                           /* pull down cathode via Rh */
  R_DDR = Probes.Rh_2;                  /* enable Rh for probe-2 */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);   /* discharge gate */
  U2_Rh = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
  U2_Rh -= ReadU(Probes.Pin_2);         /* substract voltage at cathode */

  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_DDR = Probes.Rl_2;                  /* pull down cathode via Rl */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);   /* discharge gate */
  U2_Rl = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
  U2_Rl -= ReadU(Probes.Pin_2);         /* substract voltage at cathode */

  ADC_DDR = 0;                     /* stop pulling up */


  /*
   *  process results
   */

  /* choose between both measurements */
  if (U1_Rl > U2_Rl)          /* the higher voltage wins */
  {
    U_Diff = U1_Rl - U2_Rl;   /* difference of U_Rls */
    U2_Rl = U1_Rl;
    U2_Rh = U1_Rh;
    U2_Zero = U1_Zero;
  }
  else
  {
    U_Diff = U2_Rl - U1_Rl;   /* difference of U_Rls */
  }


  /*
   *  check difference of U_Rl measurements
   *  - nearly zero for diodes, BJTs etc.
   *  - about a diode drop for enh-mode MOSFETs
   *  - >1000mV for dep-mode FETs partly conducting 
   */

  if (U_Diff > 1000) return;       /* dep-mode FET not fully turned off */


  /*
   *  U_Rh < 10mV for
   *  - resistor < 1k Ohm
   *  - very large cap
   */

  if (U2_Rh <= 10) return;         /* small resistor or very large cap */


  /*
   *  U_Zero <= 2 for resistor or diode 
   *  U_Zero > 2 for cap or diode
   *  if U_Zero > 2 then U_Rh - U_Zero < 100 for cap
   *
   *  Hints:
   *  If U_Zero > 10 and U_Rh is about U_Zero it's a large cap.
   *  As larger the cap as lower U_Rl (charging time 15ms).
   */

  if (U2_Rh > U2_Zero)             /* prevent underrun */
  {
    U_Diff = U2_Rh - U2_Zero;      /* calculate difference */
  }
  else
  {
    U_Diff = U2_Zero - U2_Rh;
  }

  if ((U2_Zero > 2) && (U_Diff < 100)) return;    /* capacitor */


  /*
   *  The voltages for a resistor will follow the equation:
   *    k = Rl + Ri_H + Ri_L
   *    Ul = k U_Rh / (1 + (k - 1) U_Rh / 5V)
   *  Allow a small tolerance.
   *  For U_Rh > 40mV we don't need to check for a resistor.
   *
   *  Hint: Actually we could change the threshold above from 10 to 40 and
   *  remove this test completely. The lowest U_Rh measured for a diode was
   *  56mV for a AA118.
   */

  if (U2_Rh < 40)             /* resistor (< 3k) */
  {
    uint32_t      a, b;

    /* calculate expected U_Rl based on measured U_Rh in mV */
    b = (R_HIGH * 10) / ((R_LOW * 10) + Config.RiH + Config.RiL);  /* k factor */
    a = b - 1;                          /* k - 1 */
    a /= 5;                             /* / 5V */
    a *= U2_Rh;                         /* *U_Rh */
    a += 1000;                          /* +1 (1000 for mV) */
    b *= 1000;                          /* for mV */
    b *= U2_Rh;                         /* *U_Rh */
    b /= a;                             /* U_Rl in mV */

    /* check if calculated U_Rl is within some % of measured value */
    U1_Zero = (unsigned int)b;
    U1_Rl = U1_Zero;
    U1_Rh = U1_Zero;
    U1_Zero /= 10;            /* 10% */
    U1_Rh += U1_Zero;         /* 110% */
    U1_Zero = (unsigned int)b;
    U1_Zero /= 33;            /* 3% */
    U1_Rl -= U1_Zero;         /* 97% (for resistors near 1k) */

    if ((U2_Rl >= U1_Rl) && (U2_Rl <= U1_Rh)) return;     /* resistor */
  }


  /*
   *  if U_Rl (Vf) is between 0.15V and 4.64V it's a diode
   */

  if ((U2_Rl > 150) && (U2_Rl < 4640))
  {
    /* if we haven't found any other component yet */
    if ((Check.Found == COMP_NONE) ||
        (Check.Found == COMP_RESISTOR))
    {
      Check.Found = COMP_DIODE;
    }

    /* save data */
    Diode = &Diodes[Check.Diodes];
    Diode->A = Probes.Pin_1;
    Diode->C = Probes.Pin_2;
    Diode->V_f = U2_Rl;       /* Vf for high measurement current */
    Diode->V_f2 = U2_Rh;      /* Vf for low measurement current */
    Check.Diodes++;
  }
}



/* ************************************************************************
 *   BJTs and FETs
 * ************************************************************************ */


/*
 *  verify MOSFET by checking the body diode
 */

void VerifyMOSFET(void)
{
  uint8_t           Flag = 0;
  uint8_t           n = 0;
  uint8_t           Anode;
  uint8_t           Cathode;
  Diode_Type        *Diode;             /* pointer to diode */

  /* set expected body diode */
  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel */
  {
    Anode = Semi.C;      /* anode at source */
    Cathode = Semi.B;    /* cathode at drain */
  }
  else                                  /* p-channel */
  {
    Anode = Semi.B;      /* anode at drain */
    Cathode = Semi.C;    /* cathode at source */
  }

  Diode = &Diodes[0];              /* first diode */

  /* check all known diodes for reversed one */
  while (n < Check.Diodes)
  {
    if ((Diode->A == Cathode) && (Diode->C == Anode))
    {
      Flag = 1;          /* signal match */
      n = 10;            /* end loop */
    }

    n++;                 /* next diode */
    Diode++;
  }

  if (Flag == 1)         /* found reversed diode */
  {
    /* this can't be a MOSFET, so let's reset */
    Check.Found = COMP_NONE;
    Check.Type = 0;
    Check.Done = 0;
  }
}



/*
 *  check for BJT or enhancement-mode MOSFET
 *
 *  requires:
 *  - BJT_Type: NPN or PNP (also used for FET channel type)
 *  - U_Rl: voltage across Rl pulled down
 */

void CheckBJTorEnhModeMOSFET(uint8_t BJT_Type, unsigned int U_Rl)
{
  uint8_t           FET_Type;           /* MOSFET type */
  unsigned int      U_R_c;              /* voltage across collector resistor */
  unsigned int      U_R_b;              /* voltage across base resistor */
  unsigned int      BJT_Level;          /* voltage threshold for BJT */
  unsigned int      FET_Level;          /* voltage threshold for FET */
  unsigned int      I_CE0;              /* leakage current */
  unsigned long     hFE_C;              /* hFE (common collector) */
  unsigned long     hFE_E;              /* hFE (common emitter) */

  /*
   *  init, set probes and measure
   */

  if (BJT_Type == TYPE_NPN)   /* NPN / n-channel */
  {
    BJT_Level = 2557;         /* voltage across base resistor (5.44µA) */
    FET_Level = 3400;         /* voltage across drain resistor (4.8mA) */
    FET_Type = TYPE_N_CHANNEL;

    /*
     *  we assume
     *  - BJT: probe-1 = C / probe-2 = E / probe-3 = B
     *  - FET: probe-1 = D / probe-2 = S / probe-3 = G
     *  probes already set to: Gnd -- probe-2 / probe-1 -- Rl -- Vcc
     *  drive base/gate via Rh instead of Rl
     */

    R_DDR = Probes.Rl_1 | Probes.Rh_3;  /* enable Rl for probe-1 & Rh for probe-3 */
    R_PORT = Probes.Rl_1 | Probes.Rh_3; /* pull up collector via Rl and base via Rh */
    wait50ms();                         /* wait to skip gate charging of a FET */
    U_R_c = Config.Vcc - ReadU(Probes.Pin_1);     /* U_R_c = Vcc - U_c */ 
    U_R_b = Config.Vcc - ReadU(Probes.Pin_3);     /* U_R_b = Vcc - U_b */
  }
  else                        /* PNP / p-channel */
  {
    BJT_Level = 977;          /* voltage across base resistor (2.1µA) */
    FET_Level = 2000;         /* voltage across drain resistor (2.8mA) */
    FET_Type = TYPE_P_CHANNEL;

    /*
     *  we assume
     *  - BJT: probe-1 = E / probe-2 = C / probe-3 = B
     *  - FET: probe-1 = S / probe-2 = D / probe-3 = G
     *  probes already set to: Gnd -- Rl - probe-2 / probe-1 -- Vcc
     *  drive base/gate via Rh instead of Rl
     */

    R_DDR = Probes.Rl_2 | Probes.Rh_3;  /* pull down base via Rh */
    U_R_c = ReadU_5ms(Probes.Pin_2);    /* U_R_c = U_c */
    U_R_b = ReadU(Probes.Pin_3);        /* U_R_b = U_b */
  }


  /*
   *  distinguish BJT from enhancement-mode MOSFET
   */

  if (U_R_b > BJT_Level)      /* U_R_b exceeds minimum level of BJT */
  {
    /*
     *  A voltage drop across the base resistor Rh means that a current
     *  is flowing constantly. So this can't be a FET.
     *
     *  Problem:
     *  A reversed collector and emitter also passes the tests, but with a
     *  lower hFE. So we need to run the BJT test twice to be sure and select
     *  the results with the higher hFE.
     */

    if (Check.Found == COMP_BJT)        /* second test run */
    {
      Check.Done = 1;                   /* no more tests needed */

      /*
       *  If the type is different from the one in the first run, we have
       *  a parasitic BJT (caused by a freewheeling diode on the same substrate).
       */

      if (!(Check.Type & BJT_Type)) Check.Type |= TYPE_PARASITIC;
    }
    else                                /* first test run */
    {
      Check.Found = COMP_BJT;
      Check.Type = BJT_Type;
    }

    /* leakage current */
    I_CE0 = GetLeakageCurrent();        /* get leakage current (in µA) */


    /*
     *  Calculate hFE via voltages and known resistors:
     *  - hFE = I_c / I_b
     *        = (U_R_c / R_c) / (U_R_b / R_b)
     *        = (U_R_c * R_b) / (U_R_b * R_c)
     *  - consider leakage current:
     *    I_c = I_c_conducting - I_c_leak
     *        = (U_R_c_conducting / R_c) - (U_R_c_leak / R_c)
     *        = (U_R_c_conducting - U_R_c_leak) / R_c
     *    -> U_R_c = U_R_c_conducting - U_R_c_leak
     *             = U_R_c_conducting - U_Rl
     */

    if (U_R_c > U_Rl) U_R_c -= U_Rl;       /* - U_Rl (leakage) */
    hFE_E = U_R_c * R_HIGH;                /* U_R_c * R_b */
    hFE_E /= U_R_b;                        /* / U_R_b */
    hFE_E *= 10;                           /* upscale to 0.1 */

    if (BJT_Type == TYPE_NPN)      /* NPN */
      hFE_E /= (R_LOW * 10) + Config.RiH;    /* / R_c in 0.1 Ohm */
    else                           /* PNP */
      hFE_E /= (R_LOW * 10) + Config.RiL;    /* / R_c in 0.1 Ohm */

    /* get hFE for common collector circuit */
    hFE_C = Get_hFE_C(BJT_Type);

    /* keep higher hFE */
    if (hFE_C > hFE_E) hFE_E = hFE_C;

    /* parasitic BJT */
    if (Check.Type & TYPE_PARASITIC)
    {
      /* may we assume that the BJT with the lower hFE is the correct one? */

      hFE_E = 0;    /* we keep the first type found at the moment */
    }

    /* only update data if hFE is higher than old one or not set yet */
    if (hFE_E > Semi.F_1)
    {
      /* save data */
      Semi.F_1 = hFE_E;            /* hFE */
      Semi.I_1 = I_CE0;            /* leakage current */
      Semi.A = Probes.Pin_3;       /* base pin */

      /* update Collector and Emitter pins */
      if (BJT_Type == TYPE_NPN)    /* NPN */
      {
        Semi.B = Probes.Pin_1;     /* collector pin */
        Semi.C = Probes.Pin_2;     /* emitter pin */
      }
      else                         /* PNP */
      {
        Semi.B = Probes.Pin_2;     /* collector pin */
        Semi.C = Probes.Pin_1;     /* emitter pin */
      }
    }
  }
  else if ((U_Rl < 97) && (U_R_c > FET_Level))    /* no BJT */
  {
    /*
     *  If there's
     *  - just a small leakage current (< 0.1mA) in non-conducting mode
     *  - a large U_R_c (= large current) when conducting
     *  - a low U_R_b (= very low gate current)
     *  we got a FET or an IGBT.
     */

    /*
     *  The drain source channel of a MOSFET is modeled as a resistor
     *  while an IGBT acts more like a diode. So we measure the voltage drop
     *  across the conducting path. A MOSFET got a low voltage drop based on
     *  it's R_DS_on and the current. An IGBT got a much higher voltage drop.
     */

    I_CE0= ReadU(Probes.Pin_1) - ReadU(Probes.Pin_2);

    if (I_CE0 < 250)          /* MOSFET */
    {
      Check.Found = COMP_FET;
      Check.Type = FET_Type | TYPE_ENHANCEMENT | TYPE_MOSFET;
    }
    else                      /* IGBT */
    {
      Check.Found = COMP_IGBT;
      Check.Type = FET_Type | TYPE_ENHANCEMENT;
    }

    Check.Done = 1;           /* transistor found */

    /* measure gate threshold voltage */
    GetGateThreshold(FET_Type);

    /* save data */
    Semi.A = Probes.Pin_3;          /* gate pin */

    if (FET_Type == TYPE_N_CHANNEL)     /* n-channel */
    {
      Semi.B = Probes.Pin_1;       /* drain pin */
      Semi.C = Probes.Pin_2;       /* source pin */
    }
    else                                /* p-channel */
    {
      Semi.B = Probes.Pin_2;       /* drain pin */
      Semi.C = Probes.Pin_1;       /* source pin */
    }
  }
}



/*
 *  check for a depletion mode FET (self conducting)
 *
 *  requires:
 *  - voltage across Rl in pull-down mode
 */

void CheckDepletionModeFET(void)
{
  unsigned int      U_1;                /* voltage #1 */
  unsigned int      U_2;                /* voltage #2 */
  unsigned int      Diff_1 = 0;         /* voltage difference #1 */
  unsigned int      Diff_2 = 0;         /* voltage difference #2 */
  uint8_t           Flag = 0;           /* signal */

  /*
   *  required probe setup (by calling function):
   *  - Gnd -- Rl -- probe-2 / probe-1 -- Vcc
   */


  /*
   *  check if we got a n-channel JFET or depletion-mode MOSFET
   *  - JFETs are depletion-mode only
   */

  if (Check.Done == 0)        /* no transistor found yet */
  {
    /* we assume: probe-1 = D / probe-2 = S / probe-3 = G */
    /* probes already set to: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    R_DDR = Probes.Rl_2 | Probes.Rh_3;  /* pull down gate via Rh */
    U_1 = ReadU_20ms(Probes.Pin_2);     /* voltage at source */

    R_PORT = Probes.Rh_3;               /* pull up gate via Rh */
    U_2 = ReadU_20ms(Probes.Pin_2);     /* voltage at source */
    Diff_1 = U_2 - U_1;                 /* source voltage difference */

    /*
     *  If the source voltage is higher when the gate is driven by a positive
     *  voltage vs. connected to ground we got a depletion-mode n-channel FET.
     *  The source resistor creates a voltage offset based on the current
     *  causing V_GS to become negative with the gate pulled down.
     */

    if (U_2 > (U_1 + 488))
    {

      /*
       *  same measurements with assumed drain and source reversed:
       *  - to detect if JFET is symmetrical
       *  - to detect drain and source of MOSFET
       */

      /* we simulate: probe-1 = S / probe-2 = D / probe-3 = G */
      /* set probes: Gnd -- Rl -- probe-1 / probe-2 -- Vcc */
      ADC_PORT = Probes.ADC_2;               /* set ADC port to high */
      ADC_DDR = Probes.ADC_2;                /* pull up drain directly */
      R_DDR = Probes.Rl_1 | Probes.Rh_3;     /* enable Rl for source and Rh for gate */
      R_PORT = 0;                            /* pull down source via Rl / pull down gate via Rh */
      U_1 = ReadU_20ms(Probes.Pin_1);        /* voltage at source */

      R_PORT = Probes.Rh_3;                  /* pull up gate via Rh */
      U_2 = ReadU_20ms(Probes.Pin_1);        /* voltage at source */
      Diff_2 = U_2 - U_1;                    /* source voltage difference */


      /*
       *  Compare gate voltages to distinguish JFET from MOSFET
       */

      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
      ADC_PORT = 0;                          /* set ADC port to low */
      ADC_DDR = Probes.ADC_2;                /* pull down source directly */
      R_DDR = Probes.Rl_1 | Probes.Rh_3;     /* enable Rl for probe-1 & Rh for probe-3 */
      R_PORT = Probes.Rl_1 | Probes.Rh_3;    /* pull up drain via Rl / pull up gate via Rh */

      U_1 = ReadU_20ms(Probes.Pin_3);        /* get voltage at gate */

      if (U_1 > 3911)              /* MOSFET */
      {
        /* n channel depletion-mode MOSFET */ 
        Check.Type = TYPE_N_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET;
      }
      else                         /* JFET */
      {
        /* n channel JFET (depletion-mode only) */
        Check.Type = TYPE_N_CHANNEL | TYPE_DEPLETION | TYPE_JFET;
      }

      Flag = 1;                    /* signal match */
    }
  }


  /*
   *  check if we got a p-channel JFET or depletion-mode MOSFET
   *  - JFETs are depletion-mode only
   */

  if ((Check.Done == 0) && (Flag == 0))      /* no transistor found yet */
  {
    /* we assume: probe-1 = S / probe-2 = D / probe-3 = G */
    /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
    ADC_PORT = 0;                       /* set ADC port to Gnd */
    ADC_DDR = Probes.ADC_2;             /* pull down drain directly */
    R_DDR = Probes.Rl_1 | Probes.Rh_3;  /* enable Rl for probe-1 & Rh for probe-3 */
    R_PORT = Probes.Rl_1 | Probes.Rh_3; /* pull up source via Rl / pull up gate via Rh */
    U_1 = ReadU_20ms(Probes.Pin_1);     /* get voltage at source */

    R_PORT = Probes.Rl_1;               /* pull down gate via Rh */
    U_2 = ReadU_20ms(Probes.Pin_1);     /* get voltage at source */
    Diff_1 = U_1 - U_2;                 /* source voltage difference */

    /*
     *  If the source voltage is higher when the gate is driven by a positive
     *  voltage vs. connected to ground we got a depletion-mode p-channel FET.
     *  The source resistor creates a voltage offset based on the current
     *  causing V_GS to become positive with the gate pulled up.
     */

    if (U_1 > (U_2 + 488))
    {

      /*
       *  same measurements with assumed drain and source reversed:
       *  - to detect if JFET is symmetrical
       *  - to detect drain and source of MOSFET
       */

      /* we simulate: probe-1 = D / probe-2 = S / probe-3 = G */
      /* set probes: Gnd -- probe-1 / probe-2 -- Rl -- Vcc */
      ADC_DDR = Probes.ADC_1;             /* pull down drain directly */
      R_DDR = Probes.Rl_2 | Probes.Rh_3;  /* enable Rl for probe-2 & Rh for probe-3 */
      R_PORT = Probes.Rl_2 | Probes.Rh_3; /* pull up source via Rl / pull up gate via Rh */
      U_1 = ReadU_20ms(Probes.Pin_2);     /* get voltage at source */

      R_PORT = Probes.Rl_2;               /* pull down gate via Rh */
      U_2 = ReadU_20ms(Probes.Pin_2);     /* get voltage at source */
      Diff_2 = U_1 - U_2;                 /* source voltage difference */


      /*
       *  Compare gate voltages to distinguish JFET from MOSFET
       */

      /* set probes: probe-2 = HiZ / probe-1 -- Vcc */
      ADC_PORT = Probes.ADC_1;          /* pull up source directly */
      ADC_DDR = Probes.ADC_1;           /* enable pull up for source */
      /* gate is still pulled down via Rh */
      U_1 = ReadU_20ms(Probes.Pin_3);   /* get voltage at gate */

      if (U_1 < 977)               /* MOSFET */
      {
        /* p-channel depletion-mode MOSFET */ 
        Check.Type =  TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET;
      }
      else                         /* JFET */
      {
        /* p-channel JFET (depletion-mode only) */
        Check.Type = TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_JFET;
      }

      Flag = 1;                    /* signal match */
    }
  }


  /*
   *  on match process and save data
   */

  if (Flag == 1)         /* found depletion-mode FET */
  {
    /* common stuff */
    Check.Found = COMP_FET;
    Check.Done = 1;
    Semi.A = Probes.Pin_3;         /* gate pin */

    /*
     *  drain & source pinout
     *  - larger voltage difference wins
     */

    if (Diff_1 > Diff_2)      /* drain and source as assumed */
    {
      Semi.B = Probes.Pin_1;       /* drain pin */
      Semi.C = Probes.Pin_2;       /* source pin */
    }
    else                      /* drain and source reversed */
    {
      Semi.B = Probes.Pin_2;       /* drain pin */
      Semi.C = Probes.Pin_1;       /* source pin */
    }

    /*
     *  drain & source symmetry
     *  - if both voltage differences are about the same we got a
     *    symmetrical FET
     */

    U_2 = Diff_1 / 50;             /* 2% of Diff_1 */
    U_1 = Diff_1 - U_2;            /* 98% */
    U_2 += Diff_1;                 /* 102% */
    if ((Diff_2 >= U_1) && (Diff_2 <= U_2))
    {
      Check.Type |= TYPE_SYMMETRICAL; 
    }
  }
}



/* ************************************************************************
 *   special devices
 * ************************************************************************ */


/*
 *  check for Thyristor and Triac
 *
 *  returns:
 *  - 1 if component was found
 *  - 0 if component wasn't found
 */

uint8_t CheckThyristorTriac(void)
{
  uint8_t           Flag = 0;           /* return value */
  unsigned int      U_1;                /* voltage #1 */
  unsigned int      U_2;                /* voltage #2 */
  unsigned int      V_GT;               /* gate trigger voltage */

  /*
   *  check for a Thyristor (SCR) or Triac
   *  - A thyristor conducts also after the gate is discharged as long
   *    as the load current stays alive and doesn't reverse polarity.
   *  - A triac is similar to a pair of anti-parallel thyristors but is
   *    triggered also by negative gate current. 
   *  - It's possible that the tester doesn't deliver enough current, so
   *    it can't detect all types.
   */

  /*
   *  probes need to be set already to:
   *    Gnd -- probe-2 / probe-1 -- Rl -- Vcc / probe-3 -- Rl -- Vcc
   */

  /* we assume: probe-1 = A / probe-2 = C / probe-3 = G for a SCR */
  /*            probe-1 = MT2 / probe-2 = MT1 / probe-3 = G for a triac */  

  /* V_GT (gate trigger voltage) */
  U_1 = ReadU(Probes.Pin_3);            /* voltage at gate */
  U_2 = ReadU(Probes.Pin_2);            /* voltage at cathode */
  V_GT = U_1 - U_2;                     /* = Ug - Uc */

  /* discharge gate and check load current */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);    /* discharge gate */
  U_1 = ReadU_5ms(Probes.Pin_1);        /* get voltage at anode */

  /* simulate short loss of current and check load current again */ 
  R_PORT = 0;                           /* pull down anode */
  wait5ms();
  R_PORT = Probes.Rl_1;                 /* and pull up anode again */
  U_2 = ReadU_5ms(Probes.Pin_1);        /* get voltage at anode (below Rl) */

  /* voltages at anode match behaviour of thyristor or triac */
  if ((U_1 < 1600) && (U_2 > 4400))
  {
    /*
     *  Now we check if the DUT is a thyristor or a triac:
     *  - set Gate to HiZ and reverse Anode & Cathode
     *  - check if DUT doesn't conduct
     *  - trigger gate by pulling it down to Gnd for a short moment
     *  - check if DUT passes current
     *  A thyrister shouldn't conduct but a triac should (Q3 operation mode).
     */

    /* reverse Anode and Cathode (MT2 and MT1) */
    /* set probes: Gnd -- probe-1 / probe-2 -- Rl -- Vcc  */
    R_DDR = 0;                          /* disable all probe resistors */
    R_PORT = 0;
    ADC_PORT = Probes.ADC_2;            /* pull up Cathode directly */
    wait5ms();
    R_DDR = Probes.Rl_1;                /* pull down Anode via Rl */ 
    /* probe-3 = gate is in HiZ mode */

    /* check if DUT doesn't conduct */
    U_1 = ReadU_5ms(Probes.Pin_1);      /* get voltage at Anode */

    if (U_1 <= 244)      /* voltage at Anode is low (no current) */
    {
      /* trigger the gate with a negative current (Triac: Q3) */
      PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);

      /* check for conduction */
      U_1 = ReadU_5ms(Probes.Pin_1);    /* get voltage at Anode */

      if (U_1 < 733)         /* no current -> Thyristor */
      {
        Check.Found = COMP_THYRISTOR;   /* we found a Thyristor */
        Check.Done = 1;                 /* detected component */
        Flag = 2;                       /* save data and signal success */
      }
      else                   /* got current -> Triac */
      {
        /*
         *  To verify the Triac we stop the current flow for a moment and check
         *  the voltage at MT2 again. The Triac shouldn't conduct anymore,
         */

        /* drop load current for a moment */
        R_PORT = Probes.Rl_1;           /* pull up MT2 via Rl */
        wait5ms();
        R_PORT = 0;                     /* and pull down MT2 via Rl */

        /* and check load current again */
        U_2 = ReadU_5ms(Probes.Pin_1);  /* get voltage at MT2 */

        if (U_2 <= 244)       /* no current */
        {
          if (Check.Found == COMP_TRIAC)     /* second test run */
          {
            Check.Done = 1;             /* no more tests needed */
          }

          Check.Found = COMP_TRIAC;     /* found Triac */

          /*
           *  Triac could be in Q3 or Q4 operation mode. If G and MT1 are swapped
           *  the triac would pass the former check in Q4 mode but the current
           *  through MT2 would be a little bit lower. Another issue is that some
           *  Triacs don't support Q4. So we support up to two test runs and prefer
           *  the one with the higher voltage at MT2.
           */

          if (U_1 > Semi.I_1)      /* first run or higher current */
          {
            Semi.I_1 = U_1;        /* update reference value */
            Flag = 2;              /* save data and signal success */
          }
          else                     /* wrong pinout */
          {
            Flag = 1;              /* signal success, but don't save data */
          }
        }
      }
    }
  }

  if (Flag == 2)         /* save data and signal success */
  {
    /* save data */
    Semi.A = Probes.Pin_3;    /* Gate pin */
    Semi.B = Probes.Pin_1;    /* Anode/MT2 pin */
    Semi.C = Probes.Pin_2;    /* Cathode/MT1 pin */
    Semi.U_1 = V_GT;          /* gate trigger voltage (in mV) */

    Flag = 1;                 /* signal success */
  }

  return Flag;
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef SEMI_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

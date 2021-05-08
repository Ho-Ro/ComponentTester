/* ************************************************************************
 *
 *   semiconductor tests and measurements
 *
 *   (c) 2012-2013 by Markus Reschke
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
  unsigned int      Ri;            /* internal resistance of µC */


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
  unsigned long     Uth = 0;       /* gate threshold voltage */
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
      Uth += ADCW;                        /* U_g = U_measued */
    }
    else                                /* p-channel */
    {
      Uth += (1023 - ADCW);               /* U_g = Vcc - U_measured */
    }
  }

  /* calculate V_th */
  Uth /= 10;                     /* average of 10 samples */
  Uth *= Config.Vcc;             /* convert to voltage */
  Uth /= 1024;                   /* using 10 bit resolution */

  /* save data */
  FET.V_th = (unsigned int)Uth;
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
  unsigned int      U2_Rl;              /* Vf #2 with Rl pull-up */
  unsigned int      U2_Rh;              /* Vf #2 with Rh pull-up */
  unsigned int      U2_Zero;            /* Vf #2 zero */

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
   *    in not conducting, to be able to get Vf of the protection diode.
   *    So we discharge the gate and run the measurements twice for p and n
   *    channel FETs.
   *  - Take care about the internal voltage drop of the µC at the cathode
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
  /* set probes: Gnd -- probe-2 / probe-1 -- Rl or Rh -- Vcc */
  ADC_PORT = 0;
  ADC_DDR = Probes.ADC_2;               /* pull down cathode directly */
  /* R_DDR is set to HiZ by DischargeProbes(); */
  U1_Zero = ReadU(Probes.Pin_1);        /* get voltage at anode */

  /* measure voltage across DUT (Vf) with Rh */
  R_DDR = Probes.Rh_1;                  /* enable Rh for probe-1 */
  R_PORT = Probes.Rh_1;                 /* pull up anode via Rh */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLUP);     /* discharge gate */
  U1_Rh = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
                                        /* neglect voltage at cathode */

  /* measure voltage across DUT (Vf) with Rl */
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
  /* set probes: Gnd -- probe-2 / probe-1 -- Rl or Rh -- Vcc */
  ADC_PORT = 0;
  ADC_DDR = Probes.ADC_2;               /* pull down cathode directly */
  U2_Zero = ReadU(Probes.Pin_1);        /* get voltage at anode */

  /* measure voltage across DUT (Vf) with Rh */
  R_DDR = Probes.Rh_1;                  /* enable Rh for probe-1 */
  R_PORT = Probes.Rh_1;                 /* pull up anode via Rh */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);   /* discharge gate */
  U2_Rh = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
                                        /* neglect voltage at cathode */

  /* measure voltage across DUT (Vf) with Rl */
  R_DDR = Probes.Rl_1;                  /* enable Rl for probe-1 */
  R_PORT = Probes.Rl_1;                 /* pull up anode via Rl */   
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);   /* discharge gate */
  U2_Rl = ReadU_5ms(Probes.Pin_1);      /* get voltage at anode */
  U2_Rl -= ReadU(Probes.Pin_2);         /* substract voltage at cathode */


  R_PORT = 0;                      /* stop pulling up */


  /*
   *  process results
   */

  /* choose between measurements of p and n channel setup */
  if (U1_Rl > U2_Rl)          /* the higher voltage wins */
  {
    U2_Rl = U1_Rl;
    U2_Rh = U1_Rh;
    U2_Zero = U1_Zero;
  }


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

  U1_Zero = U2_Rh - U2_Zero;       /* voltage difference */

  if ((U2_Zero > 2) && (U1_Zero < 100)) return;   /* capacitor */


  /*
   *  The voltages for a resistor will follow the equation:
   *    k = Rl + Ri_H + Ri_L
   *    Ul = k U_Rh / (1 + (k - 1) U_Rh / 5V)
   *  Allow a tolerance of 3%.
   *  For U_Rh > 40mV we don't need to check for a resistor.
   *
   *  Hint: Actually we could change the threshold above from 10 t0 40 and
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
    U1_Zero /= 50;            /* 2% */
    U1_Rh += U1_Zero;         /* 102% */
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
 *   BJT and FET
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
    Anode = FET.S;
    Cathode = FET.D;
  }
  else                                  /* p-channel */
  {
    Anode = FET.D;
    Cathode = FET.S;
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
   *  distinguish BJT from depletion-mode MOSFET
   */

  if (U_R_b > BJT_Level)      /* U_R_b exceeds minimum level of BJT */
  {
    /*
     *  A voltage drop across the base resistor Rh means that a current
     *  is flowing constantly. So this can't be a FET.
     *
     *  Problem:
     *  A reversed collector and emitter also passes the tests, but with
     *  a low hFE. So we need to run two tests to be sure and select the
     *  test results with the higher hFE.
     */

    /* two test runs needed at maximium to get right hFE & pins */
    if (Check.Found == COMP_BJT) Check.Done = 1;

    Check.Found = COMP_BJT;
    Check.Type = BJT_Type;

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

    /* keep largest hFE */
    if (hFE_C > hFE_E) hFE_E = hFE_C;

    /* only update data if hFE is larger than old one */ 
    if (hFE_E > BJT.hFE)
    {
      /* save data */
      BJT.hFE = hFE_E;
      BJT.I_CE0 = I_CE0;
      BJT.B = Probes.Pin_3;

      if (BJT_Type == TYPE_NPN)    /* NPN */
      {
        BJT.C = Probes.Pin_1;
        BJT.E = Probes.Pin_2;
      }
      else                         /* PNP */
      {
        BJT.C = Probes.Pin_2;
        BJT.E = Probes.Pin_1;
      }
    }

#if 0
    /*
     *  Check for proper emitter and collector:
     *  - I_c is much lower for reversed emitter and collector.
     *  - So we reverse the probes and measure I_c (= U_R_c / R_c) again.
     *  - Since R_c is constant we may simply compare U_R_c.
     *
     *  This is an alternative solution instead of running the check two times.
     */

    ADC_DDR = 0;              /* set ADC port to HiZ mode */
    R_DDR = 0;                /* set resistor port to HiZ mode */

    if (BJT_Type == TYPE_NPN)   /* NPN */
    {
      /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
      /* set probes: Gnd -- probe-1 / probe-2 -- Rl -- Vcc */
      ADC_PORT = 0;
      ADC_DDR = Probes.ADC_1;                     /* pull-down emitter directly */
      R_PORT = Probes.Rl_2 | Probes.Rh_3;         /* pull-up base via Rh */
      R_DDR = Probes.Rl_2 | Probes.Rh_3;          /* enable probe resistors */
      U_R_b = Config.Vcc - ReadU_5ms(Probes.Pin_2);    /* U_R_c = Vcc - U_c */        
    }
    else                        /* PNP */
    { 
      /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
      /* set probes: Gnd -- Rl - probe-1 / probe-2 -- Vcc */
      R_PORT = 0;
      R_DDR = Probes.Rl_1 | Probes.Rh_3;     /* pull down base via Rh */
      ADC_DDR = Probes.ADC_2;
      ADC_PORT = Probes.ADC_2;               /* pull-up emitter directly */
      U_R_b = ReadU_5ms(Probes.Pin_1);       /* U_R_c = U_c */
    }

    /* if not reversed, BJT is identified */
//    U_R_b *= 10;                   /* be much larger */
    if (U_R_c > U_R_b)             /* I_c > I_c_reversed */
    {
      /* move other stuff here: save data & Comp= */
      Check.Done = 1;
    }
#endif

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
    FET.G = Probes.Pin_3;

    if (FET_Type == TYPE_N_CHANNEL)     /* n-channel */
    {
      FET.D = Probes.Pin_1;
      FET.S = Probes.Pin_2;      
    }
    else                                /* p-channel */
    {
      FET.D = Probes.Pin_2;
      FET.S = Probes.Pin_1;
    }
  }
}



/*
 *  check for a depletion mode FET (self conducting)
 *
 *  requires:
 *  - voltage across Rl in pull-down mode
 */

void CheckDepletionModeFET(unsigned int U_Rl_L)
{
  unsigned int      U_1;                /* voltage #1 */
  unsigned int      U_2;                /* voltage #2 */


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

    /*
     *  If the source voltage is higher when the gate is driven by a positive
     *  voltage vs. connected to ground we got a depletion-mode n-channel FET.
     *  The source resistor creates a voltage offset based on the current
     *  causing V_GS to become negative with the gate pulled down.
     */

    if (U_2 > (U_1 + 488))
    {
      /*
       *  Compare gate voltages to distinguish JFET from MOSFET
       */

      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
      ADC_PORT = 0;                          /* set ADC port to low */
      ADC_DDR = Probes.ADC_2;                /* pull down source directly */
      R_DDR = Probes.Rl_1 | Probes.Rh_3;     /* enable Rl for probe-1 & Rh for probe-3 */
      R_PORT = Probes.Rl_1 | Probes.Rh_3;    /* pull up drain via Rl / pull up gate via Rh */

      U_2 = ReadU_20ms(Probes.Pin_3);        /* get voltage at gate */

      if (U_2 > 3911)              /* MOSFET */
      {
        /* n channel depletion-mode MOSFET */ 
        Check.Type = TYPE_N_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET;
      }
      else                         /* JFET */
      {
        /* n channel JFET (depletion-mode only) */
        Check.Type = TYPE_N_CHANNEL | TYPE_JFET;
      }

      /* save data */
      Check.Found = COMP_FET;
      Check.Done = 1;
      FET.G = Probes.Pin_3;
      FET.D = Probes.Pin_1;
      FET.S = Probes.Pin_2;
    }
  }


  /*
   *  check if we got a p-channel JFET or depletion-mode MOSFET
   *  - JFETs are depletion-mode only
   */

  if (Check.Done == 0)        /* no transistor found yet */
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

    /*
     *  If the source voltage is higher when the gate is driven by a positive
     *  voltage vs. connected to ground we got a depletion-mode p-channel FET.
     *  The source resistor creates a voltage offset based on the current
     *  causing V_GS to become positive with the gate pulled up.
     */

    if (U_1 > (U_2 + 488))
    {
      /*
       *  Compare gate voltages to distinguish JFET from MOSFET
       */

      /* set probes: probe-2 = HiZ / probe-1 -- Vcc */
      ADC_PORT = Probes.ADC_1;          /* pull up source directly */
      ADC_DDR = Probes.ADC_1;           /* enable pull up for source */
      /* gate is still pulled down via Rh */
      U_2 = ReadU_20ms(Probes.Pin_3);   /* get voltage at gate */

      if (U_2 < 977)               /* MOSFET */
      {
        /* p channel depletion-mode MOSFET */ 
        Check.Type =  TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET; //Depletion-MOSFET
      }
      else                         /* JFET */
      {
        /* p channel JFET (depletion-mode only) */
        Check.Type = TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_JFET;
      }

      /* save data */
      Check.Found = COMP_FET;
      Check.Done = 1;
      FET.G = Probes.Pin_3;
      FET.D = Probes.Pin_2;
      FET.S = Probes.Pin_1;
    }
  }
}



/* ************************************************************************
 *   special devices
 * ************************************************************************ */


/*
 *  check for thyristor and triac
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

  /*
   *  check for a thyristor (SCR) or triac
   *  - A thyristor conducts also after the gate is discharged as long
   *    as the load current stays alive and doesn't reverse polarity.
   *  - A triac is a pair of anti-parallel thyristors. 
   *  - It's possible that the tester doesn't deliver enough current, so
   *    it can't detect all types.
   */

  /*
   *  probes need to be set already to:
   *    Gnd -- probe-2 / probe-1 -- Rl -- Vcc
   */

  /* we assume: probe-1 = A / probe-2 = C / probe-3 = G */
  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);    /* discharge gate */
  U_1 = ReadU_5ms(Probes.Pin_1);        /* get voltage at anode */

  R_PORT = 0;                           /* pull down anode */
  wait5ms();
  R_PORT = Probes.Rl_1;                 /* and pull up anode again */
  U_2 = ReadU_5ms(Probes.Pin_1);        /* get voltage at anode (below Rl) */

  /* voltages match behaviour of thyristor or triac */
  if ((U_1 < 1600) && (U_2 > 4400))
  {
    Check.Found = COMP_THYRISTOR;       /* if not detected as a triac below */
    Check.Done = 1;

    /*
     *  check if we got a triac
     *  - reverse A and C (A = MT2 / C = MT1)
     *  - check if behaviour is the same
     */

    /* we assume: probe-1 = MT2 / probe-2 = MT1 / probe-3 = G */
    R_DDR = 0;                          /* disable all probe resistors */
    R_PORT = 0;
    ADC_PORT = Probes.ADC_2;            /* pull up MT1 directly */
    wait5ms();
    R_DDR = Probes.Rl_1;                /* pull down MT2 via Rl */ 
    /* probe-3/gate is in HiZ mode */

    /* triac shouldn't conduct without a triggered gate */ 
    U_1 = ReadU_5ms(Probes.Pin_1);      /* get voltage at MT2 */

    /* voltage of MT2 is low (no current) */
    if (U_1 <= 244)
    {
      /* trigger gate for reverse direction */
      R_DDR = Probes.Rl_1 | Probes.Rl_3;     /* and pull down gate via Rl */
      U_1 = ReadU_5ms(Probes.Pin_3);         /* get voltage at gate */
      U_2 = ReadU(Probes.Pin_1);             /* get voltage at MT2 */  

      /*
       * voltage at gate is ok and voltage at MT2 is high
       * (current = triac is conducting)
       */

      if ((U_1 >= 977) && (U_2 >= 733))
      {
        /* check if triac still conducts without triggered gate */ 
        R_DDR = Probes.Rl_1;                 /* set probe3 to HiZ mode */
        U_1 = ReadU_5ms(Probes.Pin_1);       /* get voltage at MT2 */

        /* voltage at MT2 is still high (current = triac is conducting) */
        if (U_1 >= 733)
        {
          /* check if triac stops conducting when load current drops to zero */
          R_PORT = Probes.Rl_1;              /* pull up MT2 via Rl */
          wait5ms();
          R_PORT = 0;                        /* and pull down MT2 via Rl */
          U_1 = ReadU_5ms(Probes.Pin_1);     /* get voltage at MT2 */

          /* voltage at MT2 is low (no current = triac is not conducting) */
          if (U_1 <= 244)
          {
            /* now we are pretty sure that the DUT is a triac */
            Check.Found = COMP_TRIAC;
          }
        }
      }
    }

    /* save data (we misuse BJT) */
    BJT.B = Probes.Pin_3;
    BJT.C = Probes.Pin_1;
    BJT.E = Probes.Pin_2;

    Flag = 1;            /* signal that we found a component */
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

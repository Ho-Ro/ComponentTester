/* ************************************************************************
 *
 *   probing testpins
 *
 *   (c) 2012-2014 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define PROBES_C


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
 *  setup probes, bitmasks for probes and test resistors
 */

void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3)
{
  /* set probe IDs */
  Probes.Pin_1 = Probe1;
  Probes.Pin_2 = Probe2;
  Probes.Pin_3 = Probe3;

  /* setup masks using bitmask tables */
  Probes.Rl_1 = eeprom_read_byte(&Rl_table[Probe1]);
  Probes.Rh_1 = Probes.Rl_1 + Probes.Rl_1;
  Probes.ADC_1 = eeprom_read_byte(&ADC_table[Probe1]);
  Probes.Rl_2 = eeprom_read_byte(&Rl_table[Probe2]);
  Probes.Rh_2 = Probes.Rl_2 + Probes.Rl_2;
  Probes.ADC_2 = eeprom_read_byte(&ADC_table[Probe2]);
  Probes.Rl_3 = eeprom_read_byte(&Rl_table[Probe3]);
  Probes.Rh_3 = Probes.Rl_3 + Probes.Rl_3;
//  Probes.ADC_3 = eeprom_read_byte(&ADC_table[Probe3]);
}



/*
 *  check for a short circuit between two probes
 *
 *  requires:
 *  - ID of first probe (0-2)
 *  - ID of second probe (0-2)
 *
 *  returns:
 *  - 0 if not shorted
 *  - 1 if shorted
 */

uint8_t ShortedProbes(uint8_t Probe1, uint8_t Probe2)
{
  uint8_t           Flag = 0;      /* return value */
  unsigned int      U1;            /* voltage at probe #1 in mV */
  unsigned int      U2;            /* voltage at probe #2 in mV */
  unsigned int      Min;           /* lower threshold */
  unsigned int      Max;           /* upper threshold */

  /*
   *  Set up a voltage divider between the two probes:
   *  - Probe1: Rl pull-up
   *  - Probe2: Rl pull-down
   *  - third probe: HiZ
   */

  R_PORT = eeprom_read_byte(&Rl_table[Probe1]);
  R_DDR = eeprom_read_byte(&Rl_table[Probe1]) | eeprom_read_byte(&Rl_table[Probe2]);

  /* read voltages */
  U1 = ReadU(Probe1);
  U2 = ReadU(Probe2);

  /*
   *  We expect both probe voltages to be about the same and
   *  to be half of Vcc (allowed difference +/- 30mV).
   */

  Min = (Config.Vcc / 2) - 30;     /* lower voltage */
  Max = (Config.Vcc / 2) + 30;     /* upper voltage */

  if ((U1 > Min) && (U1 < Max))
  { 
    if ((U2 > Min) && (U2 < Max))
    {
      Flag = 1;
    }    
  }

  /* reset port */
  R_DDR = 0;

  return Flag;
}



/*
 *  check for a short circuit between all probes
 *
 *  returns:
 *  - 0 if no probes are short-circuited
 *  - number of probe pairs short-circuited (3 = all)
 */

uint8_t AllProbesShorted(void)
{
  uint8_t           Flag = 0;      /* return value */

  /* check all possible combinations */
  Flag = ShortedProbes(TP1, TP2);
  Flag += ShortedProbes(TP1, TP3);
  Flag += ShortedProbes(TP2, TP3);

  return Flag;  
}



/*
 *  try to discharge any connected components, e.g. capacitors
 *  - detect batteries
 *  - sometimes large caps are detected as a battery
 */

void DischargeProbes(void)
{
  uint8_t           Counter;            /* loop control */
  uint8_t           Limit = 40;         /* sliding timeout (2s) */
  uint8_t           ID;                 /* test pin */
  uint8_t           DischargeMask;      /* bitmask */
  unsigned int      U_c;                /* current voltage */
  unsigned int      U_old[3];           /* old voltages */


  /*
   *  set probes to a save discharge mode (pull-down via Rh) 
   */

  /* set ADC port to HiZ input */
  ADC_DDR = 0;
  ADC_PORT = 0;

  /* all probe pins: Rh and Rl pull-down */
  R_PORT = 0;
  R_DDR = (2 << (TP1 * 2)) | (2 << (TP2 * 2)) | (2 << (TP3 * 2));
  R_DDR |= (1 << (TP1 * 2)) | (1 << (TP2 * 2)) | (1 << (TP3 * 2));

  /* get current voltages */
  U_old[0] = ReadU(TP1);
  U_old[1] = ReadU(TP2);
  U_old[2] = ReadU(TP3);

  /*
   *  try to discharge probes
   *  - We check if the voltage decreases over time.
   *  - A slow discharge rate will increase the timeout to support
   *    large caps.
   *  - A very large cap will discharge too slowly and an external voltage
   *    maybe never :-)
   */

  Counter = 1;
  ID = 2;
  DischargeMask = 0;

  while (Counter > 0)
  {
    ID++;                               /* next probe */
    if (ID > 2) ID = 0;                 /* start with probe #1 again */

    if (DischargeMask & (1 << ID))      /* skip discharged probe */
      continue;

    U_c = ReadU(ID);                    /* get voltage of probe */

    if (U_c < U_old[ID])                /* voltage decreased */
    {
      U_old[ID] = U_c;                  /* update old value */

      /* adapt timeout based on discharge rate */
      if ((Limit - Counter) < 20)
      {
        /* increase timeout while preventing overflow */
        if (Limit < (255 - 20)) Limit += 20;
      }

      Counter = 1;                      /* reset no-changes counter */
    }
    else                                /* voltage not decreased */
    {
      /* increase limit if we start at a low voltage */
      if ((U_c < 10) && (Limit <= 40)) Limit = 80;

      Counter++;              /* increase no-changes counter */
    }

    if (U_c <= CAP_DISCHARGED)          /* seems to be discharged */
    {
      DischargeMask |= (1 << ID);       /* set flag */
    }
    else if (U_c < 800)                 /* extra pull-down */
    {
      /* it's save now to pull-down probe pin directly */
      ADC_DDR |= eeprom_read_byte(&ADC_table[ID]);
    }

    if (DischargeMask == 0b00000111)    /* all probes discharged */
    {
      Counter = 0;                        /* end loop */
    }
    else if (Counter > Limit)             /* no decrease for some time */
    {
      /* might be a battery or a super cap */
      Check.Found = COMP_ERROR;           /* report error */
      Check.Type = TYPE_DISCHARGE;        /* discharge problem */
      Check.Probe = ID;                   /* save probe */
      Check.U = U_c;                      /* save voltage */
      Counter = 0;                        /* end loop */
    }
    else                                /* go for another round */
    {
      wdt_reset();                        /* reset watchdog */
      MilliSleep(50);                     /* wait for 50ms */
    }
  }

  /* reset probes */
  R_DDR = 0;                       /* set resistor port to input mode */
  ADC_DDR = 0;                     /* set ADC port to input mode */
}



/*
 *  pull probe up/down via probe resistor for 1 or 10 ms
 *
 *  requires:
 *  - mask for probe resistors
 *  - pull mode (bit flags):
 *    0b00000000 = down
 *    0b00000001 = up 
 *    0b00000100 = 1ms
 *    0b00001000 = 10ms
 */

void PullProbe(uint8_t Mask, uint8_t Mode)
{
  /* set pull mode */
  if (Mode & FLAG_PULLUP) R_PORT |= Mask;    /* pull-up */
  else R_PORT &= ~Mask;                      /* pull-down */
  R_DDR |= Mask;                             /* enable pulling */

  if (Mode & FLAG_1MS) wait1ms();            /* wait 1ms */
  else wait10ms();                           /* wait 10ms */

  /* reset pulling */
  R_DDR &= ~Mask;                       /* set to HiZ mode */
  R_PORT &= ~Mask;                      /* set 0 */
}



/*
 *  lookup a voltage/ratio based factor in a table and interpolate it's value
 *  - value descreases over index position
 *
 *  requires:
 *  - voltage (in mV) or ratio
 *  - table ID
 *
 *  returns:
 *  - multiplicator/factor
 */

unsigned int GetFactor(unsigned int U_in, uint8_t ID)
{
  unsigned int      Factor;             /* return value */
  unsigned int      U_Diff;             /* voltage difference to table start */
  unsigned int      Fact1, Fact2;       /* table entries */
  unsigned int      TabStart;           /* table start voltage */
  unsigned int      TabStep;            /* table step voltage */
  unsigned int      TabIndex;           /* table entries (-2) */
  uint16_t          *Table;
  uint8_t           Index;              /* table index */
  uint8_t           Diff;               /* difference to next entry */

  /*
   *  setup table specific stuff
   */

  if (ID == TABLE_SMALL_CAP)
  {
    TabStart = 1000;               /* table starts at 1000mV */
    TabStep = 50;                  /* 50mV steps between entries */
    TabIndex = 7;                  /* entries in table - 2 */
    Table = (uint16_t *)&SmallCap_table[0];    /* pointer to table start */
  }
  else if (ID == TABLE_LARGE_CAP)
  {
    TabStart = 300;                /* table starts at 1000mV */
    TabStep = 25;                  /* 25mV steps between entries */
    TabIndex = 42;                 /* entries in table - 2 */
    Table = (uint16_t *)&LargeCap_table[0];    /* pointer to table start */
  }
  #ifdef EXTRA
  else if (ID == TABLE_INDUCTOR)
  {
    TabStart = 200;                /* table starts at 200 */
    TabStep = 25;                  /* steps between entries */
    TabIndex = 30;                 /* entries in table - 2 */
    Table = (uint16_t *)&Inductor_table[0];    /* pointer to table start */
  }
  #endif
  else
  {
    return 0;
  }

  /*
   *  We interpolate the table values corresponding to the given voltage/ratio.
   */

  /* difference to start of table */
  if (U_in >= TabStart) U_Diff = U_in - TabStart;  
  else U_Diff = 0;

  /* calculate table index */
  Index = U_Diff / TabStep;             /* index (position in table) */
  Diff = U_Diff % TabStep;              /* difference to index */
  Diff = TabStep - Diff;                /* difference to next entry */

  /* prevent index overflow */
  if (Index > TabIndex) Index = TabIndex;

  /* get values for index and next entry */
  Table += Index;                       /* advance to index */
  Fact1 = MEM_read_word(Table);
  Table++;                              /* next entry */
  Fact2 = MEM_read_word(Table);

  /* interpolate values based on the difference */
  Factor = Fact1 - Fact2;
  Factor *= Diff;
  Factor += TabStep / 2;
  Factor /= TabStep;
  Factor += Fact2;

  return Factor;
}



/* ************************************************************************
 *   identify component
 * ************************************************************************ */


/*
 *  probe connected component and try to identify it
 *
 *  requires:
 *  - Probe1: ID of probe to be pulled up [0-2]
 *  - Probe2: ID of probe to be pulled down [0-2]
 *  - Probe3: ID of probe to be in HiZ mode [0-2]
 */

void CheckProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3)
{
  uint8_t           Flag;               /* temporary value */
  unsigned int      U_Rl;               /* voltage across Rl (load) */
  unsigned int      U_1;                /* voltage #1 */

  /* init */
  if (Check.Found == COMP_ERROR) return;   /* skip check on any error */
  wdt_reset();                             /* reset watchdog */
  UpdateProbes(Probe1, Probe2, Probe3);    /* update bitmasks */


  /*
   *  We measure the current from probe 2 to ground with probe 1 pulled up
   *  to 5V and probe 3 in HiZ mode to determine if we got a self-conducting
   *  part, i.e. diode, resistor or depletion-mode FET. Rl is used as current
   *  shunt.
   *
   *  In case of a FET we have to take care about the gate charge based on
   *  the channel type.
   */

  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_PORT = 0;                      /* set resistor port to Gnd */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */
  ADC_DDR = Probes.ADC_1;          /* set probe-1 to output */
  ADC_PORT = Probes.ADC_1;         /* pull-up probe-1 directly */


  /*
   *  For a possible n channel FET we pull down the gate for a few ms,
   *  assuming: probe-1 = D / probe-2 = S / probe-3 = G
   */

  PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLDOWN);   /* discharge gate via Rl */
  U_Rl = ReadU_5ms(Probes.Pin_2);                    /* get voltage at Rl */


  /*
   *  If we got conduction we could have a p channel FET. For any
   *  other part U_Rl will be the same.
   */
 
  if (U_Rl >= 977)               /* > 1.4mA */
  {
    /*
     *  For a possible p channel FET we pull up the gate for a few ms,
     *  assuming: probe-1 = S / probe-2 = D / probe-3 = G
     */

    PullProbe(Probes.Rl_3, FLAG_10MS | FLAG_PULLUP);   /* discharge gate via Rl */
    U_Rl = ReadU_5ms(Probes.Pin_2);                    /* get voltage at Rl */
  }


  /*
   *  If there's some current we could have a depletion-mode FET
   *  (self-conducting). To skip germanium BJTs with a high leakage current
   *  we check for a current larger then the usual V_CEO.
   *
   *  Other possibilities:
   *  - diode or resistor
   */

  if (U_Rl > 490)         /* > 700µA (was 92mV/130µA) */
  {
    CheckDepletionModeFET();
  }


  /*
   *  If there's nearly no conduction (just a small leakage current) between
   *  probe-1 and probe-2 we might have a semiconductor:
   *  - BJT
   *  - enhancement mode FET
   *  - Thyristor or Triac
   *  or a large resistor
   */

  if (U_Rl < 977)         /* load current < 1.4mA (resistance > 3k) */
  {
    /*
     *  check for:
     *  - PNP BJT (common emitter circuit)
     *  - p-channel MOSFET (low side switching circuit)
     */

    if (Check.Done == 0)           /* not sure yet */
    {
      /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
      /* set probes: Gnd -- Rl - probe-2 / probe-1 -- Vcc */
      R_DDR = Probes.Rl_2;                /* enable Rl for probe-2 */
      R_PORT = 0;                         /* pull down collector via Rl */
      ADC_DDR = Probes.ADC_1;             /* set probe 1 to output */
      ADC_PORT = Probes.ADC_1;            /* pull up emitter directly */
      wait5ms();
      R_DDR = Probes.Rl_2 | Probes.Rl_3;  /* pull down base via Rl */
      U_1 = ReadU_5ms(Probe2);            /* get voltage at collector */ 
 
      /*
       *  If DUT is conducting we might have a PNP BJT or p-channel FET.
       */

      if (U_1 > 3422)                   /* detected current > 4.8mA */
      {
        /* distinguish PNP BJT from p-channel MOSFET */
        CheckBJTorEnhModeMOSFET(TYPE_PNP, U_Rl);
      }
    }


    /*
     *  check for
     *  - NPN BJT (common emitter circuit)
     *  - Thyristor and Triac
     *  - n-channel MOSFET (high side switching circuit)
     */

    if (Check.Done == 0)           /* not sure yet */
    {
      /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
      ADC_DDR = Probes.ADC_2;                /* set probe-2 to output mode */
      ADC_PORT = 0;                          /* pull down probe-2 directly */
      R_DDR = Probes.Rl_1 | Probes.Rl_3;     /* select Rl for probe-1 & Rl for probe-3 */
      R_PORT = Probes.Rl_1 | Probes.Rl_3;    /* pull up collector & base via Rl */
      U_1 = ReadU_5ms(Probe1);               /* get voltage at collector */


      /*
       *  If DUT is conducting we might have a NPN BJT, something similar or
       *  a n-channel MOSFET.
       */

      if (U_1 < 1600)                   /* detected current > 4.8mA */
      {
        /* first check for thyristor and triac */
        Flag = CheckThyristorTriac();

        if (Flag == 0)                 /* no thyristor or triac */
        {
          /* we might got a NPN BJT or a n-channel MOSFET. */
          CheckBJTorEnhModeMOSFET(TYPE_NPN, U_Rl);
        }
      }
    }
  }


  /*
   *  If there's conduction between probe-1 and probe-2 we might have a
   *  - diode (conducting)
   *  - small resistor (checked later on)
   */

  else              /* load current > 1.4mA  (resistance < 3k) */
  {
    /*
     *  We check for a diode even if we already found a component to get Vf, 
     *  since there could be a body/protection diode of a transistor.
     */

    CheckDiode();
  }


  /*
   *  Check for a resistor.
   */

  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    CheckResistor();
  }


  /*
   *  Otherwise run some final checks.
   */

  else
  {
    /* verify a MOSFET */
    if ((Check.Found == COMP_FET) && (Check.Type & TYPE_MOSFET))
      VerifyMOSFET();
  }


  /* clean up */
  ADC_DDR = 0;           /* set ADC port to HiZ mode */
  ADC_PORT = 0;          /* set ADC port low */
  R_DDR = 0;             /* set resistor port to HiZ mode */
  R_PORT = 0;            /* set resistor port low */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef PROBES_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

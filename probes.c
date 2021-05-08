/* ************************************************************************
 *
 *   probing testpins
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define PROBES_C

/* local bit flags */
#define FLAG_PULLDOWN         0b00000000
#define FLAG_PULLUP           0b00000001
#define FLAG_1MS              0b00001000
#define FLAG_10MS             0b00010000



/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  local variables
 */

/* probes */
uint8_t             Probe1_Pin;         /* probe-1 */
uint8_t             Probe2_Pin;         /* probe-2 */
uint8_t             Probe3_Pin;         /* probe-3 */

/* bit masks for switching probes and test resistors */
uint8_t             Probe1_Rl;          /* Rl mask for probe-1 */
uint8_t             Probe1_Rh;          /* Rh mask for probe-1 */
uint8_t             Probe2_Rl;          /* Rl mask for probe-2 */
uint8_t             Probe2_Rh;          /* Rh mask for probe-2 */
uint8_t             Probe3_Rl;          /* Rl mask for probe-3 */
uint8_t             Probe3_Rh;          /* Rh mask for probe-3 */
uint8_t             Probe1_ADC;         /* ADC mask for probe-1 */
uint8_t             Probe2_ADC;         /* ADC mask for probe-2 */



/* ************************************************************************
 *   support functions
 * ************************************************************************ */


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

  /*
   *  Set up a voltage divider between the two probes:
   *  - Probe1: Rl pull-up
   *  - Probe2: Rl pull-down
   *  - third probe: HiZ
   */

  R_PORT = MEM_read_byte(&Rl_table[Probe1]);
  R_DDR = MEM_read_byte(&Rl_table[Probe1]) | MEM_read_byte(&Rl_table[Probe2]);

  /* read voltages */
  U1 = ReadU(Probe1);
  U2 = ReadU(Probe2);

  /*
   *  We expect both probe voltages to be about the same and
   *  to be half of Vcc (allowed difference +/- 20mV).
   */

  if ((U1 > UREF_VCC/2 - 20) && (U1 < UREF_VCC/2 + 20))
  { 
    if ((U2 > UREF_VCC/2 - 20) && (U2 < UREF_VCC/2 + 20))
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
  uint8_t           Limit = 40;         /* sliding timeout */
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
      Counter++;              /* increase no-changes counter for battery detection */
    }

    if (U_c <= CAP_DISCHARGED)          /* seems to be discharged */
    {
      DischargeMask |= (1 << ID);       /* set flag */
    }
    else if (U_c < 800)                 /* extra pull-down */
    {
      /* it's save now to pull-down probe pin directly */
      ADC_DDR |= MEM_read_byte(&ADC_table[ID]);
    }

    if (DischargeMask == 0b00000111)    /* all probes discharged */
    {
      Counter = 0;                        /* end loop */
    }
    else if (Counter > Limit)             /* no decrease for some time */
    {
      /* might be a battery or a super cap */
      CompFound = COMP_CELL;              /* report battery */
      Counter = 0;                        /* end loop */

      /* tell user */
      lcd_clear();
      lcd_fix_string(DischargeFailed_str);
    }
    else                                /* go for another round */
    {
      wdt_reset();                        /* reset watchdog */
      wait50ms();                         /* wait for 50ms */
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
 *  setup probes, bitmasks for probes and test resistors
 */

void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3)
{
  /* set probe IDs */
  Probe1_Pin = Probe1;
  Probe2_Pin = Probe2;
  Probe3_Pin = Probe3;

  /* setup masks using bitmask tables */
  Probe1_Rl = MEM_read_byte(&Rl_table[Probe1]);
  Probe1_Rh = Probe1_Rl + Probe1_Rl;
  Probe1_ADC = MEM_read_byte(&ADC_table[Probe1]);
  Probe2_Rl = MEM_read_byte(&Rl_table[Probe2]);
  Probe2_Rh = Probe2_Rl + Probe2_Rl;
  Probe2_ADC = MEM_read_byte(&ADC_table[Probe2]);
  Probe3_Rl = MEM_read_byte(&Rl_table[Probe3]);
  Probe3_Rh = Probe3_Rl + Probe3_Rl;
}



/*
 *  interpolate value from table based on voltage
 *  - value descreases over index position
 *
 *  requires:
 *  - voltage in mV
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
    TabStep = 25;                  /* 50mV steps between entries */
    TabIndex = 42;                 /* entries in table - 2 */
    Table = (uint16_t *)&LargeCap_table[0];    /* pointer to table start */
  }
  else
  {
    return 0;
  }

  /*
   *  We interpolate the table values corresponding to the given voltage.
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
  Fact1 = pgm_read_word(Table);
  Table++;                              /* next entry */
  Fact2 = pgm_read_word(Table);

  /* interpolate values based on difference */
  Factor = Fact1 - Fact2;
  Factor *= Diff;
  Factor += TabStep / 2;
  Factor /= TabStep;
  Factor += Fact2;

  return Factor;
}



/* ************************************************************************
 *   measure capacitance
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
- bc: define x (u) { return (-1000000 / (702 * l(1 - u/5000))); }

Small caps:
- R = 470k (neglect internal resistance of uC)
- U_in = 5V
- values are: (-1 / (R * ln(1 - U_c/U_in))) * 10^12p * 10^-4
  - 10^12p for pF scale
  - 10^-4 internal scale factor (make values fit in unsigned int)
- bc: define x (u) { return (-100000000 / (470000 * l(1 - u/5000))); }

*/


/*
 *  measure cap >4.7µF between two probe pins
 *
 *  requires:
 *  - Cap: pointer to capacitor data structure 
 *
 *  returns:
 *  - 3 on success
 *  - 2 if cap is too small
 *  - 1 if cap is too large
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
  if (CompFound == COMP_CELL) return 0;

  /* setup probes: Gnd -- probe 1 / probe 2 -- Rl -- Vcc */
  ADC_PORT = 0;                    /* set ADC port to low */
  ADC_DDR = Probe2_ADC;            /* pull-down probe 2 directly */
  R_PORT = 0;                      /* set resistor port to low */
  R_DDR = 0;                       /* set resistor port to HiZ */
  U_Zero = ReadU(Probe1_Pin);      /* get zero voltage (noise) */

  /* charge DUT with up to 500 pulses until it reaches 300mV */
  Pulses = 0;
  TempByte = 1;
  while (TempByte)
  {
    Pulses++;
    PullProbe(Probe1_Rl, Mode);         /* charging pulse */
    U_Cap = ReadU(Probe1_Pin);          /* get voltage */
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
      TempInt--;                     /* descrease timeout */
      U_Drop = ReadU(Probe1_Pin);    /* get voltage */
      U_Drop -= U_Zero;              /* zero offset */
      wdt_reset();                   /* reset watchdog */
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
    Cap->A = Probe2_Pin;      /* pull-down probe pin */
    Cap->B = Probe1_Pin;      /* pull-up probe pin */
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
 *  - 2 if cap is too small
 *  - 1 if cap is too large
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
  if (CompFound == COMP_CELL) return 0;

  /* set probes: Gnd -- all probes / Gnd -- Rh -- probe-1 */
  R_PORT = 0;                           /* set resistor port to low */
  /* set ADC probe pins to output mode */
  ADC_DDR = (1 << TP1) | (1 << TP2) | (1 << TP3);
  ADC_PORT = 0;                         /* set ADC port to low */
  R_DDR = Probe1_Rh;                    /* pull-down probe-1 via Rh */

  /* setup analog comparator */
  ADCSRB = (1 << ACME);                 /* use ADC multiplexer as negative input */
  ACSR =  (1 << ACBG) | (1 << ACIC);    /* use bandgap as positive input, trigger timer1 */
  ADMUX = (1 << REFS0) | Probe1_Pin;    /* switch ADC multiplexer to probe 1 */
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
  R_PORT = Probe1_Rh;                   /* pull-up probe-1 via Rh */  
                                        
  /* enable timer */
  if (CompFound == COMP_FET)
  {
    /* keep all probe pins pulled down but probe-1 */
    TempByte = (((1 << TP1) | (1 << TP2) | (1 << TP3)) & ~(1 << Probe1_Pin));    
  }
  else
  {
    TempByte = Probe2_ADC;              /* keep just probe-1 pulled down */
  }

  TCCR1B |= (1 << CS10);                /* start timer (1/1 clock divider) */
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
  U_c = ReadU(Probe1_Pin);         /* get voltage of cap */

  /* start discharging DUT */
  R_PORT = 0;                      /* pull down probe-2 via Rh */
  R_DDR = Probe1_Rh;               /* enable Rh for probe-1 again */

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

    /* subtract processing time overhead */
    if (Raw > 2) Raw -= 2;

    Scale = -12;                          /* factor is for pF scale */
    if (Raw > (UINT32_MAX / 1000))        /* prevent overflow (4.3*10^6) */
    {
      Raw /= 1000;                        /* scale down by 10^3 */
      Scale += 3;                         /* add 3 to the exponent */
    }

    /* multiply with factor from table */
    Raw *= GetFactor(Config.U_Bandgap + Config.CompOffset, TABLE_SMALL_CAP);

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
    Cap->A = Probe2_Pin;      /* pull-down probe pin */
    Cap->B = Probe1_Pin;      /* pull-up probe pin */
    Cap->Scale = Scale;       /* -12 or -9 */
    Cap->Raw = Raw;
    Cap->Value = Value;       /* max. 5.1*10^6pF or 125*10^3nF */


    /*
     *  Self-calibrate voltage offset of the analog comparator and
     *  internal bandgap reference if C is 100nF - 20µF.
     *  Changed offsets will we used in next test run.
     */

    if (((Scale == -12) && (Value >= 100000)) ||
        ((Scale == -9) && (Value <= 20000)))
    {
      signed int         Offset;
      signed long        TempLong;

      /*
       *  We can self-calibrate the offset of the internal bandgap reference
       *  by measuring a voltage lower than the bandgap reference, one time
       *  with the bandgap as reference and a second time with Vcc as
       *  reference. The common voltage source is the cap we just measured.
       */

       while (ReadU(Probe1_Pin) > 980)
       {
         /* keep discharging */
       }

       R_DDR = 0;                       /* stop discharging */

       Config.AutoScale = 0;            /* disable auto scaling */
       Ticks = ReadU(Probe1_Pin);       /* U_c with Vcc reference */
       Config.AutoScale = 1;            /* enable auto scaling again */
       Ticks2 = ReadU(Probe1_Pin);      /* U_c with bandgap reference */

       R_DDR = Probe1_Rh;               /* resume discharging */

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
         TempLong *= Config.U_Bandgap;       /* * U_ref */
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

      Offset = U_c - Config.U_Bandgap;

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


  /*
   *  Skip resistors
   *  - Check for a resistor < 10 Ohm. Might be a large cap.
   */

  if (CompFound == COMP_RESISTOR)
  {
    /* check for matching pins */
    Resistor = &Resistors[0];         /* pointer to first resistor */
    TempByte = 0;

    while (TempByte < ResistorsFound)
    {
      /* got matching pins */
      if (((Resistor->A == Probe1) && (Resistor->B == Probe2)) ||
          ((Resistor->A == Probe2) && (Resistor->B == Probe1)))
      {
        if (CmpValue(Resistor->Value, Resistor->Scale, 10UL, 0) == -1)
          TempByte = 99;                /* signal low resistance and end loop */
      }

      TempByte++;      /* next one */
      Resistor++;      /* next one */  
    }

    /* we got a valid resistor */ 
    if (TempByte != 100) return;        /* skip this one */
  }

  /* skip measurement for "dangerous" diodes */
  Diode = &Diodes[0];         /* pointer to first diode */

  for (TempByte = 0; TempByte < DiodesFound; TempByte++)
  {
    /* got matching pins and dangerous threshold voltage */
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
//else if (TempByte == 3)
//{
//  MeasureESR(Cap);
//}

  /*
   *  check for plausibility
   */

  /* if there aren't any diodes in reverse direction which could be
     detected as capacitors by mistake */
  if (DiodesFound == 0)
  {
    /* low resistance might be a large cap */
    if (CompFound == COMP_RESISTOR)
    {
      /* report capacitor for large C (> 4.3µF) */
      if (Cap->Scale >= -6) CompFound = COMP_CAPACITOR;
    }

    /* we consider values below 5pF being just ghosts */
    else if ((Cap->Scale > -12) || (Cap->Value >= 5UL))
    {
      CompFound = COMP_CAPACITOR;       /* report capacitor */
    }
  }


  /*
   *  clean up
   */

  DischargeProbes();                                   /* discharge DUT */

  /* reset all ports and pins */
  ADC_DDR = 0;                     /* set ADC port to input */
  ADC_PORT = 0;                    /* set ADC port low */
  R_DDR = 0;                       /* set resistor port to input */
  R_PORT = 0;                      /* set resistor port low */
}



/* ************************************************************************
 *   identify component
 * ************************************************************************ */


/*
 *  check for diode
 */

void CheckDiode(void)
{
  Diode_Type        *Diode;             /* pointer to diode */
  unsigned int      U1_Rl;              /* voltage #1 */
  unsigned int      U2_Rl;              /* voltage #2 */
  unsigned int      U1_Rh;              /* voltage #3 */
  unsigned int      U2_Rh;              /* voltage #4 */

  wdt_reset();                          /* reset watchdog */

  /*
   *  How-To:
   *  - Measure voltages with Rl and Rh to distinguish anti-parallel
   *    diodes from a resistor. Diodes got nearly the same voltage for
   *    low and high currents. Resistors cause U = I * R.
   *  - Take care about the internal voltage drop of the µC at the cathode
   *    for high currents.
   *  - Take care about a possible MOSFET, since we might just found
   *    an internal protection diode.
   *
   *  Problem:
   *  - Rl drives a current of about 7mA. That's not the best current to
   *    use for measuring Vf. The current for Rh is about 10.6µA. 
   */

  /* we assume: probe-1 = A / probe2 = C */
  /* set probes: Gnd -- probe-2 / probe-1 -- Rl or Rh -- Vcc */
  ADC_PORT = 0;
  ADC_DDR = Probe2_ADC;       /* pull down cathode directly */


  /*
   *  measurements for possible p-channel MOSFET
   */

  /* measure voltage across DUT (Vf) with Rl */
  R_DDR = Probe1_Rl;               /* enable Rl for probe-1 */
  R_PORT = Probe1_Rl;              /* pull up anode via Rl */
  /* discharge possible gate of a p-channel MOSFET */
  PullProbe(Probe3_Rl, FLAG_10MS | FLAG_PULLUP);
  U1_Rl = ReadU_5ms(Probe1_Pin);   /* get voltage at anode */
  U1_Rl -= ReadU(Probe2_Pin);      /* substract voltage at cathode */

  /* measure voltage across DUT (Vf) with Rh */
  R_DDR = Probe1_Rh;               /* enable Rh for probe-1 */
  R_PORT = Probe1_Rh;              /* pull up anode via Rh */
  U1_Rh = ReadU_5ms(Probe1_Pin);   /* get voltage at anode */


  /*
   *  measurements for possible n-channel MOSFET
   */

  /* measure voltage across DUT (Vf) with Rl */
  R_DDR = Probe1_Rl;               /* enable Rl for probe-1 */
  R_PORT = Probe1_Rl;              /* pull up anode via Rl */   
  /* discharge possible gate of a n-channel MOSFET */
  PullProbe(Probe3_Rl, FLAG_10MS | FLAG_PULLDOWN);
  U2_Rl = ReadU_5ms(Probe1_Pin);   /* get voltage at anode */
  U2_Rl -= ReadU(Probe2_Pin);      /* substract voltage at cathode */

  /* measure voltage across DUT (Vf) with Rh */
  R_DDR = Probe1_Rh;               /* enable Rh for probe-1 */
  R_PORT = Probe1_Rh;              /* pull up anode via Rh */
  U2_Rh = ReadU_5ms(Probe1_Pin);   /* get voltage at anode */


  /*
   *  process results
   */

  /* choose between measurements in p or n channel setup */
  if (U1_Rl >U2_Rl)        /* the higher voltage wins */
  {
    U2_Rl = U1_Rl;
    U2_Rh = U1_Rh;
  }

  /* voltage is between 0.15V and 4.64V */
  /* and we don't got resistors */
  if ((U2_Rl > 150) &&
      (U2_Rl < 4640) &&
      (U2_Rl > (U2_Rh + (U2_Rh / 8))) &&
      (U2_Rl < (U2_Rh * 8)))
  {
    /* if we haven't found any other component yet */
    if ((CompFound == COMP_NONE) ||
        (CompFound == COMP_RESISTOR))
    {
      CompFound = COMP_DIODE;
    }

    /* save data */
    Diode = &Diodes[DiodesFound];
    Diode->A = Probe1_Pin;
    Diode->C = Probe2_Pin;
    Diode->V_f = U2_Rl;       /* Vf for high measurement current */
    Diode->V_f2 = U2_Rh;      /* Vf for low measurement current */
    DiodesFound++;
  }
}



/*
 *  measure small resistors (< 100 Ohms)
 *
 *  returns:
 *  - resistance in 0.01 Ohm
 */

unsigned int SmallResistor(void)
{
  unsigned int      R = 0;              /* return value */
  uint8_t           Probe;              /* probe ID */
  uint8_t           Mode;               /* measurement mode */
  uint8_t           Counter;            /* sample counter */
  unsigned long     Value;              /* ADC sample value */
  unsigned long     Value1 = 0;         /* U_Rl temp. value */
  unsigned long     Value2 = 0;         /* U_R_i_L temp. value */

  DischargeProbes();                    /* try to discharge probes */
  if (CompFound == COMP_CELL) return R;

  /* charge: GND -- probe 2 / probe 1 -- Rl -- 5V */
  /* discharge: GND -- probe 2 / probe 1 -- Rl -- GND */
  ADC_PORT = 0;                         /* set ADC port to low */
  ADC_DDR = Probe2_ADC;                 /* pull-down probe 2 directly */
  R_PORT = 0;                           /* low by default */
  R_DDR = Probe1_Rl;                    /* enable resistor */

#define MODE_HIGH        0b00000001
#define MODE_LOW         0b00000010

  /*
   *  monster loop
   */

  Mode = MODE_HIGH;

  while (Mode > 0)
  {
    /* setup measurement */
    if (Mode & MODE_HIGH) Probe = Probe1_Pin;
    else Probe = Probe2_Pin;

    wdt_reset();              /* reset watchdog */
    Counter = 0;              /* reset loop counter */
    Value = 0;                /* reset sample value */

    /* set ADC to use bandgap reference and run a dummy conversion */
    Probe |= (1 << REFS0) | (1 << REFS1);
    ADMUX = Probe;                   /* set input channel and U reference */
    wait100us();                     /* time for voltage stabilization */
    ADCSRA |= (1 << ADSC);           /* start conversion */
    while (ADCSRA & (1 << ADSC));    /* wait until conversion is done */


    /*
     *  measurement loop (about 1ms per cycle)
     */

    while (Counter < 100)
    {
      /* create short charging pulse */
      ADC_DDR = Probe2_ADC;             /* pull-down probe 2 directly */
      R_PORT = Probe1_Rl;

      /* start ADC conversion */
      /* ADC performs S&H after 1.5 ADC cycles (12µs) */
      ADCSRA |= (1 << ADSC);            /* start conversion */

      /* wait 20µs */
      wait20us();

      /* start discharging */
      R_PORT = 0;
      ADC_DDR = Probe2_ADC | Probe1_ADC;

      /* get ADC reading (about 100µs) */
      while (ADCSRA & (1 << ADSC));     /* wait until conversion is done */
      Value += ADCW;                    /* add ADC reading */

      /* wait */
      wait400us();
      wait500us();

      Counter++;                        /* next round */
    }

    /* convert ADC reading to voltage */
    Value *= Config.U_Bandgap;
    Value /= 1024;                 /* / 1024 for 10bit ADC */
    Value /= 10;                   /* de-sample to 0.1mV */

    /* loop control */
    if (Mode & MODE_HIGH)          /* probe #1 / Rl */
    {
      Mode = MODE_LOW;
      Value1 = Value;
    }
    else                           /* probe #2 / R_i_L */
    {
      Mode = 0;
      Value2 = Value;
    }
  }


  /*
   *  process measurement
   */

  if (Value1 > Value2)             /* sanity check */
  {
    /* I = U/R = (5V - U_Rl)/(Rl + R_i_H) */
    Value = 10UL * UREF_VCC;                 /* in 0.1 mV */
    Value -= Value1;
    Value *= 1000;                           /* scale to µA */
    Value /= ((R_LOW * 10) + Config.RiH);    /* in 0.1 Ohms */

    /* U = U_Rl - U_R_i_L = U_Rl - (R_i_L * I) */
    /* U = U_probe1 - U_probe2 */
    Value1 -= Value2;                        /* in 0.1 mV */
    Value1 *= 10000;                         /* scale to 0.01 µV */

    /* R = U/I (including R of probe leads) */
    Value1 /= Value;                         /* in 0.01 Ohms */

    R = (unsigned int)Value1;
  }

#undef MODE_LOW
#undef MODE_HIGH

  /* update Uref flag for next ADC run */
  Config.RefFlag = (1 << REFS1);        /* set REFS1 bit flag */

  return R;
}



/*
 *  check for resistor
 */

void CheckResistor(void)
{
  Resistor_Type     *Resistor;          /* pointer to resistor */
  unsigned long     Value1;             /* resistance of measurement #1 */
  unsigned long     Value2;             /* resistance of measurement #2 */
  unsigned long     Value;              /* resistance value */
  unsigned long     Temp;               /* temp. value */
  int8_t            Scale;              /* resistance scaling */
  uint8_t           n;                  /* counter */

  /* voltages */
  unsigned int      U_Rl_H;             /* voltage #1 */
  unsigned int      U_Ri_L;             /* voltage #2 */
  unsigned int      U_Rl_L;             /* voltage #3 */
  unsigned int      U_Ri_H;             /* voltage #4 */
  unsigned int      U_Rh_H;             /* voltage #5 */
  unsigned int      U_Rh_L;             /* voltage #6 */

  wdt_reset();                     /* reset watchdog */

  /*
   *  resistor measurement
   *  - Set up a voltage divider with well known probe resistors and
   *    measure the voltage at the DUT.
   *  - For low resistance consider the internal resistors of the µC
   *    for pulling up/down.
   *  - Calculate resistance via the total current and the voltage
   *    at the DUT.
   *  - We could also use the voltage divider rule:
   *    (Ra / Rb) = (Ua / Ub) -> Ra = Rb * (Ua / Ub)
   */


  /*
   *  check if we got a resistor
   *  - A resistor has the same resistance in both directions.
   *  - We measure both directions with both probe resistors.
   */

  /* we assume: resistor between probe-1 and probe-2 */
  /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
  ADC_PORT = 0;                    /* set ADC port low low */
  ADC_DDR = Probe2_ADC;            /* pull down probe-2 directly */
  R_DDR = Probe1_Rl;               /* enable Rl for probe-1 */
  R_PORT = Probe1_Rl;              /* pull up probe-1 via Rl */
  U_Ri_L = ReadU_5ms(Probe2_Pin);  /* get voltage at internal R of µC */
  U_Rl_H = ReadU(Probe1_Pin);      /* get voltage at Rl pulled up */


  /*
   *  check for a capacitor
   *  - A capacitor would need some time to discharge.
   *  - So we pull down probe-1 via Rh and measure the voltage.
   *  - The voltage will drop immediately for a resistor.
   */

  /* set probes: Gnd -- probe-2 / Gnd -- Rh -- probe-1 */
  R_PORT = 0;                      /* set resistor port low */
  R_DDR = Probe1_Rh;               /* pull down probe-1 via Rh */
  U_Rh_L = ReadU_5ms(Probe1_Pin);  /* get voltage at probe 1 */

  /* we got a resistor if the voltage is near Gnd */
  if (U_Rh_L <= 20)
  {
    /* set probes: Gnd -- probe-2 / probe-1 -- Rh -- Vcc */
    R_PORT = Probe1_Rh;                 /* pull up probe-1 via Rh */
    U_Rh_H = ReadU_5ms(Probe1_Pin);     /* get voltage at Rh pulled up */

    /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    ADC_DDR = Probe1_ADC;               /* set probe-1 to output */
    ADC_PORT = Probe1_ADC;              /* pull up probe-1 directly */
    R_PORT = 0;                         /* set resistor port to low */ 
    R_DDR = Probe2_Rl;                  /* pull down probe-2 via Rl */
    U_Ri_H = ReadU_5ms(Probe1_Pin);     /* get voltage at internal R of µC */
    U_Rl_L = ReadU(Probe2_Pin);         /* get voltage at Rl pulled down */

    /* set probes: Gnd -- Rh -- probe-2 / probe-1 -- Vcc */
    R_DDR = Probe2_Rh;                  /* pull down probe-2 via Rh */
    U_Rh_L = ReadU_5ms(Probe2_Pin);     /* get voltage at Rh pulled down */

    /* if voltage breakdown is sufficient */
    if ((U_Rl_H >= 4400) || (U_Rh_H <= 97))   /* R >= 5.1k / R < 9.3k */
    {
      if (U_Rh_H < 4972)            /* R < 83.4M & prevent division by zero */
      {
        /* voltage breaks down with low test current and it is not nearly shorted  => resistor */

        Value = 0;                      /* reset value of resistor */

        if (U_Rl_L < 169)               /* R > 19.5k */
        {
          /*
           *  use measurements done with Rh
           */

          /* resistor is less 60MOhm */
          if (U_Rh_L >= 38)        /* R < 61.4M & prevent division by zero */
          {
            /*
             *  Rh pulled up (above DUT):
             *  I = U_Rh / Rh = (Vcc - U_Rh_H) / Rh
             *  R = U_R / I = U_Rh_H / ((Vcc - U_Rh_H) / Rh)
             *    = Rh * U_Rh_H / (Vcc - U_Rh_H)
             *
             *  Or via voltage divider:
             *  R = Rh * (U_dut / U_Rh)
             *    = Rh * (U_Rh_H / (Vcc - U_Rh_H))
             */

            Value1 = R_HIGH * U_Rh_H;
            Value1 /= (UREF_VCC - U_Rh_H);

            /*
             *  Rh pulled down (below DUT):
             *  I = U_Rh_L / Rh
             *  R = U_R / I = (Vcc - U_Rh_L) / (U_Rh_L / Rh)
             *    = Rh * (Vcc - U_Rh_L) / U_Rh_L
             *
             *  Or via voltage divider:
             *  R = Rh * (U_R / U_Rh)
             *    = Rh * ((Vcc - U_Rh_L) / U_Rh_L)
             */

            Value2 = R_HIGH * (UREF_VCC - U_Rh_L);
            Value2 /= U_Rh_L;

            /*
             *  calculate weighted average of both measurements
             *  - Voltages below the bandgap reference got a higher resolution
             *    (1.1mV instead of 4.9mV).
             */

            if (U_Rh_H < 990)           /* below bandgap reference */
            {
              /* weighted average for U_Rh_H */
              Value = (Value1 * 4);
              Value += Value2;
              Value /= 5;
            }
            else if (U_Rh_L < 990)      /* below bandgap reference */
            {
              /* weighted average for U_Rh_L */
              Value = (Value2 * 4);
              Value += Value1;
              Value /= 5;
            }
            else                        /* higher than bandgap reference */
            {
              /* classic average */
              Value = (Value1 + Value2) / 2;
            }

            Value += RH_OFFSET;         /* add offset value for Rh */
            Value *= 10;                /* upscale to 0.1 Ohms */
          }
        }
        else                       /* U_Rl_L: R <= 19.5k */
        {
          /*
           *  use measurements done with Rl
           */

          /* voltages below and above DUT match voltage divider */
          /* voltage below DUT can't be higher than above DUT */
          if ((U_Rl_H >= U_Ri_L) && (U_Ri_H >= U_Rl_L))
          {

            /*
             *  Rl pulled up (above DUT):
             *  I = U_Rl_RiH / (Rl + RiH) = (Vcc - U_Rl_H) / (Rl + RiH)
             *  R = U_Dut / I
             *    = (U_Rl_H - U_Ri_L) / ((Vcc - U_Rl_H) / (Rl + RiH))
             *    = (Rl + RiH) * (U_Rl_H - U_Ri_L) / (Vcc - U_Rl_H)
             *
             *  Or via voltage divider:
             *  R = (Rl + RiH) * (U_R_RiL / U_Rl_RiH) - RiL
             *    = (Rl + RiH) * (U_R_RiL / (Vcc - U_dut_RiL)) - RiL
             */

            if (U_Rl_H == UREF_VCC) U_Rl_H = UREF_VCC - 1;   /* prevent division by zero */
            Value1 = (R_LOW * 10) + Config.RiH;        /* Rl + RiH in 0.1 Ohm */
            Value1 *= (U_Rl_H - U_Ri_L);
            Value1 /= (UREF_VCC - U_Rl_H);

            /*
             *  Rl pulled down (below DUT):
             *  I = U_Rl_RiL / (Rl + RiL)
             *  R = U_R / I
             *    = (U_Ri_H - U_Rl_L) / (U_Rl_RiL / (Rl + RiL))
             *    = (Rl + RiL) * (U_Ri_H - U_Rl_L) / U_Rl_RiL
             *
             *  Or via voltage divider:
             *  R = (Rl + RiL) * (U_R_RiH / U_Rl_RiL) - RiH
             *    = (Rl + RiL) * ((Vcc - U_Rl_RiL) / U_Rl_RiL) - RiH
             */

            Value2 = (R_LOW * 10) + Config.RiL;   /* Rl + RiL in 0.1 Ohms */
            Value2 *= (U_Ri_H - U_Rl_L);
            Value2 /= U_Rl_L;

            /*
             *  calculate weighted average of both measurements
             *  - Voltages below the bandgap reference got a higher resolution
             *    (1.1mV instead of 4.9mV).
             */

            if (U_Rl_H < 990)           /* below bandgap reference */
            {
              /* weighted average for U_Rh_H */
              Value = (Value1 * 4);
              Value += Value2;
              Value /= 5;
            }
            else if (U_Rl_L < 990)      /* below bandgap reference */
            {
              /* weighted average for U_Rh_L */
              Value = (Value2 * 4);
              Value += Value1;
              Value /= 5;
            }
            else                        /* higher than bandgap reference */
            {
              /* classic average */
              Value = (Value1 + Value2) / 2;
            }
          }
          else      /* may happen for very low resistances */
          {
            if (U_Rl_L > 4750) Value = 1;    /* U_Rl_L: R < 15 Ohms */
            /* this will trigger the low resistance measurement below */
          }
        }


        /*
         *  process results of the resistance measurement
         */

        if (Value > 0)             /* valid resistor */
        {
          Scale = -1;              /* 0.1 Ohm by default */


          /*
           *  meassure small resistor <10 Ohm with special method
           */

          if (Value < 100UL)
          {
            Value = (unsigned long)SmallResistor();
            Scale = -2;                      /* 0.01 Ohm */

            /* auto-zero */
            if (Value > Config.RZero) Value -= Config.RZero;
            else Value = 0;
          }


          /*
           *  check for measurement in reversed direction
           */

          n = 0;
          while (n < ResistorsFound)
          {
            Resistor = &Resistors[n];

            if (Resistor->HiZ == Probe3_Pin)    /* same HiZ probe */
            {
              /* this is the reverse measurement */

              /*
               *  check if the reverse measurement is within a specific tolerance
               */

              /* set lower and upper tolerance limits */
              if (CmpValue(Value, Scale, 1, 0) == -1)  /* < 1 Ohm */
              {
                Temp = Value / 2;            /* 50% */
              }
              else                                     /* >= 1 Ohm */
              {
                Temp = Value / 20;           /* 5% */
              }

              Value1 = Value - Temp;         /* 95% or 50% */
              Value2 = Value + Temp;         /* 105% or 150% */

              /* special case for very low resistance */
              if (CmpValue(Value, Scale, 1, -1) == -1) /* < 0.1 Ohm */
              {
                Value1 = 0;                  /* 0 */
                Value2 = Value * 5;          /* 500% */
                if (Value2 == 0) Value2 = 5; /* special case */
              }

              /* check if value matches given tolerance */
              if ((CmpValue(Resistor->Value, Resistor->Scale, Value1, Scale) >= 0) &&
                  (CmpValue(Resistor->Value, Resistor->Scale, Value2, Scale) <= 0))
              {
                n = 100;             /* end loop and signal match */
              }
              else                 /* no match */
              {
                n = 200;             /* end loop and signal mis-match */
              }
            }
            else                           /* no match */
            {
              n++;                          /* next one */
            }
          }

          /* we got a new resistor */
          if (n != 100)
          {
            CompFound = COMP_RESISTOR;

            /* save data */
            Resistor = &Resistors[ResistorsFound];      /* free dataset */
            Resistor->A = Probe2_Pin;
            Resistor->B = Probe1_Pin;
            Resistor->HiZ = Probe3_Pin;
            Resistor->Value = Value;
            Resistor->Scale = Scale;
            ResistorsFound++;                           /* another one found */
            if (ResistorsFound > 6) ResistorsFound--;   /* prevent array overflow */
          }
        }
      }
    }
  }
}



/*
 *  check for depletion mode FET
 *
 *  returns:
 *  - voltage across Rl in pull-down mode
 */

unsigned int CheckDepModeFET(void)
{
  unsigned int      U_Rl_L;             /* return value / voltage across Rl */
  unsigned int      U_1;                /* voltage #1 */
  unsigned int      U_2;                /* voltage #2 */


  /*
   * setup probes:
   * - Gnd -- Rl -- probe-2 / probe-1 -- Vcc
   */

  R_PORT = 0;                      /* set resistor port to low */
  R_DDR = Probe2_Rl;               /* pull down probe-2 via Rl */
  ADC_DDR = Probe1_ADC;            /* set probe-1 to output */
  ADC_PORT = Probe1_ADC;           /* pull-up probe-1 directly */


  /*
   *  some FETs require the gate being discharged
   */

  /* try n-channel */
  /* we assume: probe-1 = D / probe-2 = S / probe-3 = G */
  PullProbe(Probe3_Rl, FLAG_10MS | FLAG_PULLDOWN);     /* discharge gate via Rl */
  U_Rl_L = ReadU_5ms(Probe2_Pin);                      /* get voltage at source */
  /* for n channel we would expect a low voltage */

  if (U_Rl_L >= 977)               /* this might by a p-channel */
  {
    /* we assume: probe-1 = S / probe-2 = D / probe-3 = G */
    PullProbe(Probe3_Rl, FLAG_10MS | FLAG_PULLUP);     /* discharge gate via Rl */
    U_Rl_L = ReadU_5ms(Probe2_Pin);                    /* get voltage at drain */
  }


  /*
   *  If there's a voltage drop across Rl (= current) without any current
   *  from HiZ probe we might have a self-conducting FET.
   *
   *  Other possibilities: diode or resistor
   */

  if (U_Rl_L > 92)
  {
    /*
     *  check if we got a n-channel JFET or depletion-mode MOSFET
     */

    if (CompDone == 0)        /* no component found yet */
    {
      /* we assume: probe-1 = S / probe-2 = D / probe-3 = G */
      R_DDR = Probe2_Rl | Probe3_Rh;         /* pull down gate via Rh */
      U_1 = ReadU_20ms(Probe2_Pin);          /* voltage at source */

      R_PORT = Probe3_Rh;                    /* pull up gate via Rh */
      U_2 = ReadU_20ms(Probe2_Pin);          /* voltage at source */

      /*
       *  If the source voltage is higher when the gate is driven by a positive
       *  voltage vs. connected to ground we got a self-conducting n-channel FET. 
       */

      if (U_2 > (U_1 + 488))
      {
        /*
         *  Compare gate voltage to distinguish JFET from MOSFET
         */

        /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
        ADC_PORT = 0;                        /* set ADC port to low */
        ADC_DDR = Probe2_ADC;                /* pull down source directly */
        R_DDR = Probe1_Rl | Probe3_Rh;       /* enable Rl for probe-1 & Rh for probe-3 */
        R_PORT = Probe1_Rl | Probe3_Rh;      /* pull up drain via Rl / pull up gate via Rh */

        U_2 = ReadU_20ms(Probe3_Pin);        /* get voltage at gate */

        if (U_2 > 3911)                 /* MOSFET */
        {
          /* n channel depletion-mode MOSFET */ 
          CompType = TYPE_N_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET;
        }
        else                            /* JFET */
        {
          /* n channel JFET (depletion-mode only) */
          CompType = TYPE_N_CHANNEL | TYPE_JFET;
        }

        /* save data */
        CompFound = COMP_FET;
        CompDone = 1;
        FET.G = Probe3_Pin;
        FET.D = Probe1_Pin;
        FET.S = Probe2_Pin;
      }
    }


    /*
     *  check if we got a p-channel JFET or depletion-mode MOSFET
     */

    if (CompDone == 0)        /* no component found yet */
    {
      /* we assume: probe-1 = S / probe-2 = D / probe 3 = G */
      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */ 
      ADC_PORT = 0;                          /* set ADC port to low */
      ADC_DDR = Probe2_ADC;                  /* pull down drain directly */
      R_DDR = Probe1_Rl | Probe3_Rh;         /* enable Rl for probe-1 & Rh for probe-3 */
      R_PORT = Probe1_Rl | Probe3_Rh;        /* pull up source via Rl / pull up gate via Rh */
      U_1 = ReadU_20ms(Probe1_Pin);          /* get voltage at source */

      R_PORT = Probe1_Rl;                    /* pull down gate via Rh */
      U_2 = ReadU_20ms(Probe1_Pin);          /* get voltage at source */

      /*
       *  If the source voltage is lower when the gate is driven by a positive
       *  voltage vs. connected to ground we got a self-conducting p-channel FET. 
       */

      if (U_1 > (U_2 + 488))
      {
        /*
         *  Compare gate voltage to distinguish JFET from MOSFET
         */

        /* set probes: probe-2 = HiZ / probe-1 -- Vcc */
        ADC_PORT = Probe1_ADC;               /* pull up source directly */
        ADC_DDR = Probe1_ADC;                /* enable pull up for source */
        /* gate is still pulled down via Rh */
        U_2 = ReadU_20ms(Probe3_Pin);        /* get voltage at gate */

        if (U_2 < 977)                  /* MOSFET */
        {
          /* p channel depletion-mode MOSFET */ 
          CompType =  TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_MOSFET; //Depletion-MOSFET
        }
        else                            /* JFET */
        {
          /* p channel JFET (depletion-mode only) */
          CompType = TYPE_P_CHANNEL | TYPE_DEPLETION | TYPE_JFET;
        }

        /* save data */
        CompFound = COMP_FET;
        CompDone = 1;
        FET.G = Probe3_Pin;
        FET.D = Probe2_Pin;
        FET.S = Probe1_Pin;
      }
    }
  }

  return U_Rl_L;
}



/*
 *  measure hfe of BJT in common collector circuit (emitter follower)
 *
 *  requires:
 *  - Type: NPN or PNP
 *
 *  returns:
 *  - hfe
 */

unsigned long Get_hfe_c(uint8_t Type)
{
  unsigned long     hfe;           /* return value */
  unsigned int      U_R_e;         /* voltage across emitter resistor */
  unsigned int      U_R_b;         /* voltage across base resistor */
  unsigned int      Ri;            /* internal resistance of µC */


  /*
   *  measure hfe for a BJT in common collector circuit
   *  (emitter follower):
   *  - hfe = (I_e - I_b) / I_b
   *  - measure the voltages across the resistors and calculate the currents
   *    (resistor values are well known)
   *  - hfe = ((U_R_e / R_e) - (U_R_b / R_b)) / (U_R_b / R_b)
   */

  /*
   *  setup probes and get voltages
   */

  if (Type == TYPE_NPN)            /* NPN */
  {
    /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
    /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    ADC_DDR = Probe1_ADC;               /* set probe 1 to output */
    ADC_PORT = Probe1_ADC;              /* pull up collector directly */
    R_DDR = Probe2_Rl | Probe3_Rl;      /* select Rl for probe-2 & Rl for probe-3 */
    R_PORT = Probe3_Rl;                 /* pull up base via Rl */

    U_R_e = ReadU_5ms(Probe2_Pin);         /* U_R_e = U_e */
    U_R_b = UREF_VCC - ReadU(Probe3_Pin);  /* U_R_b = Vcc - U_b */
  }
  else                             /* PNP */
  {
    /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
    /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
    ADC_PORT = 0;                       /* set ADC port low */
    ADC_DDR = Probe2_ADC;               /* pull down collector directly */
    R_PORT = Probe1_Rl;                 /* pull up emitter via Rl */
    R_DDR = Probe1_Rl | Probe3_Rl;      /* pull down base via Rl */

    U_R_e = UREF_VCC - ReadU_5ms(Probe1_Pin);     /* U_R_e = Vcc - U_e */
    U_R_b = ReadU(Probe3_Pin);                    /* U_R_b = U_b */
  }

  if (U_R_b < 10)             /* I_b < 14µA = Darlington */
  {
    /* change base resistor from Rl to Rh and measure again */
    if (Type == TYPE_NPN)            /* NPN */
    {    
      R_DDR = Probe2_Rl | Probe3_Rh;         /* select Rl for probe-2 & Rh for probe-3 */
      R_PORT = Probe3_Rh;                    /* pull up base via Rh */

      U_R_e = ReadU_5ms(Probe2_Pin);         /* U_R_e = U_e */
      U_R_b = UREF_VCC - ReadU(Probe3_Pin);  /* U_R_b = Vcc - U_b */

      Ri = Config.RiL;                       /* get internal resistor */
    }
    else                             /* PNP */
    {
      R_DDR = Probe1_Rl | Probe3_Rh;         /* pull down base via Rh */

      U_R_e = UREF_VCC - ReadU_5ms(Probe1_Pin);  /* U_R_e = Vcc - U_e */
      U_R_b = ReadU(Probe3_Pin);                 /* U_R_b = U_b */

      Ri = Config.RiH;                       /* get internal resistor */
    }

    /*
     *  Since I_b is so small vs. I_e we'll neglect it and use
     *  hfe = I_e / I_b
     *      = (U_R_e / R_e) / (U_R_b / R_b)
     *      = (U_R_e * R_b) / (U_R_b * R_e)
     */

    if (U_R_b < 1) U_R_b = 1;                /* prevent division by zero */
    hfe =  U_R_e * R_HIGH;                   /* U_R_e * R_b */
    hfe /= U_R_b;                            /* / U_R_b */
    hfe *= 10;                               /* upscale to 0.1 */
    hfe /= (R_LOW * 10) + Ri;                /* / R_e in 0.1 Ohm */
  }
  else                        /* I_b > 14µA = standard */
  {
    /*
     *  Both resistors are the same (R_e = R_b): 
     *  - hfe = ((U_R_e / R_e) - (U_R_b / R_b)) / (U_R_b / R_b)
     *  -     = (U_R_e - U_R_b) / U_R_b 
     */

    hfe = (unsigned long)((U_R_e - U_R_b) / U_R_b);
  }

  return hfe;
}



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
  PullProbe(Probe3_Rl, FLAG_10MS | FLAG_PULLDOWN);    /* discharge gate */
  U_1 = ReadU_5ms(Probe1_Pin);          /* get voltage at anode */

  R_PORT = 0;                           /* pull down anode */
  wait5ms();
  R_PORT = Probe1_Rl;                   /* and pull up anode again */
  U_2 = ReadU_5ms(Probe1_Pin);          /* get voltage at anode (below Rl) */

  /* voltages match behaviour of thyristor or triac */
  if ((U_1 < 1600) && (U_2 > 4400))
  {
    CompFound = COMP_THYRISTOR;         /* if not detected as a triac below */
    CompDone = 1;

    /*
     *  check if we got a triac
     *  - reverse A and C (A = MT2 / C = MT1)
     *  - check if behaviour is the same
     */

    /* we assume: probe-1 = MT2 / probe-2 = MT1 / probe-3 = G */
    R_DDR = 0;                          /* disable all probe resistors */
    R_PORT = 0;
    ADC_PORT = Probe2_ADC;              /* pull up MT1 directly */
    wait5ms();
    R_DDR = Probe1_Rl;                  /* pull down MT2 via Rl */ 
    /* probe-3/gate is in HiZ mode */

    /* triac shouldn't conduct without a triggered gate */ 
    U_1 = ReadU_5ms(Probe1_Pin);        /* get voltage at MT2 */

    /* voltage of MT2 is low (no current) */
    if (U_1 <= 244)
    {
      /* trigger gate for reverse direction */
      R_DDR = Probe1_Rl | Probe3_Rl;    /* and pull down gate via Rl */
      U_1 = ReadU_5ms(Probe3_Pin);      /* get voltage at gate */
      U_2 = ReadU(Probe1_Pin);          /* get voltage at MT2 */  

      /*
       * voltage at gate is ok and voltage at MT2 is high
       * (current = triac is conducting)
       */

      if ((U_1 >= 977) && (U_2 >= 733))
      {
        /* check if triac still conducts without triggered gate */ 
        R_DDR = Probe1_Rl;              /* set probe3 to HiZ mode */
        U_1 = ReadU_5ms(Probe1_Pin);    /* get voltage at MT2 */

        /* voltage at MT2 is still high (current = triac is conducting) */
        if (U_1 >= 733)
        {
          /* check if triac stops conducting when load current drops to zero */
          R_PORT = Probe1_Rl;           /* pull up MT2 via Rl */
          wait5ms();
          R_PORT = 0;                   /* and pull down MT2 via Rl */
          U_1 = ReadU_5ms(Probe1_Pin);  /* get voltage at MT2 */

          /* voltage at MT2 is low (no current = triac is not conducting) */
          if (U_1 <= 244)
          {
            /* now we are pretty sure that the DUT is a triac */
            CompFound = COMP_TRIAC;
          }
        }
      }
    }

    /* save data (we misuse BJT) */
    BJT.B = Probe3_Pin;
    BJT.C = Probe1_Pin;
    BJT.E = Probe2_Pin;

    Flag = 1;            /* signal that we found a component */
  }

  return Flag;
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

    Drain_Rl =  Probe1_Rl;
    Drain_ADC = Probe1_ADC;
    PullMode = FLAG_10MS | FLAG_PULLDOWN;
  }
  else                             /* p-channel */
  {
    /* we assume: probe-1 = S / probe-2 = D / probe-3 = G */
    /* probe-2 is still pulled down via Rl */
    /* probe-1 is still pulled up directly */

    Drain_Rl =  Probe2_Rl;
    Drain_ADC = Probe2_ADC;
    PullMode = FLAG_10MS | FLAG_PULLUP;
  }


  /*
   *  For low reaction times we use the ADC directly.
   */

  /* sanitize bit mask for drain to prevent a never-ending loop */ 
  Drain_ADC &= 0b00000111;              /* drain */
  ADMUX = Probe3_Pin | (1 << REFS0);    /* select probe-3 for ADC input */

  /* sample 10 times */
  for (Counter = 0; Counter < 10; Counter++) 
  {
    wdt_reset();                         /* reset watchdog */

    /* discharge gate via Rl for 10 ms */
    PullProbe(Probe3_Rl, PullMode);

    /* pull up/down gate via Rh to slowly charge gate */
    R_DDR = Drain_Rl | Probe3_Rh;

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
  Uth *= UREF_VCC;               /* convert to voltage */
  Uth /= 1024;                   /* using 10 bit resolution */

  /* save data */
  FET.V_th = (unsigned int)Uth;
}



/*
 *  check for BJT or depletion-mode MOSFET
 *
 *  requires:
 *  - BJT_Type: NPN or PNP
 *  - U_Rl: voltage across Rl pulled down
 */

void CheckBJTorDepMOSFET(uint8_t BJT_Type, unsigned int U_Rl)
{
  uint8_t           FET_Type;           /* MOSFET type */
  unsigned int      U_R_c;              /* voltage across collector resistor */
  unsigned int      U_R_b;              /* voltage across base resistor */
  unsigned int      BJT_Level;
  unsigned int      FET_Level;
  unsigned long     hfe_c;              /* hfe (common collector) */
  unsigned long     hfe_e;              /* hfe (common emitter) */

  /*
   *  init, set probes and measure
   */

  if (BJT_Type == TYPE_NPN)   /* NPN */
  {
    BJT_Level = 2557;
    FET_Level = 3400;
    FET_Type = TYPE_N_CHANNEL;

    /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
    /* or:        probe-1 = D / probe-2 = S / probe-3 = G */
    /* probes already set: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
    /* drive base/gate via Rh instead of Rl */
    R_DDR = Probe1_Rl | Probe3_Rh;      /* enable Rl for probe-1 & Rh for probe-3 */
    R_PORT = Probe1_Rl | Probe3_Rh;     /* pull up collector via Rl and base via Rh */
    wait50ms();                         /* wait to skip gate charging of a FET */
    U_R_c = UREF_VCC - ReadU(Probe1_Pin);  /* U_R_c = Vcc - U_c */ 
    U_R_b = UREF_VCC - ReadU(Probe3_Pin);  /* U_R_b = Vcc - U_b */
  }
  else                        /* PNP */
  {
    BJT_Level = 977;
    FET_Level = 2000;
    FET_Type = TYPE_P_CHANNEL;

    /* drive base/gate via Rh instead of Rl */
    /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
    /* or:        probe-1 = D / probe-2 = S / probe-3 = G */
    /* probes already set: Gnd -- Rl - probe-2 / probe-1 -- Vcc */
    R_DDR = Probe2_Rl | Probe3_Rh;      /* pull down base via Rh */
    U_R_c = ReadU_5ms(Probe2_Pin);      /* U_R_c = U_c */
    U_R_b = ReadU(Probe3_Pin);          /* U_R_b = U_b */
  }


  /*
   *  distinguish BJT from depletion-mode MOSFET
   */

  if (U_R_b > BJT_Level)      /* U_R_b matches minimum level of BJT */
  {
    /*
     *  A voltage drop across the base resistor Rh means that a current
     *  is flowing constantly. So this can't be a FET.
     *
     *  Problem:
     *  A reversed collector and emitter also passes the tests, but with
     *  a low hfe. So we need to run two tests to be sure and select the
     *  test results with the higher hfe.
     */

    /* two test runs needed at maximium to get right hfe & pins */
    if (CompFound == COMP_BJT) CompDone = 1;

    CompFound = COMP_BJT;
    CompType = BJT_Type;


    /*
     *  Calculate hfe via voltages and known resistors:
     *  - hfe = I_c / I_b
     *        = (U_R_c / R_c) / (U_R_b / R_b)
     *        = (U_R_c * R_b) / (U_R_b * R_c)
     */

    hfe_e = U_R_c * R_HIGH;                /* U_R_c * R_b */
    hfe_e /= U_R_b;                        /* / U_R_b */
    hfe_e *= 10;                           /* upscale to 0.1 */

    if (BJT_Type == TYPE_NPN)      /* NPN */
      hfe_e /= (R_LOW * 10) + Config.RiH;    /* / R_c in 0.1 Ohm */
    else                           /* PNP */
      hfe_e /= (R_LOW * 10) + Config.RiL;    /* / R_c in 0.1 Ohm */

    /* get hfe for common collector circuit */
    hfe_c = Get_hfe_c(BJT_Type);

    /* keep largest hfe */
    if (hfe_c > hfe_e) hfe_e = hfe_c;

    /* only update data if hfe is larger than old one */ 
    if (hfe_e > BJT.hfe)
    {
      /* save data */
      BJT.hfe = hfe_e;
      BJT.B = Probe3_Pin;

      if (BJT_Type == TYPE_NPN)    /* NPN */
      {
        BJT.C = Probe1_Pin;
        BJT.E = Probe2_Pin;
      }
      else                         /* PNP */
      {
        BJT.C = Probe2_Pin;
        BJT.E = Probe1_Pin;
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
      ADC_DDR = Probe1_ADC;                  /* pull-down emitter directly */
      R_PORT = Probe2_Rl | Probe3_Rh;        /* pull-up base via Rh */
      R_DDR = Probe2_Rl | Probe3_Rh;         /* enable probe resistors */
      U_R_b = UREF_VCC - ReadU_5ms(Probe2_Pin);  /* U_R_c = Vcc - U_c */        
    }
    else                        /* PNP */
    { 
      /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
      /* set probes: Gnd -- Rl - probe-1 / probe-2 -- Vcc */
      R_PORT = 0;
      R_DDR = Probe1_Rl | Probe3_Rh;         /* pull down base via Rh */
      ADC_DDR = Probe2_ADC;
      ADC_PORT = Probe2_ADC;                 /* pull-up emitter directly */
      U_R_b = ReadU_5ms(Probe1_Pin);         /* U_R_c = U_c */
    }

    /* if not reversed, BJT is identified */
//    U_R_b *= 10;                   /* be much larger */
    if (U_R_c > U_R_b)             /* I_c > I_c_reversed */
    {
      /* move other stuff here: save data & Comp= */
      CompDone = 1;
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
     *  we got a FET.
     */

    CompFound = COMP_FET;
    CompType = FET_Type | TYPE_ENHANCEMENT | TYPE_MOSFET;
    CompDone = 1;

    /* measure gate threshold voltage */
    GetGateThreshold(FET_Type);

    /* save data */
    FET.G = Probe3_Pin;
    FET.D = Probe2_Pin;
    FET.S = Probe1_Pin;
  }
}



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
  wdt_reset();                             /* reset watchdog */
  UpdateProbes(Probe1, Probe2, Probe3);    /* update bitmasks */

  /* check for depletion mode FET and get U_Rl */
  U_Rl = CheckDepModeFET();


  /*
   *  If there's nearly no conduction (just a small leakage current) between
   *  probe-1 and probe-2 we might have a semiconductor:
   *  - BJT
   *  - enhancement mode FET
   *  - Thyristor or Triac
   *  or a large resistor
   */

  if (U_Rl < 977)         /* load current < 1.4mA */
  {
    /*
     *  check for:
     *  - PNP BJT (common emitter circuit)
     *  - p-channel MOSFET (low side switching circuit)
     */

    if (CompDone == 0)             /* not sure yet */
    {
      /* we assume: probe-1 = E / probe-2 = C / probe-3 = B */
      /* set probes: Gnd -- Rl - probe-2 / probe-1 -- Vcc */
      R_DDR = Probe2_Rl;                /* enable Rl for probe-2 */
      R_PORT = 0;                       /* pull down collector via Rl */
      ADC_DDR = Probe1_ADC;             /* set probe 1 to output */
      ADC_PORT = Probe1_ADC;            /* pull up emitter directly */
      wait5ms();
      R_DDR = Probe2_Rl | Probe3_Rl;    /* pull down base via Rl */
      U_1 = ReadU_5ms(Probe2);          /* get voltage at collector */ 
 
      /*
       *  If DUT is conducting we might have a PNP BJT or p-channel FET.
       */

      if (U_1 > 3422)                   /* detected current > 4.8mA */
      {
        /* distinguish PNP BJT from p-channel MOSFET */
        CheckBJTorDepMOSFET(TYPE_PNP, U_Rl);
      }
    }


    /*
     *  check for
     *  - NPN BJT (common emitter circuit)
     *  - Thyristor and Triac
     *  - n-channel MOSFET (high side switching circuit)
     */

    if (CompDone == 0)             /* not sure yet */
    {
      /* we assume: probe-1 = C / probe-2 = E / probe-3 = B */
      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc */
      ADC_DDR = Probe2_ADC;             /* set probe-2 to output mode */
      ADC_PORT = 0;                     /* pull down probe-2 directly */
      R_DDR = Probe1_Rl | Probe3_Rl;    /* select Rl for probe-1 & Rl for probe-3 */
      R_PORT = Probe1_Rl | Probe3_Rl;   /* pull up collector & base via Rl */
      U_1 = ReadU_5ms(Probe1);          /* get voltage at collector */

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
          /* It seems that we got a NPN BJT or a n-channel MOSFET. */
          CheckBJTorDepMOSFET(TYPE_NPN, U_Rl);
        }
      }
    }
  }


  /*
   *  If there's conduction between probe-1 and probe-2 we might have a
   *  - diode (conducting)
   *  - small resistor (checked later on)
   */

  else              /* load current > 1.4mA */
  {
    /* We check for a diode even if we already found a component to get Vf. */
    CheckDiode();
  }


  /*
   *  Check for a resistor.
   */

  if ((CompFound == COMP_NONE) ||
      (CompFound == COMP_RESISTOR))
  {
    CheckResistor();
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


/* local bit flags */
#undef FLAG_PULLDOWN
#undef FLAG_PULLUP
#undef FLAG_1MSMID
#undef FLAG_10MS

/* source management */
#undef PROBES_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   main part
 *
 *   (c) 2012-2014 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define MAIN_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "LCD.h"              /* LCD module */
#include "functions.h"        /* external functions */


/*
 *  local variables
 */

/* programm control */
uint8_t        RunsPassed;              /* counter for successful measurements */
uint8_t        RunsMissed;              /* counter for failed/missed measurements */



/* ************************************************************************
 *   output found components
 * ************************************************************************ */


/*
 *  wait for key press and goto display line #2
 */

void UpdateLine2(void)
{
  TestKey(3000, 11);                  /* next page */
  lcd_clear_line(2);                  /* only change line #2 */
}


/*
 *  show pinout for semiconductor
 *
 *  required:
 *  - address of pin ID table in EEPROM
 */

void Show_SemiPinout(uint8_t A, uint8_t B, uint8_t C)
{
  uint8_t           i, j;     /* counter */
  unsigned char     Pin[3];   /* component pins */
  unsigned char     ID[3];    /* component pin IDs */

  /* copy probe pins */
  Pin[0] = Semi.A;
  Pin[1] = Semi.B;
  Pin[2] = Semi.C;

  /* copy pin IDs */
  ID[0] = A;
  ID[1] = B;
  ID[2] = C;

  /* display: 123 */
  for (i = 0; i <= 2; i++)
  {
    lcd_testpin(i);
  }

  /* display: = */
  lcd_data('=');

  /* display pin IDs */
  for (i = 0; i <= 2; i++)         /* loop through probe pins */
  {
    for (j = 0; j <= 2; j++)       /* loop through component pins */
    {
      if (i == Pin[j])        /* probe pin matches */
      {
        lcd_data(ID[j]);           /* show ID */
      }
    }
  }
}



/*
 *  show failed test
 */

void Show_Fail(void)
{
  /* display info */
  lcd_fixed_string(Failed1_str);        /* display: No component */
  lcd_line(2);                          /* move to line #2 */
  lcd_fixed_string(Failed2_str);        /* display: found!*/  

  /* display numbers of diodes found */
  if (Check.Diodes > 0)                 /* diodes found */
  {
    lcd_space();                        /* display space */
    lcd_data(Check.Diodes + '0');       /* display number of diodes found */
    lcd_space();                        /* display space */
    lcd_fixed_string(Diode_AC_str);     /* display: -|>|- */    
  }

  RunsMissed++;               /* increase counter */
  RunsPassed = 0;             /* reset counter */
}



/*
 *  show error
 */

void Show_Error()
{
  if (Check.Type == TYPE_DISCHARGE)     /* discharge failed */
  {
    lcd_fixed_string(DischargeFailed_str);   /* display: Battery? */

    /* display probe number and remaining voltage */
    lcd_line(2);
    lcd_testpin(Check.Probe);
    lcd_data(':');
    lcd_space();
    DisplayValue(Check.U, -3, 'V');
  }
}



/*
 *  show single (first) resistor
 *
 *  requires:
 *  - ID1 pin ID character
 *  - ID2 pin ID character
 */

void Show_SingleResistor(uint8_t ID1, uint8_t ID2)
{
  Resistor_Type     *Resistor;     /* pointer to resistor */

  Resistor = &Resistors[0];        /* pointer to first resistor */

  /* show pinout */
  lcd_data(ID1);
  lcd_fixed_string(Resistor_str);
  lcd_data(ID2); 

  /* show resistance value */
  lcd_space();
  DisplayValue(Resistor->Value, Resistor->Scale, LCD_CHAR_OMEGA);
}



/*
 *  show resistor(s)
 */

void Show_Resistor(void)
{
  Resistor_Type     *R1;           /* pointer to resistor #1 */
  Resistor_Type     *R2;           /* pointer to resistor #2 */
  uint8_t           Pin;           /* ID of common pin */

  R1 = &Resistors[0];              /* pointer to first resistor */

  if (Check.Resistors == 1)        /* single resistor */
  {
    R2 = NULL;                     /* disable second resistor */
    Pin = R1->A;                   /* make B the first pin */
  }
  else                             /* multiple resistors */
  {
    R2 = R1;
    R2++;                          /* pointer to second resistor */

    if (Check.Resistors == 3)      /* three resistors */
    {
      Resistor_Type     *Rmax;     /* pointer to largest resistor */    

      /*
       *  3 resistors mean 2 single resistors and both resistors in series.
       *  So we have to single out that series resistor by finding the
       *  largest resistor.
       */

      Rmax = R1;                   /* starting point */
      for (Pin = 1; Pin <= 2; Pin++)
      {
        if (CmpValue(R2->Value, R2->Scale, Rmax->Value, Rmax->Scale) == 1)
        {
          Rmax = R2;          /* update largest one */
        }

        R2++;                 /* next one */
      }

      /* get the two smaller resistors */
      if (R1 == Rmax) R1++;
      R2 = R1;
      R2++;
      if (R2 == Rmax) R2++;
    }

    /* find common pin of both resistors */
    if ((R1->A == R2->A) || (R1->A == R2->B)) Pin = R1->A;
    else Pin = R1->B;
  }


  /*
   *  display the pins
   */

  /* first resistor */
  if (R1->A != Pin) lcd_testpin(R1->A);
  else lcd_testpin(R1->B);
  lcd_fixed_string(Resistor_str);
  lcd_testpin(Pin);

  if (R2)           /* second resistor */
  {
    lcd_fixed_string(Resistor_str);
    if (R2->A != Pin) lcd_testpin(R2->A);
    else lcd_testpin(R2->B);
  }


  /*
   *  display the values
   */

  /* first resistor */
  lcd_line(2);
  DisplayValue(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

  if (R2)                /* second resistor */
  {
    lcd_space();
    DisplayValue(R2->Value, R2->Scale, LCD_CHAR_OMEGA);
  }
  #ifdef EXTRA
  else                   /* single resistor */
  {
    /* get inductance and display if relevant */
    if (MeasureInductor(R1) == 1)
    {
      lcd_space();
      DisplayValue(Inductor.Value, Inductor.Scale, 'H');
    }
  }
  #endif
}


/*
 *  show capacitor
 */

void Show_Capacitor(void)
{
  Capacitor_Type    *MaxCap;       /* pointer to largest cap */
  Capacitor_Type    *Cap;          /* pointer to cap */
  uint8_t           Counter;       /* loop counter */

  /* find largest cap */
  MaxCap = &Caps[0];               /* pointer to first cap */
  Cap = MaxCap;

  for (Counter = 1; Counter <= 2; Counter++) 
  {
    Cap++;                              /* next cap */

    if (CmpValue(Cap->Value, Cap->Scale, MaxCap->Value, MaxCap->Scale) == 1)
    {
      MaxCap = Cap;
    }
  }

  /* display pinout */
  lcd_testpin(MaxCap->A);               /* display pin #1 */
  lcd_fixed_string(Cap_str);            /* display capacitor symbol */
  lcd_testpin(MaxCap->B);               /* display pin #2 */
  lcd_line(2);                          /* move to line #2 */

  /* and show capacitance */
  DisplayValue(MaxCap->Value, MaxCap->Scale, 'F');
}



/*
 *  display capacitance of a diode
 *
 *  requires:
 *  - pointer to diode structure
 */

void Show_Diode_Cap(Diode_Type *Diode)
{
  /* sanity check */
  if (Diode == NULL) return;

  /* get capacitance (opposite of flow direction) */
  MeasureCap(Diode->C, Diode->A, 0);

  /* and show capacitance */
  DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
}



/*
 *  show diode
 */

void Show_Diode(void)
{
  Diode_Type        *D1;           /* pointer to diode #1 */
  Diode_Type        *D2 = NULL;    /* pointer to diode #2 */
  uint8_t           CapFlag = 1;   /* flag for capacitance output */
  uint8_t           A = 5;         /* ID of common anode */
  uint8_t           C = 5;         /* ID of common cothode */
  uint8_t           R_Pin1 = 5;    /* B_E resistor's pin #1 */
  uint8_t           R_Pin2;        /* B_E resistor's pin #2 */
  uint16_t          I_leak;        /* leakage current */

  D1 = &Diodes[0];                 /* pointer to first diode */


  /*
   *  figure out which diodes to display
   */

  if (Check.Diodes == 1)           /* single diode */
  {
    C = D1->C;                     /* make anode first pin */
  }
  else if (Check.Diodes == 2)      /* two diodes */
  {
    D2 = D1;
    D2++;                          /* pointer to second diode */

    if (D1->A == D2->A)            /* common anode */
    {
      A = D1->A;                   /* save common anode */

      /* possible PNP BJT with low value B-E resistor and flyback diode */
      R_Pin1 = D1->C;
      R_Pin2 = D2->C;
    }
    else if (D1->C == D2->C)       /* common cathode */
    {
      C = D1->C;                   /* save common cathode */

      /* possible NPN BJT with low value B-E resistor and flyback diode */
      R_Pin1 = D1->A;
      R_Pin2 = D2->A;
    }
    else if ((D1->A == D2->C) && (D1->C == D2->A))   /* anti-parallel */
    {
      A = D1->A;                   /* anode and cathode */
      C = A;                       /* are the same */
      CapFlag = 0;                 /* skip capacitance */
    }
  }
  else if (Check.Diodes == 3)      /* three diodes */
  {
    uint8_t         n;
    uint8_t         m;

    /*
     *  Two diodes in series are additionally detected as third big diode:
     *  - Check for any possible way of 2 diodes be connected in series.
     *  - Only once the cathode of diode #1 matches the anode of diode #2.
     */

    for (n = 0; n <= 2; n++)       /* loop for first diode */
    {
      D1 = &Diodes[n];             /* get pointer of first diode */

      for (m = 0; m <= 2; m++)     /* loop for second diode */
      {
        D2 = &Diodes[m];           /* get pointer of second diode */

        if (n != m)                /* don't check same diode :-) */
        {
          if (D1->C == D2->A)      /* got match */
          {
            n = 5;                 /* end loops */
            m = 5;
          }
        }
      }
    }

    if (n < 5) D2 = NULL;          /* no match found */
    C = D1->C;                     /* cathode of first diode */
    A = 3;                         /* in series mode */
  }
  else                             /* to much diodes */
  {
    D1 = NULL;                     /* don't display any diode */
    Show_Fail();                   /* and tell user */
    return;
  }


  /*
   *  display pins 
   */

  /* first Diode */
  if (A < 3) lcd_testpin(D1->C);           /* common anode */
  else lcd_testpin(D1->A);                 /* common cathode */

  if (A < 3) lcd_fixed_string(Diode_CA_str);   /* common anode */
  else lcd_fixed_string(Diode_AC_str);         /* common cathode */

  if (A < 3) lcd_testpin(A);               /* common anode */
  else lcd_testpin(C);                     /* common cathode */

  if (D2)           /* second diode */
  {
    if (A <= 3) lcd_fixed_string(Diode_AC_str);   /* common anode or in series */
    else lcd_fixed_string(Diode_CA_str);          /* common cathode */

    if (A == C) lcd_testpin(D2->A);          /* anti parallel */
    else if (A <= 3) lcd_testpin(D2->C);     /* common anode or in series */
    else lcd_testpin(D2->A);                 /* common cathode */
  }

  /* check for B-E resistor for possible BJT */
  if (R_Pin1 < 5)                  /* possible BJT */
  {
    if (CheckSingleResistor(R_Pin1, R_Pin2) == 1) /* found B-E resistor */
    {
      /* show: PNP/NPN? */
      lcd_space();
      if (A < 3) lcd_fixed_string(PNP_str);
      else lcd_fixed_string(NPN_str);
      lcd_data('?');

      lcd_line(2);                        /* go to line #2 */
      R_Pin1 += '1';                      /* convert to character */
      R_Pin2 += '1';
      Show_SingleResistor(R_Pin1, R_Pin2);   /* show resistor */
      TestKey(3000, 11);                  /* next page */
      CapFlag = 0;                        /* skip capacitance */
    }
  }


  /*
   *  display:
   *  - Uf (forward voltage)
   *  - reverse leakage current (for single diodes)
   *  - capacitance (not for anti-parallel diodes)
   */

  /* display Uf */
  lcd_clear_line(2);                    /* only change line #2 */  
  lcd_fixed_string(Vf_str);             /* display: Vf= */

  /* first diode */
  DisplayValue(D1->V_f, -3, 'V');

  lcd_space();

  /* display low current Uf and reverse leakage current for a single diode */
  if (D2 == NULL)                       /* single diode */
  {
    /* display low current Uf if it's quite low (Ge/Schottky diode) */
    if (D1->V_f2 < 250)
    {
      lcd_data('(');
      DisplayValue(D1->V_f2, 0, 0);
      lcd_data(')');
    }

    /* reverse leakage current */
    UpdateProbes(D1->C, D1->A, 0);      /* reverse diode */
    I_leak = GetLeakageCurrent();       /* get current (in µA) */
    if (I_leak > 0)                     /* show if not zero */
    {
      UpdateLine2();                      /* key press for line #2 */
      lcd_fixed_string(I_R_str);          /* display: I_R= */
      DisplayValue(I_leak, -6, 'A');      /* display current */
    }
  }
  else                                  /* two diodes */
  {
    /* show Uf of second diode */
    DisplayValue(D2->V_f, -3, 'V');
  }

  /* display capacitance */
  if (CapFlag == 1)                     /* if requested */ 
  {
    UpdateLine2();                      /* key press for line #2 */
    lcd_fixed_string(DiodeCap_str);     /* display: C= */
    Show_Diode_Cap(D1);                 /* first diode */
    lcd_space();
    Show_Diode_Cap(D2);                 /* second diode (optional) */
  }
}



/*
 *  show intrinsic/freewheeling diode of transistor
 */

void Show_FlybackDiode(void)
{
  lcd_space();                /* display space */

  /* first pin */
  if (Check.Found == COMP_FET)     /* FET */
  {
    lcd_data('D');                 /* drain */
  }
  else                             /* BJT/IGBT */
  {
    lcd_data('C');                 /* collector */
  }

  /* diode */
  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel/NPN */
  {
    /*
     *  anode pointing to source/emitter
     *  cathode pointing to drain/collector
     */

    lcd_data(LCD_CHAR_DIODE_CA);        /* |<| */
  }
  else                                  /* p-channel/PNP */
  {
    /*
     *  anode pointing to drain/collector
     *  cathode pointing to source/emitter
     */

    lcd_data(LCD_CHAR_DIODE_AC);        /* |>| */
  }

  /* second pin */
  if (Check.Found == COMP_FET)     /* FET */
  {
    lcd_data('S');                 /* source */
  }
  else                             /* BJT/IGBT */
  {
    lcd_data('E');                 /* emitter */
  }
}



/*
 *  show BJT
 */

void Show_BJT(void)
{
  Diode_Type        *Diode;        /* pointer to diode */
  unsigned char     *String;       /* display string pointer */
  uint8_t           Counter;       /* counter */
  uint8_t           A_Pin;         /* pin acting as anode */
  uint8_t           C_Pin;         /* pin acting as cathode */
  unsigned int      V_BE;          /* V_BE */
  signed int        Slope;         /* slope of forward voltage */

  /*
   *  Mapping for Semi structure:
   *  A   - Base pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_1 - U_BE (mV)
   *  I_1 - I_CE0 (µA)
   *  F_1 - hFE
   */

  /* preset stuff based on BJT type */
  if (Check.Type & TYPE_NPN)       /* NPN */
  {
    String = (unsigned char *)NPN_str;       /* "NPN" */

    /* direction of B-E diode: B -> E */
    A_Pin = Semi.A;      /* anode at base */
    C_Pin = Semi.C;      /* cathode at emitter */
  }
  else                             /* PNP */
  {
    String = (unsigned char *)PNP_str;       /* "PNP" */

    /* direction of B-E diode: E -> B */
    A_Pin = Semi.C;      /* anode at emitter */
    C_Pin = Semi.A;      /* cathode at base */
  }

  /* display type */
  lcd_fixed_string(BJT_str);       /* display: BJT */
  lcd_space();                     /* display space */
  lcd_fixed_string(String);        /* display: NPN / PNP */

  /* parasitic BJT (freewheeling diode on same substrate) */
  if (Check.Type & TYPE_PARASITIC)
  {
    lcd_data('+');
  }

  lcd_line(2);                     /* move to line #2 */

  /* display pinout */
  Show_SemiPinout('B', 'C', 'E');

  /* optional freewheeling diode */
  if (Check.Diodes > 2)       /* transistor is a set of two diodes :-) */
  {
    Show_FlybackDiode();           /* show diode */
  }

  UpdateLine2();                   /* key press for line #2 */

  /* display either optional B-E resistor or hFE & V_BE */
  if (CheckSingleResistor(C_Pin, A_Pin) == 1)     /* found B-E resistor */
  {
    Show_SingleResistor('B', 'E');
  }
  else                                            /* no B-E resistor found */
  {
    /* hFE and V_BE */

    /* display hFE */
    lcd_fixed_string(hFE_str);     /* display: h_FE= */
    DisplayValue(Semi.F_1, 0, 0);

    /* display V_BE (taken from diode forward voltage) */
    Diode = &Diodes[0];                 /* get pointer of first diode */  
    Counter = 0;

    while (Counter < Check.Diodes)      /* check all diodes */
    {
      /* if the diode matches the transistor's B-E diode */
      if ((Diode->A == A_Pin) && (Diode->C == C_Pin))
      {
        UpdateLine2();                  /* key press for line #2 */
        lcd_fixed_string(V_BE_str);     /* display: V_BE= */

        /*
         *  Vf is quite linear for a logarithmicly scaled I_b.
         *  So we may interpolate the Vf values of low and high test current
         *  measurements for a virtual test current. Low test current is 10µA
         *  and high test current is 7mA. That's a logarithmic scale of
         *  3 decades.
         */

        /* calculate slope for one decade */
        Slope = Diode->V_f - Diode->V_f2;
        Slope /= 3;

        /* select V_BE based on hFE */
        if (Semi.F_1 < 100)             /* low hFE */
        {
          /*
           *  BJTs with low hFE are power transistors and need a large I_b
           *  to drive the load. So we simply take Vf of the high test current
           *  measurement (7mA). 
           */

          V_BE = Diode->V_f;
        }
        else if (Semi.F_1 < 250)        /* mid-range hFE */
        {
          /*
           *  BJTs with a mid-range hFE are signal transistors and need
           *  a small I_b to drive the load. So we interpolate Vf for
           *  a virtual test current of about 1mA.
           */

          V_BE = Diode->V_f - Slope;
        }
        else                            /* high hFE */
        {
          /*
           *  BJTs with a high hFE are small signal transistors and need
           *  only a very small I_b to drive the load. So we interpolate Vf
           *  for a virtual test current of about 0.1mA.
           */

          V_BE = Diode->V_f2 + Slope;
        }

        DisplayValue(V_BE, -3, 'V');

        Counter = 10;              /* end loop */
      }
      else                         /* diode doesn't match */
      {
        Counter++;                      /* increase counter */
        Diode++;                        /* next one */
      }
    }
  }

  /* I_CEO: collector emitter cutoff current (leakage) */
  if (Semi.I_1 > 0)                     /* show if not zero */
  {
    UpdateLine2();                      /* key press for line #2 */
    lcd_fixed_string(I_CEO_str);        /* display: I_CE0= */
    DisplayValue(Semi.I_1, -6, 'A');    /* display current */
  }
}



/*
 *  show MOSFET/IGBT extras
 *  - diode
 *  - V_th
 *  - Cgs
 */

void Show_FET_Extras(void)
{
  /* show instrinsic/freewheeling diode */
  if (Check.Diodes > 0)            /* diode found */
  {
    Show_FlybackDiode();           /* show diode */
  }

  /* skip remaining stuff for depletion-mode FETs/IGBTs */
  if (Check.Type & TYPE_DEPLETION) return;

  /* gate threshold voltage */
  if (Semi.U_2 != 0)
  {
    UpdateLine2();                      /* key press for line #2 */
    lcd_fixed_string(Vth_str);               /* display: Vth */
    DisplaySignedValue(Semi.U_2, -3, 'V');   /* display V_th in mV */
  }

  /* display gate-source capacitance */
  UpdateLine2();                      /* key press for line #2 */
  lcd_fixed_string(GateCap_str);      /* display: Cgs= */
  MeasureCap(Semi.A, Semi.C, 0);      /* measure capacitance */
  /* display value and unit */
  DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
}



/*
 *  show FET/IGBT channel type
 */

void Show_FET_Channel(void)
{
  lcd_space();                     /* display space */

  /* channel type */
  if (Check.Type & TYPE_N_CHANNEL)   /* n-channel */
  {
    lcd_data('N');
  }
  else                               /* p-channel */
  {
    lcd_data('P');
  }

  lcd_fixed_string(Channel_str);   /* display: -ch */
}



/*
 *  show FET/IGBT mode
 */

void Show_FET_Mode(void)
{
  lcd_space();

  if (Check.Type & TYPE_ENHANCEMENT)    /* enhancement mode */
  {
    lcd_fixed_string(Enhancement_str);
  }
  else                                  /* depletion mode */
  {
    lcd_fixed_string(Depletion_str);
  }
}



/*
 *  show FET
 */

void Show_FET(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Drain pin
   *  C   - Source pin
   *  U_2 - V_th (mV)
   */

  /* display type */
  if (Check.Type & TYPE_MOSFET)    /* MOSFET */
  {
    lcd_fixed_string(MOS_str);       /* display: MOS */
  }
  else                             /* JFET */
  {
    lcd_data('J');                   /* display: J */
  }
  lcd_fixed_string(FET_str);       /* display: FET */

  /* display channel type */
  Show_FET_Channel();
      
  /* display mode for MOSFETs*/
  if (Check.Type & TYPE_MOSFET) Show_FET_Mode();

  /* pinout */
  lcd_line(2);                     /* move to line #2 */

  if (Check.Type & TYPE_SYMMETRICAL)    /* symmetrical Drain and Source */
  {
    /* we can't distinguish D and S */
    Show_SemiPinout('G', 'x', 'x');     /* show pinout */
  }
  else                                  /* unsymmetrical Drain and Source */
  {
    Show_SemiPinout('G', 'D', 'S');     /* show pinout */
  }

  /* show diode, V_th and Cgs for MOSFETs */
  if (Check.Type & TYPE_MOSFET) Show_FET_Extras();
}



/*
 *  show IGBT  
 */

void Show_IGBT(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_2 - V_th (mV)
   */

  lcd_fixed_string(IGBT_str);      /* display: IGBT */
  Show_FET_Channel();              /* display channel type */
  Show_FET_Mode();                 /* display mode */

  lcd_line(2);                     /* move to line #2 */
  Show_SemiPinout('G', 'C', 'E');  /* show pinout */
  Show_FET_Extras();               /* show diode, V_th and C_GE */
}



/*
 *  show special components like Thyristor and Triac
 */

void Show_Special(void)
{
  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Anode/MT2 pin
   *  C   - Cathode/MT1 pin
   *  U_1 - V_GT (mV)
   */

  /* display component type any pinout */
  if (Check.Found == COMP_THYRISTOR)    /* SCR */
  {
    lcd_fixed_string(Thyristor_str);    /* display: thyristor */
    lcd_line(2);                        /* move to line #2 */
    Show_SemiPinout('G', 'A', 'C');     /* display pinout */
  }
  else                                  /* Triac */
  {
    lcd_fixed_string(Triac_str);        /* display: triac */
    lcd_line(2);                        /* move to line #2 */
    Show_SemiPinout('G', '2', '1');     /* display pinout */
  }

  /* show V_GT (gate trigger voltage) */
  if (Semi.U_1 > 0)                /* show if not zero */
  {
    UpdateLine2();                      /* key press for line #2 */
    lcd_fixed_string(V_GT_str);         /* display: V_GT: */
    DisplayValue(Semi.U_1, -3, 'V');    /* display V_GT in mV */
  }
}



/* ************************************************************************
 *   the one and only main()
 * ************************************************************************ */


/*
 *  main function
 */

int main(void)
{
  unsigned int      U_Bat;              /* voltage of power supply */
  uint8_t           Test;               /* test value */

  /*
   *  init
   */

  /* switch on power to keep me alive */
  CONTROL_DDR = (1 << POWER_CTRL);      /* set pin as output */
  CONTROL_PORT = (1 << POWER_CTRL);     /* set pin to drive power management transistor */

  /* setup µC */
  MCUCR = (1 << PUD);                        /* disable pull-up resistors globally */
  ADCSRA = (1 << ADEN) | ADC_CLOCK_DIV;      /* enable ADC and set clock divider */

  #ifdef HW_RELAY
  /* init relay (safe mode) */
                                      /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);            /* short circuit probes */
  #endif

  /* catch watchdog */  
  Test = (MCUSR & (1 << WDRF));         /* save watchdog flag */
  MCUSR &= ~(1 << WDRF);                /* reset watchdog flag */
  wdt_disable();                        /* disable watchdog */


  /*
   *  watchdog was triggered (timeout 2s)
   *  - This is after the µC done a reset driven by the watchdog.
   *  - Does only work if the capacitor at the base of the power management
   *    transistor is large enough to survive a µC reset. Otherwise the
   *    tester simply looses power.
   */

  if (Test)
  {
    lcd_clear();                        /* display was initialized before */
    lcd_fixed_string(Timeout_str);      /* display: timeout */
    lcd_line(2);
    lcd_fixed_string(Error_str);        /* display: error */
    MilliSleep(2000);                   /* give user some time to read */
    CONTROL_PORT = 0;                   /* power off myself */
    return 0;                           /* exit program */
  }


  /*
   *  init LCD module and load custom characters
   */

  lcd_init();                           /* initialize LCD */

  /* symbols for components */
  lcd_fixed_customchar(DiodeIcon1, LCD_CHAR_DIODE_AC); /* diode symbol '|>|' */
  lcd_fixed_customchar(DiodeIcon2, LCD_CHAR_DIODE_CA); /* diode symbol '|<|' */
  lcd_fixed_customchar(CapIcon, LCD_CHAR_CAP);         /* capacitor symbol '||' */
  lcd_fixed_customchar(ResIcon1, LCD_CHAR_RESISTOR_L); /* resistor symbol '[' */
  lcd_fixed_customchar(ResIcon2, LCD_CHAR_RESISTOR_R); /* resistor symbol ']' */

  #ifdef LCD_CYRILLIC
  /* kyrillish LCD character set lacks omega and µ */
  lcd_fixed_customchar(OmegaIcon, LCD_CHAR_OMEGA);     /* Omega */
  lcd_fixed_customchar(MicroIcon, LCD_CHAR_MICRO);     /* µ / micro */
  #endif

  /* return to normal output */
  lcd_line(1);                               /* move to line #1 */


  /*
   *  operation mode selection
   */

  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* default: power save */
  Config.TesterMode = MODE_CONTINOUS;        /* set default mode: continous */

  /* catch long key press */
  if (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* if test button is pressed */
  {
    MilliSleep(300);                         /* wait to catch a long key press */
    if (!(CONTROL_PIN & (1 << TEST_BUTTON))) /* if button is still pressed */
      Config.TesterMode = MODE_AUTOHOLD;     /* set auto-hold mode */
  }


  /*
   *  welcome user
   */

  lcd_fixed_string(Tester_str);         /* display: Component Tester */
  lcd_line(2);                          /* move to line #2 */
  lcd_fixed_string(Version_str);        /* display firmware version */
  MilliSleep(1000);                     /* let the user read the display */


  /*
   *  init variables
   */

  /* cycling */
  RunsMissed = 0;
  RunsPassed = 0;

  /* default offsets and values */
  Config.Samples = ADC_SAMPLES;         /* number of ADC samples */
  Config.AutoScale = 1;                 /* enable ADC auto scaling */
  Config.RefFlag = 1;                   /* no ADC reference set yet */
  Config.Vcc = UREF_VCC;                /* voltage of Vcc */
  LoadAdjust();                         /* load adjustment values */

  wdt_enable(WDTO_2S);		        /* enable watchdog (timeout 2s) */


  /*
   *  main processing cycle
   */

start:

  /* reset variabels */
  Check.Found = COMP_NONE;
  Check.Type = 0;
  Check.Done = 0;
  Check.Diodes = 0;
  Check.Resistors = 0;
  Semi.U_1 = 0;
  Semi.I_1 = 0;
  Semi.F_1 = 0;

  /* reset hardware */
  ADC_DDR = 0;                     /* set all pins of ADC port as input */
                                   /* also remove short circuit by relay */
  lcd_clear();                     /* clear LCD */


  /*
   *  voltage reference
   */

  #ifdef HW_REF25
  /* external 2.5V reference */
  Config.Samples = 200;            /* do a lot of samples for high accuracy */
  U_Bat = ReadU(TP_REF);           /* read voltage of reference (mV) */
  Config.Samples = ADC_SAMPLES;    /* set samples back to default */

  if ((U_Bat > 2250) && (U_Bat < 2750))   /* check for valid reference */
  {
    uint32_t        Temp;

    /* adjust Vcc (assuming 2.495V typically) */
    Temp = ((uint32_t)Config.Vcc * UREF_25) / U_Bat;
    Config.Vcc = (unsigned int)Temp;
  }
  #endif

  /* internal bandgap reference */
  Config.Bandgap = ReadU(0x0e);         /* dummy read for bandgap stabilization */
  Config.Samples = 200;                 /* do a lot of samples for high accuracy */
  Config.Bandgap = ReadU(0x0e);         /* get voltage of bandgap reference (mV) */
  Config.Samples = ADC_SAMPLES;         /* set samples back to default */
  Config.Bandgap += Config.RefOffset;   /* add voltage offset */ 


  /*
   *  battery check
   */

  /*
   *  ADC pin is connected to a voltage divider Rh = 10k and Rl = 3k3.
   *  Ul = (Uin / (Rh + Rl)) * Rl  ->  Uin = (Ul * (Rh + Rl)) / Rl
   *  Uin = (Ul * (10k + 3k3)) / 3k3 = 4 * Ul  
   */

  /* get current voltage */
  U_Bat = ReadU(TP_BAT);                /* read voltage (mV) */
  U_Bat *= 4;                           /* calculate U_bat (mV) */
  U_Bat += BAT_OFFSET;                  /* add offset for voltage drop */

  /* display battery voltage */
  lcd_fixed_string(Battery_str);        /* display: Bat. */
  lcd_space();
  DisplayValue(U_Bat / 10, -2, 'V');    /* display battery voltage */
  lcd_space();

  /* check limits */
  if (U_Bat < BAT_POOR)                 /* low level reached */
  {
    lcd_fixed_string(Low_str);          /* display: low */
    MilliSleep(2000);                   /* let user read info */
    goto power_off;                     /* power off */
  }
  else if (U_Bat < BAT_POOR + 1000)     /* warning level reached */
  {
    lcd_fixed_string(Weak_str);         /* display: weak */
  }
  else                                  /* ok */
  {
    lcd_fixed_string(OK_str);           /* display: ok */
  }


  /*
   *  probing
   */

  /* display start of probing */
  lcd_line(2);                     /* move to line #2 */
  lcd_fixed_string(Running_str);   /* display: probing... */

  /* try to discharge any connected component */
  DischargeProbes();
  if (Check.Found == COMP_ERROR)   /* discharge failed */
  {
    goto result;                   /* skip all other checks */
  }

  /* enter main menu if requested by short-circuiting all probes */
  if (AllProbesShorted() == 3)
  {
    MainMenu();                    /* enter mainmenu */;
    goto end;                      /* new cycle after job is is done */
  }

  /* check all 6 combinations of the 3 probes */ 
  CheckProbes(TP1, TP2, TP3);
  CheckProbes(TP2, TP1, TP3);
  CheckProbes(TP1, TP3, TP2);
  CheckProbes(TP3, TP1, TP2);
  CheckProbes(TP2, TP3, TP1);
  CheckProbes(TP3, TP2, TP1);

  /* if component might be a capacitor */
  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    /* tell user to be patient with large caps :-) */
    lcd_clear_line(2);
    lcd_fixed_string(Running_str);
    lcd_space();
    lcd_data('C');    

    /* check all possible combinations */
    MeasureCap(TP3, TP1, 0);
    MeasureCap(TP3, TP2, 1);
    MeasureCap(TP2, TP1, 2);
  }


  /*
   *  output test results
   */

result:

  lcd_clear();                     /* clear LCD */

  /* call output function based on component type */
  switch (Check.Found)
  {
    case COMP_ERROR:
      Show_Error();
      goto end;
      break;

    case COMP_DIODE:
      Show_Diode();
      break;

    case COMP_BJT:
      Show_BJT();
      break;

    case COMP_FET:
      Show_FET();
      break;

    case COMP_IGBT:
      Show_IGBT();
      break;

    case COMP_THYRISTOR:
      Show_Special();
      break;

    case COMP_TRIAC:
      Show_Special();
      break;

    case COMP_RESISTOR:
      Show_Resistor();
      break;

    case COMP_CAPACITOR:
      Show_Capacitor();
      break;

    default:                  /* no component found */
      Show_Fail();
      goto end;
  }

  /* component was found */
  RunsMissed = 0;             /* reset counter */
  RunsPassed++;               /* increase counter */


  /*
   *  take care about cycling and power-off
   */

end:

  #ifdef HW_RELAY
  ADC_DDR = (1<<TP_REF);              /* short circuit probes */
  #endif

  /* get key press or timeout */
  Test = TestKey((unsigned int)CYCLE_DELAY, 12);

  if (Test == 1)              /* short key press */
  {
    /* a second key press triggers extra functions */
    MilliSleep(50);
    Test = TestKey(300, 0);

    if (Test > 0)           /* short or long key press */
    {
      #ifdef HW_RELAY
      ADC_DDR = 0;               /* remove short circuit */
      #endif

      MainMenu();                /* enter main menu */
      goto end;                  /* re-run cycle control */
    }

    goto start;                    /* -> next round */
  }
  else if (Test == 2)         /* long key press */
  {
    goto power_off;                /* -> power off */
  }

  /* check if we should go for another round (continious mode only) */
  if ((RunsMissed < CYCLE_MAX) && (RunsPassed < CYCLE_MAX * 2))
  {
    goto start;               /* another round */
  }


power_off:

  /* display feedback (otherwise the user will wait :-) */
  lcd_clear();

  wdt_disable();                        /* disable watchdog */
  CONTROL_PORT &= ~(1 << POWER_CTRL);   /* power off myself */

  return 0;
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef MAIN_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

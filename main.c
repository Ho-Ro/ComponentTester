/* ************************************************************************
 *
 *   main part
 *
 *   (c) 2012-2017 by Markus Reschke
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
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */


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
 *  show pinout for semiconductors
 *
 *  required:
 *  - character for pin A
 *  - character for pin B
 *  - character for pin C
 */

void Show_SemiPinout(uint8_t A, uint8_t B, uint8_t C)
{
  uint8_t           i, j;     /* counter */
  unsigned char     Pin[3];   /* component pins */
  unsigned char     ID[3];    /* component pin IDs */
  #ifdef SW_PROBE_COLORS
  uint16_t          Color;         /* color value */

  Color = UI.PenColor;             /* save current color */
  #endif

  /* copy probe pin numbers */
  Pin[0] = Semi.A;
  Pin[1] = Semi.B;
  Pin[2] = Semi.C;

  /* copy pin characters/IDs */
  ID[0] = A;
  ID[1] = B;
  ID[2] = C;

  /* display: 123 */
  for (i = 0; i <= 2; i++)
  {
    LCD_ProbeNumber(i);
  }

  /* display: = */
  LCD_Char('=');

  /* display pin IDs */
  for (i = 0; i <= 2; i++)         /* loop through probe pins */
  {
    #ifdef SW_PROBE_COLORS
    UI.PenColor = ProbeColors[i];  /* set probe color */
    #endif

    for (j = 0; j <= 2; j++)       /* loop through component pins */
    {
      if (i == Pin[j])             /* probe pin matches */
      {
        LCD_Char(ID[j]);           /* show ID */
      }
    }
  }

  #ifdef SW_PROBE_COLORS
  UI.PenColor = Color;             /* restore old color */
  #endif
}



/*
 *  show simple pinout
 *
 *  required:
 *  - ID: characters for probes 1, 2 and 3
 *    0 -> not displayed
 */

void Show_SimplePinout(uint8_t ID_1, uint8_t ID_2, uint8_t ID_3)
{
  uint8_t           n;        /* counter */
  unsigned char     ID[3];    /* component pin IDs */
  #ifdef SW_PROBE_COLORS
  uint16_t          Color;    /* color value */

  Color = UI.PenColor;             /* save current color */
  #endif

  /* copy probe pin characters/IDs */
  ID[0] = ID_1;
  ID[1] = ID_2;
  ID[2] = ID_3;

  for (n = 0; n <= 2; n++)         /* loop through probe pins */
  {
    if (ID[n] != 0)                /* display this one */
    {
      LCD_ProbeNumber(n);
      LCD_Char(':');

      #ifdef SW_PROBE_COLORS
      UI.PenColor = ProbeColors[n];     /* set probe color */
      #endif

      LCD_Char(ID[n]);

      #ifdef SW_PROBE_COLORS
      UI.PenColor = Color;              /* restore old color */
      #endif

      LCD_Space();
    }
  }
}



/*
 *  show failed test
 */

void Show_Fail(void)
{
  /* display info */
  LCD_EEString(Failed1_str);            /* display: No component */
  LCD_NextLine_EEString(Failed2_str);   /* display: found!*/  

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
    LCD_EEString(DischargeFailed_str);  /* display: Battery? */

    /* display probe number and remaining voltage */
    LCD_NextLine();
    LCD_ProbeNumber(Check.Probe);
    LCD_Char(':');
    LCD_Space();
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
  LCD_Char(ID1);
  LCD_EEString(Resistor_str);
  LCD_Char(ID2); 

  /* show resistance value */
  LCD_Space();
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
  if (R1->A != Pin) LCD_ProbeNumber(R1->A);
  else LCD_ProbeNumber(R1->B);
  LCD_EEString(Resistor_str);
  LCD_ProbeNumber(Pin);

  if (R2)           /* second resistor */
  {
    LCD_EEString(Resistor_str);
    if (R2->A != Pin) LCD_ProbeNumber(R2->A);
    else LCD_ProbeNumber(R2->B);
  }


  /*
   *  display the values
   */

  /* first resistor */
  LCD_NextLine();
  DisplayValue(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

  if (R2)                /* second resistor */
  {
    LCD_Space();
    DisplayValue(R2->Value, R2->Scale, LCD_CHAR_OMEGA);
  }
  #ifdef SW_INDUCTOR
  else                   /* single resistor */
  {
    /* get inductance and display if relevant */
    if (MeasureInductor(R1) == 1)
    {
      LCD_Space();
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
  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  uint16_t          ESR;           /* ESR (in 0.01 Ohms) */
  #endif
  uint8_t           Counter;       /* loop counter */

  /* find largest cap */
  MaxCap = &Caps[0];               /* pointer to first cap */
  Cap = MaxCap;

  for (Counter = 1; Counter <= 2; Counter++) 
  {
    Cap++;                         /* next cap */

    if (CmpValue(Cap->Value, Cap->Scale, MaxCap->Value, MaxCap->Scale) == 1)
    {
      MaxCap = Cap;
    }
  }

  /* display pinout */
  LCD_ProbeNumber(MaxCap->A);      /* display pin #1 */
  LCD_EEString(Cap_str);           /* display capacitor symbol */
  LCD_ProbeNumber(MaxCap->B);      /* display pin #2 */

  /* show capacitance */
  LCD_NextLine();                  /* move to next line */
  DisplayValue(MaxCap->Value, MaxCap->Scale, 'F');

  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  /* show ESR */
  ESR = MeasureESR(MaxCap);        /* measure ESR */
  if (ESR < UINT16_MAX)            /* if successfull */
  {
    LCD_Space();
    DisplayValue(ESR, -2, LCD_CHAR_OMEGA);   /* display ESR */
  }
  #endif

  /* show discharge leakage current */
  if (MaxCap->I_leak > 0)
  {
    LCD_NextLine_EEString_Space(I_leak_str);
    DisplayValue(MaxCap->I_leak, -8, 'A');   /* in 10nA */
  }
}



/*
 *  show current (leakage or whatever)
 */

void Show_Current(const unsigned char *String)
{
  if (CmpValue(Semi.I_value, Semi.I_scale, 50, -9) >= 0)  /* show if >=50nA */
  {
    LCD_NextLine_EEString_Space(String);            /* display: <string> */
    DisplayValue(Semi.I_value, Semi.I_scale, 'A');   /* display current */
  }
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
  uint8_t           R_Pin2 = 5;    /* B_E resistor's pin #2 */
  uint8_t           n;
  uint8_t           m;

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
    /*
     *  Two diodes in series are detected as a virtual third diode:
     *  - Check for any possible way the 2 diodes could be connected in series.
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
  else                             /* too much diodes */
  {
    LCD_EEString(Diode_AC_str);         /* display: -|>|- */
    LCD_Space();                        /* display space */
    LCD_Char(Check.Diodes + '0');       /* display number of diodes found */
    return;
  }


  /*
   *  display pins 
   */

  /* first Diode */
  if (A < 3) LCD_ProbeNumber(D1->C);       /* common anode */
  else LCD_ProbeNumber(D1->A);             /* common cathode */

  if (A < 3) LCD_EEString(Diode_CA_str);   /* common anode */
  else LCD_EEString(Diode_AC_str);         /* common cathode */

  if (A < 3) LCD_ProbeNumber(A);           /* common anode */
  else LCD_ProbeNumber(C);                 /* common cathode */

  if (D2)           /* second diode */
  {
    if (A <= 3) LCD_EEString(Diode_AC_str);  /* common anode or in series */
    else LCD_EEString(Diode_CA_str);         /* common cathode */

    if (A == C) n = D2->A;              /* anti parallel */
    else if (A <= 3) n = D2->C;         /* common anode or in series */
    else n = D2->A;                     /* common cathode */
    LCD_ProbeNumber(n);                 /* display pin */
  }

  /* check for B-E resistor for possible BJT */
  if (R_Pin1 < 5)                  /* possible BJT */
  {
    /* B-E resistor below 25kOhms */
    if (CheckSingleResistor(R_Pin1, R_Pin2, 25) == 1)
    {
      /* show: PNP/NPN? */
      LCD_Space();
      if (A < 3) LCD_EEString(PNP_str);
      else LCD_EEString(NPN_str);
      LCD_Char('?');

      LCD_NextLine();                     /* go to line #2 */
      R_Pin1 += '1';                      /* convert to character */
      R_Pin2 += '1';
      Show_SingleResistor(R_Pin1, R_Pin2);   /* show resistor */
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
  LCD_NextLine_EEString_Space(Vf_str);  /* display: Vf */

  /* first diode */
  DisplayValue(D1->V_f, -3, 'V');

  LCD_Space();

  /* display low current Uf and reverse leakage current for a single diode */
  if (D2 == NULL)                       /* single diode */
  {
    /* display low current Uf if it's quite low (Ge/Schottky diode) */
    if (D1->V_f2 < 250)            /* < 250mV */
    {
      LCD_Char('(');
      DisplayValue(D1->V_f2, 0, 0);
      LCD_Char(')');
    }

    /* reverse leakage current */
    UpdateProbes(D1->C, D1->A, 0);      /* reverse diode */
    GetLeakageCurrent(1);               /* get current */
    Show_Current(I_R_str);              /* display I_R */
  }
  else                                  /* two diodes */
  {
    /* show Uf of second diode */
    DisplayValue(D2->V_f, -3, 'V');
  }

  /* display capacitance */
  if (CapFlag == 1)                     /* if feasable */ 
  {
    LCD_NextLine_EEString_Space(DiodeCap_str);  /* display: C */
    Show_Diode_Cap(D1);                 /* first diode */
    LCD_Space();
    Show_Diode_Cap(D2);                 /* second diode (optional) */
  }
}



/*
 *  show BJT
 */

void Show_BJT(void)
{
  Diode_Type        *Diode;        /* pointer to diode */
  unsigned char     *String;       /* string pointer (EEPROM) */
  uint8_t           Char;          /* character */
  uint8_t           BE_A;          /* V_BE: pin acting as anode */
  uint8_t           BE_C;          /* V_BE: pin acting as cathode */
  uint8_t           CE_A;          /* flyback diode: pin acting as anode */
  uint8_t           CE_C;          /* flyback diode: pin acting as cathode */
  uint16_t          V_BE;          /* V_BE */
  int16_t           Slope;         /* slope of forward voltage */

  /*
   *  Mapping for Semi structure:
   *  A   - Base pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_1 - U_BE (mV) (not yet)
   *  F_1 - hFE
   *  I_value/I_scale - I_CEO
   */

  /* preset stuff based on BJT type */
  if (Check.Type & TYPE_NPN)       /* NPN */
  {
    String = (unsigned char *)NPN_str;       /* "NPN" */

    /* direction of B-E diode: B -> E */
    BE_A = Semi.A;       /* anode at base */
    BE_C = Semi.C;       /* cathode at emitter */

    /* direction of optional flyback diode */
    CE_A = Semi.C;       /* anode at emitter */
    CE_C = Semi.B;       /* cathode at collector */
    Char = LCD_CHAR_DIODE_CA;      /* |<| */
  }
  else                             /* PNP */
  {
    String = (unsigned char *)PNP_str;       /* "PNP" */

    /* direction of B-E diode: E -> B */
    BE_A = Semi.C;       /* anode at emitter */
    BE_C = Semi.A;       /* cathode at base */

    /* direction of optional flyback diode */
    CE_A = Semi.B;       /* anode at collector */
    CE_C = Semi.C;       /* cathode at emitter */
    Char = LCD_CHAR_DIODE_AC;      /* |>| */
  }

  /* display type */
  LCD_EEString_Space(BJT_str);     /* display: BJT */
  LCD_EEString(String);            /* display: NPN / PNP */

  /* parasitic BJT (freewheeling diode on same substrate) */
  if (Check.Type & TYPE_PARASITIC)
  {
    LCD_Char('+');
  }

  LCD_NextLine();                  /* next line (#2) */

  /* display pinout */
  Show_SemiPinout('B', 'C', 'E');

  /* optional freewheeling diode */
  Diode = SearchDiode(CE_A, CE_C);    /* search for matching diode */
  if (Diode != NULL)                  /* got it */
  {
    LCD_Space();              /* display space */
    LCD_Char('C');            /* collector */
    LCD_Char(Char);           /* display diode symbol */
    LCD_Char('E');            /* emitter */
  }

  /*
   *  display either optional B-E resistor or hFE & V_BE
   */

  /* check for B-E resistor below 25kOhms */
  if (CheckSingleResistor(BE_C, BE_A, 25) == 1)   /* found B-E resistor */
  {
    LCD_NextLine();                /* next line (#3) */
    Show_SingleResistor('B', 'E');
    /* B-E resistor renders hFE and V_BE measurements useless */

    #ifdef SW_SYMBOLS
    LCD_FancySemiPinout();           /* display fancy pinout */
    #endif
  }
  else                                            /* no B-E resistor found */
  {
    #ifdef SW_SYMBOLS
    LCD_FancySemiPinout();           /* display fancy pinout */
    #endif

    /* hFE and V_BE */

    /* display hFE */
    LCD_NextLine_EEString_Space(h_FE_str);   /* display: hFE */
    DisplayValue(Semi.F_1, 0, 0);

    /* display V_BE (taken from diode forward voltage) */
    Diode = SearchDiode(BE_A, BE_C);    /* search for matching B-E diode */
    if (Diode != NULL)                  /* got it */
    {
      LCD_NextLine_EEString_Space(V_BE_str);   /* display: Vbe */

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
      if (Semi.F_1 < 100)               /* low hFE */
      {
        /*
         *  BJTs with low hFE are power transistors and need a large I_b
         *  to drive the load. So we simply take Vf of the high test current
         *  measurement (7mA). 
         */

        V_BE = Diode->V_f;
      }
      else if (Semi.F_1 < 250)          /* mid-range hFE */
      {
        /*
         *  BJTs with a mid-range hFE are signal transistors and need
         *  a small I_b to drive the load. So we interpolate Vf for
         *  a virtual test current of about 1mA.
         */

        V_BE = Diode->V_f - Slope;
      }
      else                              /* high hFE */
      {
        /*
         *  BJTs with a high hFE are small signal transistors and need
         *  only a very small I_b to drive the load. So we interpolate Vf
         *  for a virtual test current of about 0.1mA.
         */

        V_BE = Diode->V_f2 + Slope;
      }

      DisplayValue(V_BE, -3, 'V');
    }
  }

  /* I_CEO: collector emitter open current (leakage) */
  Show_Current(I_CEO_str);              /* display Iceo */
}



/*
 *  show MOSFET/IGBT extras
 *  - diode
 *  - V_th
 *  - Cgs
 */

void Show_FET_Extras(void)
{
  Diode_Type        *Diode;        /* pointer to diode */  
  uint8_t           Anode;         /* anode of diode */
  uint8_t           Cathode;       /* cathode of diode */
  uint8_t           Char_1;        /* pin name */
  uint8_t           Char_2;        /* pin name */
  uint8_t           Symbol;        /* diode symbol */

  /*
   *  Mapping for Semi structure:
   *  A   - Gate pin
   *  B   - Drain pin
   *  C   - Source pin
   *  U_1 - R_DS_on (0.01 Ohms)
   *  U_2 - V_th (mV)
   */

  /*
   *  show instrinsic/freewheeling diode
   */

  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel/NPN */
  {
    Anode = Semi.C;                /* source/emitter */
    Cathode = Semi.B;              /* drain/collector */
    Symbol = LCD_CHAR_DIODE_CA;    /* |<| */
  }
  else                                  /* p-channel/PNP */
  {
    Anode = Semi.B;                /* drain/collector */
    Cathode = Semi.C;              /* source/emitter */
    Symbol = LCD_CHAR_DIODE_AC;    /* |>| */
  }

  if (Check.Found == COMP_FET)     /* FET */
  {
    Char_1 = 'D';
    Char_2 = 'S';
  }
  else                             /* IGBT */
  {
    Char_1 = 'C';
    Char_2 = 'E';
  }

  /* search for matching diode */
  Diode = SearchDiode(Anode, Cathode);
  if (Diode != NULL)          /* got it */
  {
    /* show diode */
    LCD_Space();              /* space */
    LCD_Char(Char_1);         /* left pin name */
    LCD_Char(Symbol);         /* diode symbol */
    LCD_Char(Char_2);         /* right pin name */
  }

  /* skip remaining stuff for depletion-mode FETs/IGBTs */
  if (Check.Type & TYPE_DEPLETION) return;

  /* gate threshold voltage */
  if (Semi.U_2 != 0)
  {
    LCD_NextLine_EEString_Space(Vth_str);    /* display: Vth */
    DisplaySignedValue(Semi.U_2, -3, 'V');   /* display V_th in mV */
  }

  /* display gate-source capacitance */
  LCD_NextLine_EEString_Space(GateCap_str);  /* display: Cgs */
  DisplayValue(Semi.C_value, Semi.C_scale, 'F');  /* display value and unit */

  /* display R_DS_on, if available */
  if (Semi.U_1 > 0)
  {
    LCD_NextLine_EEString_Space(R_DS_str);        /* display: Rds */
    DisplayValue(Semi.U_1, -2, LCD_CHAR_OMEGA);   /* display value */
  }

  /* display Vf of diode, if available */
  if (Diode != NULL)
  {
    LCD_NextLine_EEString_Space(Vf_str);     /* display: Vf */
    DisplayValue(Diode->V_f, -3, 'V');       /* display value */
  }
}



/*
 *  show FET/IGBT channel type
 */

void Show_FET_Channel(void)
{
  LCD_Space();                     /* display space */

  /* channel type */
  if (Check.Type & TYPE_N_CHANNEL)   /* n-channel */
  {
    LCD_Char('N');
  }
  else                               /* p-channel */
  {
    LCD_Char('P');
  }

  LCD_EEString(Channel_str);         /* display: -ch */
}



/*
 *  show FET/IGBT mode
 */

void Show_FET_Mode(void)
{
  LCD_Space();

  if (Check.Type & TYPE_ENHANCEMENT)    /* enhancement mode */
  {
    LCD_EEString(Enhancement_str);
  }
  else                                  /* depletion mode */
  {
    LCD_EEString(Depletion_str);
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
   *  U_1 - R_DS_on (0.01 Ohms)
   *  U_2 - V_th (mV)
   */

  /* display type */
  if (Check.Type & TYPE_MOSFET)    /* MOSFET */
  {
    LCD_EEString(MOS_str);           /* display: MOS */
  }
  else                             /* JFET */
  {
    LCD_Char('J');                   /* display: J */
  }
  LCD_EEString(FET_str);           /* display: FET */

  /* display channel type */
  Show_FET_Channel();
      
  /* display mode for MOSFETs*/
  if (Check.Type & TYPE_MOSFET) Show_FET_Mode();

  /* pinout */
  LCD_NextLine();                       /* next line (#2) */

  if (Check.Type & TYPE_SYMMETRICAL)    /* symmetrical Drain and Source */
  {
    /* we can't distinguish D and S */
    Show_SemiPinout('G', 'x', 'x');     /* show pinout */
  }
  else                                  /* unsymmetrical Drain and Source */
  {
    Show_SemiPinout('G', 'D', 'S');     /* show pinout */
  }

  #ifdef SW_SYMBOLS
  LCD_FancySemiPinout();           /* display fancy pinout */
  #endif

  /* show diode, V_th and Cgs for MOSFETs */
  if (Check.Type & TYPE_MOSFET) Show_FET_Extras();

  /* show I_DSS for depletion mode FET */
  if (Check.Type & TYPE_DEPLETION)
  {
    Show_Current(I_DSS_str);              /* display Idss */
  }
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

  LCD_EEString(IGBT_str);          /* display: IGBT */
  Show_FET_Channel();              /* display channel type */
  Show_FET_Mode();                 /* display mode */

  LCD_NextLine();                  /* next line (#2) */
  Show_SemiPinout('G', 'C', 'E');  /* show pinout */
  #ifdef SW_SYMBOLS
  LCD_FancySemiPinout();           /* display fancy pinout */
  #endif
  Show_FET_Extras();               /* show diode, V_th and C_GE */
}



/*
 *  show Thyristor and Triac
 */

void Show_ThyristorTriac(void)
{
  /*
   *  Mapping for Semi structure:
   *        SCR        Triac
   *  A   - Gate       Gate
   *  B   - Anode      MT2
   *  C   - Cathode    MT1
   *  U_1 - V_GT (mV)
   */

  /* display component type any pinout */
  if (Check.Found == COMP_THYRISTOR)    /* SCR */
  {
    LCD_EEString(Thyristor_str);        /* display: thyristor */
    LCD_NextLine();                     /* next line (#2) */
    Show_SemiPinout('G', 'A', 'C');     /* display pinout */
  }
  else                                  /* Triac */
  {
    LCD_EEString(Triac_str);            /* display: triac */
    LCD_NextLine();                     /* next line (#2) */
    Show_SemiPinout('G', '2', '1');     /* display pinout */
  }

  #ifdef SW_SYMBOLS
  LCD_FancySemiPinout();           /* display fancy pinout */
  #endif

  /* show V_GT (gate trigger voltage) */
  if (Semi.U_1 > 0)                /* show if not zero */
  {
    LCD_NextLine_EEString_Space(V_GT_str);   /* display: V_GT */
    DisplayValue(Semi.U_1, -3, 'V');         /* display V_GT in mV */
  }
}



/*
 *  show PUT
 */

void Show_PUT(void)
{
  /*
   *  Mapping for AltSemi structure:
   *  A   - Gate
   *  B   - Anode
   *  C   - Cathode
   *  U_1 - V_f
   *  U_2 - V_T
   */

  LCD_EEString(PUT_str);              /* display: PUT */
  LCD_NextLine();                     /* move to line #2 */
  Show_SemiPinout('G', 'A', 'C');     /* display pinout */

  #ifdef SW_SYMBOLS
  LCD_FancySemiPinout();           /* display fancy pinout */
  #endif

  /* display V_T */
  LCD_NextLine_EEString_Space(V_T_str); /* display: VT */
  DisplayValue(AltSemi.U_2, -3, 'V');   /* display: V_T */

  /* display Uf */
  LCD_NextLine_EEString_Space(Vf_str);  /* display: Vf */
  DisplayValue(AltSemi.U_1, -3, 'V');   /* display Vf */
}



#ifdef SW_UJT

/*
 *  show UJT
 */

void Show_UJT(void)
{
  /*
   *  Mapping for AltSemi structure:
   *  A   - Emitter
   *  B   - B2
   *  C   - B1
   */

  LCD_EEString(UJT_str);              /* display: UJT */
  LCD_NextLine();                     /* next line (#2) */
  Show_SemiPinout('E', '2', '1');     /* display pinout */

  #ifdef SW_SYMBOLS
  LCD_FancySemiPinout();           /* display fancy pinout */
  #endif

  /* display R_BB */
  LCD_NextLine_EEString_Space(R_BB_str);  /* display: R_BB */  
  DisplayValue(Resistors[0].Value, Resistors[0].Scale, LCD_CHAR_OMEGA);
}

#endif



/* ************************************************************************
 *   the one and only main()
 * ************************************************************************ */


/*
 *  main function
 */

int main(void)
{
  uint16_t          U_Bat;         /* voltage of power supply */
  uint8_t           Test;          /* test value */
  uint32_t          Temp;          /* some value */


  /*
   *  init
   */

  /* switch on power to keep me alive */
  CONTROL_DDR = (1 << POWER_CTRL);      /* set pin as output */
  CONTROL_PORT = (1 << POWER_CTRL);     /* set pin to drive power management transistor */

  /* set up MCU */
  MCUCR = (1 << PUD);                        /* disable pull-up resistors globally */
  ADCSRA = (1 << ADEN) | ADC_CLOCK_DIV;      /* enable ADC and set clock divider */

  #ifdef HW_DISCHARGE_RELAY
  /* init discharge relay (safe mode) */
                                        /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);              /* short circuit probes */
  #endif

  /* catch watchdog */  
  Test = (MCUSR & (1 << WDRF));         /* save watchdog flag */
  MCUSR &= ~(1 << WDRF);                /* reset watchdog flag */
  wdt_disable();                        /* disable watchdog */

  Cfg.BusState = BUS_NONE;              /* no interface bus yet */

  #ifdef HW_I2C
  I2C_Setup();                          /* set up I2C bus */
  #endif

  /* LCD module */
  LCD_BusSetup();                       /* set up LCD bus */
  #ifdef HW_TOUCH
  Touch_BusSetup();                     /* set up touch screen */
  #endif


  /*
   *  watchdog was triggered (timeout 2s)
   *  - This is after the MCU done a reset driven by the watchdog.
   *  - Does only work if the capacitor at the base of the power management
   *    transistor is large enough to survive a MCU reset. Otherwise the
   *    tester simply loses power.
   */

  if (Test)
  {
    LCD_Clear();                        /* display was initialized before */
    LCD_EEString(Timeout_str);          /* display: timeout */
    LCD_NextLine_EEString(Error_str);   /* display: error */
    MilliSleep(2000);                   /* give user some time to read */
    CONTROL_PORT = 0;                   /* power off myself */
    return 0;                           /* exit program */
  }


  /*
   *  operation mode selection
   */

  Cfg.SleepMode = SLEEP_MODE_PWR_SAVE;  /* default: power save */
  UI.OP_Mode = OP_CONTINOUS;            /* set default mode: continous */
  Test = 0;                             /* key press */

  /* catch long key press */
  if (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* test button pressed */
  {
    RunsMissed = 0;

    while (Test == 0)
    {
      MilliSleep(20);
      if (!(CONTROL_PIN & (1 << TEST_BUTTON)))    /* button still pressed */
      {
        RunsMissed++;
        if (RunsMissed > 100) Test = 3;      /* >2000ms */
      }
      else                                        /* button released */
      {
        Test = 1;                            /* <300ms */
        if (RunsMissed > 15) Test = 2;       /* >300ms */
      }
    }
  }

  /* key press >300ms sets autohold mode */
  if (Test > 1) UI.OP_Mode = OP_AUTOHOLD;

  /* init LCD module */
  LCD_Init();                           /* initialize LCD */
  LCD_NextLine_Mode(LINE_STD);          /* reset line mode */
  #ifdef LCD_COLOR
    UI.PenColor = COLOR_TITLE;          /* set pen color */
  #endif
  #ifdef HW_TOUCH
  Touch_Init();                         /* init touch screen */
  #endif


  /*
   *  load saved adjustment offsets and values
   */

  if (Test == 3)              /* key press >2s resets to defaults */
  {
    SetAdjustmentDefaults();       /* set default values */
  }
  else                        /* normal mode */
  {
    /* load adjustment values: profile #1 */
    ManageAdjustmentStorage(STORAGE_LOAD, 1);
  }

  /* set extra stuff */
  #ifdef SW_CONTRAST
    LCD_Contrast(NV.Contrast);          /* set LCD contrast */
  #endif


  /*
   *  welcome user
   */

  LCD_EEString(Tester_str);             /* display: Component Tester */
  LCD_NextLine_EEString(Version_str);   /* display firmware version */
  #ifdef LCD_COLOR
    UI.PenColor = COLOR_PEN;            /* set pen color */
  #endif
  MilliSleep(1500);                     /* let the user read the display */


  /*
   *  init variables
   */

  /* cycling */
  RunsMissed = 0;
  RunsPassed = 0;

  /* default offsets and values */
  Cfg.Samples = ADC_SAMPLES;            /* number of ADC samples */
  Cfg.AutoScale = 1;                    /* enable ADC auto scaling */
  Cfg.RefFlag = 1;                      /* no ADC reference set yet */
  Cfg.Vcc = UREF_VCC;                   /* voltage of Vcc */
  wdt_enable(WDTO_2S);		        /* enable watchdog (timeout 2s) */

  #ifdef HW_TOUCH
  /* adjust touch screen if not done yet */
  if ((Touch.X_Left == 0) && (Touch.X_Right == 0))
  {
    Test = Touch_Adjust();         /* adjust touch screen */

    if (Test == 0)                 /* error */
    {
      LCD_ClearLine2();
      LCD_EEString(Error_str);          /* display: Error */
      MilliSleep(1000);                 /* smooth UI */
      TestKey(2500, CURSOR_BLINK | CURSOR_OP_MODE);
    }
  }
  #endif


  /*
   *  main processing cycle
   */

start:

  /* reset variabels */
  Check.Found = COMP_NONE;
  Check.Type = 0;
  Check.Done = DONE_NONE;
  Check.AltFound = COMP_NONE;
  Check.Diodes = 0;
  Check.Resistors = 0;
  Semi.U_1 = 0;
  Semi.U_2 = 0;
  Semi.F_1 = 0;
  Semi.I_value = 0;
  AltSemi.U_1 = 0;
  AltSemi.U_2 = 0;
  #ifdef HW_KEYS
  UI.KeyOld = KEY_NONE;
  UI.KeyStepOld = 1;
  #endif

  /* reset hardware */
  ADC_DDR = 0;                     /* set all pins of ADC port as input */
  #ifdef HW_DISCHARGE_RELAY
     /* this also switches the discharge relay to remove the short circuit */
  #endif

  LCD_NextLine_Mode(LINE_KEEP);    /* line mode: keep first line */
  LCD_Clear();                     /* clear LCD */


  /*
   *  voltage reference
   */

  #ifdef HW_REF25
  /* external 2.5V reference */
  Cfg.Samples = 200;               /* do a lot of samples for high accuracy */
  U_Bat = ReadU(TP_REF);           /* read voltage of reference (mV) */
  Cfg.Samples = ADC_SAMPLES;       /* set samples back to default */

  if ((U_Bat > 2250) && (U_Bat < 2750))   /* check for valid reference */
  {
    uint32_t        Temp;

    /* adjust Vcc (assuming 2.495V typically) */
    Temp = ((uint32_t)Cfg.Vcc * UREF_25) / U_Bat;
    Cfg.Vcc = (uint16_t)Temp;
  }
  #endif

  /* internal bandgap reference */
  Cfg.Bandgap = ReadU(ADC_BANDGAP);     /* dummy read for bandgap stabilization */
  Cfg.Samples = 200;                    /* do a lot of samples for high accuracy */
  Cfg.Bandgap = ReadU(ADC_BANDGAP);     /* get voltage of bandgap reference (mV) */
  Cfg.Samples = ADC_SAMPLES;            /* set samples back to default */
  Cfg.Bandgap += NV.RefOffset;          /* add voltage offset */ 


  /*
   *  battery check
   */

  /*
   *  ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
   *  U2 = (Uin / (R1 + R2)) * R2 
   *  Uin = (U2 * (R1 + R2)) / R2
   */

  /* get current battery voltage */
  U_Bat = ReadU(TP_BAT);                /* read voltage U2 (mV) */
  Temp = (((uint32_t)(BAT_R1 + BAT_R2) * 1000) / BAT_R2);   /* factor (0.001) */
  Temp *= U_Bat;                        /* Uin (0.001 mV) */
  Temp /= 1000;                         /* Uin (mV) */
  U_Bat = (uint16_t)Temp;
  U_Bat += BAT_OFFSET;                  /* add offset for voltage drop */

  /* display battery voltage */
  LCD_EEString_Space(Battery_str);      /* display: Bat. */
  DisplayValue(U_Bat / 10, -2, 'V');    /* display battery voltage */
  LCD_Space();

  /* check limits */
  if (U_Bat < BAT_POOR)                 /* low level reached */
  {
    LCD_EEString(Low_str);              /* display: low */
    MilliSleep(2000);                   /* let user read info */
    goto power_off;                     /* power off */
  }
  else if (U_Bat < BAT_POOR + 1000)     /* warning level reached */
  {
    LCD_EEString(Weak_str);             /* display: weak */
  }
  else                                  /* ok */
  {
    LCD_EEString(OK_str);               /* display: ok */
  }


  /*
   *  probing
   */

  /* display start of probing */
  LCD_NextLine_EEString(Running_str);   /* display: probing... */

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
  CheckProbes(PROBE_1, PROBE_2, PROBE_3);
  CheckProbes(PROBE_2, PROBE_1, PROBE_3);
  CheckProbes(PROBE_1, PROBE_3, PROBE_2);
  CheckProbes(PROBE_3, PROBE_1, PROBE_2);
  CheckProbes(PROBE_2, PROBE_3, PROBE_1);
  CheckProbes(PROBE_3, PROBE_2, PROBE_1);
  CheckAlternatives();             /* process alternatives */

  /* if component might be a capacitor */
  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    /* tell user to be patient with large caps :-) */
    LCD_Space();
    LCD_Char('C');    

    /* check all possible combinations */
    MeasureCap(PROBE_3, PROBE_1, 0);
    MeasureCap(PROBE_3, PROBE_2, 1);
    MeasureCap(PROBE_2, PROBE_1, 2);
  }


  /*
   *  output test results
   */

result:

  LCD_Clear();                     /* clear LCD */
  LCD_NextLine_Mode(LINE_KEEP | LINE_KEY);

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
      Show_ThyristorTriac();
      break;

    case COMP_TRIAC:
      Show_ThyristorTriac();
      break;

    case COMP_PUT:
      Show_PUT();
      break;

    #ifdef SW_UJT
    case COMP_UJT:
      Show_UJT();
      break;
    #endif

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

  #ifdef HW_DISCHARGE_RELAY
  ADC_DDR = (1 << TP_REF);            /* short circuit probes */
  #endif

  LCD_NextLine_Mode(LINE_STD);        /* reset next line mode */

  /* get key press or timeout */
  Test = TestKey((uint16_t)CYCLE_DELAY, CURSOR_BLINK | CURSOR_OP_MODE);

  if (Test == KEY_TIMEOUT)         /* timeout (no key press) */
  {
    /* check if we reached the maximum number of rounds (continious mode only) */
    if ((RunsMissed >= CYCLE_MAX) || (RunsPassed >= CYCLE_MAX * 2))
    {
      goto power_off;              /* -> power off */
    }
  }
  else if (Test == KEY_SHORT)      /* short key press */
  {
    /* a second key press triggers extra functions */
    MilliSleep(50);
    Test = TestKey(300, CURSOR_NONE);

    if (Test > KEY_TIMEOUT)        /* short or long key press */
    {
      #ifdef HW_DISCHARGE_RELAY
      ADC_DDR = 0;                 /* remove short circuit */
      #endif

      MainMenu();                  /* enter main menu */
      goto end;                    /* re-run cycle control */
    }
  }
  else if (Test == KEY_LONG)       /* long key press */
  {
    goto power_off;                /* -> power off */
  }
  #ifdef HW_KEYS
  else if (Test == KEY_LEFT)       /* rotary encoder: left turn */
  {
    MainMenu();                    /* enter main menu */
    goto end;                      /* re-run cycle control */
  }
  #endif

  /* default action (also for rotary encoder) */
  goto start;                 /* -> next round */


power_off:

  /* display feedback (otherwise the user will wait :-) */
  LCD_Clear();
  #ifdef LCD_COLOR
    UI.PenColor = COLOR_TITLE;          /* set pen color */
  #endif
  LCD_EEString(Bye_str);

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

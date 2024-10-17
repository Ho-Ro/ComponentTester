/* ************************************************************************
 *
 *   main part
 *
 *   (c) 2012-2024 by Markus Reschke
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

/* program control */
#if CYCLE_MAX < 255
uint8_t        MissedParts;          /* counter for failed/missed components */
#endif


/* ************************************************************************
 *   output components and errors
 * ************************************************************************ */


/*
 *  get pin designator for specific probe ID
 *
 *  requires:
 *  - probe ID [0-2]
 *
 *  returns:
 *  - pin designator
 */

uint8_t Get_SemiPinDesignator(uint8_t Probe)
{
  uint8_t           Char;          /* designator / return value */

  /* looking for matching probe ID */
  if (Probe == Semi.A)             /* matches pin A */
  {
    Char = Semi.DesA;              /* designator for pin A */
  }
  else if (Probe == Semi.B)        /* matches pin B */
  {
    Char = Semi.DesB;              /* designator for pin B */
  }
  else                             /* matches pin C */
  {
    Char = Semi.DesC;              /* designator for pin C */
  }

  return Char;
}



#if ! defined (UI_NO_TEXTPINOUT) || defined (SW_ENCODER)

/*
 *  show pinout for semiconductors
 *  - displays: 123=abc
 */

void Show_SemiPinout(void)
{
  uint8_t           n;             /* counter */

  /* display: 123 */
  for (n = 0; n <= 2; n++)         /* loop through probe pins */
  {
    Display_ProbeNumber(n);        /* display probe ID */
  }

  /* display: = */
  Display_Char('=');

  /* display pin designators */
  for (n = 0; n <= 2; n++)         /* loop through probe pins */
  {
    Display_SemiPinDesignator(n);  /* display pin designator */
  }
}

#endif



/*
 *  show simple pinout
 *  - displays: 1:a 2:b 3:c 
 *
 *  required:
 *  - DesX: designators for probes #1, #2 and #3
 *    0 -> not displayed
 */

void Show_SimplePinout(uint8_t Des1, uint8_t Des2, uint8_t Des3)
{
  uint8_t           n;        /* counter */
  unsigned char     Des[3];   /* component pin designators */
  #ifdef UI_PROBE_COLORS
  uint16_t          Color;    /* color value */

  Color = UI.PenColor;             /* save current color */
  #endif

  /* copy probe pin designators */
  Des[0] = Des1;
  Des[1] = Des2;
  Des[2] = Des3;

  for (n = 0; n <= 2; n++)         /* loop through probe pins */
  {
    if (Des[n] != 0)               /* display this one */
    {
      Display_ProbeNumber(n);
      Display_Colon();

      #ifdef UI_PROBE_COLORS
      UI.PenColor = ProbeColors[n];     /* set probe color */
      #endif

      Display_Char(Des[n]);

      #ifdef UI_PROBE_COLORS
      UI.PenColor = Color;              /* restore old color */
      #endif

      Display_Space();
    }
  }
}



#ifdef FUNC_EVALUE

/*
 *  show E series norm values
 *
 *  requires:
 *  - Value: unsigned value
 *  - Scale: exponent/multiplier (* 10^n)
 *  - ESeries: E6-192
 *  - Tolerance: in 0.1%
 *  - Unit: unit character
 */

void Show_ENormValues(uint32_t Value, int8_t Scale, uint8_t ESeries, uint8_t Tolerance, unsigned char Unit)
{
  uint8_t           n;             /* number of norm values (1 or 2) */
  uint8_t           Pos;           /* char x position */
  uint8_t           Temp;          /* temporary value */


  /*
   *  get E series norm values
   *  - 1st: Semi.I_value, Semi.I_scale
   *  - 2nd: Semi.C_value, Semi.C_scale
   */

  n = GetENormValue(Value, Scale, ESeries, Tolerance);


  /*
   *  show E series and tolerance
   */

  Display_NextLine();                   /* move to next line */

  /* display E series */
  Display_Char('E');                    /* display: E */
  Display_FullValue(ESeries, 0 , 0);    /* display E series */

  Display_Space();                      /* display: " " */

  /* display tolerance */
  Temp = Tolerance;                     /* copy tolerance */
  Pos = 0;                              /* reset decimal places */

  if (Temp < 10)                        /* < 1% */
  {
    Pos = 1;                            /* one decimal place */
  }
  Temp /= 10;                           /* scale to 1 */

  Display_FullValue(Temp, Pos, '%');    /* display tolerance */

  Display_Space();                      /* display: " " */


  /*
   *  show norm values
   */

  if (n)                      /* got norm value(s) */
  {
    Pos = UI.CharPos_X;       /* get current char position */

    /* display first norm value */
    Display_EValue(Semi.I_value, Semi.I_scale, Unit);

    if (n == 2)               /* got two norm values */
    {
      /* move to next line at same position (after E series) */
      Display_NextLine();
      LCD_CharPos(Pos, UI.CharPos_Y);

      /* display second norm value */
      Display_EValue(Semi.C_value, Semi.C_scale, Unit);
    }
  }
  else                        /* no norm value */
  {
    Display_Minus();          /* display: - */
  }
}

#endif



#ifdef FUNC_COLORCODE

/*
 *  show E series norm values as color-code
 *
 *  requires:
 *  - Value: unsigned value
 *  - Scale: exponent/multiplier (* 10^n)
 *  - ESeries: E6-192
 *  - Tolerance: in 0.1%
 *  - TolBand: color of tolerance band
 */

void Show_ENormCodes(uint32_t Value, int8_t Scale, uint8_t ESeries, uint8_t Tolerance, uint16_t TolBand)
{
  uint8_t           n;             /* number of norm values (1 or 2) */
  uint8_t           Pos;           /* char x position */

  /*
   *  tolerance band
   *  - resistor
   *    20%  10%    5%   2%  1%    0.5%  0.25% 0.1%   0.05%
   *    none silver gold red brown green blue  violet grey
   *  - inductor
   *    20%   10%    5%   4%     3%     2%  1%    0.5%  0.25% 0.1%   0.05%
   *    black silver gold yellow orange red brown green blue  violet grey
   *  - caps: many different schemas
   *
   *  multiplier reference
   *  - R:   0 (10^0)   -> 1 Ohms
   *    C: -12 (10^-12) -> 1 pF
   *    L:  -6 (10^-6)  -> 1 µH
   *  - ref_scale as function argument?
   *    scale = scale - ref_scale
   */

  /*
   *  get E series norm values
   *  - 1st: Semi.I_value, Semi.I_scale
   *  - 2nd: Semi.C_value, Semi.C_scale
   */

  n = GetENormValue(Value, Scale, ESeries, Tolerance);


  /*
   *  show E series
   */

  Display_NextLine();                   /* move to next line */

  /* display E series */
  Display_Char('E');                    /* display: E */
  Display_FullValue(ESeries, 0 , 0);    /* display E series */

  Display_Space();                      /* display: " " */


  /*
   *  show color-codes of norm values
   */

  if (n)                      /* got norm value(s) */
  {
    Pos = UI.CharPos_X;       /* get current char position */

    /* display color-code of first norm value */
    Display_ColorCode(Semi.I_value, Semi.I_scale, TolBand);

    if (n == 2)               /* got two norm values */
    {
      /* move to next line at same position (after E series) */
      Display_NextLine();
      LCD_CharPos(Pos, UI.CharPos_Y);

      /* display color-code of second norm value */
      Display_ColorCode(Semi.C_value, Semi.C_scale, TolBand);
    }
  }
  else                        /* no norm value */
  {
    Display_Minus();          /* display: - */
  }

  #if 0
  /*
   *  color testing
   */

  uint16_t Color;

  Display_NextLine();

  /* first 5 colors plus gold */
  n = 0;
  while (n < 5)
  {
    Color = DATA_read_word((uint16_t *)&ColorCode_table[n]);
    LCD_Band(Color, ALIGN_LEFT);
    n++;
  }
  LCD_Band(COLOR_CODE_GOLD, ALIGN_RIGHT);

  Display_NextLine();

  /* last 5 colors plus silver */
  while (n < 10)
  {
    Color = DATA_read_word((uint16_t *)&ColorCode_table[n]);
    LCD_Band(Color, ALIGN_LEFT);
    n++;
  }
  LCD_Band(COLOR_CODE_SILVER, ALIGN_RIGHT);

  TestKey(0, CURSOR_BLINK);
  #endif
}

#endif



#ifdef FUNC_EIA96

/*
 *  show E series norm values as EIA-96 code
 *  - implies E96 1%
 *
 *  requires:
 *  - Value: unsigned value
 *  - Scale: exponent/multiplier (* 10^n)
 */

void Show_ENormEIA96(uint32_t Value, int8_t Scale)
{
  uint8_t           n;             /* number of norm values (1 or 2) */

  /*
   *  get E series norm values
   *  - 1st: Semi.I_value, Semi.I_scale, Semi.A (index number)
   *  - 2nd: Semi.C_value, Semi.C_scale, Semi.B (index number)
   */

  n = GetENormValue(Value, Scale, E96, 10);

  /*
   *  show E series and tolerance
   */

  Display_NextLine();                   /* move to next line */

  /* display E series (E96) */
  Display_Char('E');                    /* display: E */
  Display_FullValue(96, 0, 0);          /* display E series */

  Display_Space();                      /* display: " " */

  /* display tolerance (1%) */
  Display_FullValue(1, 0, '%');         /* display tolerance */

  Display_Space();                      /* display: " " */


  /*
   *  show EIA-96 codes of norm values
   */

  if (n)                      /* got norm value(s) */
  {
    /* display EIA-96 code of first norm value */
    Display_EIA96(Semi.A, Semi.I_scale);

    if (n == 2)               /* got two norm values */
    {
      Display_Space();                  /* display: " " */

      /* display EIA-96 code of second norm value */
      Display_EIA96(Semi.B, Semi.C_scale);
    }
  }
  else                        /* no norm value */
  {
    Display_Minus();          /* display: - */
  }
}

#endif



/*
 *  show failed test
 *  - no component found
 */

void Show_Fail(void)
{
  /* display info */
  #ifdef UI_CENTER_ALIGN
    Display_CenterLine(2);                        /* center block: 2 lines */
    Display_EEString_Center(Failed1_str);         /* display: No component */
    Display_NL_EEString_Center(Failed2_str);      /* display: found! */
  #else
    Display_EEString(Failed1_str);      /* display: No component */
    Display_NL_EEString(Failed2_str);   /* display: found! */
  #endif

  #ifdef UI_QUESTION_MARK
  /* display question mark symbol */
  Check.Symbol = SYMBOL_QUESTIONMARK;   /* set symbol ID */
  Display_FancySemiPinout(3);           /* show symbol starting in line #3 */
  #endif

  #if CYCLE_MAX < 255
  MissedParts++;              /* increase counter */
  #endif
}



/*
 *  show error
 */

void Show_Error()
{
  if (Check.Type == TYPE_DISCHARGE)          /* discharge failed */
  {
    /* possibly a voltage source */
    Display_EEString(DischargeFailed_str);   /* display: Battery? */

    /* display probe number and remaining voltage */
    Display_NextLine();
    Display_ProbeNumber(Check.Probe);
    Display_Colon();
    Display_Space();
    Display_Value(Check.U, -3, 'V');
  }
  else if (Check.Type == TYPE_DETECTION)     /* detection error */
  {
    /* simply display: No component found! */
    Show_Fail();
  }
}



#ifdef UI_PROBE_COLORS

/*
 *  show single (first) resistor
 *
 *  requires:
 *  - Probe1: probe ID #1 [0-2]
 *  - Probe2: probe ID #2 [0-2]
 *  - Mode: 0 for probe ID
 *          >0 for pin designator
 */

void Show_SingleResistor(uint8_t Probe1, uint8_t Probe2, int8_t Mode)
{
  Resistor_Type     *Resistor;     /* pointer to resistor */

  Resistor = &Resistors[0];        /* pointer to first resistor */

  /*
   *  show pinout
   */

  if (Mode)
  {
    Display_SemiPinDesignator(Probe1);
  }
  else
  {
    Display_ProbeNumber(Probe1);
  }

  Display_EEString(Resistor_str);

  if (Mode)
  {
    Display_SemiPinDesignator(Probe2); 
  }
  else
  {
    Display_ProbeNumber(Probe2);
  }

  /* show resistance value */
  Display_Space();
  Display_Value(Resistor->Value, Resistor->Scale, LCD_CHAR_OMEGA);
}

#else

/*
 *  show single (first) resistor
 *
 *  requires:
 *  - ID1: pin ID #1 character
 *  - ID2: pin ID #2 character
 */

void Show_SingleResistor(uint8_t ID1, uint8_t ID2)
{
  Resistor_Type     *Resistor;     /* pointer to resistor */

  Resistor = &Resistors[0];        /* pointer to first resistor */

  /* show pinout */
  Display_Char(ID1);
  Display_EEString(Resistor_str);
  Display_Char(ID2); 

  /* show resistance value */
  Display_Space();
  Display_Value(Resistor->Value, Resistor->Scale, LCD_CHAR_OMEGA);
}

#endif



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
    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Quantity = 2;             /* got two */
    #endif

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
    if ((R1->A == R2->A) || (R1->A == R2->B))
    {
      Pin = R1->A;            /* pin A */
    }
    else
    {
      Pin = R1->B;            /* pin B */
    }
  }

  #ifdef UI_SERIAL_COMMANDS
  /* set data for remote commands */
  Info.Comp1 = (void *)R1;         /* link first resistor */
  Info.Comp2 = (void *)R2;         /* link second resistor */
  #endif


  /*
   *  display the resistor(s) and pins in line #1
   */

  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();         /* use title color */
  #endif

  /* first resistor */
  if (R1->A != Pin)           /* A is not common pin */
  {
    Display_ProbeNumber(R1->A);    /* display pin A */
  }
  else                        /* single resistor or A is common pin */
  {
    Display_ProbeNumber(R1->B);    /* display pin B */
  }

  Display_EEString(Resistor_str);
  Display_ProbeNumber(Pin);       /* display common pin */

  if (R2)           /* second resistor */
  {
    Display_EEString(Resistor_str);

    if (R2->A != Pin)         /* A is not common pin */
    {
      Display_ProbeNumber(R2->A);  /* display pin A */
    }
    else                      /* A is common pin */
    {
      Display_ProbeNumber(R2->B);  /* display pin B */
    }
  }

  #ifdef UI_COLORED_TITLES
  Display_UseOldColor();           /* use old color */
  #endif


  /*
   *  display the value(s) in line #2
   *  - optionally inductance
   *  - optionally E-series norm values
   */

  /* first resistor */
  Display_NextLine();
  Display_Value(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

  if (R2)                /* second resistor */
  {
    Display_Space();
    Display_Value(R2->Value, R2->Scale, LCD_CHAR_OMEGA);

    #ifdef SW_R_TRIMMER
    /* potentiometer/trimpot */
    uint32_t        R_Value;       /* value of R1 */
    uint32_t        Rt_Value;      /* value of R_t */
    int8_t          Scale;         /* scale of R1 and R_t */

    /* normalize R1 and R2 */
    Scale = NormalizeValue(R1->Value, R1->Scale, R2->Value, R2->Scale);
    R_Value = RescaleValue(R1->Value, R1->Scale, Scale);    /* normalized R1 */
    Rt_Value = RescaleValue(R2->Value, R2->Scale, Scale);   /* normalized R2 */

    Rt_Value += R_Value;                     /* R2 + R1 */
    if (Rt_Value > 0)                        /* sanity check */
    {
      /* show sum: R1 + R2 */
      Display_NL_EEString_Space(R_t_str);         /* display: R_t */
      Display_Value(Rt_Value, Scale, LCD_CHAR_OMEGA);  /* display sum */

      /* show R1 ratio (in %): R1 / (R1 + R2) */
      Display_NL_EEString_Space(R1_str);          /* display: R1 */
      R_Value *= 100;                             /* for % */
      R_Value /= Rt_Value;                        /* R1 / (R1 + R2) */
      Display_Value(R_Value, 0, '%');             /* display ratio in % */

      /* show R2 ratio (in %): R2 / (R1 + R2) or 100% - ratio_R1 */
      Display_Space();                            /* display space */
      Display_EEString_Space(R2_str);             /* display: R2 */
      R_Value = 100 - R_Value;                    /* 100% - ratio_R1 */
      Display_Value(R_Value, 0, '%');             /* display ratio in % */
    }
    #endif
  }
  #ifdef SW_INDUCTOR
  else                   /* single resistor */
  {
    /* get inductance and display if relevant */
    if (MeasureInductor(R1) == 1)       /* inductor */
    {
      Display_Space();
      Display_Value(Inductor.Value, Inductor.Scale, 'H');

      #ifdef UI_SERIAL_COMMANDS
      /* set data for remote commands */
      Info.Flags |= INFO_R_L;      /* inductance measured */
      #endif

      #ifdef SW_L_E6_T
      /* show E series norm values for E6 20% */
      Show_ENormValues(Inductor.Value, Inductor.Scale, E6, 200, 'H');
      #endif

      #ifdef SW_L_E12_T
      /* show E series norm values for E12 10% */
      Show_ENormValues(Inductor.Value, Inductor.Scale, E12, 100, 'H');
      #endif
    }
    #ifdef SW_R_EXX
    else                           /* no inductance */
    {
      #ifdef SW_R_E24_5_T
      /* show E series norm values for E24 5% */
      Show_ENormValues(R1->Value, R1->Scale, E24, 50, LCD_CHAR_OMEGA);
      #endif

      #ifdef SW_R_E24_5_CC
      /* show E series norm value color-codes for E24 5% */
      Show_ENormCodes(R1->Value, R1->Scale, E24, 50, COLOR_CODE_GOLD);
      #endif

      #ifdef SW_R_E24_1_T
      /* show E series norm values for E24 1% */
      Show_ENormValues(R1->Value, R1->Scale, E24, 10, LCD_CHAR_OMEGA);
      #endif

      #ifdef SW_R_E24_1_CC
      /* show E series norm value color-codes for E24 1% */
      Show_ENormCodes(R1->Value, R1->Scale, E24, 10, COLOR_CODE_BROWN);
      #endif

      #ifdef SW_R_E96_T
      /* show E series norm values for E96 1% */
      Show_ENormValues(R1->Value, R1->Scale, E96, 10, LCD_CHAR_OMEGA);
      #endif

      #ifdef SW_R_E96_CC
      /* show E series norm value color-codes for E96 1% */
      Show_ENormCodes(R1->Value, R1->Scale, E96, 10, COLOR_CODE_BROWN);
      #endif

      #ifdef SW_R_E96_EIA96
      /* show E series norm value EIA-96 codes for E96 1% */
      Show_ENormEIA96(R1->Value, R1->Scale);
      #endif
    }
    #endif
  }
  #elif defined (SW_R_EXX)
  else                             /* single resistor */
  {
    #ifdef SW_R_E24_5_T
    /* show E series norm values for E24 5% */
    Show_ENormValues(R1->Value, R1->Scale, E24, 50, LCD_CHAR_OMEGA);
    #endif

    #ifdef SW_R_E24_5_CC
    /* show E series norm value color-codes for E24 5% */
    Show_ENormCodes(R1->Value, R1->Scale, E24, 50, COLOR_CODE_GOLD);
    #endif

    #ifdef SW_R_E24_1_T
    /* show E series norm values for E24 1% */
    Show_ENormValues(R1->Value, R1->Scale, E24, 10, LCD_CHAR_OMEGA);
    #endif

    #ifdef SW_R_E24_1_CC
    /* show E series norm value color-codes for E24 1% */
    Show_ENormCodes(R1->Value, R1->Scale, E24, 10, COLOR_CODE_BROWN);
    #endif

    #ifdef SW_R_E96_T
    /* show E series norm values for E96 1% */
    Show_ENormValues(R1->Value, R1->Scale, E96, 10, LCD_CHAR_OMEGA);
    #endif

    #ifdef SW_R_E96_CC
    /* show E series norm value color-codes for E96 1% */
    Show_ENormCodes(R1->Value, R1->Scale, E96, 10, COLOR_CODE_BROWN);
    #endif

    #ifdef SW_R_E96_EIA96
    /* show E series norm value EIA-96 codes for E96 1% */
    Show_ENormEIA96(R1->Value, R1->Scale);
    #endif
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

  /*
   *  find largest cap
   */

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

  #ifdef UI_SERIAL_COMMANDS
  /* set data for remote commands */
  Info.Comp1 = (void *)MaxCap;     /* link largest cap */
  #endif


  /*
   *  display cap and pinout in line #1
   */

  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();         /* use title color */
  #endif

  Display_ProbeNumber(MaxCap->A);  /* display pin #1 */
  Display_EEString(Cap_str);       /* display capacitor symbol */
  Display_ProbeNumber(MaxCap->B);  /* display pin #2 */

  #ifdef UI_COLORED_TITLES
  Display_UseOldColor();           /* use old color */
  #endif


  /*
   *  display capacitance in line #2, optionally ESR
   */

  /* display capacitance */
  Display_NextLine();              /* move to next line */
  Display_Value(MaxCap->Value, MaxCap->Scale, 'F');

  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  /* measure and display ESR */
  ESR = MeasureESR(MaxCap);        /* measure ESR */
  if (ESR < UINT16_MAX)            /* if successful */
  {
    Display_Space();
    Display_Value(ESR, -2, LCD_CHAR_OMEGA);  /* display ESR */
  }
    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Val1 = ESR;               /* copy ESR */
    #endif
  #endif


  /*
   *  display additional stuff
   */

  /* display self-discharge equivalent leakage current */
  if (MaxCap->I_leak_Value > 0)         /* got value */
  {
    Display_NL_EEString_Space(I_leak_str);
    Display_Value(MaxCap->I_leak_Value, MaxCap->I_leak_Scale, 'A');  /* in A */
  }

  #ifdef SW_C_VLOSS
  /* display self-discharge voltage loss in % */
  if (MaxCap->U_loss > 0)               /* got value */
  {
    Display_NL_EEString_Space(U_loss_str);
    Display_Value(MaxCap->U_loss, -1, '%');  /* in 0.1% */
  }
  #endif

  #ifdef SW_C_E6_T
  /* show E series norm values for E6 20% */
  Show_ENormValues(MaxCap->Value, MaxCap->Scale, E6, 200, 'F');
  #endif

  #ifdef SW_C_E12_T
  /* show E series norm values for E12 10% */
  Show_ENormValues(MaxCap->Value, MaxCap->Scale, E12, 100, 'F');
  #endif
}



/*
 *  show current (leakage or whatever) of semiconductor
 *
 *  Mapping for Semi structure:
 *  - I_value - current 
 *  - I_scale - scale for current (10^x)
 */

void Show_SemiCurrent(const unsigned char *String)
{
  if (CmpValue(Semi.I_value, Semi.I_scale, 50, -9) >= 0)  /* show if >=50nA */
  {
    Display_NL_EEString_Space(String);               /* display: <string> */
    Display_Value(Semi.I_value, Semi.I_scale, 'A');  /* display current */
  }
}



#ifndef UI_SERIAL_COMMANDS

/*
 *  display capacitance of a diode
 *
 *  requires:
 *  - pointer to diode structure
 */

void Show_Diode_Cap(Diode_Type *Diode)
{
  /* get capacitance (reversed direction) */
  MeasureCap(Diode->C, Diode->A, 0);

  /* and show capacitance */
  Display_Value(Caps[0].Value, Caps[0].Scale, 'F');
}

#endif



/*
 *  show flyback diode of 3-pin semiconductor and pin designators
 *  - convenience function to reduce firmware size
 *
 *  requires:
 *  - Anode:   probe ID of anode [0-2]
 *  - Cathode: probe ID of cathode [0-2]
 */

void Show_SemiFlybackDiode(uint8_t Anode, uint8_t Cathode)
{
  Display_SemiPinDesignator(Anode);     /* designator for anode side */
  Display_Char(LCD_CHAR_DIODE_AC);      /* diode symbol |>| */
  Display_SemiPinDesignator(Cathode);   /* designator for cathode side */
}



/*
 *  show diode(s)
 */

void Show_Diode(void)
{
  Diode_Type        *D1;           /* pointer to diode #1 */
  Diode_Type        *D2 = NULL;    /* pointer to diode #2 */
  uint8_t           CapFlag = 1;   /* flag for capacitance output */
  uint8_t           A = 5;         /* ID of common anode */
  uint8_t           C = 5;         /* ID of common cathode */
  uint8_t           R_Pin1 = 5;    /* B_E resistor's pin #1 */
  uint8_t           R_Pin2 = 5;    /* B_E resistor's pin #2 */
  uint8_t           n;             /* counter */
  uint8_t           m;             /* counter */

  D1 = &Diodes[0];                 /* pointer to first diode */

  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();         /* use title color */
  #endif

  /*
   *  figure out which diodes to display
   */

  if (Check.Diodes == 1)           /* single diode */
  {
    C = D1->C;                     /* make anode first pin */
  }
  else if (Check.Diodes == 2)      /* two diodes */
  {
    D2 = D1;                       /* copy pointer to first diode */
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
  else                             /* too many diodes */
  {
    /* display number of diodes found in line #1 */
    Display_EEString_Space(Diode_AC_str);    /* display: -|>|- */
    Display_Char('0' + Check.Diodes);        /* display number of diodes */

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Quantity = Check.Diodes;            /* set quantity */
    #endif

    return;
  }

  #ifdef UI_SERIAL_COMMANDS
  /* set data for remote commands */
  Info.Comp1 = (void *)D1;       /* link first diode */
  Info.Comp2 = (void *)D2;       /* link second diode */
  #endif


  /*
   *  display diode(s) and pinout in line #1
   */

  /* first diode */
  if (A < 3)        /* common anode or anti-parallel: show C first */
  {
    Display_ProbeNumber(D1->C);         /* show C */
    Display_EEString(Diode_CA_str);     /* show -|<- */
    Display_ProbeNumber(A);             /* show A */
  }
  else              /* single, common cathode or in-series: show A first */
  {
    Display_ProbeNumber(D1->A);         /* show A */
    Display_EEString(Diode_AC_str);     /* show ->|- */
    Display_ProbeNumber(C);             /* show C */
  }

  if (D2)           /* second diode */
  {
    if (A == C)          /* anti-parallel */
    {
      n = D2->A;                        /* get anode */
      Display_EEString(Diode_CA_str);   /* show -|<- */
    }
    else if (A <= 3)     /* common anode or in-series */
    {
      n = D2->C;                        /* get cathode */
      Display_EEString(Diode_AC_str);   /* show ->|- */
    }
    else                 /* common cathode */
    {
      n = D2->A;                        /* get anode */
      Display_EEString(Diode_CA_str);   /* show -|<- */
    }

    Display_ProbeNumber(n);             /* display pin */

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Quantity = 2;       /* got two */
    #endif
  }

  #ifdef UI_COLORED_TITLES
  Display_UseOldColor();           /* use old color */
  #endif


  /*
   *  check for B-E resistor of possible BJT
   */

  if (R_Pin1 < 5)                  /* possible BJT */
  {
    /* B-E resistor below 25kOhms */
    if (CheckSingleResistor(R_Pin1, R_Pin2, 25) == 1)
    {
      /* show: PNP/NPN? */
      Display_Space();
      if (A < 3)                        /* PNP */
      {
        Display_EEString(PNP_str);      /* display: PNP */
        #ifdef UI_SERIAL_COMMANDS
        /* set data for remote commands */
        Info.Flags |= INFO_D_R_BE | INFO_D_BJT_PNP;    /* R_BE & PNP */
        #endif
      }
      else                              /* NPN */
      {
        Display_EEString(NPN_str);      /* display: NPN */
        #ifdef UI_SERIAL_COMMANDS
        /* set data for remote commands */
        Info.Flags |= INFO_D_R_BE | INFO_D_BJT_NPN;    /* R_BE & NPN */
        #endif
      }
      Display_Char('?');                /* display: ? */

      Display_NextLine();               /* move to line #2 */

      /* show B-E resistor */
      #ifdef UI_PROBE_COLORS
        Show_SingleResistor(R_Pin1, R_Pin2, 0);   /* show resistor */
      #else 
        R_Pin1 += '1';                  /* convert probe ID to character */
        R_Pin2 += '1';
        Show_SingleResistor(R_Pin1, R_Pin2);      /* show resistor */
      #endif

      CapFlag = 0;                      /* skip capacitance */
    }
  }


  /*
   *  display:
   *  - Uf (forward voltage)
   *  - reverse leakage current (for single diode)
   *  - capacitance (not for anti-parallel diodes)
   */

  /* display Uf */
  Display_NL_EEString_Space(Vf_str);    /* display: Vf */

  /* first diode */
  Display_Value(D1->V_f, -3, 'V');      /* in mV */

  Display_Space();

  /* display low current Uf and reverse leakage current for a single diode */
  if (D2 == NULL)                       /* single diode */
  {
    /* display low current Uf if it's quite low (Ge/Schottky diode) */
    if (D1->V_f2 < 250)            /* < 250mV */
    {
      Display_Char('(');
      Display_Value2(D1->V_f2);    /* no unit */
      Display_Char(')');
    }

    /* reverse leakage current */
    UpdateProbes2(D1->C, D1->A);        /* reverse diode */
    GetLeakageCurrent(1);               /* get current */
    Show_SemiCurrent(I_R_str);          /* display I_R */

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Flags |= INFO_D_I_R;           /* measured I_R */
    #endif
  }
  else                                  /* two diodes */
  {
    /* show Uf of second diode */
    Display_Value(D2->V_f, -3, 'V');
  }


  /*
   *  display capacitance in next line
   */

  if (CapFlag == 1)                     /* if feasable */ 
  {
    Display_NL_EEString_Space(DiodeCap_str);   /* display: C */

    #ifndef UI_SERIAL_COMMANDS
    /* first diode */
    Show_Diode_Cap(D1);                 /* measure & show capacitance */

    if (D2)                             /* second diode */
    {
      Display_Space();
      Show_Diode_Cap(D2);               /* measure & show capacitance */
    }
    #endif

    #ifdef UI_SERIAL_COMMANDS
    /* first diode (store value in Caps[0]) */
    MeasureCap(D1->C, D1->A, 0);        /* get capacitance (reversed direction) */
    Display_Value(Caps[0].Value, Caps[0].Scale, 'F');

    if (D2)                   /* second diode (store value in Caps[1]) */
    {
      Display_Space();
      MeasureCap(D2->C, D2->A, 1);      /* get capacitance (reversed direction) */
      Display_Value(Caps[1].Value, Caps[1].Scale, 'F');
    }
    #endif
  }
}



/*
 *  show BJT
 */

void Show_BJT(void)
{
  Diode_Type        *Diode;        /* pointer to diode */
  unsigned char     *String;       /* string pointer (EEPROM) */
  uint8_t           BE_A;          /* V_BE: pin acting as anode */
  uint8_t           BE_C;          /* V_BE: pin acting as cathode */
  uint8_t           CE_A;          /* flyback diode: pin acting as anode */
  uint8_t           CE_C;          /* flyback diode: pin acting as cathode */
  #ifdef SW_SCHOTTKY_BJT
  uint8_t           BC_A;          /* clamping diode: pin acting as anode */
  uint8_t           BC_C;          /* clamping diode: pin acting as cathode */
  #endif
  uint16_t          V_BE = 0;      /* V_BE */
  int16_t           Slope;         /* slope of forward voltage */

  /*
   *  Mapping for Semi structure:
   *  A   - Base pin
   *  B   - Collector pin
   *  C   - Emitter pin
   *  U_1 - U_BE (mV) (not implemented yet)
   *  U_3 - I_C/I_E (µA)
   *  F_1 - hFE
   *  F_2 - reverse hFE
   *  I_value/I_scale - I_CEO
   */

  /*
   *  preset stuff based on BJT type
   */

  if (Check.Type & TYPE_NPN)       /* NPN */
  {
    String = (unsigned char *)NPN_str;       /* "NPN" */

    /* direction of B-E diode: B -> E */
    BE_A = Semi.A;                 /* anode at base */
    BE_C = Semi.C;                 /* cathode at emitter */

    /* direction of optional flyback diode */
    CE_A = Semi.C;                 /* anode at emitter */
    CE_C = Semi.B;                 /* cathode at collector */

    #ifdef SW_SCHOTTKY_BJT
    /* direction of B-C diode: B -> C */
    BC_A = Semi.A;                 /* anode at base */
    BC_C = Semi.B;                 /* cathode at collector */
    #endif
  }
  else                             /* PNP */
  {
    String = (unsigned char *)PNP_str;       /* "PNP" */

    /* direction of B-E diode: E -> B */
    BE_A = Semi.C;                 /* anode at emitter */
    BE_C = Semi.A;                 /* cathode at base */

    /* direction of optional flyback diode */
    CE_A = Semi.B;                 /* anode at collector */
    CE_C = Semi.C;                 /* cathode at emitter */

    #ifdef SW_SCHOTTKY_BJT
    /* direction of B-C diode: C -> B */
    BC_A = Semi.B;                 /* anode at collector */
    BC_C = Semi.A;                 /* cathode at base */
    #endif
  }


  /*
   *  display type in line #1
   */

  #ifdef UI_COLORED_TITLES
    /* display: BJT */
    Display_ColoredEEString_Space(BJT_str, COLOR_TITLE);
    /* display: NPN / PNP */
    Display_ColoredEEString(String, COLOR_TITLE);
  #else
    Display_EEString_Space(BJT_str);    /* display: BJT */
    Display_EEString(String);           /* display: NPN / PNP */
  #endif

  /* parasitic BJT (freewheeling diode on same substrate) */
  if (Check.Type & TYPE_PARASITIC)
  {
    Display_Char('+');                  /* display: + */
  }


  /*
   *  display pinout in line #2
   */

  #ifndef UI_NO_TEXTPINOUT
  Display_NextLine();                   /* next line (#2) */
  Show_SemiPinout();
  #endif

  /* optional freewheeling diode */
  Diode = SearchDiode(CE_A, CE_C);      /* search for matching diode */
  if (Diode != NULL)                    /* got it */
  {
    #ifndef UI_NO_TEXTPINOUT
      /* follows pinout */
      Display_Space();                  /* space */
    #else
      /* no classic pinout */
      Display_NextLine();               /* next line (#2) */
    #endif

    /* display diode and pin designators */
    Show_SemiFlybackDiode(CE_A, CE_C);

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Flags |= INFO_BJT_D_FB;        /* found flyback diode */
    Info.Comp1 = Diode;                 /* link diode */
    #endif
  }


  /*
   *  display B-E resistor in next line if detected 
   */

  /* check for B-E resistor below 25kOhms */
  if (CheckSingleResistor(BE_C, BE_A, 25) == 1)   /* found B-E resistor */
  {
    Display_NextLine();            /* next line (#3) */
    #ifdef UI_PROBE_COLORS
      Show_SingleResistor(Semi.A, Semi.C, 1);     /* show resistor */
    #else
      Show_SingleResistor('B', 'E');              /* show resistor */
    #endif
    /* B-E resistor renders hFE and V_BE measurements useless */

    #ifdef SW_SYMBOLS
    UI.SymbolLine = 4;             /* display fancy pinout in line #4 */
    #endif

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Flags |= INFO_BJT_R_BE;   /* R_BE */
    #endif
  }


  /*
   *  display h_FE in next line
   */

  /* display h_FE */
  Display_NL_EEString_Space(h_FE_str);       /* display: hFE */
  Display_Value2(Semi.F_1);                  /* display h_FE */

  /* display hFE test circuit type */
  Display_Space();
  if (Semi.Flags & HFE_COMMON_EMITTER)         /* common emitter */
  {
    /* common emitter circuit */
    Display_Char('e');                         /* display: e */
  }
  else if (Semi.Flags & HFE_COMMON_COLLECTOR)  /* common collector */
  {
    /* common collector circuit */
    Display_Char('c');                         /* display: c */
  }

  #ifdef SW_HFE_CURRENT
  /* display test current for hFE measurement */
  Display_NL_EEString(I_str);                  /* display: I_ */
  if (Semi.Flags & HFE_COMMON_EMITTER)         /* common emitter */
  {
    /* I_C */
    Display_Char('C');                         /* display: C */
  }
  else if (Semi.Flags & HFE_COMMON_COLLECTOR)  /* common collector */
  {
    /* I_E */
    Display_Char('E');                         /* display: E */
  }
  Display_Space();
  Display_SignedValue(Semi.U_3, -6, 'A');      /* display I_C/I_E */
  #endif

  #ifdef SW_REVERSE_HFE
  /* display reverse hFE */
  if (Diode == NULL)               /* no freewheeling diode */
  {
    if (Semi.F_2 > 0)              /* valid value */
    {
      Display_NL_EEString_Space(h_FE_r_str);      /* display: hFEr */
      Display_Value2(Semi.F_2);                   /* display reverse h_FE */
    }
  }
  #endif


  /*
   *  display V_BE in next line
   *  (taken from diode's forward voltage)
   */

  Diode = SearchDiode(BE_A, BE_C);      /* search for matching B-E diode */
  if (Diode != NULL)                    /* found diode */
  {
    Display_NL_EEString_Space(V_BE_str);     /* display: Vbe */

    /*
     *  V_f is quite linear for a logarithmicly scaled I_b.
     *  So we may interpolate the V_f values of low and high test current
     *  measurements for a virtual test current. Low test current is 10µA
     *  and high test current is 7mA. That's a logarithmic scale of
     *  3 decades.
     */

    /* calculate slope for one decade */
    Slope = Diode->V_f - Diode->V_f2;
    Slope /= 3;

    /* select V_BE based on hFE */
    if (Semi.F_1 < 100)                 /* low h_FE */
    {
      /*
       *  BJTs with low hFE are power transistors and need a large I_b
       *  to drive the load. So we simply take Vf of the high test current
       *  measurement (7mA). 
       */

      V_BE = Diode->V_f;
    }
    else if (Semi.F_1 < 250)            /* mid-range h_FE */
    {
      /*
       *  BJTs with a mid-range hFE are signal transistors and need
       *  a small I_b to drive the load. So we interpolate Vf for
       *  a virtual test current of about 1mA.
       */

      V_BE = Diode->V_f - Slope;
    }
    else                                /* high h_FE */
    {
      /*
       *  BJTs with a high hFE are small signal transistors and need
       *  only a very small I_b to drive the load. So we interpolate Vf
       *  for a virtual test current of about 0.1mA.
       */

      V_BE = Diode->V_f2 + Slope;
    }

    Display_Value(V_BE, -3, 'V');       /* in mV */

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Val1 = V_BE;                   /* copy V_BE */
    #endif
  }

  /* I_CEO: collector emitter open current (leakage) */
  Show_SemiCurrent(I_CEO_str);          /* display I_CEO */


  #ifdef SW_SCHOTTKY_BJT
  /*
   *  Schottky transistor / Schottky-clamped BJT
   *  - V_BC of a Germanium BJT is as low as V_f of a Schottky diode.
   *    So we check only Silicon BJTs.
   *  - display V_f of clamping diode
   */

  /* check for Si BJT */
  if (V_BE > 500)                       /* V_BE > 500mV */
  {
    Diode = SearchDiode(BC_A, BC_C);    /* search for matching diode */
    if (Diode != NULL)                  /* found diode */
    {
      /* check for Schottky diode */
      if (Diode->V_f < 450)             /* V_f < 450mV */
      {
        Display_NextLine();             /* next line */

        /* display base-collector diode and pin designators */
        Show_SemiFlybackDiode(BC_A, BC_C);

        /* display diode's V_f */
        Display_Space();                     /* display space */
        Display_Value(Diode->V_f, -3, 'V');  /* display V_f */

        #ifdef UI_SERIAL_COMMANDS
        /* set data for remote commands */
        Info.Flags |= INFO_BJT_SCHOTTKY;     /* Schottky-clamped BJT */
        Info.Comp2 = Diode;                  /* link clamping diode */
        #endif
      }
    }
  }
  #endif
}



/*
 *  show MOSFET/JFET/IGBT extras
 *  - diode (intrinsic or flyback)
 *  - V_th
 *  - Cgs
 *  - R_DS_on
 *  - V_f of diode
 */

void Show_FET_Extras(void)
{
  Diode_Type        *Diode;        /* pointer to diode */  
  uint8_t           Anode;         /* anode of diode */
  uint8_t           Cathode;       /* cathode of diode */

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

  /* get expected anode and cathode pins */
  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel/NPN */
  {
    Anode = Semi.C;                /* source/emitter */
    Cathode = Semi.B;              /* drain/collector */
  }
  else                                  /* p-channel/PNP */
  {
    Anode = Semi.B;                /* drain/collector */
    Cathode = Semi.C;              /* source/emitter */
  }

  /* search for matching diode */
  Diode = SearchDiode(Anode, Cathode);

  if (Diode != NULL)               /* if available */
  {
    #ifdef UI_NO_BODYDIODE_TEXTPINOUT
    if (! (Check.Type & TYPE_MOSFET))   /* no MOSFET */
    {
      /* show diode for anything but a MOSFET */
    #endif

      #ifndef UI_NO_TEXTPINOUT
        /* follows pinout */
        Display_Space();                /* space */
      #else
        /* no classic pinout */
        Display_NextLine();             /* next line (#2) */
      #endif

      /* show diode and pin designators */
      Show_SemiFlybackDiode(Anode, Cathode);

    #ifdef UI_NO_BODYDIODE_TEXTPINOUT
    }
    #endif

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Flags |= INFO_FET_D_FB;   /* found diode */
    Info.Comp1 = Diode;            /* link diode data */
    #endif

    /* todo: move output of Vf here? */
  }


  /*
   *  for enhancement mode FET/IGBT show:
   *  - V_th
   *  - C_GS
   *  - R_DS_on
   */

  if (Check.Type & TYPE_ENHANCEMENT)    /* enhancement mode */
  {
    /* display gate threshold voltage V_th */
    if (Semi.U_2 != 0)                       /* if available */
    {
      Display_NL_EEString_Space(Vth_str);         /* display: Vth */
      Display_SignedValue(Semi.U_2, -3, 'V');     /* display V_th in mV */

      #ifdef UI_SERIAL_COMMANDS
      /* set data for remote commands */
      Info.Flags |= INFO_FET_V_TH;                /* measured Vth */
      #endif
    }

    /* display gate-source capacitance C_GS */
    /* todo: display "Cge" for IGBT? */
    Display_NL_EEString_Space(Cgs_str);                /* display: Cgs */
    Display_Value(Semi.C_value, Semi.C_scale, 'F');    /* display value and unit */

    #ifdef UI_SERIAL_COMMANDS
    /* set data for remote commands */
    Info.Flags |= INFO_FET_C_GS;                  /* measured C_GS */
    #endif

    /* display R_DS_on */
    if (Semi.U_1 > 0)                             /* if available */
    {
      Display_NL_EEString_Space(R_DS_str);             /* display: Rds */
      Display_Value(Semi.U_1, -2, LCD_CHAR_OMEGA);     /* display value */

      #ifdef UI_SERIAL_COMMANDS
      /* set data for remote commands */
      Info.Flags |= INFO_FET_R_DS;                /* measured R_DS */
      #endif
    }
  }


  /*
   *  show V_f of diode (instrinsic or flyback)
   */

  /* display V_f of diode */
  if (Diode != NULL)                    /* if available */
  {
    Display_NextLine();                      /* new line */
    Display_Char(LCD_CHAR_DIODE_AC);         /* diode symbol |>| */
    Display_Space();                         /* space */
    Display_EEString_Space(Vf_str);          /* display: Vf */
    Display_Value(Diode->V_f, -3, 'V');      /* display value */
  }
}



/*
 *  show FET/IGBT channel type
 */

void Show_FET_Channel(void)
{
  Display_Space();                      /* display space */

  /* channel type */
  if (Check.Type & TYPE_N_CHANNEL)      /* n-channel */
  {
    Display_Char('N');                  /* display: N */
  }
  else                                  /* p-channel */
  {
    Display_Char('P');                  /* display: P */
  }

  Display_EEString(Channel_str);        /* display: -ch */
}



/*
 *  show FET/IGBT mode
 */

void Show_FET_Mode(void)
{
  Display_Space();                      /* display space */

  if (Check.Type & TYPE_ENHANCEMENT)    /* enhancement mode */
  {
    Display_EEString(Enhancement_str);  /* display: enh. */
  }
  else                                  /* depletion mode */
  {
    Display_EEString(Depletion_str);    /* display: dep. */
  }
}



/*
 *  show FET (MOSFET & JFET)
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
   *  U_3 - V_GS(off) (mV)
   */

  /*
   *  display type, channel and mode in line #1
   */

  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();         /* use title color */
  #endif

  /* display type */
  if (Check.Type & TYPE_MOSFET)    /* MOSFET */
  {
    Display_EEString(MOS_str);          /* display: MOS */
  }
  else                             /* JFET */
  {
    Display_Char('J');                  /* display: J */
  }
  Display_EEString(FET_str);       /* display: FET */

  /* display channel type */
  Show_FET_Channel();
      
  /* display mode for MOSFET */
  if (Check.Type & TYPE_MOSFET) Show_FET_Mode();

  #ifdef UI_COLORED_TITLES
  Display_UseOldColor();           /* use old color */
  #endif


  /*
   *  display pinout in line #2
   */

  #ifndef UI_NO_TEXTPINOUT
  Display_NextLine();                   /* next line (#2) */
  Show_SemiPinout();                    /* show pinout */
  #endif


  /*
   *  display additional stuff
   */

  /* show body diode, V_th, Cgs, etc. for MOSFET */
  /* or optional flyback diode plus V_f for JFET */
  Show_FET_Extras();

  /* show I_DSS and V_GS(off) for depletion mode FET */
  if (Check.Type & TYPE_DEPLETION)
  {
    /* I_DSS */
    Show_SemiCurrent(I_DSS_str);        /* display Idss */

    /* V_GS(off) */
    if (Semi.U_3 != 0)                  /* valid value */
    {
      Display_NL_EEString_Space(V_GSoff_str);     /* display: V_GS(off) */
      #ifdef SW_SYMBOLS
      /* we need some space for the symbol: move value to next line */
      Display_NextLine();
      Display_Space();
      #endif
      Display_SignedValue(Semi.U_3, -3, 'V');     /* display V_GS(off) in mV */
    }
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

  /* display type in line #1 */
  #ifdef UI_COLORED_TITLES
  Display_UseTitleColor();         /* use title color */
  #endif

  Display_EEString(IGBT_str);      /* display: IGBT */
  Show_FET_Channel();              /* display channel type */
  Show_FET_Mode();                 /* display mode */

  #ifdef UI_COLORED_TITLES
  Display_UseOldColor();           /* use old color */
  #endif

  #ifndef UI_NO_TEXTPINOUT
  /* display pinout in line #2 */
  Display_NextLine();              /* next line (#2) */
  Show_SemiPinout();               /* show pinout */
  #endif

  /* display additional stuff */
  Show_FET_Extras();               /* show diode, V_th and C_GE */
}



/*
 *  show Thyristor and TRIAC
 */

void Show_ThyristorTriac(void)
{
  /*
   *  Mapping for Semi structure:
   *        SCR        TRIAC
   *  A   - Gate       Gate
   *  B   - Anode      MT2
   *  C   - Cathode    MT1
   *  U_1 - V_GT (mV)
   */

  /* display component type in line #1 and pinout in line #2 */
  if (Check.Found == COMP_THYRISTOR)    /* SCR */
  {
    #ifdef UI_COLORED_TITLES
      /* display: thyristor */
      Display_ColoredEEString(Thyristor_str, COLOR_TITLE);
    #else
      Display_EEString(Thyristor_str);  /* display: thyristor */
    #endif
  }
  else                                  /* TRIAC */
  {
    #ifdef UI_COLORED_TITLES
      /* display: TRIAC */
      Display_ColoredEEString(Triac_str, COLOR_TITLE);
    #else
      Display_EEString(Triac_str);      /* display: TRIAC */
    #endif
  }

  #ifndef UI_NO_TEXTPINOUT
  Display_NextLine();                   /* next line (#2) */
  Show_SemiPinout();                    /* display pinout */
  #endif

  /* show V_GT (gate trigger voltage) in line #3 */
  if (Semi.U_1 > 0)                /* show if not zero */
  {
    Display_NL_EEString_Space(V_GT_str);     /* display: V_GT */
    Display_Value(Semi.U_1, -3, 'V');        /* display V_GT in mV */
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
   *  U_1 - V_f (mV)
   *  U_2 - V_T (mV)
   *
   *  Mapping for Semi structure:
   *  A   - Gate
   *  B   - Anode
   *  C   - Cathode
   */

  /* display type in line #1 */
  #ifdef UI_COLORED_TITLES
    /* display: PUT */
    Display_ColoredEEString(PUT_str, COLOR_TITLE);
  #else
    Display_EEString(PUT_str);          /* display: PUT */
  #endif

  #ifndef UI_NO_TEXTPINOUT
  /* display pinout in line #2 */
  Display_NextLine();                   /* move to line #2 */
  Show_SemiPinout();                    /* display pinout */
  #endif

  /* display V_T in line #3 */
  Display_NL_EEString_Space(V_T_str);   /* display: VT */
  Display_Value(AltSemi.U_2, -3, 'V');  /* display V_T */

  /* display V_f in line #4 */
  Display_NL_EEString_Space(Vf_str);    /* display: Vf */
  Display_Value(AltSemi.U_1, -3, 'V');  /* display V_f */
}



#ifdef SW_UJT

/*
 *  show UJT (n-type)
 */

void Show_UJT(void)
{
  /*
   *  Mapping for AltSemi structure:
   *  A   - Emitter
   *  B   - B2
   *  C   - B1
   +
   *  Mapping for Semi structure:
   *  A   - Gate
   *  B   - Anode
   *  C   - Cathode
   */

  /* display type in line #1 */
  #ifdef UI_COLORED_TITLES
    /* display: UJT */
    Display_ColoredEEString(UJT_str, COLOR_TITLE);
  #else
    Display_EEString(UJT_str);          /* display: UJT */
  #endif

  #ifndef UI_NO_TEXTPINOUT
  /* display pinout in line #2 */  
  Display_NextLine();                   /* next line (#2) */
  Show_SemiPinout();                    /* display pinout */
  #endif

  /* display r_BB in line #3 */
  Display_NL_EEString_Space(R_BB_str);  /* display: R_BB */  
  Display_Value(Resistors[0].Value, Resistors[0].Scale, LCD_CHAR_OMEGA);
}

#endif



#ifdef HW_PROBE_ZENER

/*
 *  show Zener diode / external voltage
 */

void Show_Zener(void)
{
  /*
   *  Anode and cathode are fixed (dedicated probes).
   *
   *  Mapping for Semi structure:
   *  U1   - V_Z (Zener voltage, mV)
   */

  /* display type in line #1 */
  #ifdef UI_COLORED_TITLES
    /* display: Zener */
    Display_ColoredEEString(Zener_str, COLOR_TITLE);
  #else
    Display_EEString(Zener_str);        /* display: Zener */
  #endif

  /* display voltage in line #2 */  
  Display_NextLine();                   /* next line (#2) */
  Display_Value(Semi.U_1, -3, 'V');     /* display voltage */

  #ifdef UI_ZENER_DIODE
  /* display Zener diode symbol */
  Check.Symbol = SYMBOL_DIODE_ZENER;    /* set symbol ID */
  Display_FancySemiPinout(3);           /* show symbol starting in line #3 */
  #endif
}

#endif



/* ************************************************************************
 *   voltage reference
 * ************************************************************************ */


/*
 *  manage voltage references
 */

void CheckVoltageRefs(void)
{
  #ifdef HW_REF25
  uint16_t          U_Ref;         /* reference voltage */
  uint32_t          Temp;          /* temporary value */
  #endif


  /*
   *  external 2.5V voltage reference
   */

  #ifdef HW_REF25
  Cfg.Samples = 200;               /* perform 200 ADC samples for high accuracy */
  U_Ref = ReadU(TP_REF);           /* read voltage of reference (mV) */

  /* check for valid voltage range */
  if ((U_Ref > 2250) && (U_Ref < 2750))      /* voltage is fine */
  {
    /* adjust Vcc (assuming 2.495V typically) */
    Temp = ((uint32_t)Cfg.Vcc * UREF_25) / U_Ref;
    Cfg.Vcc = (uint16_t)Temp;

    Cfg.OP_Mode |= OP_EXT_REF;          /* set flag */
  }
  else                                       /* voltage out of range */
  {
    Cfg.OP_Mode &= ~OP_EXT_REF;         /* clear flag */
  }
  #endif


  /*
   *  internal 1.1V bandgap reference
   */

  Cfg.Bandgap = ReadU(ADC_CHAN_BANDGAP);     /* dummy read for bandgap stabilization */
  Cfg.Samples = 200;                         /* perform 200 ADC samples for high accuracy */
  Cfg.Bandgap = ReadU(ADC_CHAN_BANDGAP);     /* get voltage of bandgap reference (mV) */
  Cfg.Bandgap += NV.RefOffset;               /* add voltage offset */

  /* clean up */
  Cfg.Samples = ADC_SAMPLES;            /* set ADC samples back to default */
}



/* ************************************************************************
 *   power control and monitoring
 * ************************************************************************ */


/*
 *  power off
 */

void PowerOff(void)
{
  /* display feedback (otherwise the user will wait :) */
  LCD_Clear();
  #ifdef LCD_COLOR
  UI.PenColor = COLOR_INFO;             /* set pen color */
  #endif
  #ifdef UI_CENTER_ALIGN
    Display_CenterLine(1);              /* center block: 1 line */
    Display_EEString_Center(Bye_str);   /* display: Bye! */
  #else
    Display_EEString(Bye_str);          /* display: Bye! */
  #endif

  /* disable stuff */
  cli();                                /* disable interrupts */
  wdt_disable();                        /* disable watchdog */

  /* power down */
  #ifdef POWER_SWITCH_SOFT
  POWER_PORT &= ~(1 << POWER_CTRL);     /* power off myself: set pin low */
    #ifdef PASSIVE_POWER_CTRL
    POWER_DDR |= (1 << POWER_CTRL);     /* set pin to output mode */
    #endif
  #endif

  /*
   *  As long as the user keeps the test button pressed the MCU is still
   *  powered. Therefor we make sure that nothing happens by forcing the
   *  MCU to sleep which also saves power. Same for a classic power switch.
   */

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  /* set sleep mode to "power down" */
  sleep_mode();                         /* enter sleep mode */
}



#ifndef BAT_NONE

/*
 *  display battery status
 *  - uses voltage stored in Cfg.Vbat
 */

void ShowBattery(void)
{
  #ifdef UI_BATTERY
  uint8_t           Char1;         /* battery icon left part */
  uint8_t           Char2;         /* battery icon right part */
  #endif

  #ifndef UI_BATTERY
  /* display battery info (text) */
  Display_EEString_Space(Battery_str);     /* display: Bat. */

    #ifdef BAT_EXT_UNMONITORED
    /* check for unmonitored external power supply */
    if (Cfg.Vbat < 900)            /* < 0.9V */
    {
      /* powered by unmonitored external PSU */
      /* low voltage caused by diode's leakage current */

      /* display status */
      #ifdef LCD_COLOR
      UI.PenColor = COLOR_BAT_OK;       /* use OK color */
      #endif
      Display_EEString(External_str);   /* display: ext */
    }
    else                           /* battery operation */
    {
      /* powered by battery */
    #endif

      /* display battery voltage */
      Display_Value(Cfg.Vbat / 10, -2, 'V');
      Display_Space();

      /* display status (ok, weak, low) */
      if (Cfg.Vbat < BAT_LOW)           /* low level reached */
      {
        #ifdef LCD_COLOR
        UI.PenColor = COLOR_BAT_LOW;    /* set LOW color */
        #endif
        Display_EEString(Low_str);      /* display: low */
      }
      else if (Cfg.Vbat < BAT_WEAK)     /* warning level reached */
      {
        #ifdef LCD_COLOR
        UI.PenColor = COLOR_BAT_WEAK;   /* set WEAK color */
        #endif
        Display_EEString(Weak_str);     /* display: weak */
      }
      else                              /* ok */
      {
        #ifdef LCD_COLOR
        UI.PenColor = COLOR_BAT_OK;     /* set OK color */
        #endif
        Display_EEString(OK_str);       /* display: ok */
      }

    #ifdef BAT_EXT_UNMONITORED
    }
    #endif

    #ifdef LCD_COLOR
    UI.PenColor = COLOR_PEN;       /* set color back to default */
    #endif

  #endif


  #ifdef UI_BATTERY
  /* display battery info (small symbol) */

  /* get battery status */
    #ifdef BAT_EXT_UNMONITORED
    /* check for unmonitored external power supply */
    if (Cfg.Vbat < 900)                 /* < 0.9V */
    {
      /* powered by unmonitored external PSU */
      /* low voltage caused by diode's leakage current */
      #ifdef LCD_COLOR
      UI.PenColor = COLOR_BAT_OK;       /* set OK color */
      #endif
      Char1 = LCD_CHAR_BAT_LH;          /* left: high */
      Char2 = LCD_CHAR_BAT_RH;          /* right: high */
    }
    else
    #endif
    if (Cfg.Vbat < BAT_LOW)             /* low level reached */
    {
      #ifdef LCD_COLOR
      UI.PenColor = COLOR_BAT_LOW;      /* set LOW color */
      #endif
      Char1 = LCD_CHAR_BAT_LL;          /* left: low */
      Char2 = LCD_CHAR_BAT_RL;          /* right: low */
    }
    else if (Cfg.Vbat < BAT_WEAK)       /* warning level reached */
    {
      #ifdef LCD_COLOR
      UI.PenColor = COLOR_BAT_WEAK;     /* set WEAK color */
      #endif
      Char1 = LCD_CHAR_BAT_LH;          /* left: high */
      Char2 = LCD_CHAR_BAT_RL;          /* right: low */
    }
    else                                /* ok */
    {
      #ifdef LCD_COLOR
      UI.PenColor = COLOR_BAT_OK;       /* set OK color */
      #endif
      Char1 = LCD_CHAR_BAT_LH;          /* left: high */
      Char2 = LCD_CHAR_BAT_RH;          /* right: high */
    }

  /* display small battery symbol */
  Display_Char(Char1);             /* display left part of symbol */
  Display_Char(Char2);             /* display right part of symbol */
  Display_Space();                 /* display space */

    #ifdef LCD_COLOR
    UI.PenColor = COLOR_PEN;       /* set color back to default */
    #endif

  /* display battery voltage */
    #ifdef BAT_EXT_UNMONITORED
    /* check for unmonitored external power supply */
    if (Cfg.Vbat < 900)            /* < 0.9V */
    {
      Display_EEString(External_str);   /* display: ext */
    }
    else                           /* battery operation */
    {
      /* powered by battery */
    #endif

      /* display battery voltage */
      Display_Value(Cfg.Vbat / 10, -2, 'V');

    #ifdef BAT_EXT_UNMONITORED
    }
    #endif

  #endif
}



/*
 *  check battery
 *  - store battery voltage in Cfg.Vbat
 *  - power off in case of a low battery
 */

void CheckBattery(void)
{
  uint16_t          U_Bat;         /* battery voltage */

  /* get current battery voltage */
  U_Bat = ReadU(TP_BAT);           /* read voltage (mV) */

  #ifdef BAT_DIVIDER
  uint32_t          Temp;          /* temporary value */

  /*
   *  ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
   *  - U2 = (Uin / (R1 + R2)) * R2 
   *  - Uin = (U2 * (R1 + R2)) / R2
   */

  Temp = (((uint32_t)(BAT_R1 + BAT_R2) * 1000) / BAT_R2);   /* factor (0.001) */
  Temp *= U_Bat;                   /* Uin (0.001 mV) */
  Temp /= 1000;                    /* Uin (mV) */
  U_Bat = (uint16_t)Temp;          /* keep 2 bytes */
  #endif

  U_Bat += BAT_OFFSET;             /* add offset for voltage drop */
  Cfg.Vbat = U_Bat;                /* save battery voltage */
  Cfg.BatTimer = 100;              /* reset timer for next battery check (in 100ms) */
                                   /* about 10s */

  /* check for low-voltage situation */
  if (U_Bat < BAT_LOW)             /* low level reached */
  {
    #ifdef BAT_EXT_UNMONITORED
    /* not for unmonitored external power supply */
    if (U_Bat >= 900)                   /* >= 0.9V */
    {
      /* battery operation */
    #endif

      #ifdef UI_COLORED_CURSOR
      /* because of TestKey() we have to reset the pen color */
      UI.PenColor = COLOR_PEN;     /* set default pen color */
      #endif

      LCD_Clear();                 /* clear display */
      ShowBattery();               /* display battery status */
      MilliSleep(3000);            /* let user read info */
      PowerOff();                  /* power off */

    #ifdef BAT_EXT_UNMONITORED
    }
    #endif
  }
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
  uint8_t           Test;          /* test value */
  uint8_t           Key;           /* user feedback */


  /*
   *  init hardware
   */

  #ifdef POWER_SWITCH_SOFT
    #ifdef PASSIVE_POWER_CTRL
      /* switch on power to keep me alive (passive way) */
      /* pin in input mode and low by default */
    #else
      /* switch on power to keep me alive (standard way) */
      POWER_DDR = (1 << POWER_CTRL);        /* set pin as output */
      POWER_PORT = (1 << POWER_CTRL);       /* set pin high to drive power management transistor */
    #endif
  #endif

  /* set up MCU */
  MCUCR = (1 << PUD);                   /* disable pull-up resistors globally */
  ADCSRA = (1 << ADEN) | ADC_CLOCK_DIV; /* enable ADC and set clock divider */

  #ifdef HW_DISCHARGE_RELAY
  /* init discharge relay (safe mode): short circuit probes */
                                        /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);              /* disable relay */
  #endif

  /* catch watchdog */  
  Test = (MCUSR & (1 << WDRF));         /* save watchdog flag */
  MCUSR &= ~(1 << WDRF);                /* reset watchdog flag */
  wdt_disable();                        /* disable watchdog */


  /*
   *  set important default values
   */

  #if defined (UI_AUTOHOLD) || defined (UI_SERIAL_COMMANDS)
    /* reset mode/state flags and set auto-hold mode */
    Cfg.OP_Mode = OP_AUTOHOLD;          /* set auto-hold */
  #else
    /* reset mode/state flags and set continuous mode */
    Cfg.OP_Mode = OP_NONE;              /* none = continuous */
  #endif
  Cfg.OP_Control = OP_OUT_LCD;          /* reset control/signal flags */
                                        /* enable output to display */
  #ifdef SAVE_POWER
  Cfg.SleepMode = SLEEP_MODE_PWR_SAVE;  /* sleep mode: power save */
  #endif                                /* we have to keep Timer2 running */


  /*
   *  set up busses and interfaces
   */

  /* test push button */
  /* set to input by default */

  #ifdef HW_SERIAL
  /* hardware or bitbang USART */
  Serial_Setup();                       /* set up TTL serial interface */
  #endif

  #ifdef HW_I2C
  /* hardware or bitbang I2C */
  I2C_Setup();                          /* set up I2C bus */
  #endif

  #ifdef HW_SPI
  /* hardware or bitbang SPI */
  SPI_Setup();                          /* set up SPI bus */
  #endif

  /* display module */
  LCD_BusSetup();                       /* set up display bus */

  #ifdef HW_TOUCH
  /* touch screen */
  Touch_BusSetup();                     /* set up touch screen bus */
  #endif

  #ifdef ONEWIRE_IO_PIN
  /* OneWire with dedicated IO pin */
  OneWire_Setup();                      /* set up OneWire bus */
  #endif

  #ifdef HW_MAX6675
  /* MAX6675 thermocouple converter */
  MAX6675_BusSetup();                   /* set up MAX6675's bus */
  #endif

  #ifdef HW_MAX31855
  /* MAX31855 thermocouple converter */
  MAX31855_BusSetup();                  /* set up MAX31855's bus */
  #endif


  /*
   *  watchdog was triggered (timeout 2s)
   *  - This is after the MCU has performed a reset driven by the watchdog.
   *  - Does only work if the capacitor at the base of the power management
   *    transistor is large enough to survive a MCU reset. Otherwise the
   *    tester simply loses power.
   */

  if (Test)
  {
    /*
     *  inform user
     *  - Display was initialized before but some global variables in the
     *    driver might be zeroed. Drivers with line tracking won't clear screen.
     */

    LCD_Clear();                        /* clear display */
    #ifdef LCD_COLOR
    UI.PenColor = COLOR_ERROR;          /* set pen color */
    #endif
    #ifdef UI_CENTER_ALIGN
      Display_CenterLine(2);                 /* center block: 2 lines */
      Display_EEString_Center(Timeout_str);  /* display: timeout */
      Display_NL_EEString_Center(Error_str); /* display: error */
    #else
      Display_EEString(Timeout_str);    /* display: timeout */
      Display_NL_EEString(Error_str);   /* display: error */
    #endif
    MilliSleep(2000);                   /* give user some time to read */

    /* power down */
    #ifdef POWER_SWITCH_SOFT
      POWER_PORT &= ~(1 << POWER_CTRL);      /* power off myself: set pin low */
      #ifdef PASSIVE_POWER_CTRL
      POWER_DDR |= (1 << POWER_CTRL);        /* set pin to output mode */
      #endif
    #elif defined (POWER_SWITCH_MANUAL)
      /* enter sleep mode to prevent any further action */
      /* user should power off tester */
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* set sleep mode to "power down" */
      sleep_mode();                          /* enter sleep mode */
    #endif

    return 0;                           /* exit program */
  }


  /*
   *  operation mode selection
   *  - short key press -> continuous mode
   *  - long key press -> auto-hold mode
   *  - very long key press -> reset to defaults
   */

  Key = 0;                              /* reset key press type */

  /* catch key press */
  if (!(BUTTON_PIN & (1 << TEST_BUTTON)))    /* test button pressed */
  {
    Test = 0;                      /* ticks counter */

    while (Key == 0)               /* loop until we got a type */
    {
      MilliSleep(20);                   /* wait 20ms */

      if (!(BUTTON_PIN & (1 << TEST_BUTTON)))     /* button still pressed */
      {
        Test++;                         /* increase counter */
        if (Test > 100) Key = 3;        /* >2000ms */
      }
      else                                        /* button released */
      {
        Key = 1;                        /* <300ms */
        if (Test > 15) Key = 2;         /* >300ms */
      }
    }
  }

  #ifndef UI_SERIAL_COMMANDS
  /* key press >300ms selects alternative operation mode */
  if (Key > 1)
  {
    #ifdef UI_AUTOHOLD
      /* change mode to continuous */
      Cfg.OP_Mode &= ~OP_AUTOHOLD;      /* clear auto-hold */
    #else
      /* change mode to auto-hold */
      Cfg.OP_Mode |= OP_AUTOHOLD;       /* set auto-hold */
    #endif
  }
  #endif

  #ifdef POWER_OFF_TIMEOUT
  /* automatic power-off for auto-hold mode */
  if (Cfg.OP_Mode & OP_AUTOHOLD)        /* in auto-hold mode */
  {
    Cfg.OP_Control |= OP_PWR_TIMEOUT;   /* enable power-off timeout */
  }
  #endif


  /*
   *  init display module
   */

  #ifdef SW_DISPLAY_ID
  Cfg.DisplayID = 0;                    /* reset display ID */
  #endif

  LCD_Init();                           /* initialize LCD */

  UI.LineMode = LINE_STD;               /* reset next-line mode */
  #ifdef LCD_COLOR
  UI.PenColor = COLOR_INFO;             /* set pen color */
  #endif

  #ifdef HW_TOUCH
  Touch_Init();                         /* init touch screen */
  #endif


  /*
   *  init additional hardware
   */

  #ifdef HW_BUZZER
  /* set up port pin for buzzer control: off by default */
  BUZZER_PORT &= ~(1 << BUZZER_CTRL);   /* set pin low */
  BUZZER_DDR |= (1 << BUZZER_CTRL);     /* enable output */
  #endif

  #ifdef ZENER_SWITCHED
  /* set up port pin for boost converter control: off by default */
    #ifdef ZENER_BOOST_HIGH
      /* high active */
      BOOST_PORT &= ~(1 << BOOST_CTRL);    /* set pin low */
    #else
      /* low active */
      BOOST_PORT |= (1 << BOOST_CTRL);     /* set pin high */
    #endif
  BOOST_DDR |= (1 << BOOST_CTRL);          /* enable output */
  #endif

  #ifdef HW_FLASHLIGHT
  /* set up port pin for flashlight control: off by default */
  FLASHLIGHT_PORT &= ~(1 << FLASHLIGHT_CTRL);   /* set pin low */
  FLASHLIGHT_DDR |= (1 << FLASHLIGHT_CTRL);     /* enable output */
  #endif


  /*
   *  load saved adjustment offsets and values
   */

  if (Key == 3)               /* key press >2s resets to defaults */
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
  LCD_Contrast(NV.Contrast);            /* set LCD contrast */
  #endif


  /*
   *  welcome user
   */

  #ifdef UI_SERIAL_COPY
  Display_Serial_On();                  /* enable serial output & NL */
  #endif

  #ifdef UI_CENTER_ALIGN
    Display_CenterLine(2);                   /* center block: 2 lines */
    Display_EEString_Center(Tester_str);     /* display: Component Tester */
    Display_NL_EEString_Center(Version_str); /* display firmware version */
  #else
    Display_EEString(Tester_str);       /* display: Component Tester */
    Display_NL_EEString(Version_str);   /* display firmware version */
  #endif

  #ifdef SW_DISPLAY_ID
  /* show ID of display controller */
  Display_Space();                      /* display space */
  Display_HexValue(Cfg.DisplayID, 16);  /* display ID */
  #endif

  #ifdef UI_SERIAL_COPY
  Display_Serial_Off();                 /* disable serial output & NL */
  #endif

  #ifdef LCD_COLOR
  UI.PenColor = COLOR_PEN;              /* set pen color */
  #endif

  MilliSleep(1500);                     /* let the user read the display */


  /*
   *  init variables
   */

  /* cycling */
  #if CYCLE_MAX < 255
  MissedParts = 0;                      /* reset counter */
  #endif
  Key = KEY_POWER_ON;                   /* just powered on */

  /* default offsets and values */
  Cfg.Samples = ADC_SAMPLES;            /* number of ADC samples */
  Cfg.AutoScale = 1;                    /* enable ADC auto scaling */
  Cfg.Ref = 1;                          /* no ADC reference set yet */
  Cfg.Vcc = UREF_VCC;                   /* voltage of Vcc */

  /* MCU */
  wdt_enable(WDTO_2S);		        /* enable watchdog: timeout 2s */


  /*
   *  user interaction after power-on
   */

  #ifdef HW_TOUCH
  /* adjust touch screen if not done yet */
  if ((Touch.X_Start == 0) && (Touch.X_Stop == 0))     /* defaults */
  {
    Test = Touch_Adjust();         /* adjust touch screen */

    if (Test == 0)                 /* error */
    {
      LCD_ClearLine2();
      #ifdef UI_CENTER_ALIGN
        Display_CenterLine(1);               /* center block: 1 line */
        Display_EEString_Center(Error_str);  /* display: Error */
      #else
        Display_EEString(Error_str);    /* display: Error */
      #endif
      MilliSleep(1000);                 /* smooth UI */
      TestKey(2500, CURSOR_BLINK | CHECK_OP_MODE | CHECK_BAT);
    }
  }
  #endif

  #ifdef UI_CHOOSE_PROFILE
  /* select adjustment profile */
  AdjustmentMenu(STORAGE_LOAD | STORAGE_SHORT);
  #endif


  /*
   *  interrupts
   */

  sei();                           /* enable interrupts */


  /*
   *  main processing loop (probing cycle)
   */

cycle_start:

  /* reset variables */
  Check.Found = COMP_NONE;         /* no component */
  Check.Type = 0;                  /* reset type flags */
  Check.Done = DONE_NONE;          /* no transistor */
  Check.AltFound = COMP_NONE;      /* no alternative component */
  Check.Diodes = 0;                /* reset diode counter */
  Check.Resistors = 0;             /* reset resistor counter */
  Semi.Flags = 0;                  /* reset flags */
  Semi.U_1 = 0;                    /* reset values */
  Semi.U_2 = 0;
  Semi.U_3 = 0;
  Semi.F_1 = 0;
  #ifdef SW_REVERSE_HFE
  Semi.F_2 = 0;
  #endif
  Semi.I_value = 0;
  AltSemi.U_1 = 0;
  AltSemi.U_2 = 0;
  #ifdef UI_SERIAL_COMMANDS
  Info.Quantity = 0;               /* zero components */
  Info.Selected = 1;               /* select first component */
  Info.Flags = INFO_NONE;          /* reset flags */
  Info.Comp1 = NULL;               /* reset pointer to first component */
  Info.Comp2 = NULL;               /* reset pointer to second component */
  #endif
  #ifdef HW_KEYS
  UI.KeyOld = KEY_NONE;            /* no key */
  UI.KeyStepOld = 1;               /* step size 1 */
  #endif
  #ifdef SW_SYMBOLS
  UI.SymbolLine = 3;               /* default: line #3 */
  #endif

  /* reset hardware */
  ADC_DDR = 0;                     /* set all pins of ADC port as input */
  #ifdef HW_DISCHARGE_RELAY
    /*
     *  This also enables the discharge relay via the external reference
     *  and removes the short circuit.
     */
  #endif

  UI.LineMode = LINE_KEEP;              /* next-line mode: keep first line */
  LCD_Clear();                          /* clear LCD */


  /*
   *  voltage references
   */

  CheckVoltageRefs();                   /* manage voltage references */


  /*
   *  battery check (default display)
   */

  #if defined (BAT_NONE) || defined (UI_BATTERY_LASTLINE)
    /* no battery monitoring */
    Display_EEString(Tester_str);       /* display (line #1): Component Tester */
  #else
    /* battery monitoring */
    CheckBattery();                     /* check battery voltage */
                                        /* will power off on low battery */
    ShowBattery();                      /* display (line #1) battery status */
  #endif


  /*
   *  probing
   */

  #ifdef UI_SERIAL_COMMANDS
  /* skip first probing after power-on */
  if (Key == KEY_POWER_ON)         /* first cycle */
  {
    goto cycle_control;            /* skip probing */
    /* will also change Key */
  }
  #endif

  /* display start of probing */
  #ifdef UI_CENTER_ALIGN
    Display_CenterLine(1);                   /* center block: 1 line */
    /* move text left by one char for optional ' C' */
    UI.CharMax_X--;                          /* simulate shorter line */
    Display_EEString_Center(Probing_str);    /* display: probing... */
    UI.CharMax_X++;                          /* original line size */
  #else
    Display_NL_EEString(Probing_str);        /* display (line #2): probing... */
  #endif

  /* try to discharge any connected component */
  DischargeProbes();
  if (Check.Found == COMP_ERROR)   /* discharge failed */
  {
    goto show_component;           /* skip all other checks */
  }

  #ifdef UI_SHORT_CIRCUIT_MENU
  /* enter main menu if requested by short-circuiting all probes */
  if (ShortedProbes() == 3)        /* all probes short-circuited */
  {
    Key = KEY_MAINMENU;            /* trigger main menu */
    goto cycle_action;             /* perform action */
  }
  #endif

  /* check all 6 combinations of the 3 probes */
  CheckProbes(PROBE_1, PROBE_2, PROBE_3);
  CheckProbes(PROBE_2, PROBE_1, PROBE_3);
  CheckProbes(PROBE_1, PROBE_3, PROBE_2);
  CheckProbes(PROBE_3, PROBE_1, PROBE_2);
  CheckProbes(PROBE_2, PROBE_3, PROBE_1);
  CheckProbes(PROBE_3, PROBE_2, PROBE_1);

  CheckAlternatives();             /* process alternatives */
  SemiPinDesignators();            /* manage semi pin designators */

  /* if component might be a capacitor */
  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    /* check for capacitors */

    /* tell user to be patient with large caps :) */
    Display_Space();
    Display_Char('C');    

    /* check all possible combinations */
    MeasureCap(PROBE_3, PROBE_1, 0);
    MeasureCap(PROBE_3, PROBE_2, 1);
    MeasureCap(PROBE_2, PROBE_1, 2);
  }

  #ifdef HW_PROBE_ZENER
  /* when no component is found check for Zener diode */
  if (Check.Found == COMP_NONE)
  {
    CheckZener();
  }
  #endif


  /*
   *  output test results
   */

show_component:

  LCD_Clear();                     /* clear LCD */

  /* next-line mode */
  Test = LINE_KEEP | LINE_KEY;     /* keep first line and wait for key/timeout */
  #ifdef UI_SERIAL_COMMANDS
  if (Key == KEY_PROBE)            /* probing by command */
  {
    Test = LINE_KEEP;              /* don't wait for key/timeout */
  }
  #endif
  UI.LineMode = Test;              /* change mode */

  #ifdef UI_SERIAL_COPY
  Display_Serial_On();             /* enable serial output & NL */
  #endif

  #ifdef UI_SERIAL_COMMANDS
  if (Check.Found >= COMP_RESISTOR)
  {
    Info.Quantity = 1;             /* got one at least */
  }
  #endif

  #ifdef UI_PROBING_DONE_BEEP
    /* buzzer: short beep for probing result (probing done) */
    #ifdef BUZZER_ACTIVE
    BUZZER_PORT |= (1 << BUZZER_CTRL);       /* enable: set pin high */
    MilliSleep(20);                          /* wait for 20 ms */
    BUZZER_PORT &= ~(1 << BUZZER_CTRL);      /* disable: set pin low */
    #endif

    #ifdef BUZZER_PASSIVE
    PassiveBuzzer(BUZZER_FREQ_LOW);          /* low frequency beep */
    #endif
  #endif

  #ifdef UI_AUTOHOLD_FOUND
  /* when in continuous mode and some component is found */
  if ((! (Cfg.OP_Mode & OP_AUTOHOLD)) &&
      (Check.Found >= COMP_RESISTOR))
  {
    /* set flag and switch temporarily to auto-hold mode */
    Cfg.OP_Mode |= OP_AUTOHOLD_TEMP | OP_AUTOHOLD;
  }
  #endif

  /* call output function based on component type */
  switch (Check.Found)
  {
    case COMP_ERROR:          /* error */
      Show_Error();
      break;

    case COMP_DIODE:          /* diode */
      Show_Diode();
      break;

    case COMP_BJT:            /* BJT */
      Show_BJT();
      break;

    case COMP_FET:            /* FET */
      Show_FET();
      break;

    case COMP_IGBT:           /* IGBT */
      Show_IGBT();
      break;

    case COMP_THYRISTOR:      /* Thyristor/SCR */
      Show_ThyristorTriac();
      break;

    case COMP_TRIAC:          /* TRIAC */
      Show_ThyristorTriac();
      break;

    case COMP_PUT:            /* PUT */
      Show_PUT();
      break;

    #ifdef SW_UJT
    case COMP_UJT:            /* UJT */
      Show_UJT();
      break;
    #endif

    case COMP_RESISTOR:       /* resistor(s) */
      Show_Resistor();
      break;

    case COMP_CAPACITOR:      /* capacitor(s) */
      Show_Capacitor();
      break;

    #ifdef HW_PROBE_ZENER
    case COMP_ZENER:          /* Zener diode */
      Show_Zener();
      break;
    #endif

    default:                  /* no component found */
      Show_Fail();
      break;
  }

  #ifdef UI_SERIAL_COPY
  Display_Serial_Off();            /* disable serial output & NL */
  #endif

  #ifdef SW_SYMBOLS
  /* display fancy pinout for 3-pin semiconductors */
  if (Check.Found >= COMP_BJT)     /* 3-pin semi */
  {
    if (UI.SymbolLine)             /* not zero */
    {
      Display_FancySemiPinout(UI.SymbolLine);     /* display pinout */
    }
  }
  #endif

  #if ! defined (BAT_NONE) && defined (UI_BATTERY_LASTLINE)
  /* alternative display of battery status in last line */
  CheckBattery();                  /* check battery voltage */
                                   /* will power off on low battery */
  Display_LastLine();              /* manage last line */
  LCD_CharPos(1, UI.CharMax_Y);    /* move to start of last line */
  ShowBattery();                   /* display battery status */
  #endif

  #ifdef UI_SERIAL_COMMANDS
  /* feedback for remote command 'PROBE' */
  if (Key == KEY_PROBE)       /* probing by command */
  {
    Display_Serial_Only();              /* switch output to serial */
    Display_EEString_NL(Cmd_OK_str);    /* send: OK & newline */
    Display_LCD_Only();                 /* switch output back to display */

    /* We don't have to restore the next-line mode since it will be 
       changed a few lines down anyway. */
  }
  #endif

  #if CYCLE_MAX < 255
  /* component was found */
  if (Check.Found >= COMP_RESISTOR)
  {
    MissedParts = 0;          /* reset counter */
  }
  #endif


  /*
   *  manage cycling and power-off
   */

cycle_control:

  #ifdef HW_DISCHARGE_RELAY
  /* discharge relay: short circuit probes */
                                   /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);         /* disable relay */
  #endif

  #ifdef SERIAL_RW
  Serial_Ctrl(SER_RX_RESUME);      /* enable TTL serial RX */
  #endif

  UI.LineMode = LINE_STD;          /* reset next-line mode */

  /* wait for key press or timeout */
  #ifdef UI_KEY_HINTS
    Display_LastLine();
    UI.KeyHint = (unsigned char *)Menu_or_Test_str;
    Key = TestKey((uint16_t)CYCLE_DELAY, CURSOR_BLINK | CURSOR_TEXT | CHECK_OP_MODE | CHECK_KEY_TWICE | CHECK_BAT);
  #else
    Key = TestKey((uint16_t)CYCLE_DELAY, CURSOR_BLINK | CHECK_OP_MODE | CHECK_KEY_TWICE | CHECK_BAT);
  #endif

  if (Key == KEY_TIMEOUT)          /* timeout (no key press) */
  {
    /* implies continuous mode */

    #if CYCLE_MAX < 255
    /* check if we reached the maximum number of missed parts in a row */
    if (MissedParts >= CYCLE_MAX)
    {
      Key = KEY_POWER_OFF;         /* signal power off */
    }
    #endif
  }
  else if (Key == KEY_TWICE)       /* two short key presses */
  {
    Key = KEY_MAINMENU;            /* signal main menu */
  }
  else if (Key == KEY_LONG)        /* long key press */
  {
    Key = KEY_POWER_OFF;           /* signal power off */
  }
  #ifdef HW_KEYS
  else if (Key == KEY_LEFT)        /* rotary encoder: left turn */
  {
    Key = KEY_MAINMENU;            /* signal main menu */
  }
  #endif
  #ifdef SERIAL_RW
  else if (Key == KEY_COMMAND)     /* remote command */
  {
    #ifdef UI_SERIAL_COMMANDS
    Key = KEY_NONE;                /* reset key */
    Display_Serial_Only();         /* switch output to serial */
    Test = GetCommand();           /* get command */
    if (Test != CMD_NONE)          /* valid command */
    {
      Key = RunCommand(Test);      /* run command */
    }
    Display_LCD_Only();            /* switch output back to display */

    /* if we got a virtual key perform requested action */
    if (Key != KEY_NONE) goto cycle_action;
    #endif

    goto cycle_control;            /* re-run cycle control */
  }
  #endif

#if defined (UI_SHORT_CIRCUIT_MENU) || defined (UI_SERIAL_COMMANDS)
cycle_action:
#endif

  #ifdef SERIAL_RW
  Serial_Ctrl(SER_RX_PAUSE);       /* disable TTL serial RX */
  /* todo: when we got a locked buffer meanwhile? */
  #endif

  #ifdef UI_AUTOHOLD_FOUND
  /* when temporary auto-hold is enabled */
  if (Cfg.OP_Mode & OP_AUTOHOLD_TEMP)
  {
    /* clear flag and switch back to continuous mode */
    Cfg.OP_Mode &= ~(OP_AUTOHOLD_TEMP | OP_AUTOHOLD);
  }
  #endif


  if (Key == KEY_MAINMENU)         /* run main menu */
  {
    #ifdef SAVE_POWER
    /* change sleep mode the Idle to keep timers & other stuff running */
    Test = Cfg.SleepMode;               /* get current mode */
    Cfg.SleepMode = SLEEP_MODE_IDLE;    /* change sleep mode to Idle */
    #endif

    #ifdef HW_DISCHARGE_RELAY
    /* discharge relay: remove short circuit */
    ADC_DDR = 0;                   /* enable relay (via external reference) */
    /* todo: move this to MainMenu()? (after selecting item) */
    #endif

    #ifdef UI_MAINMENU_AUTOEXIT
      /* run main menu once and return to probe cycle */
      MainMenu();                  /* enter main menu */
    #else
      /* run main menu until explicit exit */
      while (MainMenu() != KEY_EXIT)
      {
        /* keep running main menu */
      }
    #endif

    #ifdef SAVE_POWER
    /* change sleep mode back */
    Cfg.SleepMode = Test;          /* change sleep mode back */
    #endif

    goto cycle_control;            /* re-run cycle control */
  }
  else if (Key == KEY_POWER_OFF)   /* power off */
  {
    PowerOff();                    /* power off */
  }
  else                             /* default action */
  {
    goto cycle_start;              /* next round */
  }

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

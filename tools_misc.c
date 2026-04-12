/* ************************************************************************
 *
 *   misc tools (hardware and software options)
 *
 *   (c) 2012-2024 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define TOOLS_MISC_C



/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */



/* ************************************************************************
 *   support functions
 * ************************************************************************ */


#ifdef FUNC_PROBE_PINOUT

/*
 *  display probe pinout
 *
 *  required:
 *  - Mode
 *    PROBES_PWM         PWM signal
 *    PROBES_ESR         ESR measurement
 *    PROBES_RCL         monitoring RCL
 *    PROBES_RINGTESTER  ring tester
 *    PROBES_DIODE       diode
 */

void ProbePinout(uint8_t Mode)
{
  uint8_t           Char1 = 0;     /* designator for probe #1 */
  uint8_t           Char2 = 0;     /* designator for probe #2 */
  uint8_t           Char3 = 0;     /* designator for probe #3 */

  LCD_ClearLine2();                /* info goes to line #2 */


  /*
   *  set probe pinout based on mode
   */

  switch (Mode)
  {
    #ifdef SW_PROBEPINOUT_PWM
    case PROBES_PWM:
      /* probe #1: Gnd / probe #2: signal / probe #3: Gnd */
      Char1 = '-';
      Char2 = 's';
      Char3 = '-';
      break;
    #endif

    #ifdef SW_PROBEPINOUT_ESR
    case PROBES_ESR:
      /* probe #1: + / probe #3: - */
      Char1 = '+';
      Char2 = 0;
      Char3 = '-';
      break;
    #endif

    #ifdef SW_PROBEPINOUT_RCL
    case PROBES_RCL:
      /* probe #1: * / probe #3: * */
      Char1 = '*';
      Char2 = 0;
      Char3 = '*';
      break;
    #endif

    #if defined (HW_RING_TESTER) && defined (RING_TESTER_PROBES)
    case PROBES_RINGTESTER:
      /* probe #1: Vcc / probe #2: pulse out / probe #3: Gnd */
      Char1 = '+';
      Char2 = 'p';
      Char3 = '-';
      break;
    #endif

    #if defined (SW_PHOTODIODE)
    case PROBES_DIODE:
      /* probe #1: Anode / probe #3: Cathode */
      Char1 = 'A';
      Char2 = 0;
      Char3 = 'C';
      break;
    #endif
  }

  Show_SimplePinout(Char1, Char2, Char3);    /* display pinout */

  /* wait for any key press or 5s */
  TestKey(5000, CHECK_BAT);
  LCD_ClearLine2();                /* clear line #2 */
}

#endif



/* ************************************************************************
 *   Zener tool / external voltage
 * ************************************************************************ */


#if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)

/*
 *  Zener tool (standard mode)
 *  - hardware option for voltage measurement of Zener diode 
 *  - uses dedicated analog input (TP_ZENER) with voltage divider
 *    (default 10:1)
 *  - boost converter enabled by test push button (default) or
 *    dedicated I/O pin (switched mode)
 */

void Zener_Tool(void)
{
  uint8_t                Run = 1;            /* control flag */
  uint8_t                Counter;            /* length of key press */
  uint8_t                Counter2 = 0;       /* time between two key presses */
  uint16_t               U1;                 /* current voltage */
  uint16_t               Min = UINT16_MAX;   /* minimal voltage */
  #ifdef ZENER_DIVIDER_CUSTOM
  uint32_t               Value;              /* value */
  #endif

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Zener */
    Display_ColoredEEString(Zener_str, COLOR_TITLE);
  #else
    Display_EEString(Zener_str);   /* display: Zener */
  #endif
  Display_NextLine();              /* line #2 */
  Display_Minus();                 /* display "no value" */
  #ifdef UI_ZENER_DIODE
  /* display Zener diode symbol */
  Check.Symbol = SYMBOL_DIODE_ZENER;    /* set symbol ID */
  Display_FancySemiPinout(3);           /* show symbol starting in line #3 */
  #endif


  /*
   *  processing loop
   */

  while (Run > 0)
  {
    /*
     *  manage timing
     */

    Counter = 0;              /* reset key press time */
    MilliSleep(30);           /* delay by 30ms */
    Counter2++;               /* increase delay time counter */
    if (Counter2 > 200)       /* prevent overflow & timer (about 6s) */
    {
      Counter2 = 10;          /* reset counter (above delay for quick key presses) */
      #ifndef BAT_NONE
      CheckBattery();         /* and check battery */
      #endif
    }


    /*
     *  key press triggers measurement
     *  - also enables boost converter via hardware
     *  - two short key presses exit tool
     */

    while (!(BUTTON_PIN & (1 << TEST_BUTTON)))    /* as long as key is pressed */
    {
      #ifdef ZENER_SWITCHED
      /* turn on boost converter */
        #ifdef ZENER_BOOST_HIGH
          /* high active */
          BOOST_PORT |= (1 << BOOST_CTRL);        /* set pin high */
        #else
          /* low active */
          BOOST_PORT &= ~(1 << BOOST_CTRL);       /* set pin low */
        #endif
      #endif

      /* get voltage (10:1 voltage divider) */
      U1 = ReadU(TP_ZENER);        /* read voltage (in mV) */

      #ifndef ZENER_DIVIDER_CUSTOM
      /* ADC pin is connected to a 10:1 voltage divider */
      /* so U1's scale is 10mV */
      #endif

      #ifdef ZENER_DIVIDER_CUSTOM
      /*
       *  ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
       *  - U2 = (Uin / (R1 + R2)) * R2 
       *  - Uin = (U2 * (R1 + R2)) / R2
       */

      Value = (((uint32_t)(ZENER_R1 + ZENER_R2) * 1000) / ZENER_R2);  /* factor (0.001) */
      Value *= U1;                     /* voltage (0.001 mV) */
      Value /= 1000;                   /* scale to mV */
      U1 = (uint16_t)Value;            /* keep 2 bytes */
      #endif

      /* display voltage */
      if (Counter % 8 == 0)        /* every 8 loop runs (240ms) */
      {
        LCD_ClearLine2();               /* clear line #2 */
        #ifndef ZENER_DIVIDER_CUSTOM
          Display_Value(U1, -2, 'V');   /* display current voltage (10mV) */
        #else
          Display_Value(U1, -3, 'V');   /* display current voltage (1mV) */
        #endif
      }

      /* data hold */
      if (Counter == 0)            /* first loop run */
      {
        Min = UINT16_MAX;          /* reset minimum to default */
      }
      else if (Counter >= 10)      /* ensure stable voltage */
      {
        if (U1 < Min) Min = U1;    /* update minimum */
      }

      /* timing */
      MilliSleep(30);              /* delay next run / also debounce by 30ms */
      Counter++;                   /* increase key press time counter */
      if (Counter > 100)           /* prevent overflow & timer (about 3s) */
      {
        Counter = 12;              /* reset counter (above time for short key press) */
        #ifndef BAT_NONE
        CheckBattery();            /* and check battery */
        #endif
      }
    }

    #ifdef ZENER_SWITCHED
    /* turn off boost converter */
      #ifdef ZENER_BOOST_HIGH
        /* high active */
        BOOST_PORT &= ~(1 << BOOST_CTRL);    /* set pin low */
      #else
        /* low active */
        BOOST_PORT |= (1 << BOOST_CTRL);     /* set pin high */
      #endif
    #endif


    /*
     *  user interface logic
     */

    if (Counter > 0)                         /* key was pressed */
    {
      /* detect two quick key presses */
      if (Run == 2)                          /* flag for short key press set */
      {
        if (Counter2 <= 8)                   /* short delay between key presses <= 250ms */
        {
          Run = 0;                           /* end loop */
        }
        else                                 /* long delay between key presses */
        {
          Run = 1;                           /* reset flag */
        }
      }
      else                                   /* flag not set */
      {
        if (Counter <= 10)                   /* short key press <= 300ms */
        {
          Run = 2;                           /* set flag */
        }
      }

      /* display hold value */
      LCD_ClearLine2();

      if (Min != UINT16_MAX)       /* got updated value */
      {
        #ifndef ZENER_DIVIDER_CUSTOM
          Display_Value(Min, -2, 'V');     /* display minimal voltage */
        #else
          Display_Value(Min, -3, 'V');     /* display minimal voltage */
        #endif
        Display_Space();
        Display_EEString(Min_str);         /* display: Min */
      }
      else                         /* unchanged default */
      {
        Display_Minus();                   /* display "no value" */
      }

      Counter2 = 0;                /* reset delay time */
    }
  }
}

#endif



#if defined (HW_ZENER) && defined (ZENER_UNSWITCHED)

/*
 *  Zener tool (alternative mode: unswitched boost converter)
 *  - hardware option for voltage measurement of Zener diode
 *    or external voltage
 *  - uses dedicated analog input (TP_ZENER) with voltage divider
 *    (default 10:1)
 *  - boost converter runs all the time or circuit without boost
 *    converter (just measuring external voltage)
 */

void Zener_Tool(void)
{
  uint8_t                Run = 1;       /* control flag */
  uint8_t                Test;          /* user feedback */
  uint16_t               U1;            /* voltage */
  #ifdef ZENER_DIVIDER_CUSTOM
  uint32_t               Value;         /* value */
  #endif

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Zener */
    Display_ColoredEEString(Zener_str, COLOR_TITLE);
  #else
    Display_EEString(Zener_str);   /* display: Zener */
  #endif
  #ifdef UI_ZENER_DIODE
  /* display Zener diode symbol */
  Check.Symbol = SYMBOL_DIODE_ZENER;    /* set symbol ID */
  Display_FancySemiPinout(3);           /* show symbol starting in line #3 */
  #endif


  /*
   *  processing loop
   */

  while (Run)
  {
    /* get voltage */
    U1 = ReadU(TP_ZENER);          /* read voltage (in mV) */

    #ifndef ZENER_DIVIDER_CUSTOM
    /* ADC pin is connected to a 10:1 voltage divider */
    /* so U1's scale is 10mV */
    #endif

    #ifdef ZENER_DIVIDER_CUSTOM
    /*
     *  ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
     *  - U2 = (Uin / (R1 + R2)) * R2 
     *  - Uin = (U2 * (R1 + R2)) / R2
     */

    Value = (((uint32_t)(ZENER_R1 + ZENER_R2) * 1000) / ZENER_R2);  /* factor (0.001) */
    Value *= U1;                     /* voltage (0.001 mV) */
    Value /= 1000;                   /* scale to mV */
    U1 = (uint16_t)Value;            /* keep 2 bytes */
    #endif

    /* display voltage */
    LCD_ClearLine2();              /* clear line #2 */
    #ifndef ZENER_DIVIDER_CUSTOM
      Display_Value(U1, -2, 'V');  /* display current voltage */
    #else
      Display_Value(U1, -3, 'V');  /* display current voltage */
    #endif

    /* user feedback (1s delay) */
    Test = TestKey(1000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Run = 0;                     /* end processing loop */
    }
  }
}

#endif



#ifdef HW_PROBE_ZENER

/*
 *  check for Zener diode
 *  - hardware option for voltage measurement of Zener diode
 *    or external voltage
 *  - uses dedicated analog input (TP_ZENER) with voltage divider
 *    (default 10:1)
 *  - boost converter runs all the time, circuit without boost
 *    converter (just measuring external voltage), or boost converter
 *    switched by dedicated I/O pin
 */

void CheckZener(void)
{
  uint16_t               U1;            /* voltage */
  #ifdef ZENER_DIVIDER_CUSTOM
  uint32_t               Value;         /* value */
  #endif

  #ifdef ZENER_SWITCHED
  /* turn on boost converter */
    #ifdef ZENER_BOOST_HIGH
      /* high active */
      BOOST_PORT |= (1 << BOOST_CTRL);       /* set pin high */
    #else
      /* low active */
      BOOST_PORT &= ~(1 << BOOST_CTRL);      /* set pin low */
    #endif
  MilliSleep(300);                           /* time for stabilization */
  #endif

  /* get voltage */
  U1 = ReadU(TP_ZENER);            /* read voltage (in mV) */

  #ifdef ZENER_SWITCHED
  /* turn off boost converter */
    #ifdef ZENER_BOOST_HIGH
      /* high active */
      BOOST_PORT &= ~(1 << BOOST_CTRL);      /* set pin low */
    #else
      /* low active */
      BOOST_PORT |= (1 << BOOST_CTRL);       /* set pin high */
    #endif
  #endif

  #ifndef ZENER_DIVIDER_CUSTOM
  /* ADC pin is connected to a 10:1 voltage divider */
  U1 *= 10;                        /* voltage (mV) */
  #endif

  #ifdef ZENER_DIVIDER_CUSTOM
  /*
   *  ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
   *  - U2 = (Uin / (R1 + R2)) * R2 
   *  - Uin = (U2 * (R1 + R2)) / R2
   */

  Value = (((uint32_t)(ZENER_R1 + ZENER_R2) * 1000) / ZENER_R2);  /* factor (0.001) */
  Value *= U1;                     /* voltage (0.001 mV) */
  Value /= 1000;                   /* scale to mV */
  U1 = (uint16_t)Value;            /* keep 2 bytes */
  #endif

  /* check for valid voltage */
  if ((U1 >= ZENER_VOLTAGE_MIN) && (U1 <= ZENER_VOLTAGE_MAX))
  {
    Check.Found = COMP_ZENER;      /* we got a Zener */
    Semi.U_1 = U1;                 /* save voltage (V_Z) */
  }
}

#endif



/* ************************************************************************
 *   ESR tool
 * ************************************************************************ */


#ifdef SW_ESR_TOOL

/*
 *  ESR tool
 *  - uses probe #1 (pos) and probe #3 (neg)
 */

void ESR_Tool(void)
{
  uint8_t           Run = 1;       /* control flag */
  uint8_t           Test;          /* temp. value */
  Capacitor_Type    *Cap;          /* pointer to cap */
  uint16_t          ESR;           /* ESR (in 0.01 Ohms) */

  Check.Diodes = 0;                /* disable diode check in cap measurement */
  Cap = &Caps[0];                  /* pointer to first cap */

  #ifdef HW_DISCHARGE_RELAY
  /* discharge relay: short circuit probes */
                                   /* ADC_PORT should be 0 */
  ADC_DDR = (1 << TP_REF);         /* disable relay */
  #endif

  /* show tool info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: ESR */
    Display_ColoredEEString(ESR_str, COLOR_TITLE);
  #else
    Display_EEString(ESR_str);     /* display: ESR */
  #endif
  ProbePinout(PROBES_ESR);         /* show probes used */
  Display_Minus();                 /* display "no value" */

  while (Run > 0)
  {
    /*
     *  short or long key press -> measure
     *  two short key presses -> exit tool
     */

    /* wait for user feedback */
    Test = TestKey(0, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)              /* two short key presses */
    {
      Run = 0;                          /* end loop */
    }

    /* measure cap */
    if (Run > 0)                        /* key pressed */
    {
      #ifdef HW_DISCHARGE_RELAY
      /* discharge relay: remove short circuit */
                                        /* ADC_PORT should be 0 */
      ADC_DDR = 0;                      /* enable relay (via extrenal reference) */
      #endif

      LCD_ClearLine2();                 /* update line #2 */
      Display_EEString(Probing_str);    /* display: probing... */
      Check.Found = COMP_NONE;          /* no component */
      MeasureCap(PROBE_1, PROBE_3, 0);  /* probe-1 = Vcc, probe-3 = Gnd */
      LCD_ClearLine2();                 /* update line #2 */
      
      if (Check.Found == COMP_CAPACITOR)     /* found capacitor */
      {
        /* show capacitance */
        Display_Value(Cap->Value, Cap->Scale, 'F');

        /* show ESR */
        Display_Space();
        ESR = MeasureESR(Cap);
        if (ESR < UINT16_MAX)           /* got valid ESR */
        {
          Display_Value(ESR, -2, LCD_CHAR_OMEGA);
        }
        else                            /* no ESR */
        {
          Display_Minus();
        }
      }
      else                                   /* no capacitor */
      {
        Display_Minus();
      }

      #ifdef HW_DISCHARGE_RELAY
      ADC_DDR = (1 << TP_REF);          /* short circuit probes */
      #endif
    }
  }

  #ifdef HW_DISCHARGE_RELAY
  ADC_DDR = 0;                     /* remove short circuit */
  #endif
}

#endif



/* ************************************************************************
 *   rotary encoder check
 * ************************************************************************ */


#ifdef SW_ENCODER

/* local constants for direction */
#define DIR_NONE         0b00000000     /* no turn or error */
#define DIR_RIGHT        0b00000001     /* turned to the right */
#define DIR_LEFT         0b00000010     /* turned to the left */


/*
 *  check rotary encoder
 *
 *  requires:
 *  - pointer to encoder history
 */

uint8_t CheckEncoder(uint8_t *History)
{
  uint8_t           Action = DIR_NONE;       /* return value */
  uint8_t           Old_AB;                  /* old AB state */
  uint8_t           AB = 0;                  /* new AB state */
  uint8_t           Dir;                     /* turning direction */
  uint8_t           Steps;                   /* encoder steps */
  uint8_t           Temp;                    /* temporary value */

  /* we assume: probe-1 = A / probe-2 = B / probe-3 = Common */
  /* set up probes: probe-1 -- Rl -- Vcc / probe-2 -- Rl -- Vcc / Gnd -- probe-3 */
  R_PORT = Probes.Rl_1 | Probes.Rl_2;   /* pullup via Rl */
  R_DDR =  Probes.Rl_1 | Probes.Rl_2;   /* enable pull-up resistors */
  ADC_PORT = 0;                         /* pull down directly */
  ADC_DDR = Probes.Pin_3;               /* enable Gnd for probe-3 */
  wait500us();                          /* settle time */

  /* get A & B signals */
  Temp = ADC_PIN;
  if (Temp & Probes.Pin_1) AB = 0b00000010;
  if (Temp & Probes.Pin_2) AB |= 0b00000001;

  R_DDR = 0;                  /* reset probes */
  ADC_DDR = 0;

  /* unpack history */
  Temp = *History;
  Old_AB = Temp & 0b00000011;      /* old AB state, first 2 bits */
  Temp >>=2 ;                      /* move 2 bits */
  Dir = Temp & 0b00000011;         /* direction, next 2 bits */
  Temp >>= 2;                      /* move 2 bits */
  Steps = Temp;                    /* steps, remaining 4 bits */

  /* update state history */
  if (Dir == (DIR_RIGHT | DIR_LEFT))    /* first scan */
  {
    Old_AB = AB;              /* set as last state */
    Dir = DIR_NONE;           /* reset direction */
  }

  /* process signals */
  if (Old_AB != AB)           /* signals changed */
  {
    /* check if only one bit has changed (Gray code) */
    Temp = AB ^ Old_AB;                 /* get bit difference */
    if (!(Temp & 0b00000001)) Temp >>= 1;
    if (Temp == 1)                      /* valid change */
    {
      /* determine direction */
      /* Gray code: 00 01 11 10 */
      Temp = 0b10001101;                /* expected values for a right turn */
      Temp >>= (Old_AB * 2);            /* get expected value by shifting */
      Temp &= 0b00000011;               /* select value */
      if (Temp == AB)                   /* value matches */
        Temp = DIR_RIGHT;               /* turn to the right */
      else                              /* value mismatches */
        Temp = DIR_LEFT;                /* turn to the left */

      /* detection logic */
      if (Temp == Dir)                  /* turn in same direction */
      {
        Steps++;                        /* got another step */

        /* for proper detection we need 4 Gray code steps */
        if (Steps == 4)                 /* got 4 steps */
        {
          LCD_ClearLine2();

          /*
           *  The turning direction determines A and B:
           *  - right: A = Probe #1 / B = Probe #2
           *  - left:  A = Probe #2 / B = Probe #1
           */

          if (Dir == DIR_RIGHT)         /* right */
          {
            Semi.A = Probes.ID_1;
            Semi.B = Probes.ID_2;
          }
          else                          /* left */
          {
            Semi.A = Probes.ID_2;
            Semi.B = Probes.ID_1;
          }

          Semi.C = Probes.ID_3;         /* Common */

          /* display pinout */
          Semi.DesA = 'A';              /* pin designator for A */
          Semi.DesB = 'B';              /* pin designator for B */
          Semi.DesC = 'C';              /* pin designator for C */
          Show_SemiPinout();

          Steps = 0;                      /* reset steps */
          Action = Temp;                  /* signal valid step */
        }
      }
      else                         /* turn has changed direction */
      {
        Steps = 1;                 /* first step for new direction */
      }

      Dir = Temp;                  /* update direction */
    }
    else                                /* invalid change */
    {
      Dir = DIR_RIGHT | DIR_LEFT;       /* trigger reset of history */
    }
  }

  /* pack new history */
  Temp = AB;             /* AB state, first 2 bits */
  Dir <<= 2;             /* direction, next 2 bits */
  Temp |= Dir;
  Steps <<= 4;           /* steps, remaining 4 bits */
  Temp |= Steps;
  *History = Temp;       /* save new history */

  return Action;
}


/*
 *  rotary encoder check
 *  - uses standard probes
 */

void Encoder_Tool(void)
{
  uint8_t      Flag;          /* flag/counter */
  uint8_t      History[3];    /* encoder history */

  /*
   *  History:
   *  - 000000xx AB state
   *  - 0000xx00 turning direction
   *  - xxxx0000 steps               
   */

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Rotary Encoder */
    Display_ColoredEEString(Encoder_str, COLOR_TITLE);
  #else
    Display_EEString(Encoder_str);      /* display: Rotary Encoder */
  #endif

  /* init array */
  for (Flag = 0; Flag <= 2; Flag++)
  {
    History[Flag] = (DIR_RIGHT | DIR_LEFT) << 2;
  }

  /* processing loop */
  Flag = 5;
  while (Flag < 10)
  {
    wdt_reset();

    if (Flag == 5)                 /* ask user to turn */
    {
      LCD_ClearLine2();
      Display_EEString(TurnRight_str);    /* display: Turn right! */
      Flag = 0;                           /* reset flag */
    }

    UpdateProbes(PROBE_1, PROBE_2, PROBE_3);      /* check first pinout */
    Flag = CheckEncoder(&History[0]);

    if (Flag == 0)
    {
      UpdateProbes(PROBE_1, PROBE_3, PROBE_2);    /* check second pinout */
      Flag = CheckEncoder(&History[1]);
    }

    if (Flag == 0)
    {    
      UpdateProbes(PROBE_2, PROBE_3, PROBE_1);    /* check third pinout */
      Flag = CheckEncoder(&History[2]);
    }

    if (Flag > 0)             /* detected encoder */
    {
      /* let the user read or skip the text */
      TestKey(3000, CURSOR_STEADY | CHECK_OP_MODE | CHECK_BAT);
      Flag = 5;                    /* reset flag */
    }
    else                      /* nothing found yet */
    {
      if (!(BUTTON_PIN & (1 << TEST_BUTTON)))     /* if key is pressed */
      {
        MilliSleep(100);           /* smooth UI */
        Flag = 10;                 /* end loop */
      }
    }
  }
}

/* clean up local constants for direction */
#undef DIR_LEFT
#undef DIR_RIGHT
#undef DIR_NONE

#endif



/* ************************************************************************
 *   opto coupler check
 * ************************************************************************ */


#ifdef SW_OPTO_COUPLER

/*
 *  check for LED
 *  - simple wrapper for CheckDiode()
 *
 *  requires:
 *  - Probe1: ID of positive probe (anode)
 *  - Probe2: ID of negative probe (cathode)
 */

void Check_LED(uint8_t Probe1, uint8_t Probe2)
{
  uint16_t          U1;                 /* voltage */

  UpdateProbes2(Probe1, Probe2);        /* update probes */

  /* we assume: probe-1 = A / probe2 = C */
  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_PORT = 0;                      /* set resistor port to Gnd */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */
  ADC_DDR = Probes.Pin_1;          /* set probe-1 to output */
  ADC_PORT = Probes.Pin_1;         /* pull-up probe-1 directly */

  U1 = ReadU_5ms(Probes.Ch_2);     /* voltage at Rl (cathode) */

  if (U1 >= 977)         /*  not just a leakage current (> 1.4mA) */
  {
    CheckDiode();        /* run standard diode check */
  }
}



/*
 *  check opto couplers
 *  - uses standard probes
 *  - pins which have to be connected (common Gnd):
 *    - LED's cathode and BJT's emitter 
 *    - LED's cathode and TRIAC's MT2
 *  - supports:
 *    - BJT
 *    - TRIAC (with and without zero crossing circuit)
 */

void OptoCoupler_Tool(void)
{
  uint8_t           Run = 1;            /* loop control */
  uint8_t           Test;               /* user input */
  uint16_t          U1, U2;             /* voltages */
  uint16_t          U3, U4;             /* voltages */
  uint32_t          CTR = 0;            /* CTR in % */

  /* local constants for status */
  #define DETECTED_LED        50
  #define DETECTED_BJT       100
  #define DETECTED_TRIAC     101

  /* init */
  /* next-line mode: keep first line and wait for key/timeout */
  UI.LineMode = LINE_KEEP | LINE_KEY;

  /* display info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Opto Coupler */
    Display_ColoredEEString(OptoCoupler_str, COLOR_TITLE);
  #else
    Display_EEString(OptoCoupler_str);  /* display: Opto Coupler */
  #endif
  Display_NL_EEString(Start_str);       /* display: Start */


  /*
   *  processing loop
   */

  while (Run)
  {
    /* user input */

    /* wait for user feedback */
    Test = TestKey(0, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Run = 0;                     /* end loop */
    }

    if (Run)                       /* check opto coupler */
    {
      /* update display */
      LCD_Clear();
      #ifdef UI_SERIAL_COPY
      Display_Serial_On();         /* enable serial output & NL */
      #endif
      #ifdef UI_COLORED_TITLES
        /* display: Opto Coupler */
        Display_ColoredEEString(OptoCoupler_str, COLOR_TITLE);
      #else
        Display_EEString(OptoCoupler_str);   /* display: Opto Coupler */
      #endif
      Display_NextLine();
      Test = 0;                    /* reset status */

      /*
       *  scan for LED
       */

      Check.Found = COMP_NONE;          /* reset component search */
      Check.Diodes = 0;                 /* reset number of diodes */

      /* check all possible probe combinations */
      Check_LED(PROBE_1, PROBE_2);
      Check_LED(PROBE_2, PROBE_1);
      Check_LED(PROBE_1, PROBE_3);
      Check_LED(PROBE_3, PROBE_1);
      Check_LED(PROBE_2, PROBE_3);
      Check_LED(PROBE_3, PROBE_2);

      if (Check.Diodes == 1)       /* got one */
      {
        /* update probes for remaining checks (3rd probe is done automatically) */
        UpdateProbes2(Diodes[0].A, Diodes[0].C);

        Test = DETECTED_LED;            /* proceed with other checks */
      }


      /*
       *  we assume:
       *  probe-1 = LED's anode
       *  probe-2 = LED's cathode & BJT's emitter or TRIAC's MT2
       *  probe-3 = BJT's collector or TRIAC's MT1
       */


      /*
       *  check for BJT and TRIAC
       *  - BJT conducts only while LED is lit.
       *  - TRIAC keeps conducting as long as load current flows.
       *    Some types with zero crossing circuit got an inhibit voltage
       *    of about 5V.
       */

      if (Test == DETECTED_LED)         /* LED detected */
      {
        /* set probes: probe-2 -- Gnd / probe-3 -- Rl -- Vcc */
        ADC_DDR = Probes.Pin_2;              /* set probe-2 to output */
        ADC_PORT = 0;                        /* pull down probe-2 directly */
        R_DDR = Probes.Rl_1 | Probes.Rl_3;   /* select Rl for probe-1 & Rl for probe-3 */
        R_PORT = Probes.Rl_3;                /* pull up collector via Rl */
        U1 = ReadU_5ms(Probes.Ch_3);         /* voltage at collector when LED is off */

        /* make sure we have no conduction without the LED lit */
        if (U1 > 4000)        /* allow a leakage current of 1.5mA */
        {
          /* simulate zero crossing in case of a TRIAC with zero crossing circuit */
          R_PORT = Probes.Rl_1;                /* turn on LED */
          wait1ms();                           /* wait a tad */
          R_PORT = Probes.Rl_1 | Probes.Rl_3;  /* also pull up collector via Rl */
          U1 = ReadU_5ms(Probes.Ch_3);         /* voltage at collector when LED is on */

          R_PORT = Probes.Rl_3;                /* turn off LED */
          U2 = ReadU_5ms(Probes.Ch_3);         /* voltage at collector when LED is off */

          /* we should have conduction when the LED is lit */
          if (U1 <= 4000)          /* more than 1.5mA */
          {
            if (U2 >= 4000)        /* no conduction, allow some leakage current */
            {
              Test = DETECTED_BJT;      /* BJT type */
            }
            else                   /* conduction */
            {
              /* check if both voltages are about the same */
              U3 = U1;
              U3 /= 8;             /* 12.5% */
              U4 = U1 - U3;        /* lower threshold */
              U3 += U1;            /* upper threshold */
              if ((U2 > U4) && (U2 < U3))
              {
                Test = DETECTED_TRIAC;  /* TRIAC type */
              }
            }
          }
        }

        R_DDR = Probes.Rl_1;                 /* set probe-3 to HiZ */
      }


      /*
       *  measure CRT for BJT type
       */

      if (Test == DETECTED_BJT)         /* BJT type */
      {
        /* change probes: probe-3 -- Vcc */
        ADC_DDR = Probes.Pin_2 | Probes.Pin_3;    /* set probe-3 to output */
        ADC_PORT = Probes.Pin_3;                  /* pull up probe-3 directly */

        /* get voltages at current shunts */
        Cfg.Samples = 10;               /* just a few samples for 1ms runtime */
        R_PORT = Probes.Rl_1;           /* turn LED on */
        wait1ms();                      /* time for propagation delay */
        U1 = ReadU(Probes.Ch_1);        /* voltage at LED's anode (Rl) */
        U2 = ReadU(Probes.Ch_2);        /* voltage at emitter (RiL) */
        R_PORT = 0;                     /* turn LED off */
        Cfg.Samples = ADC_SAMPLES;      /* reset samples to default */

        /* calculate LED's If */
        /* If = (Vcc - U1) / (RiH + Rl) */
        U3 = Cfg.Vcc - U1;              /* Vcc - U1 (mV) */
        CTR = (uint32_t)U3;
        CTR *= 10000;                   /* scale to 0.0001 mV */
        U4 = NV.RiH + (R_LOW * 10);     /* RiH + Rl (0.1 Ohms) */
        CTR /= U4;                      /* If = U/R in µA */
        U3 = (uint16_t)CTR;             /* If in µA */

        /* calculate BJT's Ie */
        /* Ie = I_total - If = (U2 / RiL) - If */
        CTR = (uint32_t)U2;             /* U2 (mV) */
        CTR *= 10000;                   /* scale to 0.0001 mV */
        CTR /= NV.RiL;                  /* /RiL in 0.1 Ohms -> I_total (µA) */ 
        CTR -= U3;                      /* Ie = I_total - If (µA) */

        /* calculate CTR */
        /* CTR = Ie / If */
        CTR *= 100;                     /* scale up to % */
        CTR /= U3;                      /* Ie / If (%) */
      }


      /*
       *  Measure turn-on and turn-off times
       *  - Unfortunately we can't use the analog comparator in conjunction
       *    with Timer1, because the 1.1V bandgap reference would limit the
       *    time measurement to opto couplers with a CTR > 200%.
       */

      if (Test == DETECTED_BJT)    /* BJT type */
      {
        U1 = UINT16_MAX;           /* reset value */
        U2 = UINT16_MAX;

        ADC_DDR = Probes.Pin_2;              /* set probe-2 to output */
        ADC_PORT = 0;                        /* pull down probe-2 directly */
        R_DDR = Probes.Rl_1 | Probes.Rl_3;   /* select Rl for probe-1 & Rl for probe-3 */
        R_PORT = Probes.Rl_3;                /* pull up collector via Rl */

        U1 = ReadU_5ms(Probes.Ch_3);         /* voltage at collector when LED is off */

        /* make sure we have no conduction without the LED lit */
        if (U1 > 4000)        /* allow a leakage current of 1.5mA */
        {
          Test = Probes.Pin_3;     /* port pin mask for probe-3 */

          /*
           *  turn-on delay
           */

          Run = 0;                                /* zero counter */
          R_PORT = Probes.Rl_1 | Probes.Rl_3;     /* turn on LED */

          /*
           *  wait for logic low level (<2.0V)
           *  - MCU cycles for full loop run: 7
           */

          while (ADC_PIN & Test)
          {
            Run++;                      /* increase counter */
            if (Run > 250) break;       /* check for overflow */
          }

          if (Run <= 250)          /* no overrun */
          {
            U1 = Run * 70;                   /* delay (0.1 MCU cycles) */
            U1 /= MCU_CYCLES_PER_US;         /* delay (0.1 µs) */
          }


          /*
           *  turn-off delay
           */

          Run = 0;                                /* zero counter */
          R_PORT = Probes.Rl_3;                   /* turn off LED */

          /*
           *  wait for logic high level (>2.5V)
           *  - MCU cycles for full loop run: 7
           */

          while (!(ADC_PIN & Test))
          {
            Run++;                      /* increase counter */
            if (Run > 250) break;       /* check for overflow */
          }

          if (Run <= 250)          /* no overrun */
          {
            U2 = Run * 70;                   /* delay (0.1 MCU cycles) */
            U2 /= MCU_CYCLES_PER_US;         /* delay (0.1 µs) */
          }

          Run = 1;            /* reset value */
          Test = 100;         /* reset value */
        }
      }


      /*
       *  display result
       */

      if (Test == DETECTED_BJT)         /* BJT type */
      {
        Display_EEString(BJT_str);      /* display: BJT */

        Display_NL_EEString_Space(CTR_str);       /* display: CTR */
        Display_Value(CTR, 0, '%');               /* display CTR */

        Display_NL_EEString_Space(If_str);        /* display: If */
        Display_Value(U3, -6, 'A');               /* display If */

        if (U1 < UINT16_MAX)       /* valid t_on */
        {
          Display_NL_EEString_Space(t_on_str);    /* display: t_on */
          if (U1 < 10)        /* < 1µs */
          {
            Display_Char('<');
            U1 = 10;          /* 1µs */
          }
          Display_Value(U1, -7, 's');
        }

        if (U2 < UINT16_MAX)       /* valid t_off */
        {
          Display_NL_EEString_Space(t_off_str);   /* display: t_off */
          if (U2 < 10)        /* < 1µs */
          {
            Display_Char('<');
            U2 = 10;          /* 1µs */
          }
          Display_Value(U2, -7, 's');
        }

        Display_NL_EEString_Space(Vf_str);        /* display: Vf */
        Display_Value(Diodes[0].V_f, -3, 'V');    /* display Vf */
      }
      else if (Test == DETECTED_TRIAC)  /* TRIAC type */
      {
        Display_EEString(Triac_str);    /* display: TRIAC */

        Display_NL_EEString_Space(Vf_str);        /* display: Vf */
        Display_Value(Diodes[0].V_f, -3, 'V');    /* display Vf */
      }
      else                              /* none found */
      {
        Display_EEString(None_str);     /* display: None */
      }

      #ifdef UI_SERIAL_COPY
      Display_Serial_Off();        /* disable serial output & NL */
      #endif
    }
  }

  /* clean up local constants for status */
  #undef DETECTED_LED
  #undef DETECTED_BJT
  #undef DETECTED_TRIAC
}

#endif



/* ************************************************************************
 *   capacitor leakage current
 * ************************************************************************ */


#ifdef SW_CAP_LEAKAGE

/*
 *  tool for measuring the leakage current of a capacitor
 *  - uses probe #1 (pos) and probe #3 (neg)
 *  - requires display with more than 2 lines
 */

void Cap_Leakage(void)
{
  uint8_t           Flag;               /* loop control flag */
  uint8_t           Test = 0;           /* user feedback */
  uint8_t           Mode;               /* mode */
  uint16_t          U1 = 0;             /* voltage #1 */
  uint32_t          Value;              /* temp. value */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run flag */
  #define CHANGED_MODE        0b00000100     /* mode has changed */

  /* local constants for Mode */
  #define MODE_NONE           0         /* no mode (show pinout) */
  #define MODE_HIGH           1         /* charge cap: high current */
  #define MODE_LOW            2         /* charge cap: low current */
  #define MODE_DISCHARGE      3         /* discharge cap */

  /* show info */
  LCD_Clear();                          /* clear display */
  #ifdef UI_COLORED_TITLES
    /* display: cap leakage */
    Display_ColoredEEString(CapLeak_str, COLOR_TITLE);
  #else
    Display_EEString(CapLeak_str);      /* display: cap leakage */
  #endif

  /* set start values */
  Flag = RUN_FLAG | CHANGED_MODE;
  Mode = MODE_NONE;

  UpdateProbes(PROBE_1, 0, PROBE_3);    /* update register bits and probes */

  while (Flag > 0)       /* processing loop */
  {
    /*
     *  display mode and set probes
     */

    if (Flag & CHANGED_MODE)       /* mode has changed */
    {
      LCD_ClearLine2();            /* clear line #2 */

      switch (Mode)                /* based on mode */
      {
        case MODE_NONE:            /* display pinout */
          /* probe-1: Vcc / probe-3: Gnd */
          Show_SimplePinout('+', 0, '-');
          LCD_ClearLine(3);             /* clear line #3 */
          break;

        case MODE_HIGH:            /* charge cap with high current (Rl) */
          Display_EEString_Space(CapCharge_str);
          Display_EEString(CapHigh_str);

          /* set probes: probe-3 -- Rl -- Gnd / probe-1 -- Vcc */
          ADC_DDR = 0;                  /* set to HiZ */
          R_DDR = Probes.Rl_3;          /* select Rl for probe-3 */
          R_PORT = 0;                   /* pull down probe-3 via Rl */
          ADC_PORT = Probes.Pin_1;      /* pull up probe-1 directly */
          ADC_DDR = Probes.Pin_1;       /* enable pull-up of probe-1 */
          break;

        case MODE_LOW:             /* charge cap with low current (Rh) */
          /* max. charge current I = 5V/Rh = 10.6µA */
          Display_EEString_Space(CapCharge_str);
          Display_EEString(CapLow_str);

          /* set probes: probe-3 -- Rh -- Gnd / probe-1 -- Vcc */
          /* simply switch pull-down resistor Rl to Rh */
          R_DDR = Probes.Rh_3;          /* select Rh for probe-3 */
          break;

        case MODE_DISCHARGE:       /* discharge cap */
          Display_EEString(CapDischarge_str);
          /* set probes: probe-3 -- Gnd / probe-1 -- Rl -- Gnd */
          ADC_DDR = 0;                  /* set to HiZ */
          R_DDR = Probes.Rl_1;          /* select Rl for probe-1 */
          /* R_PORT set to 0 already: pull down probe-1 via Rl */
          ADC_DDR = Probes.Pin_3;       /* set probe-3 to output */
          ADC_PORT = 0;                 /* pull down probe-3 directly */
          break;
      }

      Flag &= ~CHANGED_MODE;       /* clear flag */
    }


    /*
     *  manage modes
     */

    if (Mode != MODE_NONE)
    {
      LCD_ClearLine3();            /* clear line #3 */

      switch (Mode)                /* based on mode */
      {
        case MODE_HIGH:            /* charge cap with high current (Rl) */
          /* voltage across Rl and RiL at probe-3 */
          U1 = ReadU(Probes.Ch_3);           /* read voltage at probe-3 */

          /* calculate current: I = U / R (ignore R_Zero) */
          Value = U1;                        /* U across Rl and RiL in mV */
          Value *= 100000;                   /* scale to 0.01 µV */
          Value /= ((R_LOW * 10) + NV.RiL);  /* 0.01 µV / 0.1 Ohms = 0.1 µA */
          Display_Value(Value, -7, 'A');     /* display current */

          /* change to low current mode when current is quite low */
          if (U1 <= 3)                       /* I <= 4.2µA */
          {
            Mode = MODE_LOW;                 /* low current mode */
            Flag |= CHANGED_MODE;            /* set flag for changed mode */
          }
          break;

        case MODE_LOW:             /* charge cap with low current (Rh) */
          /* voltage across Rh at probe-3 (ignore RiL) */
          U1 = ReadU(Probes.Ch_3);           /* read voltage at probe-3 */

          if (U1 > CAP_DISCHARGED)      /* minimum exceeded */
          {
            /* calculate current: I = U / R */
            Value = U1;                        /* in mV */
            Value *= 10000;                    /* scale to 0.1 µV */
            Value /= (R_HIGH / 1000);          /* 0.1 µV / kOhms = 0.1 nA */
            Display_Value(Value, -10, 'A');    /* display current */
          }
          else                          /* in the noise floor */
          {
            Display_Minus();
          }
          break;

        case MODE_DISCHARGE:       /* discharge cap */
          /* voltage at cap (probe-1) */
          U1 = ReadU(Probes.Ch_1);           /* read voltage at probe-1 */
          Display_Value(U1, -3, 'V');        /* display voltage */

          /* check if cap is discharged */
          if (U1 <= CAP_DISCHARGED)          /* < threshold */
          {
            /* start new check cycle */
            Mode = MODE_NONE;                /* show pinout */
            Flag |= CHANGED_MODE;            /* set flag for changed mode */
          }
          break;
      }

      /* common display output */
      if ((Mode == MODE_HIGH) || (Mode == MODE_LOW))
      {
        /* display voltage across current shunt (Rl or Rh) */
        Display_Space();
        Display_Char('(');
        Display_Value(U1, -3, 'V');          /* display voltage */
        Display_Char(')');
      }
    }


    /*
     *  user feedback
     *  - short key press -> next step
     *  - two short key presses -> exit tool
     */

    if (! (Flag & CHANGED_MODE))        /* skip when mode has changed */
    {
      /* wait for user feedback or timeout of 2s */
      Test = TestKey(2000, CHECK_KEY_TWICE | CHECK_BAT);
      /* also delay for next loop run */

      if (Test == KEY_SHORT)            /* short key press */
      {
        Test = 100;                     /* next mode */
      }
      else if (Test == KEY_TWICE)       /* two short key presses */
      {
        Flag = 0;                       /* end processing loop */
      }
      #ifdef HW_KEYS
      else if (Test == KEY_RIGHT)       /* right key */
      {
        Test = 100;                     /* next mode */
      }
      #endif

      if (Test == 100)                  /* next mode */
      {
        /* change mode */
        if (Mode == MODE_NONE)          /* pinout mode */
        {
          Mode = MODE_HIGH;             /* charge cap with high current */
          Flag |= CHANGED_MODE;         /* set flag for changed mode */
        }
        else                            /* any other mode */
        {
          Mode = MODE_DISCHARGE;        /* discharge cap */
          Flag |= CHANGED_MODE;         /* set flag for changed mode */
        }
      }
    }
  }


  /*
   *  clean up
   */

  /* local constants for Mode */
  #undef MODE_NONE
  #undef MODE_CHARGE
  #undef MODE_LEAK
  #undef MODE_DISCHARGE

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef CHANGED_MODE
}

#endif



/* ************************************************************************
 *   monitoring R/C/L
 * ************************************************************************ */


#ifdef SW_MONITOR_R

/*
 *  monitor R on probes #1 and #3
 */

void Monitor_R(void)
{
  uint8_t           Flag = 1;           /* loop control flag */
  uint8_t           Test;               /* user feedback */
  Resistor_Type     *R1;                /* pointer to resistor #1 */

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: R monitor */
    Display_ColoredEEString(Monitor_R_str, COLOR_TITLE);
  #else
    Display_EEString(Monitor_R_str);    /* display: R monitor */
  #endif
  ProbePinout(PROBES_RCL);              /* show probes used */

  /* init */
  UpdateProbes2(PROBE_1, PROBE_3);      /* update probes */
  R1 = &Resistors[0];                   /* pointer to first resistor */
  /* increase number of samples to lower spread of measurement values */
  Cfg.Samples = 100;                    /* perform 100 ADC samples */


  /*
   *  processing loop
   */

  while (Flag)
  {
    /* measure R and display value */
    Check.Resistors = 0;                /* reset resistor counter */
    CheckResistor();                    /* check for resistor */
    LCD_ClearLine2();                   /* clear line #2 */

    if (Check.Resistors == 1)           /* found resistor */
    {
      /* display value */
      Display_Value(R1->Value, R1->Scale, LCD_CHAR_OMEGA);
    }
    else                                /* no resistor */
    {
      Display_Minus();                  /* display: nothing */
    }

    /* user feedback (1s delay) */
    Test = TestKey(1000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                    /* end processing loop */
    }
  }

  /* clean up */
  Cfg.Samples = ADC_SAMPLES;       /* set ADC samples back to default */
}

#endif



#ifdef SW_MONITOR_C

/*
 *  monitor C on probes #1 and #3
 *  - optionally ESR
 */

void Monitor_C(void)
{
  uint8_t           Flag = 1;           /* loop control flag */
  uint8_t           Test;               /* user feedback */
  Capacitor_Type    *Cap;               /* pointer to cap */
  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  uint16_t          ESR;                /* ESR (in 0.01 Ohms) */
  #endif

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: C monitor */
    Display_ColoredEEString(Monitor_C_str, COLOR_TITLE);
  #else
    Display_EEString(Monitor_C_str);    /* display: C monitor */
  #endif
  ProbePinout(PROBES_RCL);              /* show probes used */

  /* init */
  Check.Diodes = 0;                     /* reset diode counter */
  Cap = &Caps[0];                       /* pointer to first cap */


  /*
   *  processing loop
   */

  while (Flag)
  {
    #ifdef SW_MONITOR_HOLD_ESR
    if (Flag == 2)                      /* display former measurement */
    {
      LCD_ClearLine3();                 /* clear line #3 */

      Display_Colon();                            /* display: ':' */
      Display_Value(Cap->Value, Cap->Scale, 'F'); /* display capacitance */
      Display_Space();                            /* display: ' ' */
      Display_Value(ESR, -2, LCD_CHAR_OMEGA);     /* display ESR */

      Flag = 1;                        /* reset flag */
    }
    #endif

    /* measure and display C */
    Check.Found = COMP_NONE;                 /* no component */
    /* keep probe order of normal probing cycle */
    MeasureCap(PROBE_3, PROBE_1, 0);         /* measure capacitance */
    LCD_ClearLine2();                        /* clear line #2 */

    if (Check.Found == COMP_CAPACITOR)       /* found cap */
    {
      /* display capacitance */
      Display_Value(Cap->Value, Cap->Scale, 'F');

      #if defined (SW_ESR) || defined (SW_OLD_ESR)
      /* measure and show ESR */
      ESR = MeasureESR(Cap);                 /* measure ESR */
      if (ESR < UINT16_MAX)                  /* if successfull */
      {
        Display_Space();
        Display_Value(ESR, -2, LCD_CHAR_OMEGA);   /* display ESR */

        #ifdef SW_MONITOR_HOLD_ESR
        Flag = 2;                            /* signal valid ESR */
        #endif
      }
      #endif
    }
    else                                     /* no cap */
    {
      Display_Minus();                       /* display: nothing */
    }

    /* user feedback (2s delay) */
    Test = TestKey(2000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                    /* end processing loop */
    }
  }
}

#endif



#ifdef SW_MONITOR_L

/*
 *  monitor L on probes #1 and #3
 */

void Monitor_L(void)
{
  uint8_t           Flag = 1;           /* loop control flag */
  uint8_t           Test;               /* user feedback */
  Resistor_Type     *R1;                /* pointer to resistor #1 */

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: L monitor */
    Display_ColoredEEString(Monitor_L_str, COLOR_TITLE);
  #else
    Display_EEString(Monitor_L_str);    /* display: L monitor */
  #endif
  ProbePinout(PROBES_RCL);              /* show probes used */

  /* init */
  R1 = &Resistors[0];                   /* pointer to first resistor */


  /*
   *  processing loop
   */

  while (Flag)
  {
    #ifdef SW_MONITOR_HOLD_L
    if (Flag == 2)                      /* display former measurement */
    {
      LCD_ClearLine3();                 /* clear line #3 */

      Display_Colon();                  /* display: ':' */
      Display_Value(Inductor.Value, Inductor.Scale, 'H');   /* display L */

      Flag = 1;                         /* reset flag */
    }
    #endif

    /* measure R */
    UpdateProbes2(PROBE_1, PROBE_3);    /* update probes */
    Check.Resistors = 0;                /* reset resistor counter */
    CheckResistor();                    /* check for resistor */
    LCD_ClearLine2();                   /* clear line #2 */

    if (Check.Resistors == 1)           /* found resistor */
    {
      /* get inductance and display if relevant */
      if (MeasureInductor(R1) == 1)
      {
        /* display inductance */
        Display_Value(Inductor.Value, Inductor.Scale, 'H');

        #ifdef SW_MONITOR_HOLD_L
        Flag = 2;                       /* signal valid L */
        #endif
      }
      else                              /* no inductor */
      {
        Display_Minus();                /* display: nothing */
      }
    }
    else                                /* no resistor */
    {
      Display_Minus();                  /* display: nothing */
    }

    /* user feedback (1s delay) */
    Test = TestKey(1000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                    /* end processing loop */
    }
  }
}

#endif



#ifdef SW_MONITOR_RCL

/*
 *  monitor R plus L, or C plus ESR on probes #1 and #3
 */

void Monitor_RCL(void)
{
  uint8_t           Run = 1;            /* loop control flag */
  uint8_t           Test;               /* user feedback */
  Resistor_Type     *R1;                /* pointer to resistor #1 */
  Capacitor_Type    *Cap;               /* pointer to cap */
  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  uint16_t          ESR;                /* ESR (in 0.01 Ohms) */
  #endif

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: RCL monitor */
    Display_ColoredEEString(Monitor_RCL_str, COLOR_TITLE);
  #else
    Display_EEString(Monitor_RCL_str);  /* display: RCL monitor */
  #endif
  ProbePinout(PROBES_RCL);              /* show probes used */

  /* init */
  R1 = &Resistors[0];                   /* pointer to first resistor */
  Cap = &Caps[0];                       /* pointer to first cap */
  Check.Diodes = 0;                     /* reset diode counter */


  /*
   *  processing loop
   */

  while (Run)
  {
    /*
     *  check for R, L and C
     */

    Check.Found = COMP_NONE;            /* no component */

    if (Run != COMP_CAPACITOR)          /* none, R or L */
    {
      /* increase number of samples to lower spread of measurement values */
      Cfg.Samples = 100;                /* perform 100 ADC samples */

      /* measure R */
      UpdateProbes2(PROBE_1, PROBE_3);       /* update probes */
      Check.Resistors = 0;                   /* reset resistor counter */
      CheckResistor();                       /* check for resistor */

      if (Check.Resistors == 1)         /* found resistor */
      {
        Check.Found = COMP_RESISTOR;         /* is R */

        /* measure L */
        if (MeasureInductor(R1) == 1)        /* got inductance */
        {
          Run = COMP_INDUCTOR;               /* is an inductor */
        }
        else                                 /* no inductance */
        {
          Run = COMP_RESISTOR;               /* is a resistor */
        }
      }
      else                              /* no resistor */
      {
        Run = 1;                             /* reset to "no component" */
      }

      Cfg.Samples = ADC_SAMPLES;        /* set ADC samples back to default */
    }

    if (Run != COMP_INDUCTOR)           /* none, C or R */
    {
      /* measure capacitance */

      /* keep probe order of normal probing cycle */
      MeasureCap(PROBE_3, PROBE_1, 0);       /* measure capacitance */

      if (Check.Found == COMP_CAPACITOR)     /* found cap */
      {
        Run = COMP_CAPACITOR;                /* is a cap */
      }
      else if (Run != COMP_RESISTOR)         /* no cap and no resistor */
      {
        Run = 1;                             /* reset to "no component" */
      }
    }


    /*
     *  display measurement
     */

    LCD_ClearLine2();                   /* clear line #2 */

    if (Run == 1)                       /* no component */
    {
      Display_Minus();                  /* display: nothing */
    }
    else if (Run == COMP_CAPACITOR)     /* C */
    {
      /* display capacitance */
      Display_Value(Cap->Value, Cap->Scale, 'F');

      #if defined (SW_ESR) || defined (SW_OLD_ESR)
      /* show ESR */
      ESR = MeasureESR(Cap);                 /* measure ESR */
      if (ESR < UINT16_MAX)                  /* if successfull */
      {
        Display_Space();
        Display_Value(ESR, -2, LCD_CHAR_OMEGA);   /* display ESR */
      }
      #endif
    }
    else                                 /* R or L */
    {
      /* display resistance */
      Display_Value(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

      if (Run == COMP_INDUCTOR)          /* L */
      {
        /* display inductance */
        Display_Space();
        Display_Value(Inductor.Value, Inductor.Scale, 'H');
      }
    }


    /* user feedback (1s delay) */
    Test = TestKey(1000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Run = 0;                     /* end processing loop */
    }
  }
}

#endif



#ifdef SW_MONITOR_RL

/*
 *  monitor R plus L on probes #1 and #3
 */

void Monitor_RL(void)
{
  uint8_t           Flag = 1;           /* loop control flag */
  uint8_t           Test;               /* user feedback */
  Resistor_Type     *R1;                /* pointer to resistor #1 */

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: monitor RL */
    Display_ColoredEEString(Monitor_RL_str, COLOR_TITLE);
  #else
    Display_EEString(Monitor_RL_str);   /* display: monitor RL */
  #endif
  ProbePinout(PROBES_RCL);              /* show probes used */

  /* init */
  R1 = &Resistors[0];                   /* pointer to first resistor */
  /* increase number of samples to lower spread of measurement values */
  Cfg.Samples = 100;                    /* perform 100 ADC samples */


  /*
   *  processing loop
   */

  while (Flag)
  {
    /* measure R and display value */
    UpdateProbes2(PROBE_1, PROBE_3);    /* update probes */
    Check.Resistors = 0;                /* reset resistor counter */
    CheckResistor();                    /* check for resistor */
    LCD_ClearLine2();                   /* clear line #2 */

    if (Check.Resistors == 1)           /* found resistor */
    {
      /* display value */
      Display_Value(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

      /* get inductance and display if relevant */
      if (MeasureInductor(R1) == 1)
      {
        Display_Space();
        Display_Value(Inductor.Value, Inductor.Scale, 'H');
      }
    }
    else                                /* no resistor */
    {
      Display_Minus();                  /* display: nothing */
    }

    /* user feedback (1s delay) */
    Test = TestKey(1000, CHECK_KEY_TWICE | CHECK_BAT | CURSOR_STEADY);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                    /* end processing loop */
    }
  }

  /* clean up */
  Cfg.Samples = ADC_SAMPLES;       /* set ADC samples back to default */
}

#endif



/* ************************************************************************
 *   logic probe
 * ************************************************************************ */


#ifdef HW_LOGIC_PROBE

/*
 *  Logic Probe
 *  - analog input: TP_LOGIC
 *  - uses voltage divider (default: 4:1)
 *    LOGIC_PROBE_R1 and LOGIC_PROBE_R2
 */

void LogicProbe(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* user feedback */
  uint8_t           Item;               /* menu item */
  uint8_t           VccIndex;           /* index for Vcc table */
  unsigned char     State;              /* logic state */
  uint16_t          U1;                 /* measured voltage */
  uint16_t          U_max = 0;          /* Vcc/Vdd */
  uint16_t          U_low = 0;          /* voltage threshold for low */
  uint16_t          U_high = 0;         /* voltage threshold for high */
  uint32_t          Value;              /* temporary value */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define CHANGE_TYPE         0b00000010     /* change logic type */
  #define CHANGE_LOW          0b00001000     /* change low threshold */
  #define CHANGE_HIGH         0b00010000     /* change high threshold */

  /* local constants for Item */
  #define ITEM_TYPE      1              /* logic type (family and Vcc) */
  #define ITEM_LOW       2              /* voltage threshold for low */
  #define ITEM_HIGH      3              /* voltage threshold for high */


  /*
   *  show info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Logic Probe */
    Display_ColoredEEString(LogicProbe_str, COLOR_TITLE);
  #else
    Display_EEString(LogicProbe_str);   /* display: Logic Probe */
  #endif


  /*
   *  init
   */

  ADC_DDR &= ~(1 << TP_LOGIC);          /* set pin to HiZ */
  Cfg.Samples = 5;                      /* do just 5 samples to be fast */
  VccIndex = 0;                         /* TTL 5V */
  Item = ITEM_TYPE;                     /* default item */
  Flag = RUN_FLAG | CHANGE_TYPE | CHANGE_LOW | CHANGE_HIGH;


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  display family and Vcc/Vdd
     */

    if (Flag & CHANGE_TYPE)
    {
      LCD_ClearLine2();                 /* line #2 */
      MarkItem(ITEM_TYPE, Item);        /* mark item if selected */

      /* display family */
      if (VccIndex == 0)                /* TTL */
      {
        Display_EEString(TTL_str);      /* display: TTL */
      }
      else                              /* CMOS */
      {
        Display_EEString(CMOS_str);     /* display: CMOS */
      }

      /* read Vcc/Vdd from table */
      U_max = DATA_read_word(&Logic_Vcc_table[VccIndex]);

      /* display Vcc/Vdd */
      Display_Space();
      U1 = U_max / 100;                 /* scale to 0.1V */
      Display_Value(U1, -1, 'V');       /* display voltage in 0.1V */

      /* update thresholds */
      if (VccIndex == 0)                /* TTL */
      {
        /* fixed thresholds for 5V */
        U_low = 800;                    /* 0.8V */
        U_high = 2000;                  /* 2.0V */
      }
      else                              /* CMOS */
      {
        /* thresholds based on Vdd (in mV) */
        /* L: 0.3 Vdd */
        U_low = U1 * 30;                /* U in 0.1V * 0.3 * 100 */

        /* H: 0.7 Vdd */
        U_high = U1 * 70;               /* U in 0.1V * 0.7 * 100  */
      }

      Flag &= ~CHANGE_TYPE;             /* clear flag */
    }


    /*
     *  display low threshold
     */

    if (Flag & CHANGE_LOW)
    {
      LCD_ClearLine3();                 /* clear line #3 */
      MarkItem(ITEM_LOW, Item);         /* mark item if selected */
      LCD_Char('L');                    /* display: L */
      Display_Space();
      U1 = U_low / 100;                 /* scale to 0.1V */
      Display_Value(U1, -1, 'V');       /* display voltage in 0.1V */

      Flag &= ~CHANGE_LOW;              /* clear flag */
    }


    /*
     *  display high threshold
     */

    if (Flag & CHANGE_HIGH)
    {
      LCD_ClearLine(4);                 /* line #4 */
      LCD_CharPos(1, 4);
      MarkItem(ITEM_HIGH, Item);        /* mark item if selected */
      LCD_Char('H');                    /* display: H */
      Display_Space();
      U1 = U_high / 100;                /* scale to 0.1V */
      Display_Value(U1, -1, 'V');       /* display voltage in 0.1V */

      Flag &= ~CHANGE_HIGH;             /* clear flag */
    }


    /*
     *  display logic state
     *  - 0, 1 or Z
     *  - ADC pin is connected to a voltage divider (top: R1 / bottom: R2).
     *    - U2 = (Uin / (R1 + R2)) * R2 
     *    - Uin = (U2 * (R1 + R2)) / R2
     */

    /* get voltage */
    U1 = ReadU(TP_LOGIC);          /* read voltage */

    /* consider voltage divider */
    Value = (((uint32_t)(LOGIC_PROBE_R1 + LOGIC_PROBE_R2) * 1000) / LOGIC_PROBE_R2);  /* factor (0.001) */
    Value *= U1;                   /* voltage (0.001 mV) */
    Value /= 1000;                 /* scale to mV */
    U1 = (uint16_t)Value;          /* keep 2 bytes */

    /* compare with thresholds  */
    if (U1 <= U_low)               /* below low threshold */
    {
      State = '0';                 /* 0 for low */
    }
    else if (U1 >= U_high)         /* above high threshold */
    {
      State = '1';                 /* 1 for high */
    }
    else                           /* in between (undefined, HiZ) */
    {
      State = 'Z';                 /* z for undefined/HiZ */
    }

    /* display state and voltage */
    LCD_ClearLine(5);                   /* line #5 */
    LCD_CharPos(1, 5);
    #ifdef LCD_COLOR
    UI.PenColor = COLOR_SYMBOL;         /* change color */
    #endif
    LCD_Char(State);                    /* display state */
    #ifdef LCD_COLOR
    UI.PenColor = COLOR_PEN;            /* reset color */
    #endif
    Display_Space();
    Display_Value(U1, -3, 'V');         /* display voltage */


    /*
     *  user feedback
     */

    /* check for user feedback and slow down update rate */
    Test = TestKey(200, CHECK_KEY_TWICE | CHECK_BAT);

    /* process user input */
    if (Test == KEY_SHORT)              /* short key press */
    {
      /* select next item */
      Item++;                           /* next item */

      if (Item > ITEM_HIGH)             /* overflow */
      {
        Item = ITEM_TYPE;               /* ... to first item */
      }

      Flag |= CHANGE_TYPE | CHANGE_LOW | CHANGE_HIGH;       /* update display */
    }
    else if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    else if (Test == KEY_RIGHT)         /* right key */
    {
      if (Item == ITEM_TYPE)                 /* family and Vcc/Vdd */
      {
        /* next set */
        VccIndex++;                          /* increase index */
        if (VccIndex >= NUM_LOGIC_TYPES)     /* overflow */
        {
          VccIndex = 0;                      /* roll over to first */
        }
        Flag |= CHANGE_TYPE | CHANGE_LOW | CHANGE_HIGH;     /* update display */
      }
      else if (Item == ITEM_LOW)             /* low threshold */
      {
        /* increase low threshold */
        U_low += 100;                        /* + 100mV */
        if (U_low > U_high)                  /* prevent overflow */
        {
          U_low = U_high;                    /* limit to high threshold */
        }
        Flag |= CHANGE_LOW;                  /* update display */
      }
      else if (Item == ITEM_HIGH)            /* high threshold */
      {
        /* increase high threshold */
        U_high += 100;                       /* + 100mV */
        if (U_high > U_max)                  /* prevent overflow */
        {
          U_high = U_max;                    /* limit to Vcc/Vdd */
        }
        Flag |= CHANGE_HIGH;                 /* update display */
      }
    }
    else if (Test == KEY_LEFT)          /* left key */
    {
      if (Item == ITEM_TYPE)                 /* family and Vcc/Vdd */
      {
        /* previous set */
        if (VccIndex > 0)                    /* prevent underun */
        {
          VccIndex--;                        /* decrease index */
        }
        else                                 /* roll over */
        {
          VccIndex = NUM_LOGIC_TYPES - 1;    /* ... to last */
        }
        Flag |= CHANGE_TYPE | CHANGE_LOW | CHANGE_HIGH;     /* update display */
      }
      else if (Item == ITEM_LOW)             /* low threshold */
      {
        /* decrease low threshold */
        if (U_low >= 100)                    /* prevent underflow (limit to 0V) */
        {
          U_low -= 100;                      /* - 100mV */
        }
        Flag |= CHANGE_LOW;                  /* update display */
      }
      else if (Item == ITEM_HIGH)            /* high threshold */
      {
        /* decrease high threshold */
        U1 = U_low + 100;                    /* limit to low threshold */
        if (U_high >= U1)                    /* prevent underflow */
        {
          U_high -= 100;                     /* - 100mV */
        }
        Flag |= CHANGE_HIGH;                 /* update display */
      }
    }
  }


  /*
   *  clean up
   */

  /* global settings */
  Cfg.Samples = ADC_SAMPLES;            /* set ADC samples back to default */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef CHANGE_TYPE
  #undef CHANGE_LOW
  #undef CHANGE_HIGH

  /* local constants for Item */
  #undef ITEM_LOW
  #undef ITEM_HIGH
}

#endif



/* ************************************************************************
 *   continuity check
 * ************************************************************************ */


#ifdef SW_CONTINUITY_CHECK

/*
 *  continuity check
 *  - uses probes #1 (pos) and #3 (neg)
 *  - requires buzzer
 */

void ContinuityCheck(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* user feedback */
  uint16_t          U1;                 /* measured voltage #1 */
  uint16_t          U2;                 /* measured voltage #2 */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define BEEP_SHORT          0b00000010     /* short beep */


  /*
   *  show info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Continuity */
    Display_ColoredEEString(ContinuityCheck_str, COLOR_TITLE);
  #else
    Display_EEString(ContinuityCheck_str);   /* display: Continuity */
  #endif
  ProbePinout(PROBES_ESR);         /* show probes used */


  /*
   *  init
   */

  UpdateProbes(PROBE_1, PROBE_2, PROBE_3);        /* update probes */

  /* set probes: Vcc -- Rl -- probe #1 / probe #3 -- Gnd */
  R_PORT = Probes.Rl_1;                 /* pull up probe #1 via Rl */
  R_DDR = Probes.Rl_1;                  /* enable pull-up resistor */
  ADC_PORT = 0;                         /* pull down directly */
  ADC_DDR = Probes.Pin_3;               /* enable Gnd for probe #3 */

  Cfg.Samples = 5;                      /* do just 5 samples to be fast */
  Flag = RUN_FLAG;                      /* enter processing loop */


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  check voltage across probes
     *  and output result
     */

    U1 = ReadU(Probes.Ch_1);            /* read voltage at probe #1 */
    U2 = ReadU(Probes.Ch_3);            /* read voltage at probe #3 */

    /* consider voltage drop by RiL */
    if (U1 > U2)                        /* sanity check */
    {
      U1 -= U2;                         /* subtract voltage of RiL */
    }

    /* compare voltage with thresholds */
    if (U1 < 100)                       /* < 100mV */
    {
      /* short: continuous beep */

      #ifdef BUZZER_ACTIVE
      /* enable buzzer */
      BUZZER_PORT |= (1 << BUZZER_CTRL);     /* set pin high */
      #endif

      #ifdef BUZZER_PASSIVE
      PassiveBuzzer(BUZZER_FREQ_HIGH);       /* high frequency beep */
      #endif
    }
    else if (U1 <= 700)                 /* 100-700mV */
    {
      /* pn junction: short beep */

      #ifdef BUZZER_ACTIVE
      /* enable buzzer */
      BUZZER_PORT |= (1 << BUZZER_CTRL);     /* set pin high */
      Flag |= BEEP_SHORT;                    /* set flag */
      #endif

      #ifdef BUZZER_PASSIVE
      PassiveBuzzer(BUZZER_FREQ_LOW);        /* low frequency beep */
      #endif
    }
    else                                /* > 700mV */
    {
      /* something else or open circuit: no beep */

      #ifdef BUZZER_ACTIVE
      /* disable buzzer */
      BUZZER_PORT &= ~(1 << BUZZER_CTRL);    /* set pin low */
      #endif
    }

    /* display voltage */
    LCD_ClearLine2();                   /* line #2 */
    Display_Value(U1, -3, 'V');         /* display voltage */
    /* this is also used as delay for the short beep */
    /* todo: do we need an additional delay for fast displays? */

    #ifdef BUZZER_ACTIVE
    if (Flag & BEEP_SHORT)              /* short beep */
    {
      /* disable buzzer */
      BUZZER_PORT &= ~(1 << BUZZER_CTRL);    /* set pin low */
      Flag &= ~BEEP_SHORT;                   /* clear flag */
    }
    #endif


    /*
     *  user feedback
     */

    /* check for user feedback and slow down update rate */
    Test = TestKey(50, CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                    /* end loop */
    }
  }


  /*
   *  clean up
   */

  /* global settings */
  Cfg.Samples = ADC_SAMPLES;            /* set ADC samples back to default */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef BEEP_SHORT
}

#endif



/* ************************************************************************
 *   flashlight / general purpose switched output
 * ************************************************************************ */


#ifdef HW_FLASHLIGHT

/*
 *  flashlight / general purpose switched output
 *  - toggles output pin between high and low
 */

void Flashlight(void)
{
  uint8_t           Flag;          /* status flag */


  /*
   *  toggle output
   */

  /* get current state */
  Flag = FLASHLIGHT_PORT;          /* read port register */
  Flag &= (1 << FLASHLIGHT_CTRL);  /* filter pin */

  /* set output based on current state */
  if (Flag)                   /* pin high */
  {
    /* toggle off */
    FLASHLIGHT_PORT &= ~(1 << FLASHLIGHT_CTRL);   /* set pin low */
  }
  else                        /* pin low */
  {
    /* toggle on */
    FLASHLIGHT_PORT |= (1 << FLASHLIGHT_CTRL);    /* set pin high */
  }
}

#endif



/* ************************************************************************
 *   photodiode check
 * ************************************************************************ */


#ifdef SW_PHOTODIODE

/*
 *  check photodiodes
 *  - supports reverse-bias and no-bias mode
 *  - uses probes #1 (anode) and #3 (cathode)
 */

void PhotodiodeCheck(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* user feedback */
  uint16_t          U;                  /* measured voltage */
  uint16_t          R = 0;              /* resistance (current shunt) */
  uint32_t          I;                  /* current I_P */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define NO_BIAS             0b00000010     /* no-bias mode */
  #define REVERSE_BIAS        0b00000100     /* reverse-bias mode */
  #define UPDATE_BIAS         0b00001000     /* update bias mode */


  /*
   *  show info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Photodiode */
    Display_ColoredEEString(Photodiode_str, COLOR_TITLE);
  #else
    Display_EEString(Photodiode_str);        /* display: Photodiode */
  #endif
  ProbePinout(PROBES_DIODE);                 /* show probes used */


  /*
   *  init
   */

  UpdateProbes(PROBE_1, PROBE_2, PROBE_3);   /* update probes */

  /* enter processing loop and set reverse-bias mode */
  Flag = RUN_FLAG | REVERSE_BIAS | UPDATE_BIAS;


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  set up bias mode
     */

    if (Flag & UPDATE_BIAS)        /* update requested */
    {
      /* probe #1: anode, probe #3: cathode */

      if (Flag & REVERSE_BIAS)     /* reverse-bias mode */
      {
        /* safeguard when switching from no-bias mode */
        ADC_DDR = 0;                    /* remove direct pull down */

        /* set probes: Vcc -- Rl -- probe #3 / probe #1 -- Gnd */
        R_PORT = Probes.Rl_3;           /* pull up probe #3 via Rl */
        R_DDR = Probes.Rl_3;            /* enable pull-up resistor */
        ADC_PORT = 0;                   /* pull down directly */
        ADC_DDR = Probes.Pin_1;         /* enable Gnd for probe #1 */

        /* current shunt: Rl + RiH */
        R = (R_LOW * 10) + NV.RiH;      /* in 0.1 Ohms */
      }
      else                         /* no-bias mode */
      {
        /* set probes: probe #1 -- Rl -- Gnd / probe #3 -- Gnd */
        R_PORT = 0;                     /* pull down ... */
        R_DDR = Probes.Rl_1;            /* ... probe #1 via Rl */
        ADC_PORT = 0;                   /* pull down directly */
        ADC_DDR = Probes.Pin_3;         /* enable Gnd for probe #3 */

        /* current shunt: Rl + RiL */
        R = (R_LOW * 10) + NV.RiL;      /* in 0.1 Ohms */
      }

      Flag &= ~UPDATE_BIAS;                  /* clear flag */
    }


    /*
     *  monitor current I_P
     *  - measure voltage across current shunt Rl
     *  - calculate current
     *  - display current
     */

    /* measure voltage */
    if (Flag & REVERSE_BIAS)       /* reverse-bias mode */
    {
      U = Cfg.Vcc - ReadU(Probes.Ch_3); /* voltage at probe #3 (cathode), in mV */
    }
    else                           /* no-bias mode */
    {
      U = ReadU(Probes.Ch_1);      /* voltage at probe #1 (anode), in mV */
    }

    /* calculate I_P (= U / R) */
    I = U * 100000;                /* scale voltage to 0.01 µV */
    I /= R;                        /* / R (in 0.1 Ohms) -> I in 0.1 µA */ 

    /* display I_P */
    LCD_ClearLine2();              /* clear line #2 */
    if (Flag & REVERSE_BIAS)       /* reverse-bias mode */
    {
      Display_EEString_Space(ReverseBias_str);    /* display: rev */
    }
    else                           /* no-bias mode */
    {
      Display_EEString_Space(NoBias_str);         /* display: no */
    }
    Display_Value(I, -7, 'A');     /* display current */


    /*
     *  user feedback
     */

    /* check for user feedback and slow down update rate */
    Test = TestKey(200, CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_SHORT)         /* short key press */
    {
      /* toggle bias mode */
      if (Flag & REVERSE_BIAS)     /* in reverse-bias mode */
      {
        /* switch to no-bias mode */
        Flag &= ~REVERSE_BIAS;               /* clear flag */
        Flag |= NO_BIAS | UPDATE_BIAS;       /* set flag for new mode */
      }
      else                         /* in no-bias mode */
      {
        /* switch to reverse-bias mode */
        Flag &= ~NO_BIAS;                    /* clear flag */
        Flag |= REVERSE_BIAS | UPDATE_BIAS;  /* set flag for new mode */
      }
    }
    else if (Test == KEY_TWICE)    /* two short key presses */
    {
      Flag = 0;                    /* end loop */
    }
  }


  /*
   *  clean up
   */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef NO_BIAS
  #undef REVERSE_BIAS
  #undef UPDATE_BIAS
}

#endif



/* ************************************************************************
 *   diode/LED check
 * ************************************************************************ */


#ifdef SW_DIODE_LED

/*
 *  display diode pinout and Uf
 *
 *  requires:
 *  - Diode: pointer of diode data
 */

void Show_Single_Diode(Diode_Type *Diode)
{
  Display_ProbeNumber(Diode->A);        /* show probe number of anode */
  Display_EEString(Diode_AC_str);       /* show: ->|- */
  Display_ProbeNumber(Diode->C);        /* show probe number of cathode */
  Display_Space();
  Display_Value(Diode->V_f, -3, 'V');   /* show Uf (in mV) */
}



/*
 *  quick-check diodes and LEDs
 *  - uses probes #1 and #3
 *  - requires a display with >= 3 text lines
 */

void Diode_LED_Check(void)
{
  uint8_t           Flag = 1;           /* loop control */
  uint8_t           Test;               /* user feedback */


  /*
   *  show info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: diode/LED */
    Display_ColoredEEString(Diode_LED_str, COLOR_TITLE);
  #else
    Display_EEString(Diode_LED_str);         /* display: diode/LED */
  #endif
  ProbePinout(PROBES_RCL);                   /* show probes used */


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  check for diodes
     */

    /* reset values */
    Check.Diodes = 0;                   /* reset diode counter */

    /* check for diode in one direction (probe #1: A, probe #3: C */
    UpdateProbes2(PROBE_1, PROBE_3);         /* update probes */
    CheckDiode();                            /* run diode check */

    /* check for diode in other direction (probe #1: C, probe #3: A */
    UpdateProbes2(PROBE_3, PROBE_1);         /* update probes */
    CheckDiode();                            /* run diode check */


    /*
     *  process results
     */

    LCD_ClearLine(3);                   /* clear line #3 */
    LCD_ClearLine2();                   /* clear line #2 */

    if (Check.Diodes == 0)              /* no diode */
    {
      Display_Minus();                  /* display: - */
    }
    else                                /* one diode or more */
    {
      Show_Single_Diode(&Diodes[0]);    /* display pinout and Uf of first diode */
    }

    if (Check.Diodes == 2)              /* two anti-parallel diodes */
    {
      LCD_CharPos(1, 3);                /* go to start of line #3 */
      Show_Single_Diode(&Diodes[1]);    /* display pinout and Uf of second diode */
    }


    /*
     *  user feedback
     */

    /* check for user feedback and slow down update rate */
    Test = TestKey(250, CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)    /* two short key presses */
    {
      Flag = 0;                    /* end loop */
    }
  }
}

#endif



/* ************************************************************************
 *   Voltmeter 0-5V DC
 * ************************************************************************ */


#ifdef SW_METER_5VDC

/*
 *  Voltmeter 0-5V DC
 *  - input impedance:
 *    470 kOhms (Rh, default)
 *    680 Ohms + RiL (Rl)
 *  - with optional buzzer:
 *    beep when default threshold is exceeded
 *  - with additional keys and buzzer:
 *    adjustable threshold, beep when exceeded
 *  - uses probes #1 (positive) and #3 (ground)
 */

void Meter_5VDC(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test = 0;           /* user feedback */
  uint16_t          U;                  /* voltage */
  #ifdef HW_BUZZER
  uint16_t          Threshold;          /* voltage threshold */
  uint8_t           n;
  #endif

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define BUZZER_ON           0b00000010     /* buzzer enabled */
  #define UPDATE_THRESHOLD    0b00000100     /* update threshold */
  #define INPUT_RH            0b00001000     /* use Rh as input impedance */
  #define UPDATE_INPUT        0b00010000     /* update input impedance */

  /* default voltage threshold: scale to mV */
  #define THRESHOLD           (METER_5VDC_THRESHOLD * 100)


  /*
   *  show info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: 5V-Meter */
    Display_ColoredEEString(Meter_5VDC_str, COLOR_TITLE);
  #else
    Display_EEString(Meter_5VDC_str);        /* display: 5V Meter */
  #endif
  ProbePinout(PROBES_ESR);                   /* show probes used */


  /*
   *  init
   */

  UpdateProbes(PROBE_1, PROBE_2, PROBE_3);   /* update probes */

  /* set probes: Gnd - probe #3 / probe #1 - Rh - Gnd */
  ADC_PORT = 0;                    /* pull down directly */
  ADC_DDR = Probes.Pin_3;          /* enable Gnd for probe #3 */
  R_PORT = 0;                      /* pull down */
  R_DDR = Probes.Rh_1;             /* pull down probe #1 via Rh */

  #ifdef HW_BUZZER
  Threshold = THRESHOLD;           /* set default */
  #endif

  /* enter loop, Rh as input impedance, update input status */
  Flag = RUN_FLAG | INPUT_RH | UPDATE_INPUT;


  /*
   *  processing loop
   */

  while (Flag > 0)
  {
    /*
     *  update input impedance status
     */

    if (Flag & UPDATE_INPUT)       /* update requested */
    {
      /* show title */
      LCD_ClearLine(1);                 /* clear line #1 */
      LCD_CharPos(1, 1);                /* pos #1 in line #1 */
      #ifdef UI_COLORED_TITLES
        /* display: 5V-Meter */
        Display_ColoredEEString(Meter_5VDC_str, COLOR_TITLE);
      #else
        /* display: 5V Meter */
        Display_EEString(Meter_5VDC_str);
      #endif

      /* show input impedance */
      Display_Space();

      if (Flag & INPUT_RH)              /* Rh selected */
      {
        Display_Char('H');              /* display: H */
      }
      else                              /* Rl selected */
      {
        Display_Char('L');              /* display: L */
      }

      Flag &= ~UPDATE_INPUT;            /* clear flag */
    }

    /* smooth UI after long key press */
    if (Test == KEY_LONG)          /* long key press */
    {
      SmoothLongKeyPress();             /* delay next key press */
    }


    #ifdef HW_BUZZER
    /*
     *  update threshold status
     */

    if (Flag & UPDATE_THRESHOLD)   /* update requested */
    {
      /* clear threshold anyway */
      LCD_CharPos(9, 2);                /* pos #9 in line #2 */
      LCD_ClearLine(0);                 /* clear rest of line */

      if (Flag & BUZZER_ON)             /* buzzer enabled */
      {
        /* show threshold */
        U = Threshold / 100;            /* scale to 100mV */
        LCD_CharPos(9, 2);              /* pos #9 in line #2 */
        Display_Char('(');              /* display: ( */
        Display_Value(U, -1, 'V');      /* display voltage */
        Display_Char(')');              /* display: ) */
      }

      Flag &= ~UPDATE_THRESHOLD;        /* clear flag */
    }
    #endif


    /*
     *  measure voltage
     */

    U = ReadU(Probes.Ch_1);        /* voltage at probe #1 (postive), in mV */


    /*
     *  display voltage
     */

    /* clear pos #1 - #6 in line #2 */
    LCD_CharPos(1, 2);             /* pos #1 in line #2 */
    for (n = 0; n < 6; n++)        /* clear pos #1 - #6 */
    {
      Display_Space();
    }

    /* display voltage */
    LCD_CharPos(1, 2);             /* pos #1 in line #2 */
    Display_Value(U, -3, 'V');     /* display voltage */

    #ifdef HW_BUZZER
    if (Flag & BUZZER_ON)          /* buzzer enabled */
    {
      if (U >= Threshold)          /* threshold exceeded */
      {
        #ifdef BUZZER_ACTIVE
        /* enable buzzer */
        BUZZER_PORT |= (1 << BUZZER_CTRL);     /* set pin high */

        wait20ms();                            /* wait 20 ms */
   
        /* disable buzzer */
        BUZZER_PORT &= ~(1 << BUZZER_CTRL);    /* set pin low */
        #endif

        #ifdef BUZZER_PASSIVE
        PassiveBuzzer(BUZZER_FREQ_HIGH);       /* high frequency beep */
        #endif
      }
    }
    #endif


    /*
     *  user feedback
     */

    /* check for user feedback and slow down update rate */
    Test = TestKey(250, CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    #ifdef HW_BUZZER
    else if (Test == KEY_SHORT)    /* short key press */
    {
      /* toggle buzzer */
      if (Flag & BUZZER_ON)             /* buzzer enabled */
      {
        /* disable buzzer */
        Flag &= ~BUZZER_ON;             /* clear flag */
      }
      else                              /* buzzer disabled */
      {
        /* enable buzzer */
        Flag |= BUZZER_ON;              /* set flag */
      }

      Flag |= UPDATE_THRESHOLD;         /* update status */
    }
    #endif
    #if defined (HW_BUZZER) && defined (HW_KEYS)
    else if (Test == KEY_RIGHT)    /* right turn */
    {
      /* increase voltage threshold (by 0.1V) */
      if (Threshold <= 4900)            /* prevent overrun */
      {
        Threshold += 100;               /* + 0.1 V */
        Flag |= UPDATE_THRESHOLD;       /* update status */
      }
    }
    else if (Test == KEY_LEFT)     /* left turn */
    {
      /* decrease voltage threshold (by 0.1V) */
      if (Threshold >= 100)             /* prevent underrun */
      {
        Threshold -= 100;               /* - 0.1 V */
        Flag |= UPDATE_THRESHOLD;       /* update status */
      }
    }
    else if (Test == KEY_LONG)     /* long key press */
    {
      /* change input impedance */
      if (Flag & INPUT_RH)              /* Rh selected */
      {
        /* change to Rl */
        /* set probes: probe #1 - Rl - Gnd */
        R_DDR = Probes.Rl_1;            /* pull down probe #1 via Rl */
        Flag &= ~INPUT_RH;              /* clear flag */
      }
      else                              /* Rl selected */
      {
        /* change to Rh */
        /* set probes: probe #1 - Rh - Gnd */
        R_DDR = Probes.Rh_1;            /* pull down probe #1 via Rh */
        Flag |= INPUT_RH;               /* set flag */
      }

      Flag |= UPDATE_INPUT;             /* update status */

#if 0
      /* alternative: reset voltage threshold */
      if (Flag & BUZZER_ON)             /* buzzer enabled */
      {
        /* set threshold to default value */
        Threshold = THRESHOLD;          /* set default */
        Flag |= UPDATE_THRESHOLD;       /* update status */
      }
#endif
    }
    #endif
  }


  /*
   *  clean up
   */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef BUZZER_ON
  #undef UPDATE_THRESHOLD
  #undef INPUT_RH
  #undef UPDATE_INPUT

  /* default threshold */
  #undef THRESHOLD
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef TOOLS_MISC_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

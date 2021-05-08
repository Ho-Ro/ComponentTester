/* ************************************************************************
 *
 *   misc tools (hardware and software options)
 *
 *   (c) 2012-2021 by Markus Reschke
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



/*
 *  local constants
 *  - simplify ifdefs
 */

/* ProbePinout() */
#if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS) || defined (SW_SQUAREWAVE) || defined (SW_SERVO) || defined (SW_ESR_TOOL)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif

#if defined (SW_MONITOR_R) || defined (SW_MONITOR_C) || defined (SW_MONITOR_L) || defined(SW_MONITOR_RCL) || defined(SW_MONITOR_RL)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif


/* ************************************************************************
 *   support functions
 * ************************************************************************ */


#ifdef FUNC_PROBE_PINOUT

/*
 *  display probe pinout
 *
 *  required:
 *  - Mode
 *    PROBES_PWM  PWM signal
 *    PROBES_ESR  ESR measurement
 *    PROBES_RCL  monitoring RCL
 */

void ProbePinout(uint8_t Mode)
{
  uint8_t           ID_1 = 0;      /* character for probe #1 */
  uint8_t           ID_2 = 0;      /* character for probe #2 */
  uint8_t           ID_3 = 0;      /* character for probe #3 */

  LCD_ClearLine2();                /* info goes to line #2 */

  if (Mode == PROBES_PWM)          /* PWM signal */
  {
    /* probe #1: Gnd / probe #2: signal / probe #3: Gnd */
    ID_1 = '-';
    ID_2 = 's';
    ID_3 = '-';
  }
  #ifdef SW_ESR_TOOL
  else if (Mode == PROBES_ESR)     /* ESR measurement */
  {
    /* probe #1: + / probe #3: - */
    ID_1 = '+';
    ID_2 = 0;
    ID_3 = '-';
  }
  #endif
  #if defined (SW_MONITOR_R) || defined (SW_MONITOR_C) || defined (SW_MONITOR_L) || defined(SW_MONITOR_RCL) || defined(SW_MONITOR_RL)
  else if (Mode == PROBES_RCL)     /* monitoring RCL */
  {
    /* probe #1: * / probe #3: * */
    ID_1 = '*';
    ID_2 = 0;
    ID_3 = '*';
  }
  #endif

  Show_SimplePinout(ID_1, ID_2, ID_3);  /* display pinout */

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
 *  - test push button enables boost converter
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
  Display_NextLine();
  Display_Minus();                 /* display "no value" */


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
          Display_Value(U1, -2, 'V');   /* display current voltage */
        #else
          Display_Value(U1, -3, 'V');   /* display current voltage */
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
 *  Zener tool (alternative mode)
 *  - hardware option for voltage measurement of Zener diode
 *    or external voltage
 *  - uses dedicated analog input (TP_ZENER) with voltage divider
 *    (default 10:1)
 *  - boost converter runs all the time or circuit without boost converter
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
 *  - boost converter runs all the time or circuit without boost converter
 */

void CheckZener(void)
{
  uint16_t               U1;            /* voltage */
  #ifdef ZENER_DIVIDER_CUSTOM
  uint32_t               Value;         /* value */
  #endif

  /* get voltage */
  U1 = ReadU(TP_ZENER);            /* read voltage (in mV) */

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

/* local constants */
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
          Show_SemiPinout('A', 'B', 'C');

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

/* local constants */
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
  uint8_t           Probe3;             /* ID of probe #3 */
  uint16_t          U1;                 /* voltage */

  /* update all three probes */
  Probe3 = GetThirdProbe(Probe1, Probe2);    /* get third one */
  UpdateProbes(Probe1, Probe2, Probe3);      /* update probes */

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

  /* status */
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
        /* update all three probes for remaining checks */
        Test = GetThirdProbe(Diodes[0].A, Diodes[0].C);  /* get third probe */
        UpdateProbes(Diodes[0].A, Diodes[0].C, Test);    /* update probes */

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

  /* clean up */
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

  /* control flags (bitfield) */
  #define RUN_FLAG            0b00000001     /* run flag */
  #define CHANGED_MODE        0b00000100     /* mode has changed */

  /* mode */
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
      LCD_ClearLine(3);            /* clear line #3 */
      LCD_CharPos(1, 3);           /* move to line #3 */

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

  /* clean up */
  #undef MODE_NONE
  #undef MODE_CHARGE
  #undef MODE_LEAK
  #undef MODE_DISCHARGE

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
  UpdateProbes(PROBE_1, PROBE_3, 0);    /* set probes */
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
    /* measure and display C */
    Check.Found = COMP_NONE;                 /* no component */
    MeasureCap(PROBE_1, PROBE_3, 0);         /* measure capacitance */
    LCD_ClearLine2();                        /* clear line #2 */

    if (Check.Found == COMP_CAPACITOR)       /* found cap */
    {
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
    /* measure R */
    UpdateProbes(PROBE_1, PROBE_3, 0);  /* set probes */
    Check.Resistors = 0;                /* reset resistor counter */
    CheckResistor();                    /* check for resistor */
    LCD_ClearLine2();                   /* clear line #2 */

    if (Check.Resistors == 1)           /* found resistor */
    {
      /* get inductance and display if relevant */
      if (MeasureInductor(R1) == 1)
      {
        Display_Value(Inductor.Value, Inductor.Scale, 'H');
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
      UpdateProbes(PROBE_1, PROBE_3, 0);     /* set probes */
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
      MeasureCap(PROBE_1, PROBE_3, 0);       /* measure capacitance */

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
    UpdateProbes(PROBE_1, PROBE_3, 0);  /* set probes */
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
 *   clean-up of local constants
 * ************************************************************************ */


/* probes */
#undef PROBES_PWM
#undef PROBES_ESR
#undef PROBES_RCL

/* source management */
#undef TOOLS_MISC_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

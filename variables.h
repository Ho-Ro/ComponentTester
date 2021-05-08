/* ************************************************************************
 *
 *   global variables
 *
 *   (c) 2012-2014 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  include header files
 */

/* local includes */
#include "LCD.h"              /* LCD module */
#include "config.h"


/* ************************************************************************
 *   internal variables
 * ************************************************************************ */


/*
 *  only included by
 *  - main.c
 */

#if defined (MAIN_C)

  /*
   *  global variables
   */

  /* buffers */
  char              OutBuffer[12];           /* output buffer */

  /* configuration */
  Config_Type       Config;                  /* tester modes, offsets and values */

  /* probing */
  Probe_Type        Probes;                  /* test probes */
  Check_Type        Check;                   /* checking/testing */

  /* components */
  Resistor_Type     Resistors[3];            /* resistors (3 combinations) */
  Capacitor_Type    Caps[3];                 /* capacitors (3 combinations) */
  Diode_Type        Diodes[6];               /* diodes (3 combinations in 2 directions) */
  Semi_Type         Semi;                    /* semiconductor (BJT, FET, ...) */

  #ifdef SW_INDUCTOR
    Inductor_Type   Inductor;                /* inductor */
  #endif

  #ifdef HW_ENCODER
    RotaryEncoder_Type        Enc;           /* rotary encoder */
  #endif


  /*
   *  NVRAM values (stored in EEPROM) with their defaults
   */

#define CHECKSUM (uint8_t)R_MCU_LOW + (uint8_t)R_MCU_HIGH + (uint8_t)R_ZERO + C_ZERO + (uint8_t)UREF_OFFSET + (uint8_t)COMPARATOR_OFFSET
  const uint16_t    NV_RiL EEMEM = R_MCU_LOW;
  const uint16_t    NV_RiH EEMEM = R_MCU_HIGH;
  const uint16_t    NV_RZero EEMEM = R_ZERO;
  const uint8_t     NV_CapZero EEMEM = C_ZERO;
  const int8_t      NV_RefOffset EEMEM = UREF_OFFSET;
  const int8_t      NV_CompOffset EEMEM = COMPARATOR_OFFSET;
//  const int8_t      NV_Flags EEMEM = 0;
  const uint8_t     NV_Checksum EEMEM = (uint8_t)(CHECKSUM);


  /*
   *  constant strings (stored in EEPROM)
   */

  /* language specific: German */
  #if defined (UI_GERMAN)
    /*
     *  LCD ä 11100001 -> á
     *  LCD ö 11101111 -> ï
     *  LCD ü 11110101 -> õ
     *  LCD ß 11100010 -> â
     */
    const unsigned char Running_str[] EEMEM = "Suche...";
    const unsigned char Weak_str[] EEMEM = "schwach";
    const unsigned char Low_str[] EEMEM = "leer";
    const unsigned char Failed1_str[] EEMEM = "Kein Bauteil";
    const unsigned char Failed2_str[] EEMEM = "gefunden!";
    const unsigned char Done_str[] EEMEM = "fertig!";
    const unsigned char Select_str[] EEMEM = "Wáhle";
    const unsigned char Selftest_str[] EEMEM = "Selbsttest";
    const unsigned char Adjustment_str[] EEMEM = "Abgleich";
    const unsigned char Save_str[] EEMEM = "Speichern";
    const unsigned char Show_str[] EEMEM = "Werte zeigen";
    const unsigned char Remove_str[] EEMEM = "Entferne";
    const unsigned char Create_str[] EEMEM = "Baue";
    const unsigned char ShortCircuit_str[] EEMEM = "Kurzschluss!";
    const unsigned char DischargeFailed_str[] EEMEM = "Batterie?";
    const unsigned char Error_str[] EEMEM = "Fehler!";
    const unsigned char Exit_str[] EEMEM = "Abbrechen";
    const unsigned char BJT_str[] EEMEM = "Transistor";
    const unsigned char Thyristor_str[] EEMEM = "Thyristor";
    const unsigned char Triac_str[] EEMEM = "Triac";
    const unsigned char Bye_str[] EEMEM = "Ciao!";
    #ifdef SW_SIGNAL_GEN
      const unsigned char FreqGen_str[] EEMEM = "Signal Gen.";
    #endif
    #ifdef HW_ZENER
      const unsigned char Zener_str[] EEMEM = "Zener";
      const unsigned char Min_str[] EEMEM = "Min";
    #endif
    #ifdef HW_FREQ_COUNTER
      const unsigned char FreqCounter_str[] EEMEM = "Freq. Záhler";
    #endif
    #ifdef SW_ENCODER
      const unsigned char Encoder_str[] EEMEM = "Dreh-Encoder";
      const unsigned char TurnRight_str[] EEMEM = "Drehe rechts!";
    #endif

  /* language specific: Czech */
  #elif defined (UI_CZECH)
    const unsigned char Running_str[] EEMEM = "Probiha mereni..";
    const unsigned char Weak_str[] EEMEM = "slaba!";
    const unsigned char Low_str[] EEMEM =  "vybita!";
    const unsigned char Failed1_str[] EEMEM = "Zadna soucastka";
    const unsigned char Failed2_str[] EEMEM = "neznama - vadna";
    const unsigned char Done_str[] EEMEM = "hotovo!";
    const unsigned char Select_str[] EEMEM = "Vyber:";
    const unsigned char Selftest_str[] EEMEM = "Autotest";
    const unsigned char Adjustment_str[] EEMEM = "Kalibrace";
    const unsigned char Save_str[] EEMEM = "Ulozit";
    const unsigned char Show_str[] EEMEM = "Zobraz hodnoty";
    const unsigned char Remove_str[] EEMEM = "Odstranit";
    const unsigned char Create_str[] EEMEM = "Udelej";
    const unsigned char ShortCircuit_str[] EEMEM = "zkrat!";
    const unsigned char DischargeFailed_str[] EEMEM = "Baterie?";
    const unsigned char Error_str[] EEMEM = "Chyba!";
    const unsigned char Exit_str[] EEMEM = "Prerusit";
    const unsigned char BJT_str[] EEMEM = "Tranzistor";
    const unsigned char Thyristor_str[] EEMEM = "Tyrystor";
    const unsigned char Triac_str[] EEMEM = "Triak";
    const unsigned char Bye_str[] EEMEM = "Nashledanou...";
    #ifdef SW_SIGNAL_GEN
      const unsigned char FreqGen_str[] EEMEM = "Signal Gen.";
    #endif
    #ifdef HW_ZENER
      const unsigned char Zener_str[] EEMEM = "Zener";
      const unsigned char Min_str[] EEMEM = "Min";
    #endif
    #ifdef HW_FREQ_COUNTER
      const unsigned char FreqCounter_str[] EEMEM = "Freq. Counter";
    #endif
    #ifdef SW_ENCODER
      const unsigned char Encoder_str[] EEMEM = "Rotary Encoder";
      const unsigned char TurnRight_str[] EEMEM = "Turn right!";
    #endif

  /* language specific: another language */
  #elif defined (UI_WHATEVER)

  /* language specific: English (default) */
  #else
    const unsigned char Running_str[] EEMEM = "Probing...";
    const unsigned char Weak_str[] EEMEM = "weak";
    const unsigned char Low_str[] EEMEM = "low";
    const unsigned char Failed1_str[] EEMEM = "No component";
    const unsigned char Failed2_str[] EEMEM = "found!";
    const unsigned char Done_str[] EEMEM = "done!";
    const unsigned char Select_str[] EEMEM = "Select";
    const unsigned char Selftest_str[] EEMEM = "Selftest";
    const unsigned char Adjustment_str[] EEMEM = "Adjustment";
    const unsigned char Save_str[] EEMEM = "Save";
    const unsigned char Show_str[] EEMEM = "Show Values";
    const unsigned char Remove_str[] EEMEM = "Remove";
    const unsigned char Create_str[] EEMEM = "Create";
    const unsigned char ShortCircuit_str[] EEMEM = "Short Circuit!";
    const unsigned char DischargeFailed_str[] EEMEM = "Battery?";
    const unsigned char Error_str[] EEMEM = "Error!";
    const unsigned char Exit_str[] EEMEM = "Exit";
    const unsigned char BJT_str[] EEMEM = "BJT";
    const unsigned char Thyristor_str[] EEMEM = "SCR";
    const unsigned char Triac_str[] EEMEM = "Triac";
    const unsigned char Bye_str[] EEMEM = "Bye!";
    #ifdef SW_SIGNAL_GEN
      const unsigned char FreqGen_str[] EEMEM = "Signal Gen.";
    #endif
    #ifdef HW_ZENER
      const unsigned char Zener_str[] EEMEM = "Zener";
      const unsigned char Min_str[] EEMEM = "Min";
    #endif
    #ifdef HW_FREQ_COUNTER
      const unsigned char FreqCounter_str[] EEMEM = "Freq. Counter";
    #endif
    #ifdef SW_ENCODER
      const unsigned char Encoder_str[] EEMEM = "Rotary Encoder";
      const unsigned char TurnRight_str[] EEMEM = "Turn right!";
    #endif
  #endif

  /* language independent */
  const unsigned char Tester_str[] EEMEM = "Component Tester";
  const unsigned char Battery_str[] EEMEM = "Bat.";
  const unsigned char OK_str[] EEMEM = "ok";
  const unsigned char MOS_str[] EEMEM = "MOS";
  const unsigned char FET_str[] EEMEM = "FET";
  const unsigned char Channel_str[] EEMEM = "-ch";
  const unsigned char Enhancement_str[] EEMEM = "enh.";
  const unsigned char Depletion_str[] EEMEM = "dep.";
  const unsigned char IGBT_str[] EEMEM = "IGBT";
  const unsigned char GateCap_str[] EEMEM = "Cgs";
  const unsigned char NPN_str[] EEMEM = "NPN";
  const unsigned char PNP_str[] EEMEM = "PNP";
  const unsigned char hFE_str[] EEMEM ="h_FE";
  const unsigned char V_BE_str[] EEMEM ="V_BE";
  const unsigned char V_GT_str[] EEMEM ="V_GT";
  const unsigned char I_CEO_str[] EEMEM = "I_CEO";
  const unsigned char Vf_str[] EEMEM = "Vf";
  const unsigned char DiodeCap_str[] EEMEM = "C";
  const unsigned char Vth_str[] EEMEM = "Vth";
  const unsigned char I_R_str[] EEMEM = "I_R";
  const unsigned char Timeout_str[] EEMEM = "Timeout";
  const unsigned char URef_str[] EEMEM = "Vref";
  const unsigned char RhLow_str[] EEMEM = "Rh-";
  const unsigned char RhHigh_str[] EEMEM = "Rh+";
  const unsigned char RiLow_str[] EEMEM = "Ri-";
  const unsigned char RiHigh_str[] EEMEM = "Ri+";
  const unsigned char Rl_str[] EEMEM = "+Rl-";
  const unsigned char Rh_str[] EEMEM = "+Rh-";
  const unsigned char ProbeComb_str[] EEMEM = "12 13 23";
  const unsigned char CapOffset_str[] EEMEM = "C0";
  const unsigned char ROffset_str[] EEMEM = "R0";
  const unsigned char Vcc_str[] EEMEM = "Vcc";
  const unsigned char CompOffset_str[] EEMEM = "AComp";
  const unsigned char Checksum_str[] EEMEM = "ChkSum";

  #ifdef SW_ESR
    const unsigned char Probes_str[] EEMEM = "Pins";
    const unsigned char ESR_str[] EEMEM = "ESR";
    const unsigned char ESR_Probes_str[] EEMEM = "1-2";
  #endif
  #ifdef SW_PWM
    const unsigned char PWM_str[] EEMEM = "PWM";
    const unsigned char PWM_Probes_str[] EEMEM = "2-13";
    const unsigned char Hertz_str[] EEMEM = "Hz";
  #endif


  /* component symbols */
  const unsigned char Cap_str[] EEMEM = {'-',LCD_CHAR_CAP, '-',0};
  const unsigned char Diode_AC_str[] EEMEM = {'-', LCD_CHAR_DIODE_AC, '-', 0};
  const unsigned char Diode_CA_str[] EEMEM = {'-', LCD_CHAR_DIODE_CA, '-', 0};
  const unsigned char Resistor_str[] EEMEM = {'-', LCD_CHAR_RESISTOR_L, LCD_CHAR_RESISTOR_R, '-', 0};

  /* version */
  const unsigned char Version_str[] EEMEM = "v1.16m";


  /*
   *  constant custom characters for LCD (stored EEPROM)
   */

  /* diode icon with anode at left side */
  const unsigned char DiodeIcon1[] EEMEM = {0x11, 0x19, 0x1d, 0x1f, 0x1d, 0x19, 0x11, 0x00};

  /* diode icon with anode at right side */
  const unsigned char DiodeIcon2[] EEMEM = {0x11, 0x13, 0x17, 0x1f, 0x17, 0x13, 0x11, 0x00};

  /* capacitor icon */
  const unsigned char CapIcon[] EEMEM = {0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x00};

  /* resistor icon #1 (left part) */
  const unsigned char ResIcon1[] EEMEM = {0x00, 0x0f, 0x08, 0x18, 0x08, 0x0f, 0x00, 0x00};

  /* resistor icon #2 (right part) */
  const unsigned char ResIcon2[] EEMEM = {0x00, 0x1e, 0x02, 0x03, 0x02, 0x1e, 0x00, 0x00};

  /* cyrillic character set (omega and µ aren't included) */
  #ifdef LCD_CYRILLIC
    /* omega */
    const unsigned char OmegaIcon[] EEMEM = {0, 0, 14, 17, 17, 10, 27, 0};

    /* µ */
    const unsigned char MicroIcon[] EEMEM = {0, 17, 17, 17, 19, 29, 16, 16};
  #endif


  /*
   *  contant tables (stored in EEPROM or PROGMEM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M (used by value display) */
  const unsigned char Prefix_table[] EEMEM = {'p', 'n', LCD_CHAR_MICRO, 'm', 0, 'k', 'M'};

  /* voltage based factors for large caps (using Rl) */
  /* voltage in mV:                             300    325    350    375    400    425    450    475    500    525    550    575    600    625    650   675   700   725   750   775   800   825   850   875   900   925   950   975  1000  1025  1050  1075  1100  1125  1150  1175  1200  1225  1250  1275  1300  1325  1350  1375  1400 */
  const uint16_t LargeCap_table[] MEM_TEXT = {23022, 21195, 19629, 18272, 17084, 16036, 15104, 14271, 13520, 12841, 12224, 11660, 11143, 10668, 10229, 9822, 9445, 9093, 8765, 8458, 8170, 7900, 7645, 7405, 7178, 6963, 6760, 6567, 6384, 6209, 6043, 5885, 5733, 5589, 5450, 5318, 5191, 5069, 4952, 4839, 4731, 4627, 4526, 4430, 4336};

  /* voltage based factors for small caps (using Rh) */
  /* voltages in mV:                         1000 1050 1100 1150 1200 1250 1300 1350 1400 */
  const uint16_t SmallCap_table[] MEM_TEXT = {954, 903, 856, 814, 775, 740, 707, 676, 648};
//const uint16_t SmallCap_table[] MEM_TEXT = {9535, 9026, 8563, 8141, 7753, 7396, 7066, 6761, 6477}; 

  #ifdef SW_PWM
    /* PWM menu: frequencies */    
    const uint16_t PWM_Freq_table[] MEM_TEXT = {100, 250, 500, 1000, 2500, 5000, 10000, 25000};
  #endif

  #ifdef SW_INDUCTOR
    /* ratio based factors for inductors */
    /* ratio:                                    200   225   250   275   300   325   350   375   400   425   450   475   500   525   550   575   600   625  650  675  700  725  750  775  800  825  850  875  900  925  950  975 */
    const uint16_t Inductor_table[] MEM_TEXT = {4481, 3923, 3476, 3110, 2804, 2544, 2321, 2128, 1958, 1807, 1673, 1552, 1443, 1343, 1252, 1169, 1091, 1020, 953, 890, 831, 775, 721, 670, 621, 574, 527, 481, 434, 386, 334, 271};
  #endif

  #if defined (HW_FREQ_COUNTER) || defined (SW_SIGNAL_GEN)
    /* Timer1 prescalers and corresponding bitmasks */
    const uint16_t T1_Prescaler_table[] MEM_TEXT = {1, 8, 64, 256, 1024};
    const uint8_t T1_Bitmask_table[] MEM_TEXT = {(1 << CS10), (1 << CS11), (1 << CS11) | (1 << CS10), (1 << CS12), (1 << CS12) | (1 << CS10)};
  #endif


  /*
   *  bitmask tables for probe settings (stored in EEPROM)
   *  - they save some bytes in the firmware.
   */

  /* bitmasks for Rl probe resistors based on probe ID */
  const unsigned char Rl_table[] EEMEM = {(1 << (TP1 * 2)), (1 << (TP2 * 2)), (1 << (TP3 * 2))};

  /* bitmasks for Rh probe resistors based on probe ID */
  const unsigned char Rh_table[] EEMEM = {(2 << (TP1 * 2)), (2 << (TP2 * 2)), (2 << (TP3 * 2))};

  /* bitmasks for ADC pins based on probe ID */
  const unsigned char ADC_table[] EEMEM = {(1 << TP1), (1 << TP2), (1 << TP3)};



/* ************************************************************************
 *   external variables
 * ************************************************************************ */


/*
 *  included by all other source files
 */

#else

  /*
   *  global variables
   */

  /* buffers */
  extern char            OutBuffer[12];           /* output buffer */

  /* configuration */
  extern Config_Type     Config;                  /* offsets and values */

  /* probing */
  extern Probe_Type      Probes;                  /* test probes */
  extern Check_Type      Check;                   /* checking/testing */

  /* components */
  extern Resistor_Type   Resistors[3];            /* resistors (3 combinations) */
  extern Capacitor_Type  Caps[3];                 /* capacitors (3 combinations) */
  extern Diode_Type      Diodes[6];               /* diodes (3 combinations in 2 directions) */
  extern Semi_Type       Semi;                    /* semiconductor (BJT, FET, ...) */

  #ifdef SW_INDUCTOR
    extern Inductor_Type          Inductor;       /* inductor */
  #endif

  #ifdef HW_ENCODER
    extern RotaryEncoder_Type      Enc;           /* rotary encoder */
  #endif


  /*
   *  NVRAM values (stored in EEPROM) with their defaults
   */

  extern const uint16_t  NV_RiL;
  extern const uint16_t  NV_RiH;
  extern const uint16_t  NV_RZero;
  extern const uint8_t   NV_CapZero;
  extern const int8_t    NV_RefOffset;
  extern const int8_t    NV_CompOffset;
  extern const uint8_t   NV_Checksum;


  /*
   *  constant strings (stored in EEPROM)
   */

  extern const unsigned char Running_str[];
  extern const unsigned char Done_str[];
  extern const unsigned char Select_str[];
  extern const unsigned char Selftest_str[];
  extern const unsigned char Adjustment_str[];
  extern const unsigned char Save_str[];
  extern const unsigned char Show_str[];
  extern const unsigned char Remove_str[];
  extern const unsigned char Create_str[];
  extern const unsigned char ShortCircuit_str[];
  extern const unsigned char DischargeFailed_str[];
  extern const unsigned char Error_str[];
  extern const unsigned char Exit_str[];

  extern const unsigned char URef_str[];
  extern const unsigned char RhLow_str[];
  extern const unsigned char RhHigh_str[];
  extern const unsigned char RiLow_str[];
  extern const unsigned char RiHigh_str[];
  extern const unsigned char Rl_str[];
  extern const unsigned char Rh_str[];
  extern const unsigned char ProbeComb_str[];
  extern const unsigned char CapOffset_str[];
  extern const unsigned char ROffset_str[];
  extern const unsigned char Vcc_str[];
  extern const unsigned char CompOffset_str[];
  extern const unsigned char Checksum_str[];

  /* options */
  #ifdef SW_ESR
    extern const unsigned char Probes_str[];
    extern const unsigned char ESR_str[];
    extern const unsigned char ESR_Probes_str[];
  #endif
  #ifdef SW_PWM
    extern const unsigned char PWM_str[];
    extern const unsigned char PWM_Probes_str[];
    extern const unsigned char Hertz_str[];
  #endif
  #ifdef SW_SIGNAL_GEN
    extern const unsigned char FreqGen_str[];
  #endif
  #ifdef HW_ZENER
    extern const unsigned char Zener_str[];
    extern const unsigned char Min_str[];
  #endif
  #ifdef HW_FREQ_COUNTER
    extern const unsigned char FreqCounter_str[];
  #endif
  #ifdef SW_ENCODER
    extern const unsigned char Encoder_str[];
    extern const unsigned char TurnRight_str[];
  #endif

  /*
   *  constant tables (stored in EEPROM or PROGMEM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M (used by value display) */
  extern const unsigned char Prefix_table[];

  /* voltage based factors for large caps (using Rl) */
  extern const uint16_t LargeCap_table[];

  /* voltage based factors for small caps (using Rh) */
  extern const uint16_t SmallCap_table[];

  #ifdef SW_PWM
    /* PWM menu: frequencies */
    extern const uint16_t PWM_Freq_table[];
  #endif

  #ifdef SW_INDUCTOR
    /* voltage based factors for inductors */
    extern const uint16_t Inductor_table[];
  #endif

  #if defined (HW_FREQ_COUNTER) || defined (SW_SIGNAL_GEN)
    /* Timer1 prescalers and corresponding bitmasks */
    extern const uint16_t T1_Prescaler_table[];
    extern const uint8_t T1_Bitmask_table[];
  #endif


  /*
   *  bitmask tables for probe settings
   *  - they save some bytes in the firmware
   */

  /* bitmasks for Rl probe resistors based on probe ID */
  extern const unsigned char Rl_table[];

  /* bitmasks for ADC pins based on probe ID */
  extern const unsigned char ADC_table[];

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

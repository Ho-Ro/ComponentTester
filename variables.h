/* ************************************************************************
 *
 *   global variables
 *
 *   (c) 2012-2013 by Markus Reschke
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
  char           OutBuffer[12];           /* output buffer */

  /* configuration */
  Config_Type    Config;                  /* offsets and values */

  /* probing */
  uint8_t        CompDone;                /* flag for component detection done */
  uint8_t        CompFound;               /* component type which was found */ 
  uint8_t        CompType;                /* component specific subtype */
  uint8_t        ResistorsFound;          /* number of resistors found */
  uint8_t        DiodesFound;             /* number of diodes found */

  /* components */
  Resistor_Type  Resistors[3];            /* resistors (3 combinations) */
  Capacitor_Type Caps[3];                 /* capacitors (3 combinations) */
  Diode_Type     Diodes[6];               /* diodes (3 combinations in 2 directions) */
  BJT_Type       BJT;                     /* bipolar junction transistor */
  FET_Type       FET;                     /* FET */
  Error_Type     Error;                   /* error */


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
  const uint8_t     NV_Checksum EEMEM = (uint8_t)(CHECKSUM);


  /*
   *  constant strings (stored in EEPROM)
   */

  /* language specific: German */
  #if defined (UI_GERMAN)
    const unsigned char Mode_str[] EEMEM = "Modus:";
    const unsigned char Continous_str[] EEMEM = "Fortlaufend";
    const unsigned char AutoHold_str[] EEMEM = "Einzelschritt";
    const unsigned char Running_str[] EEMEM = "Suche...";
    const unsigned char Weak_str[] EEMEM = "schwach";
    const unsigned char Low_str[] EEMEM = "leer";
    const unsigned char Failed1_str[] EEMEM = "Kein Bauteil";
    const unsigned char Failed2_str[] EEMEM = "gefunden!";
    const unsigned char Thyristor_str[] EEMEM = "Thyristor";
    const unsigned char Triac_str[] EEMEM = "Triac";
    const unsigned char GAK_str[] EEMEM = "GAK=";
    const unsigned char Done_str[] EEMEM = "fertig!";
    const unsigned char Select_str[] EEMEM = "Wähle";
    const unsigned char Selftest_str[] EEMEM = "Selbsttest";
    const unsigned char Adjustment_str[] EEMEM = "Abgleich";
    const unsigned char Save_str[] EEMEM = "Speichern";
    const unsigned char Show_str[] EEMEM = "Werte zeigen";
    const unsigned char Remove_str[] EEMEM = "Entferne";
    const unsigned char Create_str[] EEMEM = "Baue";
    const unsigned char ShortCircuit_str[] EEMEM = "Kurzschluss!";
    const unsigned char DischargeFailed_str[] EEMEM = "Batterie?";
    const unsigned char Error_str[] EEMEM = "Fehler!";

  /* language specific: another language */
  #elif defined (UI_WHATEVER)


  /* language specific: English (default) */
  #else
    const unsigned char Mode_str[] EEMEM = "Mode:";
    const unsigned char Continous_str[] EEMEM = "Continous";
    const unsigned char AutoHold_str[] EEMEM = "Auto Hold";
    const unsigned char Running_str[] EEMEM = "Probing...";
    const unsigned char Weak_str[] EEMEM = "weak";
    const unsigned char Low_str[] EEMEM = "low";
    const unsigned char Failed1_str[] EEMEM = "No component";
    const unsigned char Failed2_str[] EEMEM = "found!";
    const unsigned char Thyristor_str[] EEMEM = "SCR";
    const unsigned char Triac_str[] EEMEM = "Triac";
    const unsigned char GAK_str[] EEMEM = "GAC=";
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
  #endif

  /* language independent */
  const unsigned char Battery_str[] EEMEM = "Bat.";
  const unsigned char OK_str[] EEMEM = "ok";
  const unsigned char MOS_str[] EEMEM = "MOS";
  const unsigned char FET_str[] EEMEM = "FET";
  const unsigned char Channel_str[] EEMEM = "-ch";
  const unsigned char Enhancement_str[] EEMEM = "enh.";
  const unsigned char Depletion_str[] EEMEM = "dep.";
  const unsigned char IGBT_str[] EEMEM = "IGBT";
  const unsigned char GateCap_str[] EEMEM = "Cgs=";
  const unsigned char GDS_str[] EEMEM = "GDS=";
  const unsigned char NPN_str[] EEMEM = "NPN";
  const unsigned char PNP_str[] EEMEM = "PNP";
  const unsigned char EBC_str[] EEMEM = "EBC=";
  const unsigned char hfe_str[] EEMEM ="B=";
  const unsigned char Vf_str[] EEMEM = "Vf=";
  const unsigned char DiodeCap_str[] EEMEM = "C=";
  const unsigned char Vth_str[] EEMEM = "Vth=";
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
  const unsigned char CompOffset_str[] EEMEM = "AComp";
  const unsigned char Checksum_str[] EEMEM = "ChkSum";
  const unsigned char PWM_str[] EEMEM = "PWM";
  const unsigned char Hertz_str[] EEMEM = "Hz";

  const unsigned char Cap_str[] EEMEM = {'-',LCD_CHAR_CAP, '-',0};
  const unsigned char Diode_AC_str[] EEMEM = {'-', LCD_CHAR_DIODE1, '-', 0};
  const unsigned char Diode_CA_str[] EEMEM = {'-', LCD_CHAR_DIODE2, '-', 0};
  const unsigned char Diodes_str[] EEMEM = {'*', LCD_CHAR_DIODE1, ' ', ' ', 0};
  const unsigned char Resistor_str[] EEMEM = {'-', LCD_CHAR_RESIS1, LCD_CHAR_RESIS2, '-', 0};

  const unsigned char Version_str[] EEMEM = "v1.07m";


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

  /* PWM menu: frequencies */
  const uint16_t PWM_Freq_table[] MEM_TEXT = {100, 250, 500, 1000, 2500, 5000, 10000, 25000};

  /* voltage based factors for large caps (using Rl) */
  /* voltage in mV:                             300    325    350    375    400    425    450    475    500    525    550    575    600    625    650   675   700   725   750   775   800   825   850   875   900   925   950   975  1000  1025  1050  1075  1100  1125  1150  1175  1200  1225  1250  1275  1300  1325  1350  1375  1400 */
  const uint16_t LargeCap_table[] MEM_TEXT = {23022, 21195, 19629, 18272, 17084, 16036, 15104, 14271, 13520, 12841, 12224, 11660, 11143, 10668, 10229, 9822, 9445, 9093, 8765, 8458, 8170, 7900, 7645, 7405, 7178, 6963, 6760, 6567, 6384, 6209, 6043, 5885, 5733, 5589, 5450, 5318, 5191, 5069, 4952, 4839, 4731, 4627, 4526, 4430, 4336};


  /* voltage based factors for small caps (using Rh) */
  /* voltages in mV:                         1000 1050 1100 1150 1200 1250 1300 1350 1400 */
  const uint16_t SmallCap_table[] MEM_TEXT = {954, 903, 856, 814, 775, 740, 707, 676, 648};
//const uint16_t SmallCap_table[] MEM_TEXT = {9535, 9026, 8563, 8141, 7753, 7396, 7066, 6761, 6477}; 


  /*
   *  bitmask tables for probe settings (stored in EEPROM)
   *  - they save some bytes in the firmware.
   */

  /* bitmasks for Rl probe resistors based on probe ID */
  const unsigned char Rl_table[] EEMEM = {(1 << (TP1 * 2)), (1 << (TP2 * 2)), (1 << (TP3 * 2))};

  /* bitmasks for Rh probe resistors based on probe ID */
//  const unsigned char Rh_table[] EEMEM = {(2 << (TP1 * 2)), (2 << (TP2 * 2)), (2 << (TP3 * 2))};

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
  extern char           OutBuffer[12];           /* output buffer */

  /* configuration */
  extern Config_Type    Config;                  /* offsets and values */

  /* probing */
  extern uint8_t        CompDone;                /* flag: component detection done */
  extern uint8_t        CompFound;               /* component type which was found */ 
  extern uint8_t        CompType;                /* component specific type */
  extern uint8_t        ResistorsFound;          /* number of resistors found */
  extern uint8_t        DiodesFound;             /* number of diodes found */

  /* components */
  extern Resistor_Type  Resistors[3];            /* resistors (3 combinations) */
  extern Capacitor_Type Caps[3];                 /* capacitors (3 combinations) */
  extern Diode_Type     Diodes[6];               /* diodes (3 combinations in 2 directions) */
  extern BJT_Type       BJT;                     /* bipolar junction transistor */
  extern FET_Type       FET;                     /* FET */
  extern Error_Type     Error;                   /* error */


  /*
   *  NVRAM values (stored in EEPROM) with their defaults
   */

  extern const uint16_t    NV_RiL;
  extern const uint16_t    NV_RiH;
  extern const uint16_t    NV_RZero;
  extern const uint8_t     NV_CapZero;
  extern const int8_t      NV_RefOffset;
  extern const int8_t      NV_CompOffset;
  extern const uint8_t     NV_Checksum;


  /*
   *  constant strings (stored in EEPROM)
   */

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
  extern const unsigned char CompOffset_str[];
  extern const unsigned char Checksum_str[];
  extern const unsigned char PWM_str[];
  extern const unsigned char Hertz_str[];


  /*
   *  constant tables (stored in EEPROM or PROGMEM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M (used by value display) */
  extern const unsigned char Prefix_table[];

  /* PWM menu: frequencies */
  extern const uint16_t PWM_Freq_table[];

  /* voltage based factors for large caps (using Rl) */
  extern const uint16_t LargeCap_table[];

  /* voltage based factors for small caps (using Rh) */
  extern const uint16_t SmallCap_table[];


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

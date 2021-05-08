/* ************************************************************************
 *
 *   global variables
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
  uint8_t        CompType;                /* component specific type */
  uint8_t        ResistorsFound;          /* number of resistors found */
  uint8_t        DiodesFound;             /* number of diodes found */

  /* components */
  Resistor_Type  Resistors[3];            /* resistors (3 combinations) */
  Capacitor_Type Caps[3];                 /* capacitors (3 combinations) */
  Diode_Type     Diodes[6];               /* diodes (3 combinations in 2 directions) */
  BJT_Type       BJT;                     /* bipolar junction transistor */
  FET_Type       FET;                     /* FET */


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
   *  constant strings (stored in PROGMEM or EEPROM)
   */

  /* language specific: German */
  #if defined (UI_GERMAN)
    const unsigned char Mode_str[] MEM_TEXT = "Modus:";
    const unsigned char Continous_str[] MEM_TEXT = "Fortlaufend";
    const unsigned char AutoHold_str[] MEM_TEXT = "Einzelschritt";
    const unsigned char Running_str[] MEM_TEXT = "Suche...";
    const unsigned char Weak_str[] MEM_TEXT = "schwach";
    const unsigned char Low_str[] MEM_TEXT = "leer";
    const unsigned char Failed1_str[] MEM_TEXT = "Kein Bauteil";
    const unsigned char Failed2_str[] MEM_TEXT = "gefunden!";
    const unsigned char Thyristor_str[] MEM_TEXT = "Thyristor";
    const unsigned char Triac_str[] MEM_TEXT = "Triac";
    const unsigned char GAK_str[] MEM_TEXT = "GAK=";
    const unsigned char Done_str[] MEM_TEXT = "fertig";
    const unsigned char Selftest_str[] MEM_TEXT = "Selbsttest";
    const unsigned char Calibration_str[] MEM_TEXT = "Kalibrierung";
    const unsigned char Save_str[] MEM_TEXT = "Speichern";
    const unsigned char Show_str[] MEM_TEXT = "Werte";
    const unsigned char Remove_str[] MEM_TEXT = "Entferne";
    const unsigned char ShortCircuit_str[] MEM_TEXT = "Kurzschluss!";
    const unsigned char DischargeFailed_str[] MEM_TEXT = "Batterie?";
    const unsigned char Error_str[] MEM_TEXT = "Fehler";
  #endif

  /* language specific: English */
  #if defined (UI_ENGLISH)
    const unsigned char Mode_str[] MEM_TEXT = "Mode:";
    const unsigned char Continous_str[] MEM_TEXT = "Continous";
    const unsigned char AutoHold_str[] MEM_TEXT = "Auto Hold";
    const unsigned char Running_str[] MEM_TEXT = "Probing...";
    const unsigned char Weak_str[] MEM_TEXT = "weak";
    const unsigned char Low_str[] MEM_TEXT = "low";
    const unsigned char Failed1_str[] MEM_TEXT = "No component";
    const unsigned char Failed2_str[] MEM_TEXT = "found!";
    const unsigned char Thyristor_str[] MEM_TEXT = "SCR";
    const unsigned char Triac_str[] MEM_TEXT = "Triac";
    const unsigned char GAK_str[] MEM_TEXT = "GAC=";
    const unsigned char Done_str[] MEM_TEXT = "done";
    const unsigned char Selftest_str[] MEM_TEXT = "Selftest";
    const unsigned char Calibration_str[] MEM_TEXT = "Calibration";
    const unsigned char Save_str[] MEM_TEXT = "Save";
    const unsigned char Show_str[] MEM_TEXT = "Values";
    const unsigned char Remove_str[] MEM_TEXT = "Remove";
    const unsigned char ShortCircuit_str[] MEM_TEXT = "Short Circuit!";
    const unsigned char DischargeFailed_str[] MEM_TEXT = "Battery?";
    const unsigned char Error_str[] MEM_TEXT = "Error";
  #endif

  /* language independent */
  const unsigned char Battery_str[] MEM_TEXT = "Bat.";
  const unsigned char OK_str[] MEM_TEXT = "ok";
  const unsigned char MOS_str[] MEM_TEXT = "MOS";
  const unsigned char FET_str[] MEM_TEXT = "FET";
  const unsigned char Channel_str[] MEM_TEXT = "-ch";
  const unsigned char Enhancement_str[] MEM_TEXT = "enh.";
  const unsigned char Depletion_str[] MEM_TEXT = "dep.";
  const unsigned char IGBT_str[] MEM_TEXT = "IGBT";
  const unsigned char GateCap_str[] MEM_TEXT = "Cgs=";
  const unsigned char GDS_str[] MEM_TEXT = "GDS=";
  const unsigned char NPN_str[] MEM_TEXT = "NPN";
  const unsigned char PNP_str[] MEM_TEXT = "PNP";
  const unsigned char EBC_str[] MEM_TEXT = "EBC=";
  const unsigned char hfe_str[] MEM_TEXT ="B=";
  const unsigned char Vf_str[] MEM_TEXT = "Vf=";
  const unsigned char DiodeCap_str[] MEM_TEXT = "C=";
  const unsigned char Vth_str[] MEM_TEXT = "Vth=";
  const unsigned char Timeout_str[] MEM_TEXT = "Timeout";
  const unsigned char URef_str[] MEM_TEXT = "Vref";
  const unsigned char RhLow_str[] MEM_TEXT = "Rh-";
  const unsigned char RhHigh_str[] MEM_TEXT = "Rh+";
  const unsigned char RiLow_str[] MEM_TEXT = "Ri-";
  const unsigned char RiHigh_str[] MEM_TEXT = "Ri+";
  const unsigned char Rl_str[] MEM_TEXT = "+Rl-";
  const unsigned char Rh_str[] MEM_TEXT = "+Rh-";
  const unsigned char ProbeComb_str[] MEM_TEXT = "12 13 23";
  const unsigned char CapOffset_str[] MEM_TEXT = "C0";
  const unsigned char ROffset_str[] MEM_TEXT = "R0";
  const unsigned char CompOffset_str[] MEM_TEXT = "AComp";
  const unsigned char Checksum_str[] MEM_TEXT = "ChkSum";

  const unsigned char Cap_str[] MEM_TEXT = {'-',LCD_CHAR_CAP, '-',0};
  const unsigned char Diode_AC_str[] MEM_TEXT = {'-', LCD_CHAR_DIODE1, '-', 0};
  const unsigned char Diode_CA_str[] MEM_TEXT = {'-', LCD_CHAR_DIODE2, '-', 0};
  const unsigned char Diodes_str[] MEM_TEXT = {'*', LCD_CHAR_DIODE1, ' ', ' ', 0};
  const unsigned char Resistor_str[] MEM_TEXT = {'-', LCD_CHAR_RESIS1, LCD_CHAR_RESIS2, '-', 0};

  const unsigned char Version_str[] MEM_TEXT = "v1.03m";


  /*
   *  constant custom characters for LCD (stored in PROGMEM or EEPROM)
   */

  /* diode icon with anode at left side */
  const unsigned char DiodeIcon1[] MEM_TEXT = {0x11, 0x19, 0x1d, 0x1f, 0x1d, 0x19, 0x11, 0x00};

  /* diode icon with anode at right side */
  const unsigned char DiodeIcon2[] MEM_TEXT = {0x11, 0x13, 0x17, 0x1f, 0x17, 0x13, 0x11, 0x00};

  /* capacitor icon */
  const unsigned char CapIcon[] MEM_TEXT = {0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x00};

  /* resistor icon #1 (left part) */
  const unsigned char ResIcon1[] MEM_TEXT = {0x00, 0x0f, 0x08, 0x18, 0x08, 0x0f, 0x00, 0x00};

  /* resistor icon #2 (right part) */
  const unsigned char ResIcon2[] MEM_TEXT = {0x00, 0x1e, 0x02, 0x03, 0x02, 0x1e, 0x00, 0x00};

  /* cyrillic character set (omega and µ aren't included) */
  #ifdef LCD_CYRILLIC
    /* omega */
    const unsigned char OmegaIcon[] MEM_TEXT = {0, 0, 14, 17, 17, 10, 27, 0};

    /* µ */
    const unsigned char MicroIcon[] MEM_TEXT = {0, 17, 17, 17, 19, 29, 16, 16};
  #endif


  /*
   *  contant tables (stored in PROGMEM or EEPROM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M */
  const unsigned char Prefix_table[] PROGMEM = {'p', 'n', LCD_CHAR_MICRO, 'm', 0, 'k', 'M'};

  /* voltage based factors for large caps (using Rl) */
  /* voltage in mV:                             300    325    350    375    400    425    450    475    500    525    550    575    600    625    650   675   700   725   750   775   800   825   850   875   900   925   950   975  1000  1025  1050  1075  1100  1125  1150  1175  1200  1225  1250  1275  1300  1325  1350  1375  1400 */
//const uint16_t LargeCap_table[] MEM_TEXT = {22447, 20665, 19138, 17815, 16657, 15635, 14727, 13914, 13182, 12520, 11918, 11369, 10865, 10401,  9973, 9577, 9209, 8866, 8546, 8247, 7966, 7702, 7454, 7220, 6999, 6789, 6591, 6403, 6224, 6054, 5892, 5738, 5590, 5449, 5314, 5185, 5061, 4942, 4828, 4718, 4613, 4511, 4413, 4319, 4228};
  const uint16_t LargeCap_table[] MEM_TEXT = {23022, 21195, 19629, 18272, 17084, 16036, 15104, 14271, 13520, 12841, 12224, 11660, 11143, 10668, 10229, 9822, 9445, 9093, 8765, 8458, 8170, 7900, 7645, 7405, 7178, 6963, 6760, 6567, 6384, 6209, 6043, 5885, 5733, 5589, 5450, 5318, 5191, 5069, 4952, 4839, 4731, 4627, 4526, 4430, 4336};


  /* voltage based factors for small caps (using Rh) */
  /* voltages in mV:                         1000 1050 1100 1150 1200 1250 1300 1350 1400 */
  const uint16_t SmallCap_table[] MEM_TEXT = {954, 903, 856, 814, 775, 740, 707, 676, 648};
//const uint16_t SmallCap_table[] MEM_TEXT = {9535, 9026, 8563, 8141, 7753, 7396, 7066, 6761, 6477}; 


  /*
   *  bitmask tables for probe settings (stored in PROGMEM or EEPROM)
   *  - they save some bytes in the firmware.
   */

  /* bitmasks for Rl probe resistors based on probe ID */
  const unsigned char Rl_table[] MEM_TEXT = {(1 << (TP1 * 2)), (1 << (TP2 * 2)), (1 << (TP3 * 2))};

  /* bitmasks for Rh probe resistors based on probe ID */
//  const unsigned char Rh_table[] MEM_TEXT = {(2 << (TP1 * 2)), (2 << (TP2 * 2)), (2 << (TP3 * 2))};

  /* bitmasks for ADC pins based on probe ID */
  const unsigned char ADC_table[] MEM_TEXT = {(1 << TP1), (1 << TP2), (1 << TP3)};



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


  /*
   *  constant strings (stored in PROGMEM or EEPROM)
   */
  extern const unsigned char DischargeFailed_str[];


  /*
   *  contant tables (stored in PROGMEM or EEPROM)
   */

  /* prefixes with negative exponent: m, µ, n, p */
  extern const unsigned char PinRLtab[];

  /* prefixes with positive exponent: k, M, G */
  extern const unsigned char PinADCtab[];

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

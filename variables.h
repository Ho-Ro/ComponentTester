/* ************************************************************************
 *
 *   global variables
 *
 *   (c) 2012-2017 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  include header files
 */

/* local includes */
#ifndef COMMON_H
  #include "common.h"
#endif

#ifndef CONFIG_H
  #include "config.h"
#endif

#ifndef COLORS_H
  #include "colors.h"
#endif



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
  UI_Type           UI;                      /* user interface */
  Config_Type       Cfg;                     /* tester modes, offsets and values */
  Adjust_Type       NV;                      /* basic adjustment offsets and values */

  #ifdef HW_TOUCH
    Touch_Type      Touch;                   /* touch screen adjustment offsets */
  #endif

  /* probing */
  Probe_Type        Probes;                  /* test probes */
  Check_Type        Check;                   /* checking/testing */

  /* components */
  Resistor_Type     Resistors[3];            /* resistors */
  Capacitor_Type    Caps[3];                 /* capacitors */
  Diode_Type        Diodes[6];               /* diodes */
  Semi_Type         Semi;                    /* common semiconductor */
  AltSemi_Type      AltSemi;                 /* special semiconductor */

  #ifdef SW_INDUCTOR
    Inductor_Type   Inductor;                /* inductor */
  #endif

  #ifdef SW_PROBE_COLORS
    /* probe color coding */
    uint16_t        ProbeColors[3] = {COLOR_PROBE_1, COLOR_PROBE_2, COLOR_PROBE_3};
  #endif

  #ifdef HW_SPI
    SPI_Type        SPI;                     /* SPI */
  #endif

  #ifdef HW_I2C
    I2C_Type        I2C;                     /* I2C */
  #endif


  /*
   *  NVRAM values (stored in EEPROM) with their defaults
   */

  /* basic adjustment values: profile #1 */
  const Adjust_Type     NV_Adjust_1 EEMEM = {R_MCU_LOW, R_MCU_HIGH, R_ZERO, C_ZERO, UREF_OFFSET, COMPARATOR_OFFSET, LCD_CONTRAST, 0};

  /* basic adjustment values: profile #2 */
  const Adjust_Type     NV_Adjust_2 EEMEM = {R_MCU_LOW, R_MCU_HIGH, R_ZERO, C_ZERO, UREF_OFFSET, COMPARATOR_OFFSET, LCD_CONTRAST, 0};

  #ifdef HW_TOUCH
    /* touch screen adjustment offsets */
    const Touch_Type    NV_Touch EEMEM = {0, 0, 0, 0, 0};
  #endif


  /*
   *  constant strings (stored in EEPROM)
   */

  /* language specific */
  #include "var_czech.h"
  #include "var_english.h"
  #include "var_german.h"
  #include "var_italian.h"
  #include "var_spanish.h"
  #include "var_russian.h"

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
  const unsigned char h_FE_str[] EEMEM ="hFE";
  const unsigned char V_BE_str[] EEMEM ="Vbe";
  const unsigned char V_GT_str[] EEMEM ="V_GT";
  const unsigned char I_CEO_str[] EEMEM = "Iceo";
  const unsigned char Vf_str[] EEMEM = "Vf";
  const unsigned char DiodeCap_str[] EEMEM = "C";
  const unsigned char Vth_str[] EEMEM = "Vth";
  const unsigned char I_R_str[] EEMEM = "I_R";
  const unsigned char V_T_str[] EEMEM = "VT";
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
  const unsigned char Profile1_str[] EEMEM = "#1";
  const unsigned char Profile2_str[] EEMEM = "#2";
  const unsigned char I_DSS_str[] EEMEM = "Idss";
  const unsigned char I_leak_str[] EEMEM = "I_l";
  const unsigned char R_DS_str[] EEMEM = "Rds";

  #if defined (SW_ESR) || defined (SW_OLD_ESR)
    const unsigned char ESR_str[] EEMEM = "ESR";
  #endif
  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] EEMEM = "PWM";
  #endif
  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS) || defined (HW_FREQ_COUNTER_EXT)
    const unsigned char Hertz_str[] EEMEM = "Hz";
  #endif
  #if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_JVC_str[] EEMEM = "JVC";
    const unsigned char IR_Kaseikyo_str[] EEMEM = "Kas";
    const unsigned char IR_Matsushita_str[] EEMEM = "Mats";
    const unsigned char IR_Motorola_str[] EEMEM = "Mot";
    const unsigned char IR_NEC_str[] EEMEM = "NEC";
    const unsigned char IR_Proton_str[] EEMEM = "Prot";
    const unsigned char IR_RC5_str[] EEMEM = "RC-5";
    const unsigned char IR_RC6_str[] EEMEM = "RC-6";
    const unsigned char IR_Samsung_str[] EEMEM = "Sams";
    const unsigned char IR_Sharp_str[] EEMEM = "Sharp";
    const unsigned char IR_SIRC_str[] EEMEM = "SIRC";
  #endif
  #ifdef SW_OPTO_COUPLER
    const unsigned char CTR_str[] EEMEM = "CTR";
    const unsigned char If_str[] EEMEM = "If";
    const unsigned char t_on_str[] EEMEM = "t_on";
    const unsigned char t_off_str[] EEMEM = "t_off";
  #endif
  #ifdef SW_UJT
    const unsigned char R_BB_str[] EEMEM = "R_BB";
  #endif

  /* component symbols */
  const unsigned char Cap_str[] EEMEM = {'-', LCD_CHAR_CAP, '-',0};
  const unsigned char Diode_AC_str[] EEMEM = {'-', LCD_CHAR_DIODE_AC, '-', 0};
  const unsigned char Diode_CA_str[] EEMEM = {'-', LCD_CHAR_DIODE_CA, '-', 0};
  const unsigned char Resistor_str[] EEMEM = {'-', LCD_CHAR_RESISTOR_L, LCD_CHAR_RESISTOR_R, '-', 0};

  /* version */
  const unsigned char Version_str[] EEMEM = "v1.30m";


  /*
   *  contant tables (stored in EEPROM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M (used by value display) */
  const unsigned char Prefix_table[] EEMEM = {'p', 'n', LCD_CHAR_MICRO, 'm', 0, 'k', 'M'};

  /* voltage based factors for large caps (using Rl) */
  /* voltage in mV:                          300    325    350    375    400    425    450    475    500    525    550    575    600    625    650   675   700   725   750   775   800   825   850   875   900   925   950   975  1000  1025  1050  1075  1100  1125  1150  1175  1200  1225  1250  1275  1300  1325  1350  1375  1400 */
  const uint16_t LargeCap_table[] EEMEM = {23022, 21195, 19629, 18272, 17084, 16036, 15104, 14271, 13520, 12841, 12224, 11660, 11143, 10668, 10229, 9822, 9445, 9093, 8765, 8458, 8170, 7900, 7645, 7405, 7178, 6963, 6760, 6567, 6384, 6209, 6043, 5885, 5733, 5589, 5450, 5318, 5191, 5069, 4952, 4839, 4731, 4627, 4526, 4430, 4336};

  /* voltage based factors for small caps (using Rh) */
  /* voltages in mV:                      1000 1050 1100 1150 1200 1250 1300 1350 1400 */
  const uint16_t SmallCap_table[] EEMEM = {954, 903, 856, 814, 775, 740, 707, 676, 648};
//const uint16_t SmallCap_table[] EEMEM = {9535, 9026, 8563, 8141, 7753, 7396, 7066, 6761, 6477}; 

  #ifdef SW_PWM_SIMPLE
    /* PWM menu: frequencies */    
    const uint16_t PWM_Freq_table[] EEMEM = {100, 250, 500, 1000, 2500, 5000, 10000, 25000};
  #endif

  #ifdef SW_INDUCTOR
    /* ratio based factors for inductors */
    /* ratio:                                 200   225   250   275   300   325   350   375   400   425   450   475   500   525   550   575   600   625  650  675  700  725  750  775  800  825  850  875  900  925  950  975 */
    const uint16_t Inductor_table[] EEMEM = {4481, 3923, 3476, 3110, 2804, 2544, 2321, 2128, 1958, 1807, 1673, 1552, 1443, 1343, 1252, 1169, 1091, 1020, 953, 890, 831, 775, 721, 670, 621, 574, 527, 481, 434, 386, 334, 271};
  #endif

  #if defined (HW_FREQ_COUNTER) || defined (SW_SQUAREWAVE)
    /* Timer1 prescalers and corresponding bitmasks */
    const uint16_t T1_Prescaler_table[] EEMEM = {1, 8, 64, 256, 1024};
    const uint8_t T1_Bitmask_table[] EEMEM = {(1 << CS10), (1 << CS11), (1 << CS11) | (1 << CS10), (1 << CS12), (1 << CS12) | (1 << CS10)};
  #endif


  /*
   *  bitmask tables for probe settings (stored in EEPROM)
   *  - they save some bytes in the firmware.
   */

  /* bitmasks for Rl probe resistors based on probe ID */
  const unsigned char Rl_table[] EEMEM = {(1 << R_RL_1), (1 << R_RL_2), (1 << R_RL_3)};

  /* bitmasks for Rh probe resistors based on probe ID */
  const unsigned char Rh_table[] EEMEM = {(1 << R_RH_1), (1 << R_RH_2), (1 << R_RH_3)};

  /* bitmasks for pins (ADC port) based on probe ID */
  const unsigned char Pin_table[] EEMEM = {(1 << TP1), (1 << TP2), (1 << TP3)};

  /* bitmasks for ADC MUX input addresses based on probe ID */
  const unsigned char ADC_table[] EEMEM = {TP1, TP2, TP3};



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
  extern char            OutBuffer[12];      /* output buffer */

  /* configuration */
  extern UI_Type         UI;                 /* user interface */
  extern Config_Type     Cfg;                /* offsets and values */
  extern Adjust_Type     NV;                 /* basic adjustment offsets and values */

  #ifdef HW_TOUCH
    extern Touch_Type    Touch;              /* touch screen adjustment offsets */
  #endif

  /* probing */
  extern Probe_Type      Probes;             /* test probes */
  extern Check_Type      Check;              /* checking/testing */

  /* components */
  extern Resistor_Type   Resistors[3];       /* resistors */
  extern Capacitor_Type  Caps[3];            /* capacitors */
  extern Diode_Type      Diodes[6];          /* diodes */
  extern Semi_Type       Semi;               /* common semiconductor */
  extern AltSemi_Type    AltSemi;            /* special semiconductor */

  #ifdef SW_INDUCTOR
    extern Inductor_Type Inductor;           /* inductor */
  #endif

  #ifdef SW_PROBE_COLORS
    extern uint16_t      ProbeColors[3];     /* probe color coding */
  #endif

  #ifdef HW_SPI
    extern SPI_Type      SPI;                /* SPI */
  #endif

  #ifdef HW_I2C
    extern I2C_Type      I2C;                /* I2C */
  #endif


  /*
   *  NVRAM values (stored in EEPROM) with their defaults
   */

  /* basic adjustment values: profile #1 */
  extern const Adjust_Type    NV_Adjust_1;

  /* basic adjustment values: profile #2 */
  extern const Adjust_Type    NV_Adjust_2;

  #ifdef HW_TOUCH
    /* touch screen adjustment offsets */
    extern const Touch_Type    NV_Touch;
  #endif


  /*
   *  constant strings (stored in EEPROM)
   */

  extern const unsigned char Running_str[];
  extern const unsigned char Done_str[];
  extern const unsigned char Select_str[];
  extern const unsigned char Selftest_str[];
  extern const unsigned char Adjustment_str[];
  extern const unsigned char Save_str[];
  extern const unsigned char Load_str[];
  extern const unsigned char Show_str[];
  extern const unsigned char Remove_str[];
  extern const unsigned char Create_str[];
  extern const unsigned char ShortCircuit_str[];
  extern const unsigned char DischargeFailed_str[];
  extern const unsigned char Error_str[];
  extern const unsigned char Exit_str[];
  extern const unsigned char Vf_str[];
  extern const unsigned char BJT_str[];
  extern const unsigned char Triac_str[];
  extern const unsigned char PUT_str[];

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
  extern const unsigned char Profile1_str[];
  extern const unsigned char Profile2_str[];

  /* options */
  #if defined (SW_ESR) || defined (SW_OLD_ESR)
    extern const unsigned char ESR_str[];
  #endif
  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    extern const unsigned char PWM_str[];    
  #endif
  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS) || defined (HW_FREQ_COUNTER_EXT)
    extern const unsigned char Hertz_str[];
  #endif
  #ifdef SW_SQUAREWAVE
    extern const unsigned char SquareWave_str[];
  #endif
  #ifdef HW_ZENER
    extern const unsigned char Zener_str[];
    extern const unsigned char Min_str[];
  #endif
  #ifdef HW_FREQ_COUNTER
    extern const unsigned char FreqCounter_str[];
  #endif
  #ifdef HW_FREQ_COUNTER_EXT
    extern const unsigned char CounterChannel_str[];
    extern const unsigned char FreqInput_str[];
    extern const unsigned char LF_Crystal_str[];
    extern const unsigned char HF_Crystal_str[];
  #endif
  #ifdef SW_ENCODER
    extern const unsigned char Encoder_str[];
    extern const unsigned char TurnRight_str[];
  #endif
  #ifdef SW_CONTRAST
    extern const unsigned char Contrast_str[];
  #endif
  #if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    extern const unsigned char IR_Detector_str[];
    extern const unsigned char IR_JVC_str[];
    extern const unsigned char IR_Kaseikyo_str[];
    extern const unsigned char IR_Matsushita_str[];
    extern const unsigned char IR_Motorola_str[];
    extern const unsigned char IR_NEC_str[];
    extern const unsigned char IR_Proton_str[];
    extern const unsigned char IR_RC5_str[];
    extern const unsigned char IR_RC6_str[];
    extern const unsigned char IR_Samsung_str[];
    extern const unsigned char IR_Sharp_str[];
    extern const unsigned char IR_SIRC_str[];
  #endif
  #ifdef SW_OPTO_COUPLER
    extern const unsigned char OptoCoupler_str[];
    extern const unsigned char Start_str[];
    extern const unsigned char None_str[];
    extern const unsigned char CTR_str[];
    extern const unsigned char If_str[];
    extern const unsigned char t_on_str[];
    extern const unsigned char t_off_str[];
  #endif
  #ifdef SW_UJT
    extern const unsigned char UJT_str[];
  #endif
  #ifdef SW_SERVO
    extern const unsigned char Servo_str[];
    extern const unsigned char Sweep_str[];
  #endif
  #ifdef HW_TOUCH
    extern const unsigned char TouchSetup_str[];
  #endif


  /*
   *  constant tables (stored in EEPROM)
   */

  /* unit prefixes: p, n, µ, m, 0, k, M (used by value display) */
  extern const unsigned char Prefix_table[];

  /* voltage based factors for large caps (using Rl) */
  extern const uint16_t LargeCap_table[];

  /* voltage based factors for small caps (using Rh) */
  extern const uint16_t SmallCap_table[];

  #ifdef SW_PWM_SIMPLE
    /* PWM menu: frequencies */
    extern const uint16_t PWM_Freq_table[];
  #endif

  #ifdef SW_INDUCTOR
    /* voltage based factors for inductors */
    extern const uint16_t Inductor_table[];
  #endif

  #if defined (HW_FREQ_COUNTER) || defined (SW_SQUAREWAVE)
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

  /* bitmasks for Rh probe resistors based on probe ID */
  extern const unsigned char Rh_table[];

  /* bitmasks for pins (ADC port) based on probe ID */
  extern const unsigned char Pin_table[];

  /* bitmasks for ADC MUX input addresses based on probe ID */
  extern const unsigned char ADC_table[];

#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

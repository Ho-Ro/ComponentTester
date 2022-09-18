/* ************************************************************************
 *
 *   global variables
 *
 *   (c) 2012-2021 by Markus Reschke
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
  char              OutBuffer[OUT_BUFFER_SIZE];   /* output buffer */
  #ifdef SERIAL_RW
    char            RX_Buffer[RX_BUFFER_SIZE];    /* serial RX buffer */
    uint8_t         RX_Pos = 0;                   /* position in buffer */
    #ifdef SERIAL_BITBANG
      uint8_t       RX_Char;                 /* RX char (bit buffer) */
      uint8_t       RX_Bits;                 /* bit counter for RX char */
    #endif
  #endif

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

  #if defined (SW_INDUCTOR) || defined (HW_LC_METER)
    Inductor_Type   Inductor;                /* inductor */
  #endif

  #ifdef UI_SERIAL_COMMANDS
    Info_Type       Info;                    /* additional component data */
  #endif

  #ifdef HW_SPI
    SPI_Type        SPI;                     /* SPI */
  #endif

  #ifdef HW_I2C
    I2C_Type        I2C;                     /* I2C */
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (SW_IR_TRANSMITTER)
    /* demodulated/raw IR code */
    uint8_t         IR_Code[IR_CODE_BYTES];  /* raw data */
  #endif


  /*
   *  NVRAM values with their defaults
   *  - stored in EEPROM
   */

  /* manage RZero */
  #ifdef R_MULTIOFFSET
    #define NV_R_ZERO         {R_ZERO, R_ZERO, R_ZERO}
  #else
    #define NV_R_ZERO         R_ZERO
  #endif

  /* manage CapZero */
  #ifdef CAP_MULTIOFFSET
    #define NV_C_ZERO         {C_ZERO, C_ZERO, C_ZERO}
  #else
    #define NV_C_ZERO         C_ZERO
  #endif

  /* basic adjustment values: profile #1 */
  const Adjust_Type     NV_Adjust_1 EEMEM = {R_MCU_LOW, R_MCU_HIGH, NV_R_ZERO, NV_C_ZERO, UREF_OFFSET, COMPARATOR_OFFSET, LCD_CONTRAST, 0};

  /* basic adjustment values: profile #2 */
  const Adjust_Type     NV_Adjust_2 EEMEM = {R_MCU_LOW, R_MCU_HIGH, NV_R_ZERO, NV_C_ZERO, UREF_OFFSET, COMPARATOR_OFFSET, LCD_CONTRAST, 0};

  #ifdef HW_TOUCH
    /* touch screen adjustment offsets */
    const Touch_Type    NV_Touch EEMEM = {0, 0, 0, 0, 0};
  #endif


  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */

  /* language specific text */
  #include "var_czech.h"
  #include "var_czech_2.h"
  #include "var_danish.h"
  #include "var_english.h"
  #include "var_german.h"
  #include "var_italian.h"
  #include "var_polish.h"
  #include "var_polish_2.h"
  #include "var_romanian.h"
  #include "var_russian.h"
  #include "var_russian_2.h"
  #include "var_spanish.h"


  /* firmware */
  const unsigned char Version_str[] MEM_TYPE = "v1.44m";


  /* common terms and texts */
  const unsigned char MOS_str[] MEM_TYPE = "MOS";
  const unsigned char FET_str[] MEM_TYPE = "FET";
  const unsigned char Channel_str[] MEM_TYPE = "-ch";
  const unsigned char Enhancement_str[] MEM_TYPE = "enh.";
  const unsigned char Depletion_str[] MEM_TYPE = "dep.";
  const unsigned char IGBT_str[] MEM_TYPE = "IGBT";
  const unsigned char Cgs_str[] MEM_TYPE = "Cgs";
  const unsigned char NPN_str[] MEM_TYPE = "NPN";
  const unsigned char PNP_str[] MEM_TYPE = "PNP";
  const unsigned char h_FE_str[] MEM_TYPE ="hFE";
  const unsigned char V_BE_str[] MEM_TYPE ="Vbe";
  const unsigned char V_GT_str[] MEM_TYPE ="V_GT";
  const unsigned char I_CEO_str[] MEM_TYPE = "Iceo";
  const unsigned char Vf_str[] MEM_TYPE = "Vf";
  const unsigned char DiodeCap_str[] MEM_TYPE = "C";
  const unsigned char Vth_str[] MEM_TYPE = "Vth";
  const unsigned char I_R_str[] MEM_TYPE = "I_R";
  const unsigned char V_T_str[] MEM_TYPE = "VT";
  const unsigned char URef_str[] MEM_TYPE = "Vref";
  const unsigned char RhLow_str[] MEM_TYPE = "Rh-";
  const unsigned char RhHigh_str[] MEM_TYPE = "Rh+";
  const unsigned char RiLow_str[] MEM_TYPE = "Ri-";
  const unsigned char RiHigh_str[] MEM_TYPE = "Ri+";
  const unsigned char Rl_str[] MEM_TYPE = "+Rl-";
  const unsigned char Rh_str[] MEM_TYPE = "+Rh-";
  const unsigned char ProbeComb_str[] MEM_TYPE = "12 13 23";
  const unsigned char CapOffset_str[] MEM_TYPE = "C0";
  const unsigned char ROffset_str[] MEM_TYPE = "R0";
  const unsigned char Vcc_str[] MEM_TYPE = "Vcc";
  const unsigned char CompOffset_str[] MEM_TYPE = "AComp";
  const unsigned char Profile1_str[] MEM_TYPE = "#1";
  const unsigned char Profile2_str[] MEM_TYPE = "#2";
  const unsigned char I_DSS_str[] MEM_TYPE = "Idss";
  const unsigned char I_leak_str[] MEM_TYPE = "I_l";
  const unsigned char R_DS_str[] MEM_TYPE = "Rds";
  const unsigned char V_GSoff_str[] MEM_TYPE = "V_GS(off)";


  /* options */
  #ifdef SW_ESR_TOOL
    const unsigned char ESR_str[] MEM_TYPE = "ESR";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_NEC_str[] MEM_TYPE = "NEC";
    const unsigned char IR_SIRC_str[] MEM_TYPE = "SIRC";
    #ifdef SW_IR_RX_EXTRA
      const unsigned char IR_IR60_str[] MEM_TYPE = "IR60";
      const unsigned char IR_RCA_str[] MEM_TYPE = "RCA";
      const unsigned char IR_RECS80_str[] MEM_TYPE = "RECS80";
      const unsigned char IR_Sanyo_str[] MEM_TYPE = "Sanyo";
      const unsigned char IR_uPD1986C_str[] MEM_TYPE = "µPD1986C";
    #endif
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (SW_IR_TRANSMITTER)
    const unsigned char IR_JVC_str[] MEM_TYPE = "JVC";
    const unsigned char IR_Kaseikyo_str[] MEM_TYPE = "Kas";
    const unsigned char IR_Matsushita_str[] MEM_TYPE = "Mat";
    const unsigned char IR_Motorola_str[] MEM_TYPE = "Mot";
    const unsigned char IR_Proton_str[] MEM_TYPE = "Prot";
    const unsigned char IR_RC5_str[] MEM_TYPE = "RC-5";
    const unsigned char IR_RC6_str[] MEM_TYPE = "RC-6";
    const unsigned char IR_Samsung_str[] MEM_TYPE = "Sams";
    const unsigned char IR_Sharp_str[] MEM_TYPE = "Sharp";
    #if defined (SW_IR_RX_EXTRA) || defined (SW_IR_TX_EXTRA)
      const unsigned char IR_Thomson_str[] MEM_TYPE = "Thom";
    #endif
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_NEC_Std_str[] MEM_TYPE = "NEC Std";
    const unsigned char IR_NEC_Ext_str[] MEM_TYPE = "NEC Ext";
    const unsigned char IR_SIRC_12_str[] MEM_TYPE = "SIRC-12";
    const unsigned char IR_SIRC_15_str[] MEM_TYPE = "SIRC-15";
    const unsigned char IR_SIRC_20_str[] MEM_TYPE = "SIRC-20";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char If_str[] MEM_TYPE = "If";
    const unsigned char t_on_str[] MEM_TYPE = "t_on";
    const unsigned char t_off_str[] MEM_TYPE = "t_off";
  #endif

  #ifdef SW_UJT
    const unsigned char R_BB_str[] MEM_TYPE = "R_BB";
  #endif

  #ifdef SW_DS18B20
    const unsigned char DS18B20_str[] MEM_TYPE = "DS18B20";
  #endif

  #ifdef SW_HFE_CURRENT
    const unsigned char I_str[] MEM_TYPE ="I_";
  #endif

  #ifdef SW_REVERSE_HFE
    const unsigned char h_FE_r_str[] MEM_TYPE ="hFEr";
  #endif

  #ifdef SW_DHTXX
    const unsigned char DHTxx_str[] MEM_TYPE ="DHTxx";
    const unsigned char RH_str[] MEM_TYPE ="RH";
    const unsigned char DHT11_str[] MEM_TYPE ="DHT11";
    const unsigned char DHT22_str[] MEM_TYPE ="DHT22";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char CRC_str[] MEM_TYPE = "CRC";
  #endif

  #ifdef SW_FONT_TEST
    const unsigned char FontTest_str[] MEM_TYPE = "Font";
  #endif

  /* component symbols */
  const unsigned char Cap_str[] MEM_TYPE = {'-', LCD_CHAR_CAP, '-',0};
  const unsigned char Diode_AC_str[] MEM_TYPE = {'-', LCD_CHAR_DIODE_AC, '-', 0};
  const unsigned char Diode_CA_str[] MEM_TYPE = {'-', LCD_CHAR_DIODE_CA, '-', 0};
  const unsigned char Resistor_str[] MEM_TYPE = {'-', LCD_CHAR_RESISTOR_L, LCD_CHAR_RESISTOR_R, '-', 0};


  /* remote commands */
  #ifdef UI_SERIAL_COMMANDS
    /* maximum string length: RX_BUFFER_SIZE - 1 */

    /* feedback */ 
    const unsigned char Cmd_ERR_str[] MEM_TYPE = "ERR";
    const unsigned char Cmd_OK_str[] MEM_TYPE = "OK";
    const unsigned char Cmd_NA_str[] MEM_TYPE = "N/A";
    const unsigned char Cmd_R_be_str[] MEM_TYPE = "R_BE";
    const unsigned char Cmd_D_fb_str[] MEM_TYPE = "D_FB";
    const unsigned char Cmd_BJT_str[] MEM_TYPE = "BJT";
    #ifdef SW_SCHOTTKY_BJT
      const unsigned char Cmd_D_Clamp_str[] MEM_TYPE = "D_CLAMP";
    #endif
    const unsigned char Cmd_SYM_str[] MEM_TYPE = "SYM";

    /* commands */
    const unsigned char Cmd_VER_str[] MEM_TYPE = "VER";
    const unsigned char Cmd_OFF_str[] MEM_TYPE = "OFF";
    const unsigned char Cmd_PROBE_str[] MEM_TYPE = "PROBE";
    const unsigned char Cmd_COMP_str[] MEM_TYPE = "COMP";
    const unsigned char Cmd_MSG_str[] MEM_TYPE = "MSG";
    const unsigned char Cmd_QTY_str[] MEM_TYPE = "QTY";
    const unsigned char Cmd_NEXT_str[] MEM_TYPE = "NEXT";
    const unsigned char Cmd_TYPE_str[] MEM_TYPE = "TYPE";
    const unsigned char Cmd_HINT_str[] MEM_TYPE = "HINT";
    const unsigned char Cmd_MHINT_str[] MEM_TYPE = "MHINT";
    const unsigned char Cmd_PIN_str[] MEM_TYPE = "PIN";
    const unsigned char Cmd_R_str[] MEM_TYPE = "R";
    const unsigned char Cmd_C_str[] MEM_TYPE = "C";
    #ifdef SW_INDUCTOR
      const unsigned char Cmd_L_str[] MEM_TYPE = "L";
    #endif
    #if defined (SW_ESR) || defined (SW_OLD_ESR)
      const unsigned char Cmd_ESR_str[] MEM_TYPE = "ESR";
    #endif
    const unsigned char Cmd_V_F_str[] MEM_TYPE = "V_F";
    const unsigned char Cmd_V_F2_str[] MEM_TYPE = "V_F2";
    const unsigned char Cmd_C_D_str[] MEM_TYPE = "C_D";
    const unsigned char Cmd_R_BE_str[] MEM_TYPE = "R_BE";
    const unsigned char Cmd_h_FE_str[] MEM_TYPE = "h_FE";
    #ifdef SW_REVERSE_HFE
      const unsigned char Cmd_h_FE_r_str[] MEM_TYPE = "h_FE_r";
    #endif
    const unsigned char Cmd_V_BE_str[] MEM_TYPE = "V_BE";
    const unsigned char Cmd_I_CEO_str[] MEM_TYPE = "I_CEO";
    const unsigned char Cmd_V_TH_str[] MEM_TYPE = "V_th";
    const unsigned char Cmd_C_GS_str[] MEM_TYPE = "C_GS";
    const unsigned char Cmd_R_DS_str[] MEM_TYPE = "R_DS";
    const unsigned char Cmd_V_GS_off_str[] MEM_TYPE = "V_GS_off";
    const unsigned char Cmd_I_DSS_str[] MEM_TYPE = "I_DSS";
    const unsigned char Cmd_C_GE_str[] MEM_TYPE = "C_GE";
    const unsigned char Cmd_V_T_str[] MEM_TYPE = "V_T";
    #ifdef SW_HFE_CURRENT
      const unsigned char Cmd_I_C_str[] MEM_TYPE = "I_C";
      const unsigned char Cmd_I_E_str[] MEM_TYPE = "I_E";
    #endif
    #ifdef HW_PROBE_ZENER
      const unsigned char Cmd_V_Z_str[] MEM_TYPE = "V_Z";
    #endif

    /* command reference table */
    const Cmd_Type Cmd_Table[] MEM_TYPE = {
      {CMD_VER, Cmd_VER_str},
      {CMD_OFF, Cmd_OFF_str},
      {CMD_PROBE, Cmd_PROBE_str},
      {CMD_COMP, Cmd_COMP_str},
      {CMD_MSG, Cmd_MSG_str},
      {CMD_QTY, Cmd_QTY_str},
      {CMD_NEXT, Cmd_NEXT_str},
      {CMD_TYPE, Cmd_TYPE_str},
      {CMD_HINT, Cmd_HINT_str},
      {CMD_MHINT, Cmd_MHINT_str},
      {CMD_PIN, Cmd_PIN_str},
      {CMD_R, Cmd_R_str},
      {CMD_C, Cmd_C_str},
      #ifdef SW_INDUCTOR
        {CMD_L, Cmd_L_str},
      #endif
      #if defined (SW_ESR) || defined (SW_OLD_ESR)
        {CMD_ESR, Cmd_ESR_str},
      #endif
      {CMD_I_L, I_leak_str},
      {CMD_V_F, Cmd_V_F_str},
      {CMD_V_F2, Cmd_V_F2_str},
      {CMD_C_D, Cmd_C_D_str},
      {CMD_I_R, I_R_str},
      {CMD_R_BE, Cmd_R_BE_str},
      {CMD_H_FE, Cmd_h_FE_str},
      #ifdef SW_REVERSE_HFE
        {CMD_H_FE_R, Cmd_h_FE_r_str},
      #endif
      {CMD_V_BE, Cmd_V_BE_str},
      {CMD_I_CEO, Cmd_I_CEO_str},
      {CMD_V_TH, Cmd_V_TH_str},
      {CMD_C_GS, Cmd_C_GS_str},
      {CMD_R_DS, Cmd_R_DS_str},
      {CMD_V_GS_OFF, Cmd_V_GS_off_str},
      {CMD_I_DSS, Cmd_I_DSS_str},
      {CMD_C_GE, Cmd_C_GE_str},
      {CMD_V_GT, V_GT_str},
      {CMD_V_T, Cmd_V_T_str},
      #ifdef SW_UJT
        {CMD_R_BB, R_BB_str},
      #endif
      #ifdef SW_HFE_CURRENT
        {CMD_I_C, Cmd_I_C_str},
        {CMD_I_E, Cmd_I_E_str},
      #endif
      #ifdef HW_PROBE_ZENER
        {CMD_V_Z, Cmd_V_Z_str},
      #endif
      {0, 0}
    };
  #endif


  /*
   *  constant tables
   *  - stored in EEPROM/Flash
   */

  /* unit prefixes: f, p, n, µ, m, 0, k, M (used by value display) */
  const unsigned char Prefix_table[NUM_PREFIXES] MEM_TYPE = {'f', 'p', 'n', LCD_CHAR_MICRO, 'm', 0, 'k', 'M'};

  /* voltage based factors for large caps (using Rl) */
  /* voltage in mV:                                          300    325    350    375    400    425    450    475    500    525    550    575    600    625    650   675   700   725   750   775   800   825   850   875   900   925   950   975  1000  1025  1050  1075  1100  1125  1150  1175  1200  1225  1250  1275  1300  1325  1350  1375  1400 */
  const uint16_t LargeCap_table[NUM_LARGE_CAP] MEM_TYPE = {23022, 21195, 19629, 18272, 17084, 16036, 15104, 14271, 13520, 12841, 12224, 11660, 11143, 10668, 10229, 9822, 9445, 9093, 8765, 8458, 8170, 7900, 7645, 7405, 7178, 6963, 6760, 6567, 6384, 6209, 6043, 5885, 5733, 5589, 5450, 5318, 5191, 5069, 4952, 4839, 4731, 4627, 4526, 4430, 4336};

  /* voltage based factors for small caps (using Rh) */
  /* voltages in mV:                                       1000  1050  1100  1150  1200  1250  1300  1350  1400 */
  const uint16_t SmallCap_table[NUM_SMALL_CAP] MEM_TYPE = { 954,  903,  856,  814,  775,  740,  707,  676,  648};
//const uint16_t SmallCap_table[NUM_SMALL_CAP] MEM_TYPE = {9535, 9026, 8563, 8141, 7753, 7396, 7066, 6761, 6477}; 

  #ifdef SW_PWM_SIMPLE
    /* PWM menu: frequencies */    
    const uint16_t PWM_Freq_table[NUM_PWM_FREQ] MEM_TYPE = {100, 250, 500, 1000, 2500, 5000, 10000, 25000};
  #endif

  #ifdef SW_INDUCTOR
    /* ratio based factors for inductors */
    /* ratio:                                                200   225   250   275   300   325   350   375   400   425   450   475   500   525   550   575   600   625  650  675  700  725  750  775  800  825  850  875  900  925  950  975 */
    const uint16_t Inductor_table[NUM_INDUCTOR] MEM_TYPE = {4481, 3923, 3476, 3110, 2804, 2544, 2321, 2128, 1958, 1807, 1673, 1552, 1443, 1343, 1252, 1169, 1091, 1020, 953, 890, 831, 775, 721, 670, 621, 574, 527, 481, 434, 386, 334, 271};
  #endif

  #if defined (HW_FREQ_COUNTER) || defined (SW_SQUAREWAVE)
    /* Timer1 prescalers and corresponding register bits */
    const uint16_t T1_Prescaler_table[NUM_TIMER1] MEM_TYPE = {1, 8, 64, 256, 1024};
    const uint8_t T1_RegBits_table[NUM_TIMER1] MEM_TYPE = {(1 << CS10), (1 << CS11), (1 << CS11) | (1 << CS10), (1 << CS12), (1 << CS12) | (1 << CS10)};
  #endif

  #ifdef UI_PROBE_COLORS
    /* probe color coding */
    uint16_t        ProbeColors[NUM_PROBE_COLORS] = {COLOR_PROBE_1, COLOR_PROBE_2, COLOR_PROBE_3};
  #endif

  #ifdef SW_E6
    /* E6 (in 0.01) */
    const uint16_t E6_table[NUM_E6] MEM_TYPE = {100, 150, 220, 330, 470, 680};  
  #endif

  #ifdef SW_E12
    /* E12 (in 0.01) */
    const uint16_t E12_table[NUM_E12] MEM_TYPE = {100, 120, 150, 180, 220, 270, 330, 390, 470, 560, 680, 820};
  #endif

  #ifdef SW_E24
    /* E24 (in 0.01) */
    const uint16_t E24_table[NUM_E24] MEM_TYPE = {100, 110, 120, 130, 150, 160, 180, 200, 220, 240, 270, 300, 330, 360, 390, 420, 470, 510, 560, 620, 680, 750, 820, 910};
  #endif

  #ifdef SW_E96
    /* E96 (in 0.01) */
    const uint16_t E96_table[NUM_E96] MEM_TYPE = {
      100, 102, 105, 107, 110, 113, 115, 118, 121, 124, 127, 130, 133, 137, 140, 143, 147, 150, 154, 158, 162, 165, 169, 174,
      178, 182, 187, 191, 196, 200, 205, 210, 215, 221, 226, 232, 237, 243, 249, 255, 261, 267, 274, 280, 287, 294, 301, 309,
      316, 324, 332, 340, 348, 357, 365, 374, 383, 392, 402, 412, 422, 432, 442, 453, 464, 475, 487, 499, 511, 523, 536, 549,
      562, 576, 590, 604, 619, 634, 649, 665, 681, 698, 715, 732, 750, 768, 787, 806, 825, 845, 866, 887, 909, 931, 953, 976}; 
  #endif

  #ifdef FUNC_COLORCODE
    /* band colors based on value                               0                 1                 2               3                  4                  5                 6                7                  8                9 */
    const uint16_t ColorCode_table[NUM_COLOR_CODES] MEM_TYPE = {COLOR_CODE_BLACK, COLOR_CODE_BROWN, COLOR_CODE_RED, COLOR_CODE_ORANGE, COLOR_CODE_YELLOW, COLOR_CODE_GREEN, COLOR_CODE_BLUE, COLOR_CODE_VIOLET, COLOR_CODE_GREY, COLOR_CODE_WHITE};
  #endif

  #ifdef FUNC_EIA96
    /* EIA-96 multiplier codes                                      0.001 0.01 0.1  1    10   100  1k   10k  100k */
    const unsigned char EIA96_Mult_table[NUM_EIA96_MULT] MEM_TYPE = {'Z', 'Y', 'X', 'A', 'B', 'C', 'D', 'E', 'F'};
  #endif


  /*
   *  tables of register bits for probe settings
   *  - stored in EEPROM/Flash
   *  - this saves some bytes in the firmware
   */

  /* register bits for Rl probe resistors based on probe ID */
  const uint8_t Rl_table[] MEM_TYPE = {(1 << R_RL_1), (1 << R_RL_2), (1 << R_RL_3)};

  /* register bits for Rh probe resistors based on probe ID */
  const uint8_t Rh_table[] MEM_TYPE = {(1 << R_RH_1), (1 << R_RH_2), (1 << R_RH_3)};

  /* register bits for ADC port pins based on probe ID */
  const uint8_t Pin_table[] MEM_TYPE = {(1 << TP1), (1 << TP2), (1 << TP3)};

  /* register bits for ADC MUX input channels based on probe ID (ADC0-7 only) */
  const uint8_t Channel_table[] MEM_TYPE = {TP1, TP2, TP3};



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
  extern char            OutBuffer[];        /* output buffer */
  #ifdef SERIAL_RW
    extern char          RX_Buffer[];        /* serial RX buffer */
    extern uint8_t       RX_Pos;             /* position in buffer */
    #ifdef SERIAL_BITBANG
      extern uint8_t     RX_Char;            /* RX char (bit buffer) */
      extern uint8_t     RX_Bits;            /* bit counter for RX char */
    #endif
  #endif

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
  extern Resistor_Type   Resistors[];        /* resistors */
  extern Capacitor_Type  Caps[];             /* capacitors */
  extern Diode_Type      Diodes[];           /* diodes */
  extern Semi_Type       Semi;               /* common semiconductor */
  extern AltSemi_Type    AltSemi;            /* special semiconductor */

  #if defined (SW_INDUCTOR) || defined (HW_LC_METER)
    extern Inductor_Type Inductor;           /* inductor */
  #endif

  #ifdef UI_SERIAL_COMMANDS
    extern Info_Type     Info;               /* additional component data */
  #endif

  #ifdef HW_SPI
    extern SPI_Type      SPI;                /* SPI */
  #endif

  #ifdef HW_I2C
    extern I2C_Type      I2C;                /* I2C */
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (SW_IR_TRANSMITTER)
    /* demodulated/raw IR code */
    extern uint8_t       IR_Code[];          /* raw data */
  #endif


  /*
   *  NVRAM values with their defaults
   *  - stored in EEPROM
   */

  /* basic adjustment values: profile #1 */
  extern const Adjust_Type    NV_Adjust_1;

  /* basic adjustment values: profile #2 */
  extern const Adjust_Type    NV_Adjust_2;

  #ifdef HW_TOUCH
    /* touch screen adjustment offsets */
    extern const Touch_Type   NV_Touch;
  #endif


  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  extern const unsigned char Version_str[];


  /* common terms and texts */
  extern const unsigned char Probing_str[];
  extern const unsigned char Failed1_str[];
  extern const unsigned char Failed2_str[];
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
  extern const unsigned char BJT_str[];
  extern const unsigned char Triac_str[];
  extern const unsigned char PUT_str[];
  extern const unsigned char MOS_str[];
  extern const unsigned char FET_str[];
  extern const unsigned char Channel_str[];
  extern const unsigned char Enhancement_str[];
  extern const unsigned char Depletion_str[];
  extern const unsigned char Cgs_str[];
  extern const unsigned char NPN_str[];
  extern const unsigned char PNP_str[];
  extern const unsigned char h_FE_str[];
  extern const unsigned char V_BE_str[];
  extern const unsigned char I_CEO_str[];
  extern const unsigned char Vf_str[];
  extern const unsigned char Vth_str[];
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


  /* units */
  extern const unsigned char Hertz_str[];


  /* options */
  #ifdef UI_KEY_HINTS
    extern const unsigned char Menu_or_Test_str[];
  #endif

  #ifdef SW_ESR_TOOL
    extern const unsigned char ESR_str[];
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    extern const unsigned char PWM_str[];    
  #endif

  #ifdef SW_SQUAREWAVE
    extern const unsigned char SquareWave_str[];
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    extern const unsigned char Zener_str[];
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
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

  #ifdef HW_EVENT_COUNTER
    extern const unsigned char EventCounter_str[];
    extern const unsigned char Count_str[];
    extern const unsigned char Time_str[];
    extern const unsigned char Events_str[];
    extern const unsigned char Stop_str[];
  #endif

  #ifdef HW_LC_METER
    extern const unsigned char LC_Meter_str[];
    extern const unsigned char Adjusting_str[];
  #endif

  #ifdef SW_ENCODER
    extern const unsigned char Encoder_str[];
    extern const unsigned char TurnRight_str[];
  #endif

  #ifdef SW_CONTRAST
    extern const unsigned char Contrast_str[];
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    extern const unsigned char IR_Detector_str[];
    extern const unsigned char IR_NEC_str[];
    extern const unsigned char IR_SIRC_str[];
    #ifdef SW_IR_RX_EXTRA
      extern const unsigned char IR_IR60_str[];
      extern const unsigned char IR_RCA_str[];
      extern const unsigned char IR_RECS80_str[];
      extern const unsigned char IR_Sanyo_str[];
      extern const unsigned char IR_uPD1986C_str[];
    #endif
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (SW_IR_TRANSMITTER)
    extern const unsigned char IR_JVC_str[];
    extern const unsigned char IR_Kaseikyo_str[];
    extern const unsigned char IR_Matsushita_str[];
    extern const unsigned char IR_Motorola_str[];
    extern const unsigned char IR_Proton_str[];
    extern const unsigned char IR_RC5_str[];
    extern const unsigned char IR_RC6_str[];
    extern const unsigned char IR_Samsung_str[];
    extern const unsigned char IR_Sharp_str[];
    #if defined (SW_IR_RX_EXTRA) || defined (SW_IR_TX_EXTRA)
      extern const unsigned char IR_Thomson_str[];
    #endif
  #endif

  #ifdef SW_IR_TRANSMITTER
    extern const unsigned char IR_Transmitter_str[];
    extern const unsigned char IR_Send_str[];
    extern const unsigned char IR_NEC_Std_str[];
    extern const unsigned char IR_NEC_Ext_str[];
    extern const unsigned char IR_SIRC_12_str[];
    extern const unsigned char IR_SIRC_15_str[];
    extern const unsigned char IR_SIRC_20_str[];
  #endif

  #ifdef SW_OPTO_COUPLER
    extern const unsigned char OptoCoupler_str[];
    extern const unsigned char None_str[];
    extern const unsigned char CTR_str[];
    extern const unsigned char If_str[];
    extern const unsigned char t_on_str[];
    extern const unsigned char t_off_str[];
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (SW_ONEWIRE_SCAN) || defined (HW_EVENT_COUNTER)
    extern const unsigned char Start_str[];
  #endif

  #ifdef SW_UJT
    extern const unsigned char UJT_str[];
    extern const unsigned char R_BB_str[];
  #endif

  #ifdef SW_SERVO
    extern const unsigned char Servo_str[];
    extern const unsigned char Sweep_str[];
  #endif

  #ifdef HW_TOUCH
    extern const unsigned char TouchSetup_str[];
  #endif

  #ifdef SW_DS18B20
    extern const unsigned char DS18B20_str[];
  #endif

  #ifdef SW_CAP_LEAKAGE
    extern const unsigned char CapLeak_str[];
    extern const unsigned char CapCharge_str[];
    extern const unsigned char CapHigh_str[];
    extern const unsigned char CapLow_str[];
    extern const unsigned char CapDischarge_str[];
  #endif

  #ifdef SW_MONITOR_R
    extern const unsigned char Monitor_R_str[];
  #endif

  #ifdef SW_MONITOR_C
    extern const unsigned char Monitor_C_str[];
  #endif

  #ifdef SW_MONITOR_L
    extern const unsigned char Monitor_L_str[];
  #endif

  #ifdef SW_MONITOR_RCL
    extern const unsigned char Monitor_RCL_str[];
  #endif

  #ifdef SW_MONITOR_RL
    extern const unsigned char Monitor_RL_str[];
  #endif

  #ifdef SW_POWER_OFF
    extern const unsigned char PowerOff_str[];
  #endif

  #ifdef SW_DHTXX
    extern const unsigned char DHTxx_str[];
    extern const unsigned char RH_str[];
    extern const unsigned char DHT11_str[];
    extern const unsigned char DHT22_str[];
  #endif

  #ifdef SW_ONEWIRE_SCAN
    extern const unsigned char OneWire_Scan_str[];
    extern const unsigned char Bus_str[];
    extern const unsigned char CRC_str[];
  #endif

  #ifdef SW_FONT_TEST
    extern const unsigned char FontTest_str[];
  #endif


  /* remote commands */
  #ifdef UI_SERIAL_COMMANDS
    /* feedback */
    extern const unsigned char Cmd_ERR_str[];
    extern const unsigned char Cmd_OK_str[];
    extern const unsigned char Cmd_NA_str[];
    extern const unsigned char Cmd_R_be_str[];
    extern const unsigned char Cmd_D_fb_str[];
    extern const unsigned char Cmd_BJT_str[];
    #ifdef SW_SCHOTTKY_BJT
      extern const unsigned char Cmd_D_Clamp_str[];
    #endif
    extern const unsigned char Cmd_SYM_str[];

    /* commands */
    extern const unsigned char Cmd_VER_str[];
    extern const unsigned char Cmd_OFF_str[];
    extern const unsigned char Cmd_PROBE_str[];
    extern const unsigned char Cmd_COMP_str[];
    extern const unsigned char Cmd_MSG_str[];
    extern const unsigned char Cmd_QTY_str[];
    extern const unsigned char Cmd_NEXT_str[];
    extern const unsigned char Cmd_TYPE_str[];
    extern const unsigned char Cmd_HINT_str[];
    extern const unsigned char Cmd_MHINT_str[];
    extern const unsigned char Cmd_PIN_str[];
    extern const unsigned char Cmd_R_str[];
    extern const unsigned char Cmd_C_str[];
    #ifdef SW_INDUCTOR
      extern const unsigned char Cmd_L_str[];
    #endif
    #if defined (SW_ESR) || defined (SW_OLD_ESR)
      extern const unsigned char Cmd_ESR_str[];
    #endif
    extern const unsigned char Cmd_V_F_str[];
    extern const unsigned char Cmd_V_F2_str[];
    extern const unsigned char Cmd_C_D_str[];
    extern const unsigned char Cmd_R_BE_str[];
    extern const unsigned char Cmd_h_FE_str[];
    #ifdef SW_REVERSE_HFE
      extern const unsigned char Cmd_h_FE_r_str[];
    #endif
    extern const unsigned char Cmd_V_BE_str[];
    extern const unsigned char Cmd_I_CEO_str[];
    extern const unsigned char Cmd_V_TH_str[];
    extern const unsigned char Cmd_C_GS_str[];
    extern const unsigned char Cmd_R_DS_str[];
    extern const unsigned char Cmd_V_GS_off_str[];
    extern const unsigned char Cmd_I_DSS_str[];
    extern const unsigned char Cmd_C_GE_str[];
    extern const unsigned char Cmd_V_T_str[];
    #ifdef SW_HFE_CURRENT
      extern const unsigned char Cmd_I_C_str[];
      extern const unsigned char Cmd_I_E_str[];
    #endif
    #ifdef HW_PROBE_ZENER
      extern const unsigned char Cmd_V_Z_str[];
    #endif

    /* command reference table */
    extern const Cmd_Type Cmd_Table[];
  #endif



  /*
   *  constant tables
   *  - stored in EEPROM/Flash
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
    /* Timer1 prescalers and corresponding register bits */
    extern const uint16_t T1_Prescaler_table[];
    extern const uint8_t T1_RegBits_table[];
  #endif

  #ifdef UI_PROBE_COLORS
    extern uint16_t      ProbeColors[];      /* probe color coding */
  #endif

  #ifdef SW_E6
  /* E6 (in 0.01) */
    extern const uint16_t E6_table[];
  #endif

  #ifdef SW_E12
  /* E12 (in 0.01) */
    extern const uint16_t E12_table[];
  #endif

  #ifdef SW_E24
    /* E24 (in 0.01) */
    extern const uint16_t E24_table[];
  #endif

  #ifdef SW_E96
    /* E96 (in 0.01) */
    extern uint16_t E96_table[];
  #endif

  #ifdef FUNC_COLORCODE
    /* band colors based on value */
    extern const uint16_t ColorCode_table[];
  #endif

  #ifdef FUNC_EIA96
    /* EIA-96 multiplier code */
    extern const unsigned char EIA96_Mult_table[];
  #endif


  /*
   *  tables of register bits for probe settings
   *  - they save some bytes in the firmware
   */

  /* register bits for Rl probe resistors based on probe ID */
  extern const uint8_t Rl_table[];

  /* register bits for Rh probe resistors based on probe ID */
  extern const uint8_t Rh_table[];

  /* register bits for ADC port pins based on probe ID */
  extern const uint8_t Pin_table[];

  /* register bits for ADC MUX input channels based on probe ID (ADC0-7 only) */
  extern const uint8_t Channel_table[];

#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

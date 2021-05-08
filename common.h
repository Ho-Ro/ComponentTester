/* ************************************************************************
 *
 *   common header file
 *
 *   (c) 2012-2017 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  include header files
 */

/* basic includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* AVR */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>


/* source management */
#define COMMON_H



/* ************************************************************************
 *   constants
 * ************************************************************************ */


/* probe IDs */
#define PROBE_1               0    /* probe #1 */
#define PROBE_2               1    /* probe #2 */
#define PROBE_3               2    /* probe #3 */


/* component IDs */
#define COMP_NONE             0
#define COMP_ERROR            1
#define COMP_MENU             2
#define COMP_RESISTOR        10
#define COMP_CAPACITOR       11
#define COMP_INDUCTOR        12
#define COMP_DIODE           20
#define COMP_BJT             30
#define COMP_FET             31
#define COMP_IGBT            32
#define COMP_TRIAC           33
#define COMP_THYRISTOR       34
#define COMP_PUT             35
#define COMP_UJT             36


/* error type IDs */
#define TYPE_DISCHARGE        1    /* discharge error */


/* FET types, also used for IGBTs (bit mask) */
#define TYPE_N_CHANNEL        0b00000001     /* n channel */
#define TYPE_P_CHANNEL        0b00000010     /* p channel */
#define TYPE_ENHANCEMENT      0b00000100     /* enhancement mode */
#define TYPE_DEPLETION        0b00001000     /* depletion mode */
#define TYPE_MOSFET           0b00010000     /* MOSFET */
#define TYPE_JFET             0b00100000     /* JFET */
#define TYPE_SYMMETRICAL      0b01000000     /* symmetrical drain/source */


/* BJT types (bit mask) */ 
#define TYPE_NPN              0b00000001     /* NPN */
#define TYPE_PNP              0b00000010     /* PNP */
#define TYPE_PARASITIC        0b00000100     /* parasitic BJT */


/* diode types (bit mask) */
#define TYPE_STANDARD         0b00000001     /* standard diode */


/* flags for semicondutor detection logic */
#define DONE_NONE             0
#define DONE_SEMI             1
#define DONE_ALTSEMI          2


/* multiplicator table IDs */
#define TABLE_SMALL_CAP       1
#define TABLE_LARGE_CAP       2
#define TABLE_INDUCTOR        3


/* bit flags for PullProbe() (bit mask) */
#define PULL_DOWN             0b00000000     /* pull down */
#define PULL_UP               0b00000001     /* pull up */
#define PULL_1MS              0b00001000     /* pull for 1ms */
#define PULL_10MS             0b00010000     /* pull for 10ms */


/* cursor mode (bit mask) */
#define CURSOR_NONE           0b00000000     /* no cursor */
#define CURSOR_STEADY         0b00000001     /* steady cursor */
#define CURSOR_BLINK          0b00000010     /* blinking cursor */
#define CURSOR_OP_MODE        0b00000100     /* consider operation mode */


/* keys (test push button etc.) */
#define KEY_TIMEOUT           0    /* timeout */
#define KEY_NONE              0    /* no key or error */
#define KEY_SHORT             1    /* test push button: short key press */
#define KEY_LONG              2    /* test push button: long key press */
#define KEY_RIGHT             3    /* rotary encoder: right turn */
                                   /* push buttons: increase */
#define KEY_LEFT              4    /* rotary encoder: left turn */
                                   /* push buttons: decrease */
#define KEY_INCDEC            5    /* push buttons: increase and decrease */


/* tester operation modes */
#define OP_NONE               0b00000000     /* undefined mode */
#define OP_CONTINOUS          0b00000001     /* continous mode */
#define OP_AUTOHOLD           0b00000010     /* auto hold mode */
/* operation signaling flags */
#define OP_BREAK_KEY          0b00000100     /* exit key processing */


/* UI line modes */
#define LINE_STD              0b00000000     /* standard mode */
#define LINE_KEY              0b00000001     /* wait for key press */
#define LINE_KEEP             0b00000010     /* keep first line */


/* storage modes */
#define STORAGE_LOAD          1    /* load adjustment values */
#define STORAGE_SAVE          2    /* save adjustment values */


/* custom chars/symbols */
#define LCD_CHAR_0            0    /* just a place holder */
#define LCD_CHAR_DIODE_AC     1    /* diode icon '>|' */
#define LCD_CHAR_DIODE_CA     2	   /* diode icon '|<' */
#define LCD_CHAR_CAP          3    /* capacitor icon '||' */
#define LCD_CHAR_OMEGA        4    /* omega */
#define LCD_CHAR_MICRO        5    /* µ / micro */
#define LCD_CHAR_RESISTOR_L   6    /* resistor left icon '[' */
#define LCD_CHAR_RESISTOR_R   7    /* resistor right icon ']' */


/* component symbols */
#define SYMBOL_BJT_NPN        0    /* BJT npn */
#define SYMBOL_BJT_PNP        1    /* BJT pnp */
#define SYMBOL_MOSFET_ENH_N   2    /* MOSFET enhancement mode, n-channel */
#define SYMBOL_MOSFET_ENH_P   3    /* MOSFET enhancement mode, p-channel */
#define SYMBOL_MOSFET_DEP_N   4    /* MOSFET depletion mode, n-channel */
#define SYMBOL_MOSFET_DEP_P   5    /* MOSFET depletion mode, p-channel */
#define SYMBOL_JFET_N         6    /* JFET n-channel */
#define SYMBOL_JFET_P         7    /* JFET p-channel */
#define SYMBOL_IGBT_ENH_N     8    /* IGBT enhancement mode, n-channel */
#define SYMBOL_IGBT_ENH_P     9    /* IGBT enhancement mode, p-channel */
#define SYMBOL_SCR           10    /* SCR / thyristor */
#define SYMBOL_TRIAC         11    /* TRIAC */
#define SYMBOL_PUT           12    /* PUT */
#define SYMBOL_UJT           13    /* UJT */


/* pinout positions (bit mask) */
#define PIN_NONE              0b00000000     /* no output */
#define PIN_LEFT              0b00000001     /* left of symbol */
#define PIN_RIGHT             0b00000010     /* right of symbol */
#define PIN_BOTTOM            0b00000100     /* bottom */
#define PIN_TOP               0b00001000     /* top */


/* interface bus status flags */
#define BUS_NONE              0b00000000     /* no bus */
#define BUS_SPI               0b00000001     /* SPI */
#define BUS_I2C               0b00000010     /* I2C */


/* SPI */
/* clock rate bitmask */
#define SPI_CLOCK_R0          0b00000001     /* divider bit 0 (SPR0) */
#define SPI_CLOCK_R1          0b00000010     /* divider bit 1 (SPR1) */
#define SPI_CLOCK_2X          0b00000100     /* double clock rate (SPI2X) */


/* I2C */
#define I2C_ERROR             0              /* bus error */
#define I2C_OK                1              /* operation done */
#define I2C_START             1              /* start condition */
#define I2C_REPEATED_START    2              /* repeated start condition */
#define I2C_DATA              1              /* data byte */
#define I2C_ADDRESS           2              /* address byte */
#define I2C_ACK               1              /* acknowledge */
#define I2C_NACK              2              /* not-acknowledge */


/* port pins of optional IO chip */
#define PCF8574_P0            0b00000000     /* pin #0 */
#define PCF8574_P1            0b00000001     /* pin #1 */
#define PCF8574_P2            0b00000010     /* pin #2 */
#define PCF8574_P3            0b00000011     /* pin #3 */
#define PCF8574_P4            0b00000100     /* pin #4 */
#define PCF8574_P5            0b00000101     /* pin #5 */
#define PCF8574_P6            0b00000110     /* pin #6 */
#define PCF8574_P7            0b00000111     /* pin #7 */



/* ************************************************************************
 *   structures
 * ************************************************************************ */


/* tester modes, offsets and values */
typedef struct
{
  uint8_t           SleepMode;     /* MCU sleep mode */
  uint8_t           Samples;       /* number of ADC samples */
  uint8_t           AutoScale;     /* flag to disable/enable ADC auto scaling */
  uint8_t           RefFlag;       /* internal control flag for ADC */
  uint16_t          Bandgap;       /* voltage of internal bandgap reference (mV) */
  uint16_t          Vcc;           /* voltage of Vcc (mV) */
  uint8_t           BusState;      /* status flags for interface busses */
} Config_Type;


/* basic adjustment offsets and values (stored in EEPROM) */
typedef struct
{
  uint16_t          RiL;           /* internal pin resistance of MCU in low mode (0.1 Ohms) */
  uint16_t          RiH;           /* internal pin resistance of MCU in high mode (0.1 Ohms) */
  uint16_t          RZero;         /* resistance of probe leads (2 in series) (0.01 Ohms) */
  uint8_t           CapZero;       /* capacity zero offset (input + leads) (pF) */
  int8_t            RefOffset;     /* voltage offset of bandgap reference (mV) */
  int8_t            CompOffset;    /* voltage offset of analog comparator (mV) */
  uint8_t           Contrast;      /* current contrast value */
  uint8_t           CheckSum;      /* checksum for stored values */
} Adjust_Type;


/* touch screen adjustment offsets (stored in EEPROM) */
typedef struct
{
  uint16_t          X_Left;        /* offset for left side */
  uint16_t          X_Right;       /* offset for right side */
  uint16_t          Y_Top;         /* offset for top */
  uint16_t          Y_Bottom;      /* offset for bottom */
  uint8_t           CheckSum;      /* checksum for stored values */
} Touch_Type;


/* user interface */
typedef struct
{
  /* UI mode */
  uint8_t           OP_Mode;       /* tester operation mode */

  /* display */
  uint8_t           LineMode;      /* line mode for LCD_NextLine() */
  uint8_t           CharPos_X;     /* current character x position */
  uint8_t           CharPos_Y;     /* current character y position */
                                   /* top left is 1/1 */
  uint8_t           CharMax_X;     /* max. characters per line */
  uint8_t           CharMax_Y;     /* max. number of lines */
  uint8_t           MaxContrast;   /* maximum contrast */
  #ifdef LCD_COLOR
  uint16_t          PenColor;      /* pen color */ 
  #endif

  /* keys (push buttons etc.) */
  #ifdef HW_KEYS
  uint8_t           KeyOld;        /* former key */
  uint8_t           KeyStep;       /* step size (1-7) */
  uint8_t           KeyStepOld;    /* former step size */
  #endif
  /* rotary encoder */
  #ifdef HW_ENCODER
  uint8_t           EncState;      /* last AB status */
  uint8_t           EncDir;        /* turning direction */
  uint8_t           EncPulses;     /* number of Gray code pulses */
  uint8_t           EncTicks;      /* time counter */
  #endif
  /* increase/decrease push buttons */
  #ifdef HW_INCDEC_KEYS
  #endif
  /* touch screen */
  #ifdef HW_TOUCH
  uint16_t          TouchRaw_X;    /* raw touch screen x position */
  uint16_t          TouchRaw_Y;    /* raw touch screen y position */
  uint8_t           TouchPos_X;    /* charater x position */
  uint8_t           TouchPos_Y;    /* charater y position */
  #endif

} UI_Type;


/* probes */
typedef struct
{
  /* probe IDs */
  uint8_t           ID_1;          /* probe-1 */
  uint8_t           ID_2;          /* probe-2 */
  uint8_t           ID_3;          /* probe-3 */

  /* backup probe IDs */
  uint8_t           ID2_1;         /* probe-1 */
  uint8_t           ID2_2;         /* probe-2 */
  uint8_t           ID2_3;         /* probe-3 */

  /* bit masks for switching probes and test resistors */
  uint8_t           Rl_1;          /* Rl mask for probe-1 */
  uint8_t           Rl_2;          /* Rl mask for probe-2 */
  uint8_t           Rl_3;          /* Rl mask for probe-3 */
  uint8_t           Rh_1;          /* Rh mask for probe-1 */
  uint8_t           Rh_2;          /* Rh mask for probe-2 */
  uint8_t           Rh_3;          /* Rh mask for probe-3 */
  uint8_t           Pin_1;         /* pin mask for probe-1 */
  uint8_t           Pin_2;         /* pin mask for probe-2 */
  uint8_t           Pin_3;         /* pin mask for probe-3 */
  uint8_t           ADC_1;         /* ADC MUX input address for probe-1 */
  uint8_t           ADC_2;         /* ADC MUX input address for probe-2 */
  uint8_t           ADC_3;         /* ADC MUX input address for probe-3 */
} Probe_Type;


/* checking/probing */
typedef struct
{
  uint8_t           Found;         /* component type */ 
  uint8_t           Type;          /* component specific subtype */
  uint8_t           Done;          /* flag for transistor detection done */
  uint8_t           AltFound;      /* alternative component type */
  uint8_t           Resistors;     /* number of resistors found */
  uint8_t           Diodes;        /* number of diodes found */
  uint8_t           Probe;         /* error: probe pin */ 
  uint16_t          U;             /* error: voltage in mV */
  #ifdef SW_SYMBOLS
  uint8_t           Symbol;        /* symbol ID */
  uint8_t           AltSymbol;     /* symbol ID for alternative component */
  #endif
} Check_Type;


/* resistor */
typedef struct
{
  uint8_t           A;             /* probe pin #1 */
  uint8_t           B;             /* probe pin #2 */
  int8_t            Scale;         /* exponent of factor (value * 10^x) */
  unsigned long     Value;         /* resistance */
} Resistor_Type;


/* capacitor */
typedef struct
{
  uint8_t           A;             /* probe pin #1 */
  uint8_t           B;             /* probe pin #2 */
  int8_t            Scale;         /* exponent of factor (value * 10^x) */
  unsigned long     Value;         /* capacitance incl. zero offset */
  unsigned long     Raw;           /* capacitance excl. zero offset */
  unsigned long     I_leak;        /* leakage current (in 10nA) */
} Capacitor_Type;


/* inductor */
typedef struct
{
  int8_t            Scale;         /* exponent of factor (value * 10^x) */
  unsigned long     Value;         /* inductance */  
} Inductor_Type;


/* diode */
typedef struct
{
  uint8_t           A;             /* probe pin connected to anode */
  uint8_t           C;             /* probe pin connected to cathode */
  uint16_t          V_f;           /* forward voltage in mV (high current) */
  uint16_t          V_f2;          /* forward voltage in mV (low current) */
} Diode_Type;


/* common semiconductors */
typedef struct
{
  uint8_t           A;             /* probe pin connected to pin A */
  uint8_t           B;             /* probe pin connected to pin B */
  uint8_t           C;             /* probe pin connected to pin C */
  uint16_t          U_1;           /* voltage #1 */
  int16_t           U_2;           /* voltage #2 (+/-) */
  uint32_t          F_1;           /* factor #1 */
  uint32_t          I_value;       /* current */
  int8_t            I_scale;       /* exponent of factor (value * 10^x) */
  uint32_t          C_value;       /* capacitance */
  int8_t            C_scale;       /* exponent of factor (value * 10^x) */
} Semi_Type;

/* 
  Mapping

           BJT          FET          SCR          Triac        IGBT
  ----------------------------------------------------------------------
  A        Base         Gate         Gate         Gate         Gate
  B        Collector    Drain        Anode        MT2          Collector
  C        Emitter      Source       Cathode      MT1          Emitter
  U_1      U_BE (mV)    R_DS (0.01)  V_GT (mV)    V_GT (mV)
  U_2                   V_th (mV)                              V_th (mV)
  F_1      hFE                                    MT2 (mV)
  I_value  I_CEO        I_DSS
  I_scale  I_CEO        I_DSS
  C_value  C_BE
  C_scale  C_BE
*/


/* special semiconductors */
typedef struct
{
  uint8_t           A;             /* probe pin connected to pin A */
  uint8_t           B;             /* probe pin connected to pin B */
  uint8_t           C;             /* probe pin connected to pin C */
  uint16_t          U_1;           /* voltage #1 */
  uint16_t          U_2;           /* voltage #2 */
} AltSemi_Type;

/* 
  Mapping

          PUT         UJT
  ------------------------------------------------------------------
  A       Gate        Emitter
  B       Anode       B2
  C       Cathode     B1
  U_1     V_f
  U_2     V_T
*/


/* SPI */
typedef struct
{
  uint8_t           ClockRate;     /* clock rate bits */
} SPI_Type;


/* I2C */
typedef struct
{
  uint8_t           Byte;          /* address/data byte */
  uint8_t           Timeout;       /* ACK timeout in 10µs */
} I2C_Type;



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

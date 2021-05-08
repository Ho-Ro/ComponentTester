/* ************************************************************************
 *
 *   common header file
 *
 *   (c) 2012-2013 by Markus Reschke
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



/* ************************************************************************
 *   constants
 * ************************************************************************ */


/* component IDs */
#define COMP_NONE             0
#define COMP_ERROR            1
#define COMP_MENU             2
#define COMP_RESISTOR        10
#define COMP_CAPACITOR       11
#define COMP_INDUCTOR        12
#define COMP_DIODE           20
#define COMP_BJT             21
#define COMP_FET             22
#define COMP_IGBT            23
#define COMP_TRIAC           24
#define COMP_THYRISTOR       25


/* error type IDs */
#define TYPE_DISCHARGE        1    /* discharge error */


/* FET type bit masks (also used for IGBTs) */
#define TYPE_N_CHANNEL        0b00000001     /* n channel */
#define TYPE_P_CHANNEL        0b00000010     /* p channel */
#define TYPE_ENHANCEMENT      0b00000100     /* enhancement mode */
#define TYPE_DEPLETION        0b00001000     /* depletion mode */
#define TYPE_MOSFET           0b00010000     /* MOSFET */
#define TYPE_JFET             0b00100000     /* JFET */
#define TYPE_IGBT             0b01000000     /* IGBT (no FET) */


/* BJT (bipolar junction transistor) type IDs */ 
#define TYPE_NPN              1    /* NPN */
#define TYPE_PNP              2    /* PNP */


/* tester operation modes */
#define MODE_CONTINOUS        0    /* continous */
#define MODE_AUTOHOLD         1    /* auto hold */


/* multiplicator tables */
#define TABLE_SMALL_CAP       1
#define TABLE_LARGE_CAP       2
#define TABLE_INDUCTOR        3


/* bit flags for PullProbe() */
#define FLAG_PULLDOWN         0b00000000
#define FLAG_PULLUP           0b00000001
#define FLAG_1MS              0b00001000
#define FLAG_10MS             0b00010000



/* ************************************************************************
 *   structures
 * ************************************************************************ */


/* tester modes, offsets and values */
typedef struct
{
  uint8_t           TesterMode;    /* tester operation mode */
  uint8_t           SleepMode;     /* MCU sleep mode */
  uint8_t           Samples;       /* number of ADC samples */
  uint8_t           AutoScale;     /* flag to disable/enable ADC auto scaling */
  uint8_t           RefFlag;       /* internal control flag for ADC */
  uint16_t          U_Bandgap;     /* voltage of internal bandgap reference (mV) */
  uint16_t          RiL;           /* internal pin resistance of µC in low mode (0.1 Ohms) */
  uint16_t          RiH;           /* internal pin resistance of µC in high mode (0.1 Ohms) */
  uint16_t          RZero;         /* resistance of probe leads (2 in series) (0.01 Ohms) */
  uint8_t           CapZero;       /* capacity zero offset (input + leads) (pF) */
  int8_t            RefOffset;     /* voltage offset of bandgap reference (mV) */
  int8_t            CompOffset;    /* voltage offset of analog comparator (mV) */
} Config_Type;


/* probes */
typedef struct
{
  /* probe pins */
  uint8_t             Pin_1;       /* probe-1 */
  uint8_t             Pin_2;       /* probe-2 */
  uint8_t             Pin_3;       /* probe-3 */

  /* bit masks for switching probes and test resistors */
  uint8_t             Rl_1;        /* Rl mask for probe-1 */
  uint8_t             Rh_1;        /* Rh mask for probe-1 */
  uint8_t             Rl_2;        /* Rl mask for probe-2 */
  uint8_t             Rh_2;        /* Rh mask for probe-2 */
  uint8_t             Rl_3;        /* Rl mask for probe-3 */
  uint8_t             Rh_3;        /* Rh mask for probe-3 */
  uint8_t             ADC_1;       /* ADC mask for probe-1 */
  uint8_t             ADC_2;       /* ADC mask for probe-2 */
} Probe_Type;


/* checking/probing */
typedef struct
{
  uint8_t           Done;          /* flag for transistor detection done */
  uint8_t           Found;         /* component type which was found */ 
  uint8_t           Type;          /* component specific subtype */
  uint8_t           Resistors;     /* number of resistors found */
  uint8_t           Diodes;        /* number of diodes found */
  uint8_t           Probe;         /* error: probe pin */ 
  uint16_t          U;             /* error: voltage left in mV */
} Check_Type;


/* resistor */
typedef struct
{
  uint8_t           A;             /* probe pin #1 */
  uint8_t           B;             /* probe pin #2 */
  uint8_t           Scale;         /* exponent of factor (value * 10^x) */
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


/* bipolar junction transistor */
typedef struct
{
  uint8_t           B;             /* probe pin connected to base */
  uint8_t           C;             /* probe pin connected to collector */
  uint8_t           E;             /* probe pin connected to emitter */
  unsigned long     hFE;           /* current amplification factor */
  /* BE voltage */
} BJT_Type;


/* FET */
typedef struct
{
  uint8_t           G;             /* test pin connected to gate */
  uint8_t           D;             /* test pin connected to drain */
  uint8_t           S;             /* test pin connected to source */
  uint16_t          V_th;          /* threshold voltage of gate in mV */
} FET_Type;


/* Error (failed discharge */
typedef struct
{

} Error_Type;



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   common header file
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
#define COMP_CELL             1
#define COMP_RESISTOR         2
#define COMP_CAPACITOR        3
#define COMP_DIODE           10
#define COMP_BJT             11
#define COMP_FET             12
#define COMP_IGBT            13
#define COMP_TRIAC           14
#define COMP_THYRISTOR       15


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



/* ************************************************************************
 *   structures
 * ************************************************************************ */


/* offsets and values */
typedef struct
{
  uint8_t           Samples;       /* number of ADC samples */
  uint8_t           RefFlag;       /* control flag for ADC */
  uint16_t          U_Bandgap;     /* voltage of internal bandgap reference (mV) */
  uint16_t          RiH;           /* internal pin resistance of �C in high mode (0.1 Ohms) */
  uint16_t          RiL;           /* internal pin resistance of �C in low mode (0.1 Ohms) */
  uint16_t          CapZero;       /* capacity zero offset (input + leads) (pF)*/
} Config_Type;


/* resistor */
typedef struct
{
  uint8_t           A;             /* probe pin #1 */
  uint8_t           B;             /* probe pin #2 */
  uint8_t           HiZ;           /* probe pin in HiZ mode */
  unsigned long     Value;         /* resistance */
  uint8_t           Scale;         /* exponent of factor (value * 10^x) */
} Resistor_Type;


/* capacitor */
typedef struct
{
  uint8_t           A;             /* probe pin #1 */
  uint8_t           B;             /* probe pin #2 */
  unsigned long     Value;         /* capacitance incl. zero offset */
  unsigned long     Raw;           /* capacitance excl. zero offset */
  int8_t            Scale;         /* exponent of factor (value * 10^x) */  
} Capacitor_Type;


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
  uint16_t          hfe;           /* current amplification factor */
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



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

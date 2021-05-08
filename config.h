/* ************************************************************************
 *
 *   global configuration, setup and settings
 *
 *   (c) 2012-2013 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* ************************************************************************
 *   Hardware options
 * ************************************************************************ */


/*
 *  2.5V voltage reference for Vcc check (PC4)
 *  - should be at least 10 times more precise than the voltage regulator
 *  - uncomment to enable
 */

//#define HW_REF25


/*
 *  Typical voltage of 2.5V voltage reference (in mV)
 *  - see datasheet of the voltage reference
 *  - or use >= 5.5 digit DMM to measure voltage
 */

#define UREF_25           2495


/*
 *  Probe protection relay for discharging caps (PC4):
 *  - low signal: short circuit probe pins
 *    high signal via external reference: remove short circuit 
 *  - uncomment to enable
 */

//#define HW_RELAY


/*
 *  voltage measurement up to 50V DC (10:1 voltage divider, PC3):
 *  - for Zener diodes
 *  - DC-DC boost converter controled by test push button
 *  - requires MCU with >=32kB Flash ans >=1kB EEPROM
 *  - uncomment to enable
 */

//#define HW_ZENER



/* ************************************************************************
 *   port and pin assignments
 * ************************************************************************ */


/*
 *  Test probes:
 *  - Must be an ADC port :-)
 *  - Lower 3 pins of the port must be used for probe pins.
 *  - Please don't change the definitions of TP1, TP2 and TP3!
 */

#define ADC_PORT         PORTC     /* ADC port data register */
#define ADC_DDR          DDRC      /* ADC port data direction register */
#define ADC_PIN          PINC      /* port input pins register */
#define TP1              PC0       /* test pin 1 (=0) */
#define TP2              PC1       /* test pin 2 (=1) */
#define TP3              PC2       /* test pin 3 (=2) */

#define TP_ZENER         PC3       /* test pin with 10:1 voltage divider */
#define TP_REF           PC4       /* test pin with 2.5V reference and relay */
#define TP_BAT           PC5       /* test pin with 4:1 voltage divider */


/*
 *  Probe resistors
 *
 *  The resistors must be connected to the lower 6 pins of the port in
 *  following sequence:
 *  - pin 0: Rl1 680R (test pin 1)
 *  - pin 1: Rh1 470k (test pin 1)
 *  - pin 2: Rl2 680R (test pin 2)
 *  - pin 3: Rh2 470k (test pin 2)
 *  - pin 4: Rl3 680R (test pin 3)
 *  - pin 5: Rh3 470k (test pin 3)
 */

#define R_PORT           PORTB     /* port data register */
#define R_DDR            DDRB      /* port data direction register */


/*
 *  push button and power management
 */

#define CONTROL_PORT     PORTD     /* port data register */
#define CONTROL_DDR      DDRD      /* port data direction register */
#define CONTROL_PIN      PIND      /* port input pins register */
#define POWER_CTRL       PD6       /* controls power (1: on / 0: off) */
#define TEST_BUTTON      PD7       /* test/start push button (low active) */


/*
 *  LCD module
 *  - Please see LCD.h!
 */



/* ************************************************************************
 *   Makefile workaround for some IDEs 
 * ************************************************************************ */


/*
 *  Oscillator startup cycles (after wakeup from power-safe mode):
 *  - typical values
 *    - internal RC:              6
 *    - full swing crystal:   16384 (also 256 or 1024 based on fuse settings)
 *    - low power crystal:    16384 (also 256 or 1024 based on fuse settings)
 *  - Please change value if it doesn't match your tester!
 */

#ifndef OSC_STARTUP
  #define OSC_STARTUP    16384
#endif



/* ************************************************************************
 *   misc settings
 * ************************************************************************ */


/*
 *  Languange of user interface. Available languages:
 *  - English (default)
 *  - German
 */

#define UI_ENGLISH
//#define UI_GERMAN


/*
 *  LCD module with cyrillic character set
 *  - uncomment if you are using such an LCD
 */

//#define LCD_CYRILLIC


/*
 *  Maximum time to wait after a measurement in continous mode (in ms).
 *  - Time between printing the result and starting a new cycle.
 */

#define CYCLE_DELAY      3000


/*
 *  Maximum number of measurements without any components found.
 *  - If that number is reached the tester powers off.
 */

#define CYCLE_MAX        5


/*
 *  Voltage drop by reverse voltage protection diode and power management.
 *  transistor (in mV):
 *  - Schottky diode about 200mV / Transistor about 100mV.
 *  - Get your DMM and measure the voltage drop!
 *  - Could be also used to compensate any offset by the voltage divider
 *    used to measure the battery voltage.
 */  

#define BAT_OFFSET       290


/*
 *  Battery low voltage (in mV).
 *  - Tester warns if BAT_POOR + 1V is reached.
 *  - Tester powers off if BAT_POOR is reached.
 *  - Voltage drop (BAT_OUT) is considered in calculation.
 */

#define BAT_POOR         6400



/* ************************************************************************
 *   measurement settings and offsets
 * ************************************************************************ */


/*
 *  ADC voltage reference based on Vcc (in mV). 
 */

#define UREF_VCC         5001


/*
 * Offset for the internal bandgap voltage reference (in mV): -100 up to 100
 *  - To compensate any difference between real value and measured value.
 *  - The ADC has a resolution of about 4.88mV for V_ref = 5V (Vcc) and
 *    1.07mV for V_ref = 1.1V (bandgap).
 *  - Will be added to measured voltage of bandgap reference.
 */

#define UREF_OFFSET      0


/*
 *  Exact values of probe resistors.
 *  - Standard value for Rl is 680 Ohms.
 *  - Standard value for Rh is 470k Ohms.
 */

/* Rl in Ohms */
#define R_LOW            680

/* Rh in Ohms */
#define R_HIGH           470000


/*
 *  Offset for systematic error of resistor measurement with Rh (470k) 
 *  in Ohms.
 */

#define RH_OFFSET        700 



/*
 *  Resistance of probe leads (in 0.01 Ohms).
 *  - Resistance of two probe leads in series.
 *  - Assuming all probe leads got same/similar resistance.
 */

#define R_ZERO           20


/* 
 *  Capacitance of the wires between PCB and terminals (in pF).
 *  Examples:
 *  - 2pF for wires 10cm long
 */

#define CAP_WIRES        2


/* 
 *  Capacitance of the probe leads connected to the tester (in pF).
 *  Examples:
 *    capacity  length of probe leads
 *    -------------------------------
 *     3pF      about 10cm
 *     9pF      about 30cm
 *    15pF      about 50cm
 */

#define CAP_PROBELEADS   9


/*
 *  Maximum voltage at which we consider a capacitor being
 *  discharged (in mV)
 */

#define CAP_DISCHARGED   2


/*
 *  Number of ADC samples to perform for each mesurement.
 *  - Valid values are in the range of 1 - 255.
 */

#define ADC_SAMPLES      25



/* ************************************************************************
 *   µC specific setup to support different AVRs
 * ************************************************************************ */


/*
 *  ATmega168
 */

#if defined(__AVR_ATmega168__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           196

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          225

  /* voltage offset of µCs analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   15

  /*
   *  capacitance of the probe tracks of the PCB and the µC (in pF)
   *  - 35 for ATmega168A
   *  - 36 for ATmega168
   */

  #define CAP_PCB             32

  /* total default capacitance (in pF): max. 255 */
  #define C_ZERO              CAP_PCB + CAP_WIRES + CAP_PROBELEADS

  /* memory layout: put stuff exceeding 512 bytes EEPROM into flash */
  #define MEM_TEXT          PROGMEM
  #define MEM_read_word(a)  pgm_read_word(a)
  #define MEM_read_byte(a)  pgm_read_byte(a)


/*
 *  ATmega328
 */

#elif defined(__AVR_ATmega328__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of µCs analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   15

  /* capacitance of the probe tracks of the PCB and the µC (in pF) */
  #define CAP_PCB             32

  /* total default capacitance (in pF): max. 255 */
  #define C_ZERO              CAP_PCB + CAP_WIRES + CAP_PROBELEADS

  /* memory layout: put stuff into EEPROM (1kB) */
  #define MEM_TEXT            EEMEM
  #define MEM_read_word(a)    eeprom_read_word(a)
  #define MEM_read_byte(a)    eeprom_read_byte(a)

  /* this MCU has 32kB Flash and 1kB EEPROM (enable extra features) */
  #define EXTRA               1


/*
 *  missing or unsupported µC
 */

#else

 #error "***********************************"
 #error "*                                 *"
 #error "*  No or wrong µC type selected!  *" 
 #error "*                                 *"
 #error "***********************************"

#endif



/* ************************************************************************
 *   ADC clock
 * ************************************************************************ */


/*
 *  selection of ADC clock 
 *  - ADC clock can be 125000 or 250000 
 *  - 250kHz is out of the full accuracy specification!
 */

#define ADC_FREQ    125000


/*
 *  define clock divider
 *  - supports 1MHz, 2MHz, 4MHz, 8MHz and 16MHz MCU clock
 *  - 4 for CPU clock of 1MHz and ADC clock of 250kHz
 *  - 128 for CPU clock of 16MHz and ADC clock of 125kHz
 */

#define CPU_FREQ    F_CPU

#if CPU_FREQ / ADC_FREQ == 4
  #define ADC_CLOCK_DIV (1 << ADPS1) 
#endif

#if CPU_FREQ / ADC_FREQ == 8
  #define ADC_CLOCK_DIV (1 << ADPS1) | (1 << ADPS0)
#endif

#if CPU_FREQ / ADC_FREQ == 16
  #define ADC_CLOCK_DIV (1 << ADPS2)
#endif

#if CPU_FREQ / ADC_FREQ == 32
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS0)
#endif

#if CPU_FREQ / ADC_FREQ == 64
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS1)
#endif

#if CPU_FREQ / ADC_FREQ == 128
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)
#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

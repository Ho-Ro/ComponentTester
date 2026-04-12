/* ************************************************************************
 *
 *   global configuration, setup and settings
 *
 *   (c) 2012-2024 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* source management */
#define CONFIG_H


/*
 *  For MCU specific settings (port and pin assignments) and display
 *  settings please edit also:
 *  - ATmega328:            config_328.h
 *  - ATmega324/644/1284:   config_644.h
 *  - ATmega640/1280/2560:  config_1280.h
 */



/* ************************************************************************
 *   Hardware options
 * ************************************************************************ */


/*
 *  rotary encoder for user interface
 *  - default pins: PD2 & PD3 (ATmega 328)
 *  - could be in parallel with LCD module
 *  - see ENCODER_PORT in config-<MCU>.h for port pins
 *  - uncomment to enable and also set ENCODER_PULSES & ENCODER_STEPS below
 *    to match your rotary encoder
 */

//#define HW_ENCODER


/*
 *  Number of Gray code pulses per step or detent for the rotary encoder
 *  - typical values: 2 or 4, rarely 1
 *  - a rotary encoder's pulse is the complete sequence of 4 Gray code pulses
 *  - adjust value to match your rotary encoder
 */

#define ENCODER_PULSES   4


/*
 *  Number of detents or steps
 *  - this is used by the detection of the rotary encoder's turning velocity
 *  - it doesn't have to match exactly and also allows you to finetune the
 *    the feedback (higher: slow down, lower: speed up)
 *  - typical values: 20, 24 or 30 
 *  - adjust value to match your rotary encoder
 */

#define ENCODER_STEPS    24


/*
 *  increase/decrease push buttons for user interface
 *  - alternative for rotary encoder
 *  - see KEY_PORT in config-<MCU>.h for port pins
 *  - uncomment to enable
 */

//#define HW_INCDEC_KEYS


/*
 *  2.5V voltage reference for Vcc check
 *  - default pin: PC4 (ATmega 328)
 *  - should be at least 10 times more precise than the voltage regulator
 *  - see TP_REF in config-<MCU>.h for port pin
 *  - uncomment to enable and also adjust UREF_25 below for your voltage
 *    reference
 */

//#define HW_REF25


/*
 *  Typical voltage of 2.5V voltage reference (in mV)
 *  - see datasheet of the voltage reference
 *  - or use >= 5.5 digit DMM to measure the voltage
 */

#define UREF_25           2495


/*
 *  Probe protection relay for discharging caps
 *  - default pin: PC4 (ATmega 328)
 *  - low signal: short circuit probe pins
 *    high signal via external reference: remove short circuit
 *  - see TP_REF in config_<MCU>.h for port pin
 *  - uncomment to enable
 */

//#define HW_DISCHARGE_RELAY


/*
 *  Zener check / voltage measurement up to 50V DC
 *  - default pin: PC3 (ATmega 328)
 *  - uses voltage divider (standard: 10:1)
 *  - DC-DC boost converter controlled by test push button
 *    (for alternative control modes see below)
 *  - see TP_ZENER in config_<MCU>.h for port pin
 *  - uncomment to enable
 */

//#define HW_ZENER


/*
 *  non-standard voltage divider for Zener check
 *  - standard voltage divider is 10:1 
 *  - ZENER_R1: top resistor in Ohms
 *  - ZENER_R2: bottom resistor in Ohms
 *  - uncomment to enable and adjust resistor values
 */

//#define ZENER_DIVIDER_CUSTOM
#define ZENER_R1         180000
#define ZENER_R2         20000


/*
 *  alternative mode for Zener check: don't switch boost converter
 *  - when the DC-DC boost converter runs all the time or when just
 *    measuring an external voltage (circuit without boost converter)
 *  - uncomment to enable
 */

//#define ZENER_UNSWITCHED


/*
 *  alternative mode for Zener check: switch converter via dedicated MCU pin
 *  - boost converter is controlled by a dedicated I/O pin
 *  - see BOOST_PORT in config_<MCU>.h for port pin
 *  - two drive methods:
 *    ZENER_BOOST_HIGH   high active / enabled when high
 *    ZENER_BOOST_LOW    low active / enabled when low
 *  - uncomment to enable and choose one drive method
 */

//#define ZENER_SWITCHED
//#define ZENER_BOOST_HIGH                /* high active */
#define ZENER_BOOST_LOW                 /* low active */


/*
 *  Zener check during normal probing
 *  - requires boost converter running all the time (ZENER_UNSWITCHED)
 *  - uncomment to enable
 *  - The min/max voltages are meant for the detection of a valid Zener voltage.
 *    The min. voltage should be higher than the noise floor, while the max.
 *    voltage should be lower than the boost converter's output voltage.
 */

//#define HW_PROBE_ZENER
#define ZENER_VOLTAGE_MIN     1000      /* min. voltage in mV */
#define ZENER_VOLTAGE_MAX     30000     /* max. voltage in mV */


/*
 *  fixed signal output
 *  - in case the MCU's OC1B pin is wired as dedicated signal output
 *    instead of driving the Rl probe resistor for test pin #2
 *  - uncomment to enable
 */

//#define HW_FIXED_SIGNAL_OUTPUT


/*
 *  basic frequency counter
 *  - default pin: T0 (PD4 ATmega 328)
 *  - uses T0 directly as frequency input
 *  - counts up to 1/4 of MCU clock rate
 *  - might be in parallel with LCD module
 *  - uncomment to enable
 */

//#define HW_FREQ_COUNTER_BASIC


/*
 *  extended frequency counter
 *  - low and high frequency crystal oscillators
 *    and buffered frequency input
 *  - prescalers 1:1 and 16:1 (32:1)
 *  - uses T0 directly as frequency input
 *  - see COUNTER_CTRL_PORT in config-<MCU>.h for port pins
 *  - requires a display with more than 2 text lines
 *  - uncomment to enable
 *  - select the circuit's prescaler setting: either 16:1 or 32:1
 */

//#define HW_FREQ_COUNTER_EXT
#define FREQ_COUNTER_PRESCALER     16   /* 16:1 */
//#define FREQ_COUNTER_PRESCALER     32   /* 32:1 */


/*
 *  ring tester (LOPT/FBT tester)
 *  - uses T0 directly as counter input
 *  - uncomment to enable
 *  - select the pulse output: either dedicated pin or probes
 *  - see RINGTESTER_PORT in config-<MCU>.h for dedicated pin 
 */

//#define HW_RING_TESTER
#define RING_TESTER_PIN                 /* dedicated pin */
//#define RING_TESTER_PROBES              /* probes */


/*
 *  event counter
 *  - default pin: T0 (PD4 ATmega 328)
 *  - uses T0 directly as event/pulse input (rising edge)
 *  - no shared operation with displays possible for T0
 *  - requires additional keys (e.g. rotary encoder) and a display with
 *    more than 5 lines
 *  - only for MCU clock of 8, 16 or 20MHz
 *  - uncomment to enable
 */

//#define HW_EVENT_COUNTER


/*
 *  trigger output for event counter
 *  - uses probe #2 as trigger output, probes #1 and #3 are Gnd
 *  - sets trigger output to high while counting
 *  - uncomment to enable
 */

//#define EVENT_COUNTER_TRIGGER_OUT


/*
 *  IR remote control detection/decoder (via dedicated MCU pin)
 *  - requires IR receiver module, e.g. TSOP series
 *  - module is connected to fixed I/O pin
 *  - see IR_PORT in config-<MCU>.h for port pin
 *  - uncomment to enable
 *  - for additional protocols also enable SW_IR_RX_EXTRA
 *  - for a confirmation beep enable SW_IR_RX_BEEP
 */

//#define HW_IR_RECEIVER


/*
 *  fixed cap for self-adjustment
 *  - see TP_CAP and ADJUST_PORT in config-<MCU>.h for port pins
 *  - uncomment to enable
 */

//#define HW_ADJUST_CAP


/*
 *  L/C meter hardware option
 *  - uses T0 directly as frequency input
 *  - see LC_CTRL_PORT in config-<MCU>.h for port pins
 *  - uncomment to enable
 */

//#define HW_LC_METER


/*
 *  L/C meter: value of reference capacitor C_p (in 0.1 pF)
 *  - should be around 1000pF
 */

#define LC_METER_C_REF        10000


/*
 *  L/C meter: also display frequency of LC oscillator
 *  - helps to spot the oscillator's frequency drifting 
 *  - requires display with more than two text lines
 *  - uncomment to enable
 */

//#define LC_METER_SHOW_FREQ


/*
 *  relay for parallel cap (sampling ADC)
 *  - uncomment to enable (not implemented yet)
 */

//#define HW_CAP_RELAY


/*
 *  Logic Probe
 *  - see TP_LOGIC in config_<MCU>.h for dedicated port pin
 *  - uses voltage divider (standard: 4:1, R1=10k, R2=3.3k, up to 20V)
 *  - LOGIC_PROBE_R1: top resistor in Ohms
 *  - LOGIC_PROBE_R2: bottom resistor in Ohms
 *  - requires additional keys (e.g. rotary encoder) and a display
 *    with more than 4 lines
 *  - uncomment to enable and adjust resistor values
 */

//#define HW_LOGIC_PROBE
#define LOGIC_PROBE_R1        10000
#define LOGIC_PROBE_R2        3300


/*
 *  Buzzer
 *  - see BUZZER_CTRL in config_<MCU>.h for port pin
 *  - buzzer types
 *    BUZZER_ACTIVE: active buzzer with integrated oscillator
 *    BUZZER_PASSIVE: passive buzzer
 *  - uncomment to enable and also select the correct buzzer type
 */

//#define HW_BUZZER
#define BUZZER_ACTIVE                   /* active buzzer */
//#define BUZZER_PASSIVE                  /* passive buzzer */


/*
 *  MAX6675 thermocouple converter
 *  - see MAX6675_CS in config_<MCU>.h for dedicated port pin
 *  - requires SPI bus and SPI read support
 *  - uncomment to enable
 */

//#define HW_MAX6675


/*
 *  MAX31855 thermocouple converter
 *  - see MAX31855_CS in config_<MCU>.h for dedicated port pin
 *  - requires SPI bus and SPI read support
 *  - uncomment to enable
 */

//#define HW_MAX31855


/*
 *  flashlight / general purpose switched output
 *  - see FLASHLIGHT_CTRL config_<MCU>.h for port pin
 *  - uncomment to enable
 */

//#define HW_FLASHLIGHT


/*
 *  BH1750VFI ambient light sensor
 *  - requires I2C bus and I2C read support
 *  - uncomment to enable and also select the correct I2C address
 */

//#define HW_BH1750
#define BH1750_I2C_ADDR       0x23      /* I2C address 0x23 (ADDR low) */
//#define BH1750_I2C_ADDR      0x5c       /* I2C address 0x5c (ADDR high) */



/* ************************************************************************
 *   software options
 * ************************************************************************ */


/*
 *  Self Test
 *  - comment out to disable
 */

#define SW_SELFTEST


/*
 *  PWM generator with simple user interface
 *  - signal output via OC1B
 *  - uncomment to enable
 */

#define SW_PWM_SIMPLE


/*
 *  PWM generator with fancy user interface
 *  - signal output via OC1B
 *  - requires additional keys and display with more than 2 text lines
 *  - uncomment to enable
 */

//#define SW_PWM_PLUS


/*
 *  PWM generator: show also pulse duration
 *  - duration based on timer's resolution
 *  - uncomment to enable
 */

//#define PWM_SHOW_DURATION


/*
 *  Inductance measurement
 *  - uncomment to enable
 */

#define SW_INDUCTOR


/*
 *  ESR measurement
 *  - requires MCU clock >= 8 MHz
 *  - choose SW_OLD_ESR for old method starting at 180nF
 *  - uncomment to enable
 */

#define SW_ESR
//#define SW_OLD_ESR


/*
 *  ESR Tool (in-circuit ESR measurement)
 *  - requires SW_ESR or SW_OLD_ESR to be enabled
 *  - uncomment to enable
 */

//#define SW_ESR_TOOL


/*
 *  check for rotary encoders
 *  - uncomment to enable
 */

//#define SW_ENCODER


/*
 *  squarewave signal generator
 *  - signal output via OC1B
 *  - requires additional keys
 *  - uncomment to enable
 */

#define SW_SQUAREWAVE


/*
 *  IR remote control detection/decoder (via probes)
 *  - requires IR receiver module, e.g. TSOP series
 *  - module will be connected to probe leads
 *  - uncomment to enable
 */

#define SW_IR_RECEIVER


/*
 *  probe pinout for IR receiver module
 *  - especially useful for testers with ZIF socket
 *  - select one
 */

#define SW_IR_RX_PINOUT_G_V_D      /* 1-Gnd 2-Vcc 3-Data (default) */
//#define SW_IR_RX_PINOUT_D_G_V      /* 1-Data 2-Gnd 3-Vcc */
//#define SW_IR_RX_PINOUT_D_V_G      /* 1-Data 2-Vcc 3-Gnd */



/*
 *  current limiting resistor for IR receiver module
 *  - for 5V only modules
 *  - Warning: any short circuit may destroy your MCU
 *  - uncomment to disable resistor
 */

//#define SW_IR_DISABLE_RESISTOR


/*
 *  confirmation beep for valid data frame/packet
 *  - requires buzzer (HW_BUZZER)
 *  - uncomment to enable
 */

//#define SW_IR_RX_BEEP


/*
 *  additional protocols for IR remote control detection/decoder
 *  - uncommon protocols which will increase flash memory usage ;)
 *  - uncomment to enable
 */

//#define SW_IR_RX_EXTRA


/*
 *  IR remote control sender
 *  - signal output via OC1B
 *  - requires additional keys and display with more than 4 text lines
 *  - also requires an IR LED with a simple driver
 *  - uncomment to enable
 */

//#define SW_IR_TRANSMITTER


/*
 *  alternative delay loop for IR remote control sender
 *  - in case the the C compiler screws up the default delay loop
 *    and causes incorrect pulse/pause timings
 *  - uncomment to enable
 */

//#define SW_IR_TX_ALTDELAY


/*
 *  additional protocols for IR remote control sender
 *  - uncommon protocols which will increase flash memory usage ;)
 *  - uncomment to enable
 */

//#define SW_IR_TX_EXTRA


/*
 *  check for opto couplers
 *  - uncomment to enable
 */

#define SW_OPTO_COUPLER


/*
 *  check for Unijunction Transistor
 *  - uncomment to enable
 */

#define SW_UJT


/*
 *  check for Schottky Transistor (Schottky-clamped BJT)
 *  - uncomment to enable
 */

#define SW_SCHOTTKY_BJT


/*
 *  Servo Check
 *  - signal output via OC1B
 *  - requires additional keys and display with more than 2 text lines
 *  - uncomment to enable
 */

//#define SW_SERVO


/*
 *  DS18B20 - OneWire temperature sensor
 *  - uncomment to enable
 *  - also enable ONEWIRE_PROBES or ONEWIRE_IO_PIN (see section 'Busses')
 *  - please see UI_ROUND_DS18B20
 */

//#define SW_DS18B20


/*
 *  DS18S20 - OneWire temperature sensor
 *  - DS18S20_HIGHRES: enable high resolution (0.01°C)
 *    normal resolution is 0.5°C
 *  - uncomment to enable
 *  - also enable ONEWIRE_PROBES or ONEWIRE_IO_PIN (see section 'Busses')
 */

//#define SW_DS18S20
//#define DS18S20_HIGHRES       /* high resolution (0.01°C) */


/*
 *  OneWire: read and display ROM code
 *  - option for OneWire related tools
 *  - requires display with more than 2 text lines
 *  - uncomment to enable
 */

//#define ONEWIRE_READ_ROM


/*
 *  scan OneWire bus for devices and list their ROM codes
 *  - requires display with more than 2 text lines
 *  - uncomment to enable
 *  - also enable ONEWIRE_PROBES or ONEWIRE_IO_PIN (see section 'Busses')
 */

//#define SW_ONEWIRE_SCAN


/*
 *  capacitor leakage check
 *  - requires display with more than two lines
 *  - uncomment to enable
 */

//#define SW_CAP_LEAKAGE


/*
 *  display reverse hFE for BJTs
 *  - hFE for collector and emitter reversed
 *  - uncomment to enable
 */

//#define SW_REVERSE_HFE


/*
 *  display I_C/I_E test current for hFE measurement
 *  - I_C for common emitter circuit
 *    I_E for common collector circuit
 *  - uncomment to enable
 */

//#define SW_HFE_CURRENT


/*
 *  display C_be (base-emitter capacitance) for BJTs
 *  - uncomment to enable
 */

//#define SW_C_BE


/*
 *  R/C/L monitors
 *  - monitor passive components connected to probes #1 and #3
 *  - monitors for L require SW_INDUCTOR to be enabled
 *  - for ESR either SW_ESR or SW_OLD_ESR needs to be enabled
 *  - uncomment to enable (one or more)
 */

//#define SW_MONITOR_R          /* just R */
//#define SW_MONITOR_C          /* just C plus ESR */
//#define SW_MONITOR_L          /* just L */
//#define SW_MONITOR_RCL        /* R plus L, or C plus ESR */
//#define SW_MONITOR_RL         /* R plus L */


/*
 *  C/L monitors: auto hold
 *  - requires display with more than two text lines
 *  - uncomment to enable (one or more)
 */

//#define SW_MONITOR_HOLD_ESR   /* auto-hold ESR (C monitor) */
//#define SW_MONITOR_HOLD_L     /* auto-hold L (L monitor) */


/*
 *  DHT11, DHT22 and compatible humidity & temperature sensors
 *  - uncomment to enable
 */

//#define SW_DHTXX


/*
 *  check resistor for matching E series norm value
 *  - requires a display with more than 2 text lines
 *  - color-code mode requires a color graphics display
 *  - uncomment to enable (one or more)
 */

//#define SW_R_E24_5_T          /* E24 5% tolerance, text */
//#define SW_R_E24_5_CC         /* E24 5% tolerance, color-code */
//#define SW_R_E24_1_T          /* E24 1% tolerance, text */
//#define SW_R_E24_1_CC         /* E24 1% tolerance, color-code */
//#define SW_R_E96_T            /* E96 1% tolerance, text */
//#define SW_R_E96_CC           /* E96 1% tolerance, color-code */
//#define SW_R_E96_EIA96        /* E96 1% tolerance, EIA-96 code */


/*
 *  check capacitor for matching E series norm value
 *  - requires a display with more than 2 text lines
 *  - uncomment to enable (one or more)
 */

//#define SW_C_E6_T             /* E6 20% tolerance, text */
//#define SW_C_E12_T            /* E12 10% tolerance, text */


/*
 *  check inductor for matching E series norm value
 *  - requires a display with more than 2 text lines
 *  - uncomment to enable (one or more)
 */

//#define SW_L_E6_T             /* E6 20% tolerance, text */
//#define SW_L_E12_T            /* E12 10% tolerance, text */


/*
 *  continuity check
 *  - requires buzzer (HW_BUZZER)
 *  - uncomment to enable
 */

//#define SW_CONTINUITY_CHECK


/*
 *  show additional info for a possible potentiometer/trimpot
 *  - shows sum of both resistors and ratios in %
 *  - uncomment to enable
 */

//#define SW_R_TRIMMER


/*
 *  show self-discharge voltage loss (in %) of a capacitor > 50nF
 *  - uncomment to enable
 */ 

//#define SW_C_VLOSS


/*
 *  photodiode check
 *  - uncomment to enable
 */

//#define SW_PHOTODIODE


/*
 *  diode/LED quick-check
 *  - requires a display with more than 2 text lines
 *  - uncomment to enable
 */

//#define SW_DIODE_LED


/*
 *  Voltmeter 0-5V DC
 *  - warning: no input protection!!!
 *  - with optional buzzer:
 *    beep when default threshold is exceeded
 *  - uncomment to enable
 */

//#define SW_METER_5VDC
#define METER_5VDC_THRESHOLD  25        /* default threshold in 100 mV */



/* ************************************************************************
 *   workarounds for some testers
 * ************************************************************************ */


/*
 *  Disable hFE measurement with common collector circuit and Rl as
 *  base resistor.
 *  - problem:
 *    hFE values are way too high.
 *  - affected testers:
 *    Hiland M644 (under investigation, possibly poor PCB design)
 *  - uncomment to enable
 */

//#define NO_HFE_C_RL


/*
 *  Alternative power control for clones with SCT15L104W management MCU.
 *  - problem:
 *    tester turns off suddenly after first probing cycle
 *  - affected testers:
 *    T7-H, presumably also other models of the TC-1 family
 *  - uncomment to enable
 */

//#define PASSIVE_POWER_CTRL



/* ************************************************************************
 *   workarounds for some IDEs 
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
 *   user interface
 * ************************************************************************ */


/*
 *  Language of user interface.
 *  - Available languages:
 *    - English (default)
 *    - Brazilian Portuguese
 *    - Czech (based on ISO 8859-1)
 *    - Czech 2 (with Czech characters based on ISO 8859-2)
 *    - Danish
 *    - French (based on ISO 8859-1)
 *    - German
 *    - Polish (based on ISO 8859-1)
 *    - Polish 2 (with Polish characters based on ISO 8859-2)
 *    - Spanish
 *    - Romanian
 *    - Russian (with cyrillic characters based on Windows-1251)
 *    - Russian 2 (with cyrillic characters based on Windows-1251)
 *  - choose one language
 */

#define UI_ENGLISH
//#define UI_BRAZILIAN
//#define UI_CZECH
//#define UI_CZECH_2
//#define UI_DANISH
//#define UI_FRENCH
//#define UI_GERMAN
//#define UI_ITALIAN
//#define UI_POLISH
//#define UI_POLISH_2
//#define UI_ROMANIAN
//#define UI_RUSSIAN
//#define UI_RUSSIAN_2
//#define UI_SPANISH


/*
 *  Use comma instead of dot to indicate a decimal fraction.
 *  - uncomment to enable
 */

//#define UI_COMMA


/*
 *  Display temperatures in Fahrenheit instead of Celsius.
 *  - uncomment to enable
 */

//#define UI_FAHRENHEIT


/*
 *  Display 4-digit values as value with metric prefix (where applicable).
 *  - 1234  -> 1.234k
 *    1234k -> 1.234M
 *    1234p -> 1.234n
 *  - uncomment to enable.
 */

//#define UI_PREFIX


/*
 *  Display hexadecimal values in uppercase instead of lowercase.
 *  - uncomment to enable
 */

//#define UI_HEX_UPPERCASE


/*
 *  Set the default operation mode to auto-hold.
 *  - instead of continuous mode
 *  - uncomment to enable
 */

//#define UI_AUTOHOLD


/*
 *  Switch temporarily to auto-hold mode when a component is detected.
 *  - only in continuous mode
 *  - uncomment to enable
 */

//#define UI_AUTOHOLD_FOUND


/*
 *  Trigger the menu also by a short circuit of all three probes.
 *  - former default behaviour
 *  - uncomment to enable
 */

//#define UI_SHORT_CIRCUIT_MENU


/*
 *  Show key hints instead of cursor if available.
 *  - currently only "Menu/Test"
 *  - requires additional keys and display with a sufficient number of
 *    text lines (recommended: >= 8 lines)
 *  - uncomment to enable
 */

//#define UI_KEY_HINTS


/*
 *  Enter menu to select adjustment profile after powering on.
 *  - uncomment to enable
 */

//#define UI_CHOOSE_PROFILE


/*
 *  Add a third profile for adjustment values.
 *  - uncomment to enable
 */

//#define UI_THREE_PROFILES


/*
 *  Output components found also via TTL serial interface.
 *  - uncomment to enable
 *  - also enable SERIAL_BITBANG or SERIAL_HARDWARE (see section 'Busses')
 */

//#define UI_SERIAL_COPY


/*
 *  Control tester via TTL serial interface.
 *  - uncomment to enable
 *  - also enable SERIAL_BITBANG or SERIAL_HARDWARE, plus SERIAL_RW
 *    (see section 'Busses') 
 */

//#define UI_SERIAL_COMMANDS


/*
 *  Maximum time to wait after probing (in ms).
 *  - applies to continuous mode only
 *  - Time between displaying the result and starting a new probing cycle.
 */

#define CYCLE_DELAY      3000


/*
 *  Maximum number of probing runs without any component found in a row.
 *  - applies to continuous mode only
 *  - If this number is reached the tester will power off.
 *  - When set to zero the tester will run only once and turn off
 *    after CYCLE_DELAY.
 *  - When set to 255 this feature will be disabled and the tester runs
 *    until it's powered off manually.
 */

#define CYCLE_MAX        5


/*
 *  Automatic power-off when no button is pressed for a while (in s).
 *  - applies to auto-hold mode only
 *  - uncomment to enable, also adjust timeout (in s)
 */

//#define POWER_OFF_TIMEOUT     60


/*
 *  fancy pinout with component symbols for 3-pin semiconductors
 *  - probe numbers left and right of symbol
 *  - requires graphics display and symbol bitmaps (config_<MCU>.h)
 *  - uncomment to enable
 */

#define SW_SYMBOLS


/*
 *  fancy pinout: show right-hand probe numbers above/below symbol
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_PINOUT_ALT


/*
 *  failed test run: display question mark symbol 
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_QUESTION_MARK


/*
 *  any Zener check: display Zener diode symbol
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_ZENER_DIODE


/*
 *  extended frequency counter: display quartz crystal symbol for LF/HF modes
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_QUARTZ_CRYSTAL


/*
 *  DS18B20/DS18S20/DHTXX: display sensor symbol
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_ONEWIRE


/*
 *  disable text based pinout for 3-pin semiconductors
 *  - requires component symbols (SW_SYMBOLS) to be enabled
 *  - uncomment to enable
 */

//#define UI_NO_TEXTPINOUT


/*
 *  disable text based pinout of body/intrinsic diode for MOSFETs
 *  - uncomment to enable
 */

//#define UI_NO_BODYDIODE_TEXTPINOUT


/*
 *  battery status: display icon
 *  - requires font with additional characters (check font!)
 *  - not available for HD44780 and ST7036 based displays
 *  - can't be used with LCD_VT100
 *  - uncomment to enable
 */

//#define UI_BATTERY


/*
 *  battery status: display in last line after showing probing result
 *  - uncomment to enable
 */

//#define UI_BATTERY_LASTLINE


/*
 *  display probe IDs using reversed colors
 *  - requires font with additional characters (check font!)
 *  - not available for HD44780 and ST7036 based displays
 *  - uncomment to enable
 */

//#define UI_PROBE_REVERSED


/*
 *  color coding for probes
 *  - requires color graphics display
 *  - uncomment to enable
 *  - edit colors.h to select correct probe colors
 *    (COLOR_PROBE_1, COLOR_PROBE_2 and COLOR_PROBE_3)
 */

#define UI_PROBE_COLORS


/*
 *  colored titles
 *  - requires color graphics display
 *  - edit colors.h to select prefered color (COLOR_TITLE)
 *  - uncomment to enable
 */

//#define UI_COLORED_TITLES


/*
 *  colored cursor and key hints
 *  - requires color graphics display
 *  - edit colors.h to select prefered color (COLOR_CURSOR)
 *  - uncomment to enable
 */

//#define UI_COLORED_CURSOR


/*
 *  colored values
 *  - just the value, not the unit
 *  - requires color graphics display
 *  - edit colors.h to select prefered color (COLOR_VALUE)
 *  - uncomment to enable
 */

//#define UI_COLORED_VALUES


/*
 *  menues: scroll menu page-wise instead of item-wise
 *  - speeds up menu operation with graphics displays,
 *    especially high resolution color displays 
 *  - uncomment to enable
 */

//#define UI_MENU_PAGEMODE


/*
 *  automatically exit main menu after running function/tool
 *  - uncomment to enable
 */

//#define UI_MAINMENU_AUTOEXIT


/*
 *  main menu: power off tester
 *  - uncomment to enable
 */

//#define SW_POWER_OFF


/*
 *  main menu: display font for test purposes
 *  - default output format:
 *    index number (hex) and 8 characters (including unavailable ones)
 *  - packed output format:
 *    no index, only available characters, complete text line
 *  - uncomment to enable
 */

//#define SW_FONT_TEST
//#define FONT_PACKED           /* packed output format */


/*
 *  main menu: display component symbols for test purposes
 *  - requires component symbols be enabled (SW_SYMBOLS)
 *  - uncomment to enable
 */

//#define SW_SYMBOL_TEST


/*
 *  Round some values if appropriate.
 *  - for
 *    - DS18B20 (0.1 °C/F)
 *  - uncomment to enable
 */

//#define UI_ROUND_DS18B20


/*
 *  Center-align infos and some other texts
 *  - requires display with more than 3 text lines 
 *  - uncomment to enable
 */

//#define UI_CENTER_ALIGN


/*
 *  confirmation beep when probing is done
 *  - requires buzzer (HW_BUZZER)
 *  - uncomment to enable
 */

//#define UI_PROBING_DONE_BEEP


/*
 *  Self-Test/Adjustment: display measurement values page-wise
 *  - requires display with 6 text lines or more
 *  - uncomment to enable
 */

//#define UI_TEST_PAGEMODE


/*
 *  storage of firmware data (texts, tables etc)
 *  - self-adjustment data is always stored in EEPROM
 *  - fonts and symbols are always stored in Flash memory
 *  - uncomment one
 */ 

#define DATA_EEPROM           /* store data in EEPROM */
//#define DATA_FLASH            /* store data in Flash */



/* ************************************************************************
 *   power management
 * ************************************************************************ */


/*
 *  type of power switch
 *  - soft-latching power switch (default)
 *    - as in the tester's reference circuit 
 *    - tester is able to power itself off
 *  - manual power switch
 *    - tester isn't able to power itself off
 *  - enable one
 */

#define POWER_SWITCH_SOFT
//#define POWER_SWITCH_MANUAL


/*
 *  Battery monitoring mode:
 *  - BAT_NONE     disable battery monitoring completely
 *  - BAT_DIRECT   direct measurement of battery voltage (< 5V)
 *  - BAT_DIVIDER  measurement via voltage divider
 *  - uncomment one of the modes
 */

//#define BAT_NONE
//#define BAT_DIRECT
#define BAT_DIVIDER


/*
 *  Unmonitored optional external power supply
 *  - Some circuits supporting an additional external power supply are designed
 *    in a way that prevents the battery monitoring to measure the voltage of
 *    the external power supply. This would trigger the low battery shut-down.
 *    The switch below will prevent the shut-down when the measured voltage is
 *    below 0.9V (caused by the diode's leakage current).
 *  - uncomment to enable
 */

//#define BAT_EXT_UNMONITORED


/*
 *  Voltage divider for battery monitoring
 *  - BAT_R1: top resistor in Ohms
 *  - BAT_R2: bottom resistor in Ohms
 *  - standard values are: R1=10k, R2=3.3k
 */

#define BAT_R1           10000
#define BAT_R2           3300


/*
 *  Voltage drop by reverse voltage protection diode and power management
 *  transistor (in mV):
 *  - or any other circuitry in the power section
 *  - Get your DMM and measure the voltage drop!
 *  - Schottky diode about 200mV / PNP BJT about 100mV.
 */  

#define BAT_OFFSET       290


/*
 *  Battery weak voltage (in mV).
 *  - Tester warns if BAT_WEAK is reached.
 *  - Voltage drop BAT_OFFSET is considered in calculation.
 */

#define BAT_WEAK         7400


/*
 *  Battery low voltage (in mV).
 *  - Tester powers off if BAT_LOW is reached.
 *  - Voltage drop BAT_OFFSET is considered in calculation.
 */

#define BAT_LOW          6400 


/*
 *  Enter sleep mode when idle to save power.
 *  - uncomment to enable
 */

#define SAVE_POWER



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
 *  - if resistors >20k measure too high or low adjust the offset accordingly
 *  - standard offset is 350 Ohms
 */

#define RH_OFFSET        350


/*
 *  Resistance of probes (in 0.01 Ohms).
 *  - default offset for PCB tracks and probe leads
 *  - resistance of two probes in series
 *  - assuming all probes have same/similar resistance
 *  - will be updated by self-adjustment
 */

#define R_ZERO           20


/*
 *  Use probe pair specific resistance offsets instead of an
 *  average value for all probes.
 *  - uncomment to enable
 */

//#define R_MULTIOFFSET


/* 
 *  Capacitance of probes (in pF).
 *  - default offset for MCU, PCB tracks and probe leads
 *  - Examples:
 *    capacitance  length
 *    -------------------------
 *     3pF         about 10cm
 *     9pF         about 30cm
 *    15pF         about 50cm
 *  - maximum value: 100
 *  - will be updated by self-adjustment
 */

#define C_ZERO           43


/*
 *  Use probe pair specific capacitance offsets instead of an
 *  average value for all probes.
 *  - uncomment to enable
 */

//#define CAP_MULTIOFFSET


/*
 *  Maximum voltage at which we consider a capacitor being
 *  discharged (in mV).
 */

#define CAP_DISCHARGED   2


/*
 *  Correction factors for capacitors (in 0.1%)
 *  - positive factor increases capacitance value
 *    negative factor decreases capacitance value
 *  - CAP_FACTOR_SMALL for caps < 4.7µF
 *  - CAP_FACTOR_MID for caps 4.7 - 47µF
 *  - CAP_FACTOR_LARGE for caps > 47µF
 */

#define CAP_FACTOR_SMALL      0      /* no correction */ 
#define CAP_FACTOR_MID        -40    /* -4.0% */
#define CAP_FACTOR_LARGE      -90    /* -9.0% */


/*
 *  Number of ADC samples to perform for each mesurement.
 *  - Valid values are in the range of 1 - 255.
 */

#define ADC_SAMPLES      25


/*
 *  100nF AREF buffer capacitor
 *  - used by some MCU boards
 *  - will increase measurement time
 *  - recommendation: replace with 1nF capacitor
 *  - uncomment to enable
 */

//#define ADC_LARGE_BUFFER_CAP



/* ************************************************************************
 *   R & D - meant for firmware developers
 * ************************************************************************ */


/*
 *  Enable read functions for display module.
 *  - display driver and interface settings have to support this
 *  - uncomment to enable
 */

//#define LCD_READ


/*
 *  Read ID of display controller.
 *  - ID is shown at welcome screen (after firmware version)
 *  - requires display read functions (LCD_READ)
 *  - recommended: serial output (UI_SERIAL_COPY)
 *  - uncomment to enable
 */

//#define SW_DISPLAY_ID


/*
 *  Read registers of display controller and output them via TTL serial.
 *  - requires display read functions (LCD_READ) and
 *    serial output (UI_SERIAL_COPY)
 *  - uncomment to enable
 */

//#define SW_DISPLAY_REG



/* ************************************************************************
 *   MCU specific setup to support different AVRs
 * ************************************************************************ */


/* MCU clock */
#define CPU_FREQ    F_CPU


/*
 *  ATmega 328/328P
 */

#if defined(__AVR_ATmega328__)
  #include "config_328.h"


/*
 *  ATmega 324P/324PA/644/644P/644PA/1284/1284P
 */

#elif defined (__AVR_ATmega324P__) || defined (__AVR_ATmega644__) || defined (__AVR_ATmega1284__)
  #include "config_644.h"


/*
 *  ATmega 640/1280/2560
 */

#elif defined (__AVR_ATmega640__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
  #include "config_1280.h"


/*
 *  missing or unsupported MCU
 */

#else
  #error <<< No or wrong MCU type selected! >>>
#endif



/* ************************************************************************
 *   Busses
 * ************************************************************************ */


/*
 *  I2C bus
 *  - might be required by some hardware
 *  - could be enabled already in display section (config_<MCU>.h)
 *  - for bit-bang I2C port and pins see I2C_PORT (config_<MCU>.h)
 *  - hardware I2C (TWI) uses automatically the proper MCU pins
 *  - uncomment either I2C_BITBANG or I2C_HARDWARE to enable
 *  - uncomment one of the bus speed modes
 */

//#define I2C_BITBANG                /* bit-bang I2C */
//#define I2C_HARDWARE               /* MCU's hardware TWI */
//#define I2C_STANDARD_MODE          /* 100kHz bus speed */
//#define I2C_FAST_MODE              /* 400kHz bus speed */
//#define I2C_RW                     /* enable I2C read support */


/*
 *  SPI bus
 *  - might be required by some hardware
 *  - could be enabled already in display section (config_<MCU>.h)
 *  - for bit-bang SPI port and pins see SPI_PORT (config_<MCU>.h)
 *  - hardware SPI uses automatically the proper MCU pins
 *  - uncomment either SPI_BITBANG or SPI_HARDWARE to enable
 */

//#define SPI_BITBANG                /* bit-bang SPI */
//#define SPI_HARDWARE               /* hardware SPI */
//#define SPI_RW                     /* enable SPI read support */
//#define SPI_SLOWDOWN               /* slow down bit-bang SPI */


/*
 *  TTL serial interface
 *  - could be enabled already in display section (config_<MCU>.h)
 *  - for bit-bang serial port and pins see SERIAL_PORT (config_<MCU>.h)
 *  - hardware serial uses automatically the proper MCU pins
 *  - uncomment either SERIAL_BITBANG or SERIAL_HARDWARE to enable
 */

//#define SERIAL_BITBANG             /* bit-bang serial */
//#define SERIAL_HARDWARE            /* hardware serial */
//#define SERIAL_RW                  /* enable serial read support */


/*
 *  OneWire bus
 *  - for dedicated I/O pin please see ONEWIRE_PORT (config_<MCU>.h)
 *  - uncomment either ONEWIRE_PROBES or ONEWIRE_IO_PIN to enable
 */

//#define ONEWIRE_PROBES             /* via probes */
//#define ONEWIRE_IO_PIN             /* via dedicated I/O pin */



/* ************************************************************************
 *   ADC clock
 * ************************************************************************ */


/*
 *  ADC clock 
 *  - The ADC clock is 125000Hz by default.
 *  - You could also set 250000Hz, but that exceeds the max. ADC clock
 *    of 200kHz for 10 bit resolution!
 *  - Special case for 20MHz MCU clock: 156250Hz
 */

#if CPU_FREQ == 20000000
  /* 20MHz MCU clock */
  #define ADC_FREQ    156250
#else
  /* all other MCU clocks */
  #define ADC_FREQ    125000
#endif



/* ************************************************************************
 *   additional stuff supporting this configuration
 * ************************************************************************ */


#include "config_support.h"



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

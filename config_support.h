/* ************************************************************************
 *
 *   support for global configuration
 *
 *   (c) 2012-2022 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* source management */
#define CONFIG_SUPPORT_H



/* ************************************************************************
 *   ADC clock
 * ************************************************************************ */


/*
 *  define clock divider
 *  - supports 1MHz, 2MHz, 4MHz, 8MHz, 16MHz and 20MHz MCU clocks
 *  - we got only 7 fixed prescalers from 2 up to 128
 */

/* 1MHz/250kHz */
#if CPU_FREQ / ADC_FREQ == 4
  #define ADC_CLOCK_DIV (1 << ADPS1) 
#endif

/* 1MHz/125kHz 2MHz/250kHz */
#if CPU_FREQ / ADC_FREQ == 8
  #define ADC_CLOCK_DIV (1 << ADPS1) | (1 << ADPS0)
#endif

/* 2MHz/125kHz 4MHz/250kHz */
#if CPU_FREQ / ADC_FREQ == 16
  #define ADC_CLOCK_DIV (1 << ADPS2)
#endif

/* 4MHz/125kHz 8MHz/250kHz */
#if CPU_FREQ / ADC_FREQ == 32
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS0)
#endif

/* 8MHz/125kHz 16MHz/250kHz */
#if CPU_FREQ / ADC_FREQ == 64
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS1)
#endif

/* 16MHz/125kHz 20MHz/156.25kHz */
#if CPU_FREQ / ADC_FREQ == 128
  #define ADC_CLOCK_DIV (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)
#endif



/* ************************************************************************
 *   derived values
 * ************************************************************************ */


/*
 *  number of MCU cycles per µs
 *  - min. 1 (for 1MHz)
 *  - max. 20 (for 20MHz)
 */

#define MCU_CYCLES_PER_US     (CPU_FREQ / 1000000)


/*
 *  number of MCU cycles per ADC cycle
 *  - min. 4
 *  - max. 128
 */ 

#define MCU_CYCLES_PER_ADC    (CPU_FREQ / ADC_FREQ)


/*
 *  time of a MCU cycle (in 0.1 ns)
 */

#define MCU_CYCLE_TIME        (10000 / (CPU_FREQ / 1000000))



/* ************************************************************************
 *   check display drivers
 * ************************************************************************ */


/*
 *  check if a display module is specified
 */

#if ! defined (LCD_TEXT) && ! defined (LCD_GRAPHIC)
  #error <<< No display module specified! >>>
#endif


/*
 *  check if more than one display module is specified
 *  - does not work for enabling same display multiple times 
 */

#ifdef LCD_HD44780
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_HD44780
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ILI9163
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ILI9163
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ILI9341
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ILI9341
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ILI9481
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ILI9481
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ILI9486
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ILI9486
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ILI9488
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ILI9488
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_PCD8544
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_PCD8544
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_PCF8814
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_PCF8814
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_SH1106
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_SH1106
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_SSD1306
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_SSD1306
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ST7036
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ST7036
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ST7565R
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ST7565R
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ST7735
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ST7735
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_SEMI_ST7735
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_SEMI_ST7735
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_ST7920
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_ST7920
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_STE2007
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_STE2007
  #else
    #define DISPLAY_ONE
  #endif
#endif

#ifdef LCD_VT100
  #ifdef DISPLAY_ONE
    #ifndef DISPLAY_MULTI
      #define DISPLAY_MULTI
    #endif
    #undef LCD_VT100
  #else
    #define DISPLAY_ONE
  #endif
#endif

/* show error */
#ifdef DISPLAY_MULTI
  #error <<< Multiple display modules specified! >>>
#endif

/* clean up */
#undef DISPLAY_ONE
#undef DISPLAY_MULTI



/* ************************************************************************
 *   options management
 * ************************************************************************ */


/*
 *  storage of program data (EEPROM/Flash)
 */

#if defined (DATA_EEPROM)
  /* memory type */
  #define MEM_TYPE            EEMEM

  /* read functions */
  #define DATA_read_byte(addr)     eeprom_read_byte(addr)
  #define DATA_read_word(addr)     eeprom_read_word(addr)
#elif defined (DATA_FLASH)
  /* memory type */
  #define MEM_TYPE            PROGMEM

  /* read functions */
  #define DATA_read_byte(addr)     pgm_read_byte(addr)
  #define DATA_read_word(addr)     pgm_read_word(addr)
#endif



/*
 *  hardware/software options
 */

/* power switch: prefer soft-latching type */
#ifdef POWER_SWITCH_SOFT
  #ifdef POWER_SWITCH_MANUAL
    #undef POWER_SWITCH_MANUAL
  #endif
#endif


/* additional keys */
/* rotary encoder, increase/decrease push buttons or touch screen */
#if defined (HW_ENCODER) || defined (HW_INCDEC_KEYS) | defined (HW_TOUCH)
  #define HW_KEYS
#endif

/* options which require additional keys */
#ifndef HW_KEYS

  /* key hints */
  #ifdef UI_KEY_HINTS
    #undef UI_KEY_HINTS
  #endif

  /* PWM+ */
  #ifdef SW_PWM_PLUS
    #undef SW_PWM_PLUS
    #define SW_PWM_SIMPLE   
  #endif

  /* squarewave generator */
  #ifdef SW_SQUAREWAVE
    #undef SW_SQUAREWAVE
  #endif

  /* Servo Check */
  #ifdef SW_SERVO
    #undef SW_SERVO
  #endif

  /* event counter */
  #ifdef HW_EVENT_COUNTER
    #undef HW_EVENT_COUNTER
  #endif

  /* IR Sender */
  #ifdef SW_IR_TRANSMITTER
    #undef SW_IR_TRANSMITTER
  #endif

  /* logic probe */
  #ifdef HW_LOGIC_PROBE
    #undef HW_LOGIC_PROBE
  #endif

#endif


/* options which require inductance measurement */
#ifndef SW_INDUCTOR

  /* L monitor */
  #ifdef SW_MONITOR_L
    #undef SW_MONITOR_L
  #endif

  /* RCL Monitor */
  #ifdef SW_MONITOR_RCL
    #undef SW_MONITOR_RCL
  #endif

  /* RL Monitor */
  #ifdef SW_MONITOR_RL
    #undef SW_MONITOR_RL
  #endif

#endif


/* options which require ESR measurement */
#if ! defined (SW_ESR) && ! defined (SW_OLD_ESR)
  /* ESR tool */
  #ifdef SW_ESR_TOOL
    #undef SW_ESR_TOOL
  #endif
#endif


/* options which require a buzzer */
#if ! defined (HW_BUZZER)

  /* continuity check */
  #ifdef SW_CONTINUITY_CHECK
    #undef SW_CONTINUITY_CHECK
  #endif

  /* probing: confirmation beep when done */
  #ifdef UI_PROBING_DONE_BEEP
    #undef UI_PROBING_DONE_BEEP
  #endif

  /* IR receiver/decoder: confirmation beep for valid frame/packet */
  #ifdef SW_IR_RX_BEEP
    #undef SW_IR_RX_BEEP
  #endif

#endif


/* options which require a MCU clock >= 8MHz */
#if CPU_FREQ < 8000000

  /* ESR measurement */
  #ifdef SW_ESR
    #undef SW_ESR
  #endif

  /* old ESR measurement */
  #ifdef SW_OLD_ESR
    #undef SW_OLD_ESR
  #endif

#endif


/* SPI: either bit-bang or hardware */
#if defined (SPI_BITBANG) && defined (SPI_HARDWARE)
  #error <<< Select either bitbang or hardware SPI! >>>
#endif

/* SPI: common switch */
#if defined (SPI_BITBANG) || defined (SPI_HARDWARE)
  #define HW_SPI
#endif

/* 9-Bit SPI requires bit-bang mode */
#ifdef SPI_9
  #ifndef SPI_BITBANG
    #error <<< 9-Bit SPI requires bit-bang mode! >>>
  #endif
#endif

/* options which require SPI */
#ifndef HW_SPI
  /* SPI read support */
  #ifdef SPI_RW
    #undef SPI_RW
  #endif
#endif

/* options which require SPI read support */
#ifndef SPI_RW
  /* MAX6675 */
  #ifdef HW_MAX6675
    #undef HW_MAX6675
  #endif
  /* MAX31855 */
  #ifdef HW_MAX31855
    #undef HW_MAX31855
  #endif
#endif


/* I2C: either bit-bang or hardware */
#if defined (I2C_BITBANG) && defined (I2C_HARDWARE)
  #error <<< Select either bitbang or hardware I2C! >>>
#endif

/* I2C: common switch */
#if defined (I2C_BITBANG) || defined (I2C_HARDWARE)
  #define HW_I2C
#endif


/* TTL serial: either bit-bang or hardware */
#if defined (SERIAL_BITBANG) && defined (SERIAL_HARDWARE)
  #error <<< Select either bitbang or hardware serial interface! >>>
#endif

/* TTL serial: common switch */
#if defined (SERIAL_BITBANG) || defined (SERIAL_HARDWARE)
  #define HW_SERIAL
#endif

/* VT100 display driver disables other options for serial interface */
#ifdef LCD_VT100
  #ifdef UI_SERIAL_COPY
    #undef UI_SERIAL_COPY
  #endif
  #ifdef UI_SERIAL_COMMANDS
    #undef UI_SERIAL_COMMANDS
  #endif  
#endif

/* options which require TTL serial */
#ifndef HW_SERIAL
  /* VT100 display */
  #ifdef LCD_VT100
    #undef LCD_VT100
  #endif
  /* serial copy */
  #ifdef UI_SERIAL_COPY
    #undef UI_SERIAL_COPY
  #endif
  /* remote commands */
  #ifdef UI_SERIAL_COMMANDS
    #undef UI_SERIAL_COMMANDS
  #endif
#endif

/* options which require TTL serial RW */
#ifndef SERIAL_RW
  #ifdef UI_SERIAL_COMMANDS
    #undef UI_SERIAL_COMMANDS
  #endif
#endif


/* OneWire */
#if defined (ONEWIRE_PROBES) && defined (ONEWIRE_IO_PIN)
  #error <<< Select either probes or dedicated IO pin for Onewire! >>>
#endif

/* options which require OneWire */
#if ! defined (ONEWIRE_PROBES) && ! defined (ONEWIRE_IO_PIN)

  /* DS18B20 */
  #ifdef SW_DS18B20
    #undef SW_DS18B20
  #endif

  /* OneWire scan */
  #ifdef SW_ONEWIRE_SCAN
    #undef SW_ONEWIRE_SCAN
  #endif

#endif


/* touchscreen */
#ifdef TOUCH_PORT
  #define HW_TOUCH
#endif


/* LCD module */
#ifdef LCD_CONTRAST
  #define SW_CONTRAST
#else
  #define LCD_CONTRAST        0
#endif


/* options which require a color display */
#ifndef LCD_COLOR

  /* color coding for probes */
  #ifdef UI_PROBE_COLORS
    #undef UI_PROBE_COLORS
  #endif

  /* colored titles */
  #ifdef UI_COLORED_TITLES
    #undef UI_COLORED_TITLES
  #endif

  /* colored values */
  #ifdef UI_COLORED_VALUES
    #undef UI_COLORED_VALUES
  #endif

  /* colored cursor and key hints */
  #ifdef UI_COLORED_CURSOR
    #undef UI_COLORED_CURSOR
  #endif

#endif


/* options which require a color graphics display */
#if ! defined (LCD_COLOR) || ! defined (LCD_GRAPHIC)

  /* resistor color-codes */
  #ifdef SW_R_E24_5_CC
    #undef SW_R_E24_5_CC
  #endif
  #ifdef SW_R_E24_1_CC
    #undef SW_R_E24_1_CC
  #endif
  #ifdef SW_R_E96_CC
    #undef SW_R_E96_CC
  #endif

#endif


/* additional font characters with reversed colors */
#if defined (UI_PROBE_REVERSED) || defined (UI_PROBE_REVERSED_X)
  #ifndef FONT_EXTRA
    #define FONT_EXTRA
  #endif
  #ifndef FONT_INVERSE
    #define FONT_INVERSE
  #endif
#endif


/* component symbols for fancy pinout */
#if defined (SYMBOLS_24X24_H) || defined (SYMBOLS_24X24_OLD_H) || defined (SYMBOLS_24X24_ALT1_H) || defined (SYMBOLS_24X24_ALT2_H)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_24X24_HF) || defined (SYMBOLS_24X24_OLD_HF) || defined (SYMBOLS_24X24_ALT1_HF) || defined (SYMBOLS_24X24_ALT2_HF)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_24X24_VFP) || defined (SYMBOLS_24X24_OLD_VFP) || defined (SYMBOLS_24X24_ALT1_VFP) || defined (SYMBOLS_24X24_ALT2_VFP)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_24X24_VP_F) || defined (SYMBOLS_24X24_OLD_VP_F) || defined (SYMBOLS_24X24_ALT1_VP_F) || defined (SYMBOLS_24X24_ALT2_VP_F)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_30X32_HF) || defined (SYMBOLS_30X32_OLD_HF) || defined (SYMBOLS_30X32_ALT1_HF) || defined (SYMBOLS_30X32_ALT2_HF)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_32X32_HF) || defined (SYMBOLS_32X32_OLD_HF) || defined (SYMBOLS_32X32_ALT1_HF) || defined (SYMBOLS_32X32_ALT2_HF)
  #define SYMBOLS_SELECTED
#endif
#if defined (SYMBOLS_32X39_HF)
  #define SYMBOLS_SELECTED
#endif


/* fancy pinout requires graphic display and symbol set */
#ifdef SW_SYMBOLS

  /* graphic display */
  #ifndef LCD_GRAPHIC
    #undef SW_SYMBOLS
  #endif

  /* symbol set */
  #ifndef SYMBOLS_SELECTED
    #undef SW_SYMBOLS
  #endif

#endif


/* additional component symbols */
#if defined (SW_UJT) || defined (UI_QUESTION_MARK) || defined (UI_ZENER_DIODE) || defined (UI_QUARTZ_CRYSTAL)
  #ifndef SYMBOLS_EXTRA
    #define SYMBOLS_EXTRA
  #endif
#endif


/* options which require component symbols / fancy pinout */
#ifndef SW_SYMBOLS

  /* question mark symbol */
  #ifdef UI_QUESTION_MARK
    #undef UI_QUESTION_MARK
  #endif

  /* Zener diode symbol */
  #ifdef UI_ZENER_DIODE
    #undef UI_ZENER_DIODE
  #endif

  /* quartz crystal symbol */
  #ifdef UI_QUARTZ_CRYSTAL
    #undef UI_QUARTZ_CRYSTAL
  #endif

  /* disabling text pinout */
  #ifdef UI_NO_TEXTPINOUT
    #undef UI_NO_TEXTPINOUT
  #endif

  /* test-output of component symbols */
  #ifdef SW_SYMBOL_TEST
    #undef SW_SYMBOL_TEST
  #endif

#endif


/* PWM generators: can't have both variants */
#if defined (SW_PWM_SIMPLE) && defined (SW_PWM_PLUS)
  #error <<< Select either PWM generator with simple UI or fancy UI! >>>
#endif


/* frequency counter */
#if defined (HW_FREQ_COUNTER_BASIC) || defined (HW_FREQ_COUNTER_EXT)
  #define HW_FREQ_COUNTER
#endif


/* ring tester */
#if defined (HW_RING_TESTER)
  /* requires pulse output */
  #if ! defined (RING_TESTER_PIN) && ! defined (RING_TESTER_PROBES)
    #error <<< Select pulse output for ring tester! >>>
  #endif
  /* prefer dedicated pin */
  #if defined (RING_TESTER_PIN) && defined (RING_TESTER_PROBES)
    #undef RING_TESTER_PROBES
  #endif
#endif


/* IR detector/decoder: can't have probes and dedicated pin */
#if defined (SW_IR_RECEIVER) && defined (HW_IR_RECEIVER)
  #error <<< Select either probes or dedicated IO pin for IR detector! >>>
#endif


/* rounding for DS18B20 requires DS18B20 support */
#ifdef UI_ROUND_DS18B20
  #ifndef SW_DS18B20
    #undef UI_ROUND_DS18B20
  #endif
#endif


/* Zener check: can't have unswitched and switched mode */
#if defined (ZENER_UNSWITCHED) && defined (ZENER_SWITCHED)
  #error <<< Select either unswitched or switched mode for Zener check! >>>
#endif


/* Zener check, switched mode: boost converter drive methods */
#ifdef ZENER_SWITCHED

  /* can't have high and low active */
  #if defined (ZENER_BOOST_HIGH) && defined (ZENER_BOOST_LOW)
    #error <<< Select either high or low active for boost converter! >>>
  #endif

  /* must have one drive method */
  #if ! defined (ZENER_BOOST_HIGH) && ! defined (ZENER_BOOST_LOW)
    #error <<< Select drive method for boost converter! >>>
  #endif

#endif


/* Zener check during normal probing requires unswitched or switched mode */
#ifdef HW_PROBE_ZENER
  #if ! defined (ZENER_UNSWITCHED) && ! defined (ZENER_SWITCHED)
    #undef HW_PROBE_ZENER
  #endif
#endif


/* read functions for display require bus with read support enabled */
#ifdef LCD_READ
  #if defined (LCD_SPI) && ! defined (SPI_RW)
    #undef LCD_READ
  #endif
  #if defined (LCD_I2C) && ! defined (I2C_RW)
    #undef LCD_READ
  #endif
  /* can't check parallel busses */
#endif

/* display ID requires read functions for display */
#ifdef SW_DISPLAY_ID
  #ifndef LCD_READ
    #undef SW_DISPLAY_ID
  #endif
#endif

/* output of display registers requires read functions for display
   and serial output */
#ifdef SW_DISPLAY_REG
  #ifndef LCD_READ
    #undef SW_DISPLAY_REG
  #endif
  #ifndef UI_SERIAL_COPY
    #undef SW_DISPLAY_REG
  #endif
#endif



/* ************************************************************************
 *   simplify ifdefs
 * ************************************************************************ */


/* ProbePinout() */
#if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS) || defined (SW_SQUAREWAVE) || defined (SW_SERVO)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif

#if defined (SW_ESR_TOOL) || defined (SW_CONTINUITY_CHECK)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif

#if defined (SW_MONITOR_R) || defined (SW_MONITOR_C) || defined (SW_MONITOR_L) || defined(SW_MONITOR_RCL) || defined(SW_MONITOR_RL)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif

#if defined (HW_RING_TESTER) && defined (RING_TESTER_PROBES)
  #ifndef FUNC_PROBE_PINOUT
    #define FUNC_PROBE_PINOUT
  #endif
#endif


/* E6 norm values */
#if defined (SW_C_E6_T) || defined (SW_L_E6_T)
  #define SW_E6
#endif

/* E12 norm values */
#if defined (SW_C_E12_T) || defined (SW_L_E12_T)
  #define SW_E12
#endif

/* E24 norm values */
#if defined (SW_R_E24_5_T) || defined (SW_R_E24_5_CC) || defined (SW_R_E24_1_T) || defined (SW_R_E24_1_CC)
  #define SW_E24
#endif

/* E96 norm values */
#if defined (SW_R_E96_T) || defined (SW_R_E96_CC) || defined (SW_R_E96_EIA96)
  #define SW_E96
#endif


/* Show_ENormValues(), Display_EValue() */
#if defined (SW_R_E24_5_T) || defined (SW_R_E24_1_T) || defined (SW_R_E96_T)
  #ifndef FUNC_EVALUE
    #define FUNC_EVALUE
  #endif
  #ifndef SW_R_EXX
    #define SW_R_EXX
  #endif
#endif

#if defined (SW_C_E6_T) || defined (SW_C_E12_T) || defined (SW_L_E6_T) || defined (SW_L_E12_T)
  #ifndef FUNC_EVALUE
    #define FUNC_EVALUE
  #endif
#endif


/* Show_ENormCodes(), Display_ColorCode() */
#if defined (SW_R_E24_5_CC) || defined (SW_R_E24_1_CC) || defined (SW_R_E96_CC)
  #ifndef FUNC_COLORCODE
    #define FUNC_COLORCODE
  #endif
  #ifndef SW_R_EXX
    #define SW_R_EXX
  #endif
#endif


/* Show_ENormEIA96(), Display_EIA96() */
#if defined (SW_R_E96_EIA96)
  #ifndef FUNC_EIA96
    #define FUNC_EIA96
  #endif
  #ifndef SW_R_EXX
    #define SW_R_EXX
  #endif
#endif


/* SmoothLongKeyPress() */
#if defined (SW_PWM_PLUS) || defined (SW_SERVO) || defined (HW_EVENT_COUNTER) || defined (HW_LC_METER)
  #ifndef FUNC_SMOOTHLONGKEYPRESS
    #define FUNC_SMOOTHLONGKEYPRESS
  #endif
#endif


/* Display_FullValue() */
#if defined (SW_SQUAREWAVE) || defined (SW_PWM_PLUS) || defined (HW_FREQ_COUNTER_EXT) || defined (SW_SERVO)
  #ifndef FUNC_DISPLAY_FULLVALUE
    #define FUNC_DISPLAY_FULLVALUE
  #endif
#endif

#if defined (HW_EVENT_COUNTER) || defined (SW_DHTXX) || defined (LC_METER_SHOW_FREQ) || defined (HW_MAX6675)
  #ifndef FUNC_DISPLAY_FULLVALUE
    #define FUNC_DISPLAY_FULLVALUE
  #endif
#endif

#if defined (FUNC_EVALUE) || defined (FUNC_COLORCODE) || defined (FUNC_EIA96)
  #ifndef FUNC_DISPLAY_FULLVALUE
    #define FUNC_DISPLAY_FULLVALUE
  #endif
#endif


/* Display_SignedFullValue() */
#if defined (SW_DS18B20) || defined (SW_DHTXX) || defined (HW_MAX31855)
  #ifndef FUNC_DISPLAY_SIGNEDFULLVALUE
    #define FUNC_DISPLAY_SIGNEDFULLVALUE
  #endif
#endif


/* Display_HexByte() */
#if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER) || defined (ONEWIRE_READ_ROM) || defined (SW_ONEWIRE_SCAN)
  #ifndef FUNC_DISPLAY_HEXBYTE
    #define FUNC_DISPLAY_HEXBYTE
  #endif
#endif

#if defined (SW_FONT_TEST) || defined (SW_SYMBOL_TEST) || defined (SW_DISPLAY_REG)
  #ifndef FUNC_DISPLAY_HEXBYTE
    #define FUNC_DISPLAY_HEXBYTE
  #endif
#endif


/* Display_HexValue() */
#if defined (SW_IR_TRANSMITTER) || defined (SW_DISPLAY_ID)
  #ifndef FUNC_DISPLAY_HEXVALUE
    #define FUNC_DISPLAY_HEXVALUE
  #endif
#endif


/* Celsius2Fahrenheit() */
#ifdef UI_FAHRENHEIT
  #if defined (SW_DS18B20) || defined (SW_DHTXX) || defined (HW_MAX6675) || defined (HW_MAX31855)
    #ifndef FUNC_CELSIUS2FAHRENHEIT
      #define FUNC_CELSIUS2FAHRENHEIT
    #endif
  #endif
#endif


/* variable Start_str */
#if defined (SW_OPTO_COUPLER) || defined (HW_EVENT_COUNTER) || defined (SW_DS18B20) || defined (SW_ONEWIRE_SCAN)
  #ifndef VAR_START_STR
    #define VAR_START_STR
  #endif
#endif

#if defined (SW_DHTXX) || defined (HW_MAX6675) || defined (HW_MAX31855)
  #ifndef VAR_START_STR
    #define VAR_START_STR
  #endif
#endif


/* Display_ColoredEEString_Center() */
#if defined (TOUCH_ADS7843) && defined (UI_COLORED_TITLES) && defined (UI_CENTER_ALIGN)
  #ifndef FUNC_DISPLAY_COLOREDEESTRING_CENTER
    #define FUNC_DISPLAY_COLOREDEESTRING_CENTER
  #endif
#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   ATmega 324/644/1284 specific global configuration, setup and settings
 *
 *   (c) 2012-2017 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* source management */
#define CONFIG_644_H



/* ************************************************************************
 *   LCD module
 * ************************************************************************ */


/*
 *  LCD module / controller
 *
 *  Please uncomment the package matching your LCD module
 *  and adjust settings.
 *
 *  To uncomment, remove the enclosing "#if 0" and "#endif" or
 *  put a "//" in front of both.
 */


/*
 *  HD44780, 4 bit parallel
 *  - if you change LCD_DB4/5/6/7 comment out LCD_DB_STD!
 */

#if 0
#define LCD_HD44780
#define LCD_TEXT                        /* character display */
#define LCD_PAR_4                       /* 4 bit parallel interface */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_DB_STD                      /* use standard pins 0-3 for DB4-7 */
#define LCD_DB4          PB4            /* port pin used for DB4 */
#define LCD_DB5          PB5            /* port pin used for DB5 */
#define LCD_DB6          PB6            /* port pin used for DB6 */
#define LCD_DB7          PB7            /* port pin used for DB7 */
#define LCD_RS           PB2            /* port pin used for RS */
#define LCD_EN1          PB3            /* port pin used for E */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       2              /* number of lines */
#define FONT_HD44780_INT                /* internal 5x7 font, international */
#endif



/*
 *  HD44780, PCF8574 based I2C backpack
 *  - if you change LCD_DB4/5/6/7 comment out LCD_DB_STD!
 *  - don't forget to enable I2C in config.h
 *  - for I2C port and pins see section "bit-bang I2C"
 *  - PCF8574T is 0x27, PCF8574AT is 0x3f
 */

#if 0
#define LCD_HD44780
#define LCD_TEXT                        /* character display */
#define LCD_PCF8574                     /* PCF8574 backpack */
#define LCD_I2C_ADDR     0x27           /* PCF8574's I2C address */
#define LCD_DB_STD                      /* use standard pins 4-7 for DB4-7 */
#define LCD_DB4          PCF8574_P4     /* port pin used for DB4 */
#define LCD_DB5          PCF8574_P5     /* port pin used for DB5 */
#define LCD_DB6          PCF8574_P6     /* port pin used for DB6 */
#define LCD_DB7          PCF8574_P7     /* port pin used for DB7 */
#define LCD_RS           PCF8574_P0     /* port pin used for RS */
#define LCD_RW           PCF8574_P1     /* port pin used for RW */
#define LCD_EN1          PCF8574_P2     /* port pin used for E */
#define LCD_LED          PCF8574_P3     /* port pin used for backlight */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       2              /* number of lines */
#define FONT_HD44780_INT                /* internal 5x7 font, international */
#endif



/*
 *  ST7565R, SPI interface
 *  - settings for Electronic Assembly EA DOGM/DOGL128-6
 */

#if 0
#define LCD_ST7565R
#define LCD_GRAPHIC                     /* monochrome graphic display */
//#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_SPI_HARDWARE                /* hardware SPI interface */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB2            /* port pin used for /RES */
#define LCD_A0           PB3            /* port pin used for A0 */
#define LCD_SCL          PB7            /* port pin used for SCL */
#define LCD_SI           PB5            /* port pin used for SI (LCD's data input) */
#define LCD_CS           PB4            /* port pin used for /CS1 (optional) */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_OFFSET_X                    /* enable x offset of 4 dots */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_START_Y      0              /* start line (0-63) */
#define LCD_CONTRAST     22             /* default contrast (0-63) */
#define FONT_8X8_V                      /* 8x8 font, vertically aligned */
#define SYMBOLS_24X24_VP                /* 24x24 symbols, vertically aligned */
#endif



/*
 *  ILI9342, SPI interface
 */

//#if 0
#define LCD_ILI9341
#define LCD_COLOR                       /* color graphic display */
//#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_SPI_HARDWARE                /* hardware SPI interface */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB2            /* port pin used for /RES */
#define LCD_CS           PB4            /* port pin used for /CS */
#define LCD_DC           PB3            /* port pin used for D/C */
#define LCD_SCK          PB7            /* port pin used for SCK */
#define LCD_SDI          PB5            /* port pin used for SDI (LCD's data input) */
#define LCD_SDO          PB6            /* port pin used for SDO (LCD's data output) */
#define LCD_DOTS_X       320            /* number of horizontal dots */
#define LCD_DOTS_Y       240            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
//#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
#define FONT_16X26_H                    /* 16x26 font, horizontally aligned */
#define SYMBOLS_32X32_H                 /* 32x32 symbols, horizontally aligned */
//#endif



/*
 *  ST7735, SPI interface
 */

#if 0
#define LCD_ST7735
#define LCD_COLOR                       /* color graphic display */
//#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_SPI_HARDWARE                /* hardware SPI interface */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB2            /* port pin used for /RESX */
#define LCD_CS           PB4            /* port pin used for /CSX (optional) */
#define LCD_DC           PB3            /* port pin used for D/CX */
#define LCD_SCL          PB7            /* port pin used for SCL */
#define LCD_SDA          PB5            /* port pin used for SDA */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       160            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
#define FONT_10X16_H                    /* 10x16 font, horizontally aligned */
#define SYMBOLS_30X32_H                 /* 30x32 symbols, horizontally aligned */
#endif



/*
 *  PCD8544, SPI interface (bit-bang)
 */

#if 0
#define LCD_PCD8544
#define LCD_GRAPHIC                     /* monochrome graphic display */
//#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_SPI_HARDWARE                /* hardware SPI interface */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB2            /* port pin used for /RES */
#define LCD_SCE          PB4            /* port pin used for /SCE (optional) */
#define LCD_DC           PB3            /* port pin used for D/C */
#define LCD_SCLK         PB7            /* port pin used for SCLK */
#define LCD_SDIN         PB5            /* port pin used for SDIN (LCD's data input) */
#define LCD_DOTS_X       84             /* number of horizontal dots */
#define LCD_DOTS_Y       48             /* number of vertical dots */
#define LCD_CONTRAST     66             /* default contrast (1-127) */
#define FONT_6X8_V                      /* 6x8 font, vertically aligned */
#endif



/*
 *  check if a LCD module is specified
 */

#if !defined(LCD_TEXT) && !defined(LCD_GRAPHIC) && !defined(LCD_COLOR)
  #error <<< No LCD module specified! >>>
#endif



/* ************************************************************************
 *   touchscreen (optional)
 * ************************************************************************ */


/*
 *  touchscreen / controller
 *
 *  Please uncomment the package matching your touchscreen
 *  and adjust settings.
 *
 *  To uncomment, remove the enclosing "#if 0" and "#endif" or
 *  put a "//" in front of both.
 */


/*
 *  ADS7843 / XPT2046 (SPI interface)
 *  - not supported yet
 */

#if 0
#define TOUCH_ADS7843
#define TOUCH_PORT       PORTB     /* port data register */
#define TOUCH_DDR        DDRB      /* port data direction register */
#define TOUCH_CS         PBx       /* port pin used for /CS */
#define TOUCH_D_CLK      PB7       /* port pin used for DCLK */
#define TOUCH_D_OUT      PB6       /* port pin used for DOUT */
#define TOUCH_D_IN       PB5       /* port pin used for DIN */
#define TOUCH_PEN        PBx       /* port pin used for /PENIRQ */
#endif




/* ************************************************************************
 *   port and pin assignments
 * ************************************************************************ */


/*
 *  Test probes:
 *  - Must be an ADC port :-)
 *  - Lower 3 pins of the port must be used for probe pins.
 *  - Please don't change the definitions of TP1, TP2 and TP3!
 */

#define ADC_PORT         PORTA     /* ADC port data register */
#define ADC_DDR          DDRA      /* ADC port data direction register */
#define ADC_PIN          PINA      /* port input pins register */
#define TP1              PA0       /* test pin 1 */
#define TP2              PA1       /* test pin 2 */
#define TP3              PA2       /* test pin 3 */

#define TP_ZENER         PA3       /* test pin with 10:1 voltage divider */
#define TP_REF           PA4       /* test pin with 2.5V reference and relay */
#define TP_BAT           PA5       /* test pin with 4:1 voltage divider */
#define TP_CAP           PA7       /* test pin for self-adjustment cap */


/*
 *  Probe resistors
 *  - For PWM/squarewave output R_RL_2 has to be PD4/OC1B.
 */

#define R_PORT           PORTD     /* port data register */
#define R_DDR            DDRD      /* port data direction register */
#define R_RL_1           PD2       /* Rl (680R) for test pin #1 */
#define R_RH_1           PD3       /* Rh (470k) for test pin #1 */
#define R_RL_2           PD4       /* Rl (680R) for test pin #2 */
#define R_RH_2           PD5       /* Rh (470k) for test pin #2 */
#define R_RL_3           PD6       /* Rl (680R) for test pin #3 */
#define R_RH_3           PD7       /* Rh (470k) for test pin #3 */


/*
 *  push button and power management
 */

#define CONTROL_PORT     PORTC     /* port data register */
#define CONTROL_DDR      DDRC      /* port data direction register */
#define CONTROL_PIN      PINC      /* port input pins register */
#define POWER_CTRL       PC6       /* controls power (1: on / 0: off) */
#define TEST_BUTTON      PC7       /* test/start push button (low active) */


/*
 *  rotary encoder
 */

#define ENCODER_PORT     PORTC     /* port data register */
#define ENCODER_DDR      DDRC      /* port data direction register */
#define ENCODER_PIN      PINC      /* port input pins register */
#define ENCODER_A        PC4       /* rotary encoder A signal */
#define ENCODER_B        PC3       /* rotary encoder B signal */


/*
 *  frequency counter
 *  - must be pin PB0/T0
 */

#define COUNTER_PORT     PORTB     /* port data register */ 
#define COUNTER_DDR      DDRB      /* port data direction register */
#define COUNTER_IN       PB0       /* signal input T0 */


/*
 *  IR detector/decoder
 *  - fixed module
 */

#define IR_PORT          PORTC     /* port data register */
#define IR_DDR           DDRC      /* port data direction register */
#define IR_PIN           PINC      /* port input pins register */
#define IR_DATA          PC2       /* data signal */


/*
 *  bit-bang I2C
 *  - hardware I2C (TWI) uses PC1 & PC0 automatically
 */

#define I2C_PORT         PORTC     /* port data register */
#define I2C_DDR          DDRC      /* port data direction register */
#define I2C_PIN          PINC      /* port input pins register */
#define I2C_SDA          PC1       /* pin for SDA */
#define I2C_SCL          PC0       /* pin for SCL */


/*
 *  fixed cap for self-adjustment
 *  - ADC pin is TP_CAP from above
 *  - settings are for 470k resistor
 *  - should be film cap with 100nF - 1000nF
 */

#define ADJUST_PORT      PORTC     /* port data register */
#define ADJUST_DDR       DDRC      /* port data direction register */
#define ADJUST_RH        PC5       /* Rh (470k) for fixed cap */


/*
 *  relay for parallel cap (sampling ADC)
 *  - TP1 & TP3
 *  - cap should have 10nF - 27nF
 */

#define CAP_PORT         PORTC     /* port data register */
#define CAP_DDR          DDRC      /* port data direction register */
#define CAP_RELAY        PC2       /* control pin */



/* ************************************************************************
 *   internal stuff
 * ************************************************************************ */


/* ADC reference selection: AVcc */
#define ADC_REF_VCC           (1 << REFS0)

/* ADC reference selection: internal 1.1V bandgap */
#define ADC_REF_BANDGAP       (1 << REFS1)

/* ADC reference selection: internal 2.56V (bandgap * 2.328) */
#define ADC_REF_256           ((1 << REFS1) | (1 << REFS0))

/* ADC reference selection bit mask */
#define ADC_REF_MASK          ((1 << REFS1) | (1 << REFS0))

/* ADC MUX channel for internal 1.1V bandgap reference */
#define ADC_BANDGAP      0x1e      /* 11110 */



/* ************************************************************************
 *   MCU specific setup to support different AVRs
 * ************************************************************************ */


/*
 *  ATmega 324P/324PA
 */

#if defined(__AVR_ATmega324P__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* capacitance of the probe tracks of the PCB and the MCU (in pF) */
  #define CAP_PCB             32

  /* this MCU has 32kB Flash, 1kB EEPROM and 2kB RAM (enable extra features) */
  #define RES_FLASH           32
  #define RES_EEPROM          1
  #define RES_RAM             2


/*
 *  ATmega 644/644P/644PA
 */
||
#elif defined(__AVR_ATmega644__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* capacitance of the probe tracks of the PCB and the MCU (in pF) */
  #define CAP_PCB             32

  /* this MCU has 64kB Flash, 2kB EEPROM and 4kB RAM (enable extra features) */
  #define RES_FLASH           64
  #define RES_EEPROM          2
  #define RES_RAM             4


/*
 *  ATmega 1284/1284P
 */

#elif defined(__AVR_ATmega1284__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* capacitance of the probe tracks of the PCB and the MCU (in pF) */
  #define CAP_PCB             32

  /* this MCU has 128kB Flash, 4kB EEPROM and 16kB RAM (enable extra features) */
  #define RES_FLASH           128
  #define RES_EEPROM          4
  #define RES_RAM             16


/*
 *  missing or unsupported MCU
 */

#else
  #error <<< No or wrong MCU type selected! >>>
#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

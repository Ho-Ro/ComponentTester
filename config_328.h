/* ************************************************************************
 *
 *   ATmega 328 specific global configuration, setup and settings
 *
 *   (c) 2012-2017 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* source management */
#define CONFIG_328_H



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
#define LCD_PORT         PORTD          /* port data register */
#define LCD_DDR          DDRD           /* port data direction register */
#define LCD_DB_STD                      /* use standard pins 0-3 for DB4-7 */
#define LCD_DB4          PD0            /* port pin used for DB4 */
#define LCD_DB5          PD1            /* port pin used for DB5 */
#define LCD_DB6          PD2            /* port pin used for DB6 */
#define LCD_DB7          PD3            /* port pin used for DB7 */
#define LCD_RS           PD4            /* port pin used for RS */
#define LCD_EN1          PD5            /* port pin used for E */
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
#define LCD_I2C_ADDR     0x3f           /* PCF8574's I2C address */
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
 *  ST7565R, SPI interface (bit-bang)
 *  - settings for Electronic Assembly EA DOGM/DOGL128-6
 *  - uses LCD_CS to support rotary encoder in parallel at PD2/3
 */

//#if 0
#define LCD_ST7565R
#define LCD_GRAPHIC                     /* monochrome graphic display */
#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_PORT         PORTD          /* port data register */
#define LCD_DDR          DDRD           /* port data direction register */
#define LCD_RESET        PD0            /* port pin used for /RES */
#define LCD_A0           PD1            /* port pin used for A0 */
#define LCD_SCL          PD2            /* port pin used for SCL */
#define LCD_SI           PD3            /* port pin used for SI (LCD's data input) */
#define LCD_CS           PD5            /* port pin used for /CS1 (optional) */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_OFFSET_X                    /* enable x offset of 4 dots */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_START_Y      0              /* start line (0-63) */
#define LCD_CONTRAST     22             /* default contrast (0-63) */
#define FONT_8X8_V                      /* 8x8 font, vertically aligned */
#define SYMBOLS_24X24_VP                /* 24x24 symbols, vertically aligned */
//#endif



/*
 *  ILI9342, SPI interface (bit-bang)
 */

#if 0
#define LCD_ILI9341
#define LCD_COLOR                       /* color graphic display */
#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_PORT         PORTD          /* port data register */
#define LCD_DDR          DDRD           /* port data direction register */
#define LCD_RES          PD4            /* port pin used for /RES */
#define LCD_CS           PD5            /* port pin used for /CS */
#define LCD_DC           PD3            /* port pin used for D/C */
#define LCD_SCK          PD2            /* port pin used for SCK */
#define LCD_SDI          PD1            /* port pin used for SDI (LCD's data input) */
#define LCD_SDO          PD0            /* port pin used for SDO (LCD's data output) */
#define LCD_DOTS_X       320            /* number of horizontal dots */
#define LCD_DOTS_Y       240            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
//#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
#define FONT_16X26_H                    /* 16x26 font, horizontally aligned */
#define SYMBOLS_32X32_H                 /* 32x32 symbols, horizontally aligned */
#endif



/*
 *  ST7735, SPI interface (bit-bang)
 */

#if 0
#define LCD_ST7735
#define LCD_COLOR                       /* color graphic display */
#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_PORT         PORTD          /* port data register */
#define LCD_DDR          DDRD           /* port data direction register */
#define LCD_RES          PD4            /* port pin used for /RESX */
#define LCD_CS           PD5            /* port pin used for /CSX (optional) */
#define LCD_DC           PD3            /* port pin used for D/CX */
#define LCD_SCL          PD2            /* port pin used for SCL */
#define LCD_SDA          PD1            /* port pin used for SDA */
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
#define LCD_SPI_BITBANG                 /* bit-bang SPI interface */
#define LCD_PORT         PORTD          /* port data register */
#define LCD_DDR          DDRD           /* port data direction register */
#define LCD_RES          PD4            /* port pin used for /RES */
#define LCD_SCE          PD5            /* port pin used for /SCE (optional) */
#define LCD_DC           PD3            /* port pin used for D/C */
#define LCD_SCLK         PD2            /* port pin used for SCLK */
#define LCD_SDIN         PD1            /* port pin used for SDIN (LCD's data input) */
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
#define TP1              PC0       /* test pin 1 */
#define TP2              PC1       /* test pin 2 */
#define TP3              PC2       /* test pin 3 */

#define TP_ZENER         PC3       /* test pin with 10:1 voltage divider */
#define TP_REF           PC4       /* test pin with 2.5V reference and relay */
#define TP_BAT           PC5       /* test pin with 4:1 voltage divider */


/*
 *  Probe resistors
 *  - For PWM/squarewave output R_RL_2 has to be PB2/OC1B.
 */

#define R_PORT           PORTB     /* port data register */
#define R_DDR            DDRB      /* port data direction register */
#define R_RL_1           PB0       /* Rl (680R) for test pin #1 */
#define R_RH_1           PB1       /* Rh (470k) for test pin #1 */
#define R_RL_2           PB2       /* Rl (680R) for test pin #2 */
#define R_RH_2           PB3       /* Rh (470k) for test pin #2 */
#define R_RL_3           PB4       /* Rl (680R) for test pin #3 */
#define R_RH_3           PB5       /* Rh (470k) for test pin #3 */


/*
 *  push button and power management
 */

#define CONTROL_PORT     PORTD     /* port data register */
#define CONTROL_DDR      DDRD      /* port data direction register */
#define CONTROL_PIN      PIND      /* port input pins register */
#define POWER_CTRL       PD6       /* controls power (1: on / 0: off) */
#define TEST_BUTTON      PD7       /* test/start push button (low active) */


/*
 *  rotary encoder
 */

#define ENCODER_PORT     PORTD     /* port data register */
#define ENCODER_DDR      DDRD      /* port data direction register */
#define ENCODER_PIN      PIND      /* port input pins register */
#define ENCODER_A        PD2       /* rotary encoder A signal */
#define ENCODER_B        PD3       /* rotary encoder B signal */


/*
 *  frequency counter
 *  - must be pin PD4/T0
 */

#define COUNTER_PORT     PORTD     /* port data register */
#define COUNTER_DDR      DDRD      /* port data direction register */
#define COUNTER_IN       PD4       /* signal input T0 */


/*
 *  IR detector/decoder
 *  - fixed module
 */

#define IR_PORT          PORTC     /* port data register */
#define IR_DDR           DDRC      /* port data direction register */
#define IR_PIN           PINC      /* port input pins register */
#define IR_DATA          PC6       /* data signal */


/*
 *  bit-bang I2C
 *  - hardware I2C (TWI) uses PC4 & PC5 automatically
 */

#define I2C_PORT         PORTD     /* port data register */
#define I2C_DDR          DDRD      /* port data direction register */
#define I2C_PIN          PIND      /* port input pins register */
#define I2C_SDA          PD0       /* pin for SDA */
#define I2C_SCL          PD1       /* pin for SCL */



/* ************************************************************************
 *   internal stuff
 * ************************************************************************ */


/* ADC reference selection: AVcc */
#define ADC_REF_VCC           (1 << REFS0)

/* ADC reference selection: internal 1.1V bandgap */
#define ADC_REF_BANDGAP       ((1 << REFS1) | (1 << REFS0))

/* ADC reference selection bit mask */
#define ADC_REF_MASK          ((1 << REFS1) | (1 << REFS0))

/* ADC MUX channel for internal 1.1V bandgap reference */
#define ADC_BANDGAP      0x0e      /* 1110 */



/* ************************************************************************
 *   MCU specific setup to support different AVRs
 * ************************************************************************ */


/*
 *  ATmega 328/328P
 */

#if defined(__AVR_ATmega328__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* capacitance of the probe tracks of the PCB and the MCU (in pF) */
  #define CAP_PCB             32

  /* this MCU has 32kB Flash, 1kB EEPROM and 2kB RAM (enable extra features) */
  #define RES_FLASH           32
  #define RES_EEPROM          1
  #define RES_RAM             2


/*
 *  missing or unsupported MCU
 */

#else
  #error <<< No or wrong MCU type selected! >>>
#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

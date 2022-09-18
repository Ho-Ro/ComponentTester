/* ************************************************************************
 *
 *   ATmega 640/1280/2560 specific global configuration, setup and settings
 *
 *   (c) 2012-2021 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* source management */
#define CONFIG_1280_H



/* ************************************************************************
 *   display module
 * ************************************************************************ */


/*
 *  display module / controller
 *
 *  Please uncomment the package matching your LCD/OLED module and adjust
 *  settings. And comment out the default package if not used.
 *
 *  To uncomment, remove the enclosing "#if 0" and "#endif" or put
 *  a "//" in front of both. To comment out, remove the "//" in front
 *  of the "#if 0" and "#endif".
 *
 *  Individual settings can be enabled by removing the leading "//", or
 *  disabled by placing a "//" in front of the setting.
 */


/*
 *  HD44780
 *  - 4 bit parallel interface
 *  - enable LCD_DB_STD when using port pins 0-3 for LCD_DB4/5/6/7
 */

#if 0
#define LCD_HD44780                     /* display controller HD44780 */
#define LCD_TEXT                        /* character display */
#define LCD_PAR_4                       /* 4 bit parallel interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_DB_STD                      /* use standard pins 0-3 for DB4-7 */
#define LCD_DB4          PB0            /* port pin used for DB4 */
#define LCD_DB5          PB1            /* port pin used for DB5 */
#define LCD_DB6          PB2            /* port pin used for DB6 */
#define LCD_DB7          PB3            /* port pin used for DB7 */
#define LCD_RS           PB4            /* port pin used for RS */
#define LCD_EN1          PB5            /* port pin used for E */
/* display settings */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       2              /* number of lines */
/* HD44780 has an internal 5x7 font */
#define FONT_HD44780_INT                /* International font (English) */
//#define FONT_HD44780_WIN1251            /* Cyrillic font (European) */
#endif



/*
 *  HD44780 with PCF8574 based backpack
 *  - I2C interface using hardware I2C
 *  - if you change LCD_DB4/5/6/7 comment out LCD_DB_STD!
 *  - hardware I2C automatically selects SDA and SCL pins
 *  - PCF8574T is 0x27, PCF8574AT is 0x3f
 */

#if 0
#define LCD_HD44780                     /* display controller HD44780 */
#define LCD_TEXT                        /* character display */
#define LCD_PCF8574                     /* PCF8574 backpack */
#define LCD_I2C_ADDR     0x27           /* PCF8574's I2C address */
/* control and data lines */
#define LCD_DB_STD                      /* use standard pins 4-7 for DB4-7 */
#define LCD_DB4          PCF8574_P4     /* port pin used for DB4 */
#define LCD_DB5          PCF8574_P5     /* port pin used for DB5 */
#define LCD_DB6          PCF8574_P6     /* port pin used for DB6 */
#define LCD_DB7          PCF8574_P7     /* port pin used for DB7 */
#define LCD_RS           PCF8574_P0     /* port pin used for RS */
#define LCD_RW           PCF8574_P1     /* port pin used for RW */
#define LCD_EN1          PCF8574_P2     /* port pin used for E */
#define LCD_LED          PCF8574_P3     /* port pin used for backlight */
/* display settings */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       2              /* number of lines */
//#define LCD_BACKLIGHT_LOW               /* backlight is low active */
/* HD44780 has an internal 5x7 font */
#define FONT_HD44780_INT                /* International font (English) */
//#define FONT_HD44780_WIN1251            /* Cyrillic font (European) */
/* I2C bus */
#define I2C_HARDWARE                    /* hardware I2C (MCU's TWI) */
#define I2C_STANDARD_MODE               /* 100kHz bus speed */
#endif



/*
 *  ILI9163
 *  - 4 wire SPI interface using hardware SPI
 */

#if 0
#define LCD_ILI9163                     /* display controller ILI9163 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB4            /* port pin used for /RESX (optional) */
#define LCD_CS           PB5            /* port pin used for /CSX (optional) */
#define LCD_DC           PB7            /* port pin used for D/CX */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SDA          PB2            /* port pin used for SDA/SDIO */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       128            /* number of vertical dots */
#define LCD_OFFSET_X      32            /* x offset of 32 dots (160-128) */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
#define LCD_BGR                         /* reverse red and blue color channels */
//#define LCD_LATE_ON                     /* turn on LCD after clearing it */
/* font and symbols: horizonally aligned & flipped */
#define FONT_8X8_HF                     /* 8x8 font */
//#define FONT_10X16_HF                   /* 10x16 font */
//#define FONT_8X8_ISO8859_2_HF           /* 8x8 Central European font */
//#define FONT_10X16_ISO8859_2_HF         /* 10x16 Central European font */
//#define FONT_8X16_WIN1251_HF            /* 8x16 cyrillic font */
//#define FONT_8X16ALT_WIN1251_HF         /* 8x16 alternative cyrillic font */
#define SYMBOLS_30X32_HF                /* 30x32 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  ILI9342
 *  - SPI interface using hardware SPI
 */

#if 0
#define LCD_ILI9341                     /* display controller ILI9341/ILI9342 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_DC           PB7            /* port pin used for D/C */
#define LCD_SCK          PB1            /* port pin used for SCK */
#define LCD_SDI          PB2            /* port pin used for SDI (data input) */
//#define LCD_SDO          PB3            /* port pin used for SDO (data output) */
/* display settings */
#define LCD_DOTS_X       320            /* number of horizontal dots */
#define LCD_DOTS_Y       240            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
//#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
//#define LCD_BGR                         /* reverse red and blue color channels */
//#define LCD_EXT_CMD_OFF                 /* extended commands disabled */
/* font and symbols: horizontally aligned & flipped */
#define FONT_16X26_HF                   /* 16x26 font */
//#define FONT_16X26_ISO8859_2_HF         /* 16x26 Central European font */
//#define FONT_16X26_WIN1251_HF           /* 16x26 cyrillic font */
#define SYMBOLS_32X32_HF                /* 32x32 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  ILI9341 with extended command set disabled (EXTC pin low)
 *  - 8 bit parallel interface
 *  - LCD_DB0 to LCD_DB7 have to match port pins 0 to 7
 */

#if 0
#define LCD_ILI9341                     /* display controller ILI9341/ILI9342 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_PAR_8                       /* 8 bit parallel interface */
/* control lines */
#define LCD_PORT         PORTC          /* port data register */
#define LCD_DDR          DDRC           /* port data direction register */
#define LCD_RES          PC0            /* port pin used for /RESX (optional) */
#define LCD_CS           PC1            /* port pin used for /CSX (optional) */
#define LCD_DC           PC2            /* port pin used for D/CX */
#define LCD_WR           PC3            /* port pin used for WRX */
#define LCD_RD           PC4            /* port pin used for RDX (optional) */
/* data lines DB0-7 */
#define LCD_PORT2        PORTL          /* port data register */
#define LCD_DDR2         DDRL           /* port data direction register */
#define LCD_PIN2         PINL           /* port data direction register */
#define LCD_DB0          PL0            /* port pin used for D0 */
#define LCD_DB1          PL1            /* port pin used for D1 */
#define LCD_DB2          PL2            /* port pin used for D2 */
#define LCD_DB3          PL3            /* port pin used for D3 */
#define LCD_DB4          PL4            /* port pin used for D4 */
#define LCD_DB5          PL5            /* port pin used for D5 */
#define LCD_DB6          PL6            /* port pin used for D6 */
#define LCD_DB7          PL7            /* port pin used for D7 */
/* display settings */
#define LCD_DOTS_X       240            /* number of horizontal dots */
#define LCD_DOTS_Y       320            /* number of vertical dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
//#define LCD_BGR                         /* reverse red and blue color channels */
#define LCD_EXT_CMD_OFF                 /* extended commands disabled */
/* font and symbols: horizontally aligned & flipped */
#define FONT_16X26_HF                   /* 16x26 font */
//#define FONT_16X26_ISO8859_2_HF         /* 16x26 Central European font */
//#define FONT_16X26_WIN1251_HF           /* 16x26 cyrillic font */
#define SYMBOLS_32X32_HF                /* 32x32 symbols */
#endif



/*
 *  ILI9481, ILI9486 or ILI9488
 *  - 8 bit parallel interface
 *  - LCD_DB0 to LCD_DB7 have to match port pins 0 to 7
 *  - ILI9488 untested
 */

//#if 0
//#define LCD_ILI9481                     /* display controller ILI9481 */
#define LCD_ILI9486                     /* display controller ILI9486 */
//#define LCD_ILI9488                     /* display controller ILI9488 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_PAR_8                       /* 8 bit parallel interface */
/* control lines */
#define LCD_PORT         PORTC          /* port data register */
#define LCD_DDR          DDRC           /* port data direction register */
#define LCD_RES          PC0            /* port pin used for /RESX (optional) */
#define LCD_CS           PC1            /* port pin used for /CSX (optional) */
#define LCD_DC           PC2            /* port pin used for D/CX */
#define LCD_WR           PC3            /* port pin used for WRX */
#define LCD_RD           PC4            /* port pin used for RDX (optional) */
/* data lines DB0-7 */
#define LCD_PORT2        PORTL          /* port data register */
#define LCD_DDR2         DDRL           /* port data direction register */
#define LCD_PIN2         PINL           /* port data direction register */
#define LCD_DB0          PL0            /* port pin used for DB0 */
#define LCD_DB1          PL1            /* port pin used for DB1 */
#define LCD_DB2          PL2            /* port pin used for DB2 */
#define LCD_DB3          PL3            /* port pin used for DB3 */
#define LCD_DB4          PL4            /* port pin used for DB4 */
#define LCD_DB5          PL5            /* port pin used for DB5 */
#define LCD_DB6          PL6            /* port pin used for DB6 */
#define LCD_DB7          PL7            /* port pin used for DB7 */
/* display settings */
#define LCD_DOTS_X       320            /* number of horizontal dots */
#define LCD_DOTS_Y       480            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
#define LCD_BGR                         /* reverse red and blue color channels */
/* font and symbols: horizontally aligned & flipped */
#define FONT_16X26_HF                   /* 16x26 font */
//#define FONT_16X26_ISO8859_2_HF         /* 16x26 Central European font */
//#define FONT_16X26_WIN1251_HF           /* 16x26 cyrillic font */
#define SYMBOLS_32X32_HF                /* 32x32 symbols */
//#endif



/*
 *  ILI9481, ILI9486 or ILI9488
 *  - SPI interface using hardware SPI
 */

#if 0
//#define LCD_ILI9481                     /* display controller ILI9481 */
//#define LCD_ILI9486                     /* display controller ILI9486 */
#define LCD_ILI9488                     /* display controller ILI9488 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_DC           PB7            /* port pin used for D/C */
#define LCD_SCK          PB1            /* port pin used for SCK */
#define LCD_SDI          PB2            /* port pin used for SDI (data input) */
//#define LCD_SDO          PB3            /* port pin used for SDO (data output) */
/* display settings */
#define LCD_DOTS_X       320            /* number of horizontal dots */
#define LCD_DOTS_Y       480            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
//#define LCD_BGR                         /* reverse red and blue color channels */
/* font and symbols: horizontally aligned & flipped */
#define FONT_16X26_HF                   /* 16x26 font */
//#define FONT_16X26_ISO8859_2_HF         /* 16x26 Central European font */
//#define FONT_16X26_WIN1251_HF           /* 16x26 cyrillic font */
#define SYMBOLS_32X32_HF                /* 32x32 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  PCD8544
 *  - SPI interface using hardware SPI
 *  - for a 180° rotated display (LCD_ROT180)
 *    - comment out "_VF" font and "_VFP" symbols
 *    - uncomment "_V_F" font and "_VP_F" symbols
 */

#if 0
#define LCD_PCD8544                     /* display controller PCD8544 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB4            /* port pin used for /RES (optional) */
#define LCD_SCE          PB5            /* port pin used for /SCE (optional) */
#define LCD_DC           PB7            /* port pin used for D/C */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SDIN         PB2            /* port pin used for SDIN (data input) */
/* display settings */
#define LCD_DOTS_X       84             /* number of horizontal dots */
#define LCD_DOTS_Y       48             /* number of vertical dots */
#define LCD_CONTRAST     66             /* default contrast (1-127) */
/* font and symbols: vertically aligned & flipped */
#define FONT_6X8_VF                     /* 6x8 font */
//#define FONT_6X8_ISO8859_2_VF           /* 6x8 Central European font */
//#define LCD_ROT180                      /* rotate output by 180° (not supported yet) */
/* font and symbols: vertically aligned, bank-wise grouping, hor. flipped */
//#define FONT_6X8_V_F                    /* 6x8 font */
//#define FONT_6X8_ISO8859_2_V_F          /* 6x8 Central European font */
//#define SYMBOLS_24X24_VP_F              /* 24x24 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  PCF8814
 *  - 3 wire SPI interface using bit-bang SPI
 */

#if 0
#define LCD_PCF8814                     /* display controller PCF8814 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SDIN         PB2            /* port pin used for SDIN (data input) */
/* display settings */
#define LCD_DOTS_X       96             /* number of horizontal dots */
#define LCD_DOTS_Y       65             /* number of vertical dots */
//#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     5              /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, page-wise grouping */
#define FONT_6X8_VF                     /* 6x8 font */
//#define FONT_6X8_ISO8859_2_VF         /* 6x8 Central European font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_BITBANG                     /* bit-bang SPI */
#define SPI_9                           /* support 9 bit frames */
#define SPI_PORT         LCD_PORT       /* SPI port data register */
#define SPI_DDR          LCD_DDR        /* SPI port data direction register */
#define SPI_SCK          LCD_SCLK       /* port pin used for SCK */
#define SPI_MOSI         LCD_SDIN       /* port pin used for MOSI */
#endif



/*
 *  SH1106
 *  - 4 wire SPI interface using hardware SPI
 *  - untested
 */

#if 0
#define LCD_SH1106                      /* display controller SH1106 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_A0           PB7            /* port pin used for A0 */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SI           PB2            /* port pin used for SI (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_OFFSET_X     2              /* enable x offset of 2 or 4 dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
//#define FONT_6X8_VF                      /* 6x8 font */
#define FONT_8X8_VF                     /* 8x8 font */
//#define FONT_8X16_VFP                   /* 8x16 font */
//#define FONT_6X8_ISO8859_2_VF           /* 6x8 Central European font */
//#define FONT_8X8_ISO8859_2_VF           /* 8x8 Central European font */
//#define FONT_8X12T_ISO8859_2_VFP        /* thin 8x12 Central European font */
//#define FONT_8X16_ISO8859_2_VFP         /* 8x16 Central European font */
//#define FONT_8X8_WIN1251_VF             /* 8x8 cyrillic font */
//#define FONT_8X8ALT_WIN1251_VF          /* 8x8 alternative cyrillic font */
//#define FONT_8X8T_WIN1251_VF            /* thin 8x8 cyrillic font */
//#define FONT_8X12T_WIN1251_VFP          /* thin 8x12 cyrillic font */
//#define FONT_8X16_WIN1251_VFP           /* 8x16 cyrillic font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  SH1106
 *  - 3 wire SPI interface using bit-bang SPI
 *  - untested
 */

#if 0
#define LCD_SH1106                      /* display controller SH1106 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SI           PB2            /* port pin used for SI (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_OFFSET_X     2              /* enable x offset of 2 or 4 dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
#define FONT_8X8_VF                     /* 8x8 font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_BITBANG                     /* bit-bang SPI */
#define SPI_9                           /* support 9 bit frames */
#define SPI_PORT         LCD_PORT       /* SPI port data register */
#define SPI_DDR          LCD_DDR        /* SPI port data direction register */
#define SPI_SCK          LCD_SCL        /* port pin used for SCK */
#define SPI_MOSI         LCD_SI         /* port pin used for MOSI */
#endif



/*
 *  SH1106
 *  - I2C interface using hardware I2C
 */

#if 0
#define LCD_SH1106                      /* display controller SH1106 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_I2C                         /* I2C interface */
#define LCD_I2C_ADDR     0x3c           /* SH1106's I2C address */
/* control lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
//#define LCD_RESET        PB0            /* port pin used for /RES (optional) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_OFFSET_X     2              /* enable x offset of 2 or 4 dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
#define FONT_8X8_VF                     /* 8x8 font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* I2C bus */
#define I2C_HARDWARE                    /* hardware I2C (MCU's TWI) */
#define I2C_STANDARD_MODE               /* 100kHz bus speed */
#endif



/*
 *  SSD1306
 *  - 4 wire SPI interface using hardware SPI
 */

#if 0
#define LCD_SSD1306                     /* display controller SSD1306 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_DC           PB7            /* port pin used for D/C */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SDIN         PB2            /* port pin used for SDIN (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
//#define FONT_6X8_VF                      /* 6x8 font */
#define FONT_8X8_VF                     /* 8x8 font */
//#define FONT_8X16_VFP                   /* 8x16 font */
//#define FONT_6X8_ISO8859_2_VF           /* 6x8 Central European font */
//#define FONT_8X8_ISO8859_2_VF           /* 8x8 Central European font */
//#define FONT_8X12T_ISO8859_2_VFP        /* thin 8x12 Central European font */
//#define FONT_8X16_ISO8859_2_VFP         /* 8x16 Central European font */
//#define FONT_8X8_WIN1251_VF             /* 8x8 cyrillic font */
//#define FONT_8X8ALT_WIN1251_VF          /* 8x8 alternative cyrillic font */
//#define FONT_8X8T_WIN1251_VF            /* thin 8x8 cyrillic font */
//#define FONT_8X12T_WIN1251_VFP          /* thin 8x12 cyrillic font */
//#define FONT_8X16_WIN1251_VFP           /* 8x16 cyrillic font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  SSD1306
 *  - 3 wire SPI interface using bit-bang SPI
 */

#if 0
#define LCD_SSD1306                     /* display controller SSD1306 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SDIN         PB2            /* port pin used for SDIN (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
#define FONT_8X8_VF                     /* 8x8 font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_BITBANG                     /* bit-bang SPI */
#define SPI_9                           /* support 9 bit frames */
#define SPI_PORT         LCD_PORT       /* SPI port data register */
#define SPI_DDR          LCD_DDR        /* SPI port data direction register */
#define SPI_SCK          LCD_SCLK       /* port pin used for SCK */
#define SPI_MOSI         LCD_SDIN       /* port pin used for MOSI */
#endif



/*
 *  SSD1306
 *  - I2C interface using hardware I2C
 */

#if 0
#define LCD_SSD1306                     /* display controller SSD1306 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_I2C                         /* I2C interface */
#define LCD_I2C_ADDR     0x3c           /* SSD1306's I2C address */
/* control lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB0            /* port pin used for /RES (optional) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     127            /* default contrast (0-255) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
#define FONT_8X8_VF                     /* 8x8 font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* I2C bus */
#define I2C_HARDWARE                    /* hardware I2C (MCU's TWI) */
#define I2C_STANDARD_MODE               /* 100kHz bus speed */
#endif



/*
 *  ST7036
 *  - 4 bit parallel interface
 *  - enable LCD_DB_STD when using port pins 0-3 for LCD_DB4/5/6/7
 *  - untested!!!
 */

#if 0
#define LCD_ST7036                      /* display controller ST7036 */
#define LCD_TEXT                        /* character display */
#define LCD_PAR_4                       /* 4 bit parallel interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_DB_STD                      /* use standard pins 0-3 for DB4-7 */
#define LCD_DB4          PB0            /* port pin used for DB4 */
#define LCD_DB5          PB1            /* port pin used for DB5 */
#define LCD_DB6          PB2            /* port pin used for DB6 */
#define LCD_DB7          PB3            /* port pin used for DB7 */
#define LCD_RS           PB4            /* port pin used for RS */
#define LCD_EN           PB5            /* port pin used for E */
//#define LCD_RW           ???            /* port pin used for R/W (optional) */
//#define LCD_RESET        ???            /* port pin used for XRESET (optional) */
/* display settings */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       3              /* number of lines */
#define LCD_EXTENDED_CMD                /* extended instruction set (EXT pin high) */
#define LCD_CONTRAST     32             /* default contrast (0-63) */
/* ST7036 has internal 5x8 font */
#define FONT_ST7036                     /* 5x8 font */
#endif



/*
 *  ST7036
 *  - 4 wire SPI interface using bit-bang SPI
 *  - untested!!!
 */

#if 0
#define LCD_ST7036                      /* display controller ST7036 */
#define LCD_TEXT                        /* character display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for XRESET (optional) */
#define LCD_CS           PB5            /* port pin used for CSB (optional) */
#define LCD_RS           PB7            /* port pin used for RS */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SI           PB2            /* port pin used for SI (data input) */
/* display settings */
#define LCD_CHAR_X       16             /* characters per line */
#define LCD_CHAR_Y       3              /* number of lines */
#define LCD_EXTENDED_CMD                /* extended instruction set (EXT pin high) */
#define LCD_CONTRAST     32             /* default contrast (0-63) */
/* ST7036 has internal 5x8 font */
#define FONT_ST7036                     /* 5x8 font */
/* SPI bus */
#define SPI_BITBANG                     /* bit-bang SPI */
#define SPI_PORT         LCD_PORT       /* SPI port data register */
#define SPI_DDR          LCD_DDR        /* SPI port data direction register */
#define SPI_SCK          LCD_SCL        /* port pin used for SCK */
#define SPI_MOSI         LCD_SI         /* port pin used for MOSI */
#endif



/*
 *  ST7565R
 *  - SPI interface using hardware SPI
 *  - settings for Electronic Assembly EA DOGM/DOGL128-6
 */

#if 0
#define LCD_ST7565R                     /* display controller ST7565R */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS1 (optional) */
#define LCD_A0           PB7            /* port pin used for A0 */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SI           PB2            /* port pin used for SI (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
#define LCD_OFFSET_X                    /* enable x offset of 4 dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_START_Y      0              /* start line (0-63) */
#define LCD_CONTRAST     22             /* default contrast (0-63) */
/* font and symbols: vertically aligned & flipped, bank-wise grouping */
//#define FONT_6X8_VF                      /* 6x8 font */
#define FONT_8X8_VF                     /* 8x8 font */
//#define FONT_8X16_VFP                   /* 8x16 font */
//#define FONT_6X8_ISO8859_2_VF           /* 6x8 Central European font */
//#define FONT_8X8_ISO8859_2_VF           /* 8x8 Central European font */
//#define FONT_8X12T_ISO8859_2_VFP        /* thin 8x12 Central European font */
//#define FONT_8X16_ISO8859_2_VFP         /* 8x16 Central European font */
//#define FONT_8X8_WIN1251_VF             /* 8x8 cyrillic font */
//#define FONT_8X8ALT_WIN1251_VF          /* 8x8 alternative cyrillic font */
//#define FONT_8X8T_WIN1251_VF            /* thin 8x8 cyrillic font */
//#define FONT_8X12T_WIN1251_VFP          /* thin 9x12 cyrillic font */
//#define FONT_8X16_WIN1251_VFP           /* 8x16 cyrillic font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  ST7735
 *  - 4 wire SPI interface using hardware SPI
 */

#if 0
#define LCD_ST7735                      /* display controller ST7735 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RES          PB4            /* port pin used for /RESX (optional) */
#define LCD_CS           PB5            /* port pin used for /CSX (optional) */
#define LCD_DC           PB7            /* port pin used for D/CX */
#define LCD_SCL          PB1            /* port pin used for SCL */
#define LCD_SDA          PB2            /* port pin used for SDA */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       160            /* number of vertical dots */
//#define LCD_OFFSET_X     4               /* enable x offset of 2 or 4 dots */
//#define LCD_OFFSET_Y     2               /* enable y offset of 1 or 2 dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
//#define LCD_BGR                         /* reverse red and blue color channels */
//#define LCD_LATE_ON                     /* turn on LCD after clearing it */
/* font and symbols: horizontally aligned & flipped */
#define FONT_10X16_HF                   /* 10x16 font */
//#define FONT_6X8_ISO8859_2_HF           /* 6x8 Central European font */
//#define FONT_8X8_ISO8859_2_HF           /* 8x8 Central European font */
//#define FONT_10X16_ISO8859_2_HF         /* 10x16 Central European font */
//#define FONT_8X16_WIN1251_HF            /* 8x16 cyrillic font */
//#define FONT_8X16ALT_WIN1251_HF         /* 8x16 alternative cyrillic font */
#define SYMBOLS_30X32_HF                /* 30x32 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  ST7920
 *  - SPI interface using hardware SPI
 *  - for a 180° rotated display (LCD_ROT180)
 *    - comment out "_H" font and symbol
 *    - uncomment "_HF" font and symbol
 */

#if 0
#define LCD_ST7920                      /* display controller ST7920 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RESET (optional) */
#define LCD_CS           PB5            /* port pin used for CS (optional) */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SID          PB2            /* port pin used for SID (data input) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
/* font and symbols: horizonally aligned */
#define FONT_8X8_H                      /* 8x8 font */
//#define FONT_8X8_ISO8859_2_H            /* 8x8 Central European font */
#define SYMBOLS_24X24_H                 /* 24x24 symbols */
//#define LCD_ROT180                      /* rotate output by 180° */
/* font and symbols: horizonally aligned & flipped */
//#define FONT_8X8_HF                     /* 8x8 font */
//#define FONT_8X8_ISO8859_2_HF           /* 8x8 Central European font */
//#define SYMBOLS_24X24_HF                /* 24x24 symbols */
/* SPI bus */
#define SPI_HARDWARE                    /* hardware SPI */
#endif



/*
 *  ST7920
 *  - 4 bit parallel interface
 *  - enable LCD_DB_STD when using port pins 0-3 for LCD_DB4/5/6/7
 *  - for a 180° rotated display (LCD_ROT180)
 *    - comment out "_H" font and symbol
 *    - uncomment "_HF" font and symbol
 */

#if 0
#define LCD_ST7920                      /* display controller ST7920 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_PAR_4                       /* 4 bit parallel interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_DB_STD                      /* use standard pins 0-3 for DB4-7 */
#define LCD_DB4          PB0            /* port pin used for DB4 */
#define LCD_DB5          PB1            /* port pin used for DB5 */
#define LCD_DB6          PB2            /* port pin used for DB6 */
#define LCD_DB7          PB3            /* port pin used for DB7 */
#define LCD_RS           PB4            /* port pin used for RS */
#define LCD_EN           PB5            /* port pin used for E */
//#define LCD_RW           PB?            /* port pin used for RW (optional) */
//#define LCD_RESET        PB?            /* port pin used for /RESET (optional) */
/* display settings */
#define LCD_DOTS_X       128            /* number of horizontal dots */
#define LCD_DOTS_Y       64             /* number of vertical dots */
/* font and symbols: horizonally aligned */
#define FONT_8X8_H                      /* 8x8 font */
#define SYMBOLS_24X24_H                 /* 24x24 symbols */
//#define LCD_ROT180                      /* rotate output by 180° */
/* font and symbols: horizonally aligned & flipped */
//#define FONT_8X8_HF                     /* 8x8 font */
//#define SYMBOLS_24X24_HF                /* 24x24 symbols */
#endif



/*
 *  STE2007
 *  - 3 wire SPI interface using bit-bang SPI
 */

#if 0
#define LCD_STE2007                     /* display controller STE2007 */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_SPI                         /* SPI interface */
/* control and data lines */
#define LCD_PORT         PORTB          /* port data register */
#define LCD_DDR          DDRB           /* port data direction register */
#define LCD_RESET        PB4            /* port pin used for /RES (optional) */
#define LCD_CS           PB5            /* port pin used for /CS (optional) */
#define LCD_SCLK         PB1            /* port pin used for SCLK */
#define LCD_SDIN         PB2            /* port pin used for SDIN (data input) */
/* display settings */
#define LCD_DOTS_X       96             /* number of horizontal dots */
#define LCD_DOTS_Y       68             /* number of vertical dots */
#define LCD_FLIP_X                      /* enable horizontal flip */
#define LCD_FLIP_Y                      /* enable vertical flip */
#define LCD_CONTRAST     16             /* default contrast (0-31) */
/* font and symbols: vertically aligned & flipped */
#define FONT_6X8_VF                     /* 6x8 font */
//#define FONT_6X8_ISO8859_2_VF         /* 6x8 Central European font */
#define SYMBOLS_24X24_VFP               /* 24x24 symbols */
/* SPI bus */
#define SPI_BITBANG                     /* bit-bang SPI */
#define SPI_9                           /* support 9 bit frames */
#define SPI_PORT         LCD_PORT       /* SPI port data register */
#define SPI_DDR          LCD_DDR        /* SPI port data direction register */
#define SPI_SCK          LCD_SCLK       /* port pin used for SCK */
#define SPI_MOSI         LCD_SDIN       /* port pin used for MOSI */
#endif



/*
 *  VT100 serial terminal, TTL serial
 */

#if 0
#define LCD_VT100                       /* pseudo display VT100 */
#define LCD_TEXT                        /* character display */
#define LCD_COLOR                       /* color display */
#define LCD_CHAR_X       40             /* characters per line */
#define LCD_CHAR_Y       24             /* number of lines */
//#define SERIAL_BITBANG                  /* bit-bang serial */
#define SERIAL_HARDWARE                 /* hardware serial */
#endif



/*
 *  R&D Display
 *  - 8 bit parallel interface
 *  - LCD_DB0 to LCD_DB7 have to match port pins 0 to 7
 */

#if 0
#define LCD_RD_DISPLAY                  /* R&D Display */
#define LCD_GRAPHIC                     /* graphic display */
#define LCD_COLOR                       /* color display */
#define LCD_PAR_8                       /* 8 bit parallel interface */
/* control lines */
#define LCD_PORT         PORTC          /* port data register */
#define LCD_DDR          DDRC           /* port data direction register */
#define LCD_RES          PC0            /* port pin used for /RESX (optional) */
#define LCD_CS           PC1            /* port pin used for /CSX (optional) */
#define LCD_DC           PC2            /* port pin used for D/CX */
#define LCD_WR           PC3            /* port pin used for WRX */
#define LCD_RD           PC4            /* port pin used for RDX */
/* data lines DB0-7 */
#define LCD_PORT2        PORTL          /* port data register */
#define LCD_DDR2         DDRL           /* port data direction register */
#define LCD_PIN2         PINL           /* port data direction register */
#define LCD_DB0          PL0            /* port pin used for D0 */
#define LCD_DB1          PL1            /* port pin used for D1 */
#define LCD_DB2          PL2            /* port pin used for D2 */
#define LCD_DB3          PL3            /* port pin used for D3 */
#define LCD_DB4          PL4            /* port pin used for D4 */
#define LCD_DB5          PL5            /* port pin used for D5 */
#define LCD_DB6          PL6            /* port pin used for D6 */
#define LCD_DB7          PL7            /* port pin used for D7 */
/* display settings */
#define LCD_DOTS_X       240            /* number of horizontal dots */
#define LCD_DOTS_Y       320            /* number of vertical dots */
//#define LCD_FLIP_X                      /* enable horizontal flip */
//#define LCD_FLIP_Y                      /* enable vertical flip */
//#define LCD_ROTATE                      /* switch X and Y (rotate by 90°) */
//#define LCD_BGR                         /* reverse red and blue color channels */
/* font and symbols: horizontally aligned & flipped */
//#define FONT_16X26_HF                   /* 16x26 font */
//#define SYMBOLS_32X32_HF                /* 32x32 symbols */
#endif



/*
 *  check if a LCD module is specified
 */

#if !defined(LCD_TEXT) && !defined(LCD_GRAPHIC)
  #error <<< No LCD module specified! >>>
#endif



/* ************************************************************************
 *   touchscreen (optional)
 * ************************************************************************ */


/*
 *  touchscreen controller
 *
 *  Please uncomment the package matching your touchscreen
 *  and adjust settings.
 *
 *  To uncomment, remove the enclosing "#if 0" and "#endif" or
 *  put a "//" in front of both.
 */


/*
 *  ADS7843 / XPT2046
 *  - SPI interface using hardware SPI
 */

#if 0
#define TOUCH_ADS7843
#define TOUCH_PORT       PORTC     /* port data register */
#define TOUCH_DDR        DDRC      /* port data direction register */
#define TOUCH_PIN        PINC      /* port input pins register */
#define TOUCH_CS         PC0       /* port pin used for /CS */
#define TOUCH_PEN        PC1       /* port pin used for /PENIRQ */
//#define TOUCH_FLIP_X               /* enable horizontal flip */
//#define TOUCH_FLIP_Y               /* enable vertical flip */
//#define TOUCH_ROTATE               /* switch X and Y (rotate by 90°) */
#define SPI_HARDWARE               /* hardware SPI */
#define SPI_RW                     /* enable SPI read support */
#endif



/* ************************************************************************
 *   port and pin assignments
 * ************************************************************************ */


/*
 *  test pins / probes:
 *  - Must be an ADC port
 *    - Port K (ADC8-15) not supported
 *  - Lower 3 pins of the port must be used for probes.
 *  - Please don't change the definitions of TP1, TP2 and TP3!
 *  - Don't share this port with POWER_CTRL or TEST_BUTTON!
 */

#define ADC_PORT         PORTF     /* port data register */
#define ADC_DDR          DDRF      /* port data direction register */
#define ADC_PIN          PINF      /* port input pins register */
#define TP1              PF0       /* test pin / probe #1 */
#define TP2              PF1       /* test pin / probe #2 */
#define TP3              PF2       /* test pin / probe #3 */

#define TP_ZENER         PF3       /* test pin for 10:1 voltage divider */
#define TP_REF           PF4       /* test pin for 2.5V reference and relay */
#define TP_BAT           PF5       /* test pin for battery (4:1 voltage divider) */
#define TP_CAP           PF7       /* test pin for self-adjustment cap */


/*
 *  probe resistors
 *  - For PWM/squarewave output via probe #2 R_RL_2 has to be PB6/OC1B.
 *  - Don't share this port with POWER_CTRL or TEST_BUTTON!
 */

#define R_PORT           PORTK     /* port data register */
#define R_DDR            DDRK      /* port data direction register */
#define R_RL_1           PK0       /* Rl (680R) for test pin #1 */
#define R_RH_1           PK1       /* Rh (470k) for test pin #1 */
#define R_RL_2           PK2       /* Rl (680R) for test pin #2 */
#define R_RH_2           PK3       /* Rh (470k) for test pin #2 */
#define R_RL_3           PK4       /* Rl (680R) for test pin #3 */
#define R_RH_3           PK5       /* Rh (470k) for test pin #3 */


/*
 *  dedicated signal output via OC1B
 *  - don't change this!
 */

#define SIGNAL_PORT      PORTB     /* port data register */
#define SIGNAL_DDR       DDRB      /* port data direction register */
#define SIGNAL_OUT       PB6       /* MCU's OC1B pin */


/*
 *  power control
 *  - can't be same port as ADC_PORT or R_PORT
 */

#define POWER_PORT       PORTA     /* port data register */
#define POWER_DDR        DDRA      /* port data direction register */
#define POWER_CTRL       PA6       /* control pin (1: on / 0: off) */


/*
 *  test push button
 *  - can't be same port as ADC_PORT or R_PORT
 */

#define BUTTON_PORT      PORTA     /* port data register */
#define BUTTON_DDR       DDRA      /* port data direction register */
#define BUTTON_PIN       PINA      /* port input pins register */
#define TEST_BUTTON      PA7       /* test/start push button (low active) */


/*
 *  rotary encoder
 */

#define ENCODER_PORT     PORTA     /* port data register */
#define ENCODER_DDR      DDRA      /* port data direction register */
#define ENCODER_PIN      PINA      /* port input pins register */
#define ENCODER_A        PA3       /* rotary encoder A signal */
#define ENCODER_B        PA1       /* rotary encoder B signal */


/*
 *  increase/decrease push buttons
 */

#define KEY_PORT         PORTA     /* port data register */
#define KEY_DDR          DDRA      /* port data direction register */
#define KEY_PIN          PINA      /* port input pins register */
#define KEY_INC          PA3       /* increase push button (low active) */
#define KEY_DEC          PA1       /* decrease push button (low active) */


/*
 *  frequency counter
 *  - basic and extented version
 *  - input must be pin PD7/T0
 */

/* counter input */
#define COUNTER_PORT          PORTD     /* port data register */
#define COUNTER_DDR           DDRD      /* port data direction register */
#define COUNTER_IN            PD7       /* signal input T0 */

/* control of extended frequency counter */
#define COUNTER_CTRL_PORT     PORTD     /* port data register */ 
#define COUNTER_CTRL_DDR      DDRD      /* port data direction register */
#define COUNTER_CTRL_DIV      PD4       /* prescaler (low 1:1, high x:1) */
#define COUNTER_CTRL_CH0      PD5       /* channel addr #0 */
#define COUNTER_CTRL_CH1      PD6       /* channel addr #1 */


/*
 *  L/C meter
 *  - frequency input must be pin PB0/T0 (uses COUNTER_IN)
 *  - control of L/C meter
 */

#define LC_CTRL_PORT     PORTD     /* port data register */ 
#define LC_CTRL_DDR      DDRD      /* port data direction register */
#define LC_CTRL_CP       PC4       /* reference cap (low: on / high: off) */
#define LC_CTRL_LC       PC5       /* L/C selection (low: C / high: L */


/*
 *  IR detector/decoder
 *  - fixed module connected to dedicated I/O pin
 */

#define IR_PORT          PORTA     /* port data register */
#define IR_DDR           DDRA      /* port data direction register */
#define IR_PIN           PINA      /* port input pins register */
#define IR_DATA          PA0       /* data signal */


/*
 *  SPI
 *  - hardware SPI uses
 *    SCK PB1, MOSI PB2, MISO PB3 and /SS PB0
 *  - could be already set in display section
 *  - unused signals can be ignored
 *  - /SS is set to output mode for hardware SPI but not used
 */

/* for bit-bang and hardware SPI */
#ifndef SPI_PORT
#define SPI_PORT         PORTB     /* port data register */
#define SPI_DDR          DDRB      /* port data direction register */
#define SPI_PIN          PINB      /* port input pins register */
#define SPI_SCK          PB1       /* pin for SCK */
#define SPI_MOSI         PB2       /* pin for MOSI */
#define SPI_MISO         PB3       /* pin for MISO */
#define SPI_SS           PB0       /* pin for /SS */
#endif


/*
 *  I2C
 *  - hardware I2C (TWI) uses
 *    SDA PD1 and SCL PD0
 *  - could be already set in display section
 */

/* for bit-bang I2C */
#ifndef I2C_PORT
#define I2C_PORT         PORTD     /* port data register */
#define I2C_DDR          DDRD      /* port data direction register */
#define I2C_PIN          PIND      /* port input pins register */
#define I2C_SDA          PD1       /* pin for SDA */
#define I2C_SCL          PD0       /* pin for SCL */
#endif


/*
 *  TTL serial interface
 *  - hardware USART uses
 *    USART0: Rx PE0 and Tx PE1
 *    USART1: Rx PD2 and Tx PD3
 *    USART2: Rx PH0 and Tx PH1
 *    USART3: Rx PJ0 and Tx PJ1
 */

/* for hardware TTL serial */
#define SERIAL_USART     0         /* use USART0 */
/* for bit-bang TTL serial */
#define SERIAL_PORT      PORTE     /* port data register */
#define SERIAL_DDR       DDRE      /* port data direction register */
#define SERIAL_PIN       PINE      /* port input pins register */
#define SERIAL_TX        PE1       /* pin for Tx (transmit) */
#define SERIAL_RX        PE0       /* pin for Rx (receive) */
#define SERIAL_PCINT     8         /* PCINT# for Rx pin */


/*
 *  OneWire
 *  - dedicated I/O pin
 */

#define ONEWIRE_PORT     PORTA     /* port data register */
#define ONEWIRE_DDR      DDRA      /* port data direction register */
#define ONEWIRE_PIN      PINA      /* port input pins register */
#define ONEWIRE_DQ       PA4       /* DQ (data line) */


/*
 *  fixed cap for self-adjustment
 *  - ADC pin is TP_CAP from above
 *  - settings are for 470k resistor
 *  - should be film cap with 100nF - 1000nF
 */

#define ADJUST_PORT      PORTA     /* port data register */
#define ADJUST_DDR       DDRA      /* port data direction register */
#define ADJUST_RH        PA5       /* Rh (470k) for fixed cap */


/*
 *  relay for parallel cap (sampling ADC)
 *  - between TP1 & TP3
 *  - cap should have 10nF - 27nF
 *  - not supported yet
 */

#define CAP_RELAY_PORT   PORTA     /* port data register */
#define CAP_RELAY_DDR    DDRA      /* port data direction register */
#define CAP_RELAY_CTRL   PA2       /* control pin */



/* ************************************************************************
 *   internal stuff
 * ************************************************************************ */


/* ADC reference selection: AVcc */
#define ADC_REF_VCC           (1 << REFS0)

/* ADC reference selection: internal 1.1V bandgap */
#define ADC_REF_BANDGAP       (1 << REFS1)

/* ADC reference selection: internal 2.56V (bandgap * 2.328) */
#define ADC_REF_256           ((1 << REFS1) | (1 << REFS0))

/* ADC reference selection: filter mask for register bits */
#define ADC_REF_MASK          ((1 << REFS1) | (1 << REFS0))

/* ADC MUX channel: internal 1.1V bandgap reference */
#define ADC_CHAN_BANDGAP      0x1e      /* 11110 */

/* ADC MUX channel: filter mask for register bits */
#define ADC_CHAN_MASK         0b00011111     /* ADMUX: MUX0-4 */
#define ADC_CHAN_MASK2        0b00001000     /* ADCSRB: MUX5 */



/* ************************************************************************
 *   MCU specific setup to support different AVRs
 * ************************************************************************ */


/*
 *  ATmega 640
 */

#if defined(__AVR_ATmega640__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* this MCU has 64kB Flash, 4kB EEPROM and 8kB RAM (enable extra features) */
  #define RES_FLASH           64
  #define RES_EEPROM          4
  #define RES_RAM             8


/*
 *  ATmega 1280
 */

#elif defined(__AVR_ATmega1280__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* this MCU has 128kB Flash, 4kB EEPROM and 8kB RAM (enable extra features) */
  #define RES_FLASH           128
  #define RES_EEPROM          4
  #define RES_RAM             8


/*
 *  ATmega 2560
 */

#elif defined(__AVR_ATmega2560__)

  /* estimated internal resistance of port to GND (in 0.1 Ohms) */
  #define R_MCU_LOW           200  /* 209 */

  /* estimated internal resistance of port to VCC (in 0.1 Ohms) */
  #define R_MCU_HIGH          220  /* 235 */

  /* voltage offset of MCU's analog comparator (in mV): -50 up to 50 */
  #define COMPARATOR_OFFSET   0

  /* this MCU has 256kB Flash, 4kB EEPROM and 8kB RAM (enable extra features) */
  #define RES_FLASH           256
  #define RES_EEPROM          4
  #define RES_RAM             8


/*
 *  missing or unsupported MCU
 */

#else
  #error <<< No or wrong MCU type selected! >>>
#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

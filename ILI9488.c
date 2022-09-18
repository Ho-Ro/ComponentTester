/* ************************************************************************
 *
 *   driver functions for ILI9488 compatible color graphic displays
 *   - 320 x 480 pixels
 *   - interfaces
 *     - 8 bit parallel (untested)
 *     - 9 bit parallel (not supported)
 *     - 16 bit parallel (untested)
 *     - 18 bit parallel (not supported)
 *     - 24 bit parallel (not supported)
 *     - 3 line SPI (not supported, would be insanely slow)
 *     - 4 line SPI (hardware SPI strongly recommended)
 *
 *   (c) 2020-2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for 8 bit parallel interface
 *             LCD_PORT/LCD_DDR:
 *    /RESX    LCD_RES (optional)
 *    /CSX     LCD_CS (optional)
 *    D/CX     LCD_DC
 *    WRX      LCD_WR
 *    RDX      LCD_RD (optional)
 *             LCD_PORT2/LCD_DDR2/LCD_PIN2:
 *    DB0      LCD_DB0 (LCD_PORT2 pin #0)
 *    DB1      LCD_DB1 (LCD_PORT2 pin #1)
 *    DB2      LCD_DB2 (LCD_PORT2 pin #2)
 *    DB3      LCD_DB3 (LCD_PORT2 pin #3)
 *    DB4      LCD_DB4 (LCD_PORT2 pin #4)
 *    DB5      LCD_DB5 (LCD_PORT2 pin #5)
 *    DB6      LCD_DB6 (LCD_PORT2 pin #6)
 *    DB7      LCD_DB7 (LCD_PORT2 pin #7)
 *  - pin assignment for 16 bit parallel interface
 *    same as 8 bit parallel but additionally
 *             LCD_PORT3/LCD_DDR3/LCD_PIN3:
 *    DB8      LCD_DB8  (LCD_PORT3 pin #0)
 *    DB9      LCD_DB9  (LCD_PORT3 pin #1)
 *    DB10     LCD_DB10 (LCD_PORT3 pin #2)
 *    DB11     LCD_DB11 (LCD_PORT3 pin #3)
 *    DB12     LCD_DB12 (LCD_PORT3 pin #4)
 *    DB13     LCD_DB13 (LCD_PORT3 pin #5)
 *    DB14     LCD_DB14 (LCD_PORT3 pin #6)
 *    DB15     LCD_DB15 (LCD_PORT3 pin #7)
 *  - max. clock rate for parallel bus
 *    25MHz write
 *    6.25MHz read register data
 *    2.2MHz read frame memory
 *  - pin assignment for 4 line SPI
 *    /RESX       Vcc or LCD_RES (optional)
 *    /CSX        Gnd or LCD_CS (optional)
 *    D/CX        LCD_DC
 *    SCL (WRX)   LCD_SCL / SPI_SCK
 *    SDA         LCD_SDA / SPI_MOSI
 *    SDO         LCD_SDO / SPI_MISO (not used yet)
 *    For hardware SPI LCD_SCL and LCD_DIN have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock: 20MHz write and 6.6MHz read
 *  - ILI9486 has a PWM output (CAPC_PWM) for controlling backlight LEDs,
 *    but it's rarely used.
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ILI9488


/*
 *  local constants
 */

/* source management */
#define LCD_DRIVER_C



/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */
#include "ILI9488.h"          /* ILI9488 specifics */


/* fonts and symbols */
/* horizontally aligned, horizontal bit order flipped */
#include "font_8x8_hf.h"
#include "font_12x16_hf.h"
#include "font_16x26_hf.h"
#include "font_10x16_iso8859-2_hf.h"
#include "font_12x16_iso8859-2_hf.h"
#include "font_16x26_iso8859-2_hf.h"
#include "font_16x26_win1251_hf.h"
#include "symbols_24x24_hf.h"
#include "symbols_32x32_hf.h"

/* sanity check */
#ifndef FONT_SET
  #error <<< No font selected! >>>
#endif
#ifdef SW_SYMBOLS
  #ifndef SYMBOL_SET
    #error <<< No symbols selected! >>>
  #endif
#endif



/*
 *  derived constants
 */

/* maximum number of pixels for X and Y direction */
#ifdef LCD_ROTATE
  #define LCD_PIXELS_X        LCD_DOTS_Y
  #define LCD_PIXELS_Y        LCD_DOTS_X
#else
  #define LCD_PIXELS_X        LCD_DOTS_X
  #define LCD_PIXELS_Y        LCD_DOTS_Y
#endif

/* number of lines and characters per line */
#define LCD_CHAR_X            (LCD_PIXELS_X / FONT_SIZE_X)
#define LCD_CHAR_Y            (LCD_PIXELS_Y / FONT_SIZE_Y)

/* component symbols */
#ifdef SW_SYMBOLS
  /* resize symbols by a factor of 2 */
  #define SYMBOL_RESIZE       2

  /* size in relation to a character */
  #define LCD_SYMBOL_CHAR_X   (((SYMBOL_SIZE_X * SYMBOL_RESIZE) + FONT_SIZE_X - 1) / FONT_SIZE_X)
  #define LCD_SYMBOL_CHAR_Y   (((SYMBOL_SIZE_Y * SYMBOL_RESIZE) + FONT_SIZE_Y - 1) / FONT_SIZE_Y)

  /* check y size: we need at least 2 lines */
  #if LCD_SYMBOL_CHAR_Y < 2
    #error <<< Symbols too small! >>>
  #endif
#endif

/* color modes */
#ifdef LCD_SPI
  /* SPI interface: use RGB666 (RGB565 not supported by SPI) */
  #define COLORMODE_RGB666
#else
  /* parallel interface: use RGB565 */
  #define COLORMODE_RGB565
#endif



/*
 *  local variables
 */

/* address window */
uint16_t            X_Start;       /* start position X (column) */
uint16_t            X_End;         /* end position X (column) */
uint16_t            Y_Start;       /* start position Y (page/row) */
uint16_t            Y_End;         /* end position Y (page/row) */

/* text line management */
uint16_t            LineFlags;     /* bitfield for up to 16 lines */

#ifdef COLORMODE_RGB666
/* colors in RGB666 8-bit frame format */
uint8_t             RGB666_FG[3];       /* foreground/pen color */
uint8_t             RGB666_BG[3];       /* background color */
#endif



/* ************************************************************************
 *   low level functions for 4 line SPI interface
 * ************************************************************************ */


/*
 *  protocol:
 *  - write: CSX low -> D/CX -> D7-0 with rising edge of SCL
 *  - D/CX: high = data / low = command
 *
 *  hints:
 *  - RGB111 and RGB666 are supported when using 4-line SPI
 *    (datasheet lists also RGB565, but this is wrong)
 */


#if defined (LCD_SPI) && ! defined (SPI_9)

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set control signals
   */

  Bits = LCD_DDR;                       /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC);                /* D/CX */

  /* optional output pins */
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RESX */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CSX */
  #endif

  LCD_DDR = Bits;                       /* set new directions */


  /* set default levels */
  #ifdef LCD_CS
  /* disable chip */
  LCD_PORT |= (1 << LCD_CS);            /* set /CSX high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  LCD_PORT |= (1 << LCD_RES);           /* set /RESX high */
  #endif


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */

  #ifdef SPI_HARDWARE

  /*
   *  set SPI clock rate (10MHz worst case)
   *  - max. MCU clock 20MHz / 2 = 10MHz
   *  - f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1)
   */

  SPI.ClockRate = SPI_CLOCK_2X;    /* set clock rate flags */

  SPI_Clock();                     /* update SPI clock */
  #endif
}



/*
 *  send a command to the display
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/CX low */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* send command */
  SPI_Write_Byte(Cmd);             /* send command byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* send data */
  SPI_Write_Byte(Data);            /* send data byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* send data: 2 bytes */
  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */
  SPI_Write_Byte((uint8_t)Data);   /* send MSB */
  SPI_Write_Byte(Byte);            /* send LSB */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}

#endif



/* ************************************************************************
 *   low level functions for 8 bit parallel interface
 *   - LCD_PORT (LCD_DDR) for control signals
 *   - LCD_PORT2 (LCD_DDR2/LCD_PIN2) for data signals 0-7
 * ************************************************************************ */


/*
 *  protocol:
 *  - write: CSX low -> D/CX -> WRX low to init write -> set D7-0 ->
 *           rising edge of WRX triggers read
 *  - read:  CSX low -> D/CX -> RDX low to trigger output ->
 *           read D7-0 with rising edge of RDX
 *  - D/CX: high = data / low = command
 *
 *  hints:
 *  - RGB565 and RGB666 are supported when using the 8-bit parallel interface
 */


#ifdef LCD_PAR_8

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set data signals
   *  - LCD_PORT2
   */

  /* all data pins are in output mode by default */
  LCD_DDR2 = 0b11111111;                /* DB0-7 */


  /*
   *  set control signals
   *  - LCD_PORT
   */

  /* set directions */
  Bits = LCD_DDR;                       /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC) | (1 << LCD_WR);     /* D/CX, WRX */

  /* optional output pins */
  #ifdef LCD_RD
  Bits |= (1 << LCD_RD);                /* RDX */
  #endif
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RES */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CS */
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */
  Bits = LCD_PORT;                      /* get current levels */

  /* basic output pins */
  Bits |= (1 << LCD_WR);                /* set WRX high */

  /* optional output pins */
  #ifdef LCD_RD
  Bits |= (1 << LCD_RD);                /* set RDX high */
  #endif
  #ifdef LCD_CS
  /* disable chip */
  Bits |= (1 << LCD_CS);                /* set /CSX high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  Bits |= (1 << LCD_RES);               /* set /RESX high */
  #endif

  LCD_PORT = Bits;                      /* set new levels */
}



/*
 *  send a byte (data or command) to the display
 *
 *  requires:
 *  - Byte: byte value to send
 */

void LCD_SendByte(uint8_t Byte)
{
  /* set data signals */
  LCD_PORT2 = Byte;                /* DB0-7 */

  /* create write strobe (rising edge takes data in) */
  LCD_PORT &= ~(1 << LCD_WR);      /* set WRX low */
                                   /* wait 15ns */
  LCD_PORT |= (1 << LCD_WR);       /* set WRX high */

  /* data hold time 10ns */
  /* next write cycle after 15ns WRX being high */
}



/*
 *  send a command to the display
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/CX low */

  /* send command */
  LCD_SendByte(Cmd);               /* send command byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* send data */
  LCD_SendByte(Data);              /* send data byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* send data */
  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */
  LCD_SendByte((uint8_t)Data);     /* send MSB */
  LCD_SendByte(Byte);              /* send LSB */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



#if 0

/*
 *  read byte from display
 *  - timing for register data
 *  - not suitable for frame memory
 *
 *  returns:
 *  - byte
 */

uint8_t LCD_ReadByte(void)
{
  uint8_t           Byte;     /* return value */

  /* set data pins to input mode before reading */
  LCD_DDR2 = 0b00000000;         /* DB0-7 */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* start read cycle (RDX low for min. 45ns) */
  LCD_PORT &= ~(1 << LCD_RD);      /* set RDX low */

  /* wait for LCD to fetch data: max. 40ns */
  asm volatile("nop\n\t"::);       /* burn a clock cycle */

  /* read data: DB0-7 */
  Byte = LCD_PIN2;

  /* end read cycle */
  LCD_PORT |= (1 << LCD_RD);       /* set RDX high */

  /* wait for LCD to release data lines: max. 80ns */
  #ifndef LCD_CS
  /* burn two clock cycles */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    ::
  );
  #endif

  /* next read cycle after 90ns RDX being high */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif

  /* set data pins back to output mode after reading */
  LCD_DDR2 = 0b11111111;         /* DB0-7 */

  return Byte;
}

#endif

#endif



/* ************************************************************************
 *   low level functions for 16 bit parallel interface
 *   - LCD_PORT (LCD_DDR) for control signals
 *   - LCD_PORT2 (LCD_DDR2/LCD_PIN2) for data signals 0-7
 *   - LCD_PORT3 (LCD_DDR3/LCD_PIN3) for data signals 8-15
 * ************************************************************************ */


/*
 *  protocol:
 *  - write: CSX low -> D/CX -> WRX low to init write -> set D15-0 ->
 *           rising edge of WRX triggers read
 *  - D/CX: high = data / low = command
 *  - commands have to be sent as 2 bytes
 *
 *  hints:
 *  - RGB565 and RGB666 are supported when using the 16-bit parallel interface
 */


#ifdef LCD_PAR_16

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set data signals
   *  - LCD_PORT2
   *  - LCD_PORT3
   */

  /* all data pins are in output mode by default */
  LCD_DDR2 = 0b11111111;                /* DB0-7 */
  LCD_DDR3 = 0b11111111;                /* DB8-15 */


  /*
   *  set control signals
   *  - LCD_PORT
   */

  /* set directions */
  Bits = LCD_DDR;                       /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC) | (1 << LCD_WR);     /* D/CX, WRX */

  /* optional output pins */
  #ifdef LCD_RD
  Bits |= (1 << LCD_RD);                /* RDX */
  #endif
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RES */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CS */
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */
  Bits = LCD_PORT;                      /* get current levels */

  /* basic output pins */
  Bits |= (1 << LCD_WR);                /* set WRX high */

  /* optional output pins */
  #ifdef LCD_RD
  Bits |= (1 << LCD_RD);                /* set RDX high */
  #endif
  #ifdef LCD_CS
  /* disable chip */
  Bits |= (1 << LCD_CS);                /* set /CSX high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  Bits |= (1 << LCD_RES);               /* set /RESX high */
  #endif

  LCD_PORT = Bits;                      /* set new levels */
}



/*
 *  send a byte (data or command) to the display
 *
 *  requires:
 *  - Byte: byte value to send
 */

void LCD_SendByte(uint8_t Byte)
{
  /* set data signals - we have to send two bytes */
  LCD_PORT2 = Byte;                /* set LSB (DB0-7) */
  LCD_PORT3 = 0;                   /* set dummy MSB (DB8-15) */

  /* create write strobe (rising edge takes data in) */
  LCD_PORT &= ~(1 << LCD_WR);      /* set WRX low */
                                   /* wait 15ns */
  LCD_PORT |= (1 << LCD_WR);       /* set WRX high */

  /* data hold time 10ns */
  /* next write cycle after 15ns WRX being high */
}



/*
 *  send a command to the display
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/CX low */

  /* send command */
  LCD_SendByte(Cmd);               /* send command byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* send data */
  LCD_SendByte(Data);              /* send data byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the display
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* set data signals */
  LCD_PORT2 = (uint8_t)Data;       /* set LSB (DB0-7) */
  Data >>= 8;                      /* get MSB */
  LCD_PORT3 = (uint8_t)Data;       /* set MSB (DB8-15) */

  /* create write strobe (rising edge takes data in) */
  LCD_PORT &= ~(1 << LCD_WR);      /* set WRX low */
                                   /* wait 15ns */
  LCD_PORT |= (1 << LCD_WR);       /* set WRX high */

  /* data hold time 10ns */
  /* next write cycle after 15ns WRX being high */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}

#endif



/* ************************************************************************
 *   conversion functions
 * ************************************************************************ */


#ifdef COLORMODE_RGB666

/*
 *  RGB565 to RGB666
 *  - DB[15-11] -> R[5-1], DB15 -> R0
 *    R[5-0] = (R[4-0] << 1) | (R4 >> 4)
 *  - DB[10-5] -> G[5-0]
 *    G[5-0] = G[5-0]
 *  - DB[4-0] -> B[5-1], DB4 -> B0
 *    B[5-0] = (B[4-0] << 1) | (B4 >> 4)
 */

/*
 *  8-bit frame format for RGB666
 *  - first byte:  R[5-0] -> D[7-2], D[1-0] are 0
 *  - second byte: G[5-0] -> D[7-2], D[1-0] are 0
 *  - third byte:  B[5-0] -> D[7-2], D[1-0] are 0
 */


/*
 *  expand RGB565 value to RGB666 in 8-bit frame format
 *
 *  requires:
 *  - Color: RGB565 color value
 *  - RGB666: pointer to array of uint8_t for R, G and B
 */

void RGB565_2_RGB666(uint16_t Color, uint8_t *RGB666)
{
  uint8_t           R6;       /* red */
  uint8_t           G6;       /* green */
  uint8_t           B6;       /* blue */

  /*
   *  blue
   */

  /* get blue-5 and convert to blue-6 */
  B6 = Color & 0b0000000000011111;      /* get DB[4-0] */
  B6 <<= 1;                             /* shift one step left */
  if (B6 & 0b00100000)                  /* DB4/B4 set (now B5) */
  {
    B6 |= 0b00000001;                   /* set B0 (copy B4 to B0) */
  }

  /* convert to frame format */
  B6 <<= 2;                             /* shift two steps left */


  /* 
   *  green
   */

  /* get green-6 */
  Color >>= 5;                          /* shift right for G */
  G6 = Color & 0b0000000000111111;      /* get DB[10-5] */

  /* convert to frame format */
  G6 <<= 2;                             /* shift two steps left */


  /*
   *  red
   */

  /* get red-5 and convert to red-6 */
  Color >>= 6;                          /* shift right for R */
  R6 = Color & 0b0000000000011111;      /* get DB[15-11] */
  R6 <<= 1;                             /* shift one step left */
  if (R6 & 0b00100000)                  /* DB15/R4 set (now R5) */
  {
    R6 |= 0b00000001;                   /* set R0 (copy R4 to R0) */
  }

  /* convert to frame format */
  R6 <<= 2;                             /* shift two steps left */

  /* copy converted values */
  *RGB666 = R6;               /* red */
  RGB666++;                   /* next byte */
  *RGB666 = G6;               /* green */
  RGB666++;                   /* next byte */
  *RGB666 = B6;               /* blue */
}

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  set address window
 *  - 0 up to (max - 1)
 */

void LCD_AddressWindow(void)
{
  /* X -> column */
  LCD_Cmd(CMD_COL_ADDR_SET);
  LCD_Data2(X_Start);               /* start column */
  LCD_Data2(X_End);                 /* end column */

  /* Y -> page/row */
  LCD_Cmd(CMD_PAGE_ADDR_SET);
  LCD_Data2(Y_Start);               /* start page */
  LCD_Data2(Y_End);                 /* end page */
}



/*
 *  set LCD character position
 *
 *  requires:
 *  - x:  horizontal position (1-)
 *  - y:  vertical position (1-)
 */

void LCD_CharPos(uint8_t x, uint8_t y)
{
  uint16_t          Mask = 1;

  /* update UI */
  UI.CharPos_X = x;
  UI.CharPos_Y = y;

  y--;                        /* start at zero */

  /* mark text line as used */
  if (y < 16)                 /* prevent overflow */
  {
    Mask <<= y;               /* shift to bit position for line */
    LineFlags |= Mask;        /* set bit for line */
  }

  /*
   *  calculate dot position
   *  - top left of character
   */

  /* horizontal position (column) */
  x--;                        /* columns start at 0 */
  Mask = x;                   /* expand to 16 bit */
  Mask *= FONT_SIZE_X;        /* offset for character */
  X_Start = Mask;             /* update start position */

  /* vertical position (page) */
  Mask = y;                   /* expand to 16 bit */
  Mask *= FONT_SIZE_Y;        /* offset for character */
  Y_Start = Mask;             /* update start position */
}



/*
 *  clear one single character line
 *
 *  requires:
 *  - Line: line number (1-)
 *    special case line 0: clear remaining space in current line
 */ 

void LCD_ClearLine(uint8_t Line)
{
  uint16_t          x;             /* x position */
  uint8_t           y;             /* y position */
  uint8_t           Pos = 1;       /* character position */

  wdt_reset();                     /* reset watchdog */

  if (Line == 0)         /* special case: rest of current line */
  {
    Line = UI.CharPos_Y;           /* get current line */
    Pos = UI.CharPos_X;            /* get current character position */
  }

  /* text line optimization */
  if (Line <= 16)                  /* prevent overflow */
  {
    y = Line - 1;                  /* bitfield starts at zero */
    x = 1;                         /* set start bit */
    x <<= y;                       /* bit for this line */

    if (! (LineFlags & x))         /* bit not set */
    {
      /* empty text line, already cleared */
      return;                      /* nothing do to */
    }
    else if (Pos == 1)             /* bit set and complete line */
    {
      /* we'll clear this line completely */
      LineFlags &= ~x;             /* clear bit */
    }
  }

  /* manage address window */
  LCD_CharPos(Pos, Line);          /* update character position */
                                   /* also updates X_Start and Y_Start */
  X_End = LCD_PIXELS_X - 1;             /* last column */
  Y_End = Y_Start + FONT_SIZE_Y - 1;    /* last row */
  y = FONT_SIZE_Y;                      /* set default */

  /* partial text line at bottom of display */
  if (Y_End > (LCD_PIXELS_Y - 1))       /* row overflow */
  {
    x = Y_End - (LCD_PIXELS_Y - 1);     /* difference */
    y -= (uint8_t)x;                    /* adjust number of rows */
    Y_End = LCD_PIXELS_Y - 1;           /* set last row */    
  }

  LCD_AddressWindow();                  /* set window */

  #ifdef COLORMODE_RGB666
  /* convert RGB565 color to RGB666 */ 
  RGB565_2_RGB666(COLOR_BACKGROUND, &RGB666_BG[0]);
  #endif

  /* clear all pixels in window */
  LCD_Cmd(CMD_MEM_WRITE);          /* start writing */

  while (y > 0)                    /* character height (pages) */
  {
    x = X_Start;                   /* reset start position */
    while (x < LCD_PIXELS_X)       /* all columns */
    {
      /* send background color */
      #if defined (COLORMODE_RGB565)
      LCD_Data2(COLOR_BACKGROUND);      /* RGB565 */
      #elif defined (COLORMODE_RGB666)
      LCD_Data(RGB666_BG[0]);           /* R6 */
      LCD_Data(RGB666_BG[1]);           /* G6 */
      LCD_Data(RGB666_BG[2]);           /* B6 */
      #endif

      x++;                              /* next column */
    }

    y--;                           /* next page */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           n = 1;         /* counter */

  /* we have to clear all dots manually :-( */
  while (n <= (LCD_CHAR_Y + 1))    /* for all text lines */
  {
    /* +1 is for a possible partial line at the bottom */

    LCD_ClearLine(n);              /* clear line */
    n++;                           /* next line */
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  initialize LCD
 */
 
void LCD_Init(void)
{
  uint8_t           Bits;          /* register bits / counter */


  /*
   *  reset display controller
   */

  #ifdef LCD_RES
  /* hardware reset */
  LCD_PORT &= ~(1 << LCD_RES);          /* set /RESX low */
  wait10ms();                           /* wait 10ms */
  LCD_PORT |= (1 << LCD_RES);           /* set /RESX high */
  MilliSleep(120);                      /* wait 120ms */
  #else
  /* software reset */
  LCD_Cmd(CMD_RESET);                   /* perform software reset */
  MilliSleep(120);                      /* wait 120ms */
  #endif


  /* 
   *  set registers of display controller
   */

  #if 0
  /* interface mode control */
  LCD_Cmd(CMD_IF_MODE_CTRL);
  LCD_Data(0);                          /* reset (default) */
  #endif

  /* pixel format for RGB image data */
  #if defined (COLORMODE_RGB565)
  LCD_Cmd(CMD_SET_PIX_FORMAT);
  LCD_Data(FLAG_DBI_16);                /* 16 bits / RGB565 */
  #elif defined (COLORMODE_RGB666)
  LCD_Cmd(CMD_SET_PIX_FORMAT);
  LCD_Data(FLAG_DBI_18);                /* 18 bits / RGB666 */
  #endif

  /* power control 1 */
  LCD_Cmd(CMD_POWER_CTRL_1);
  LCD_Data(FLAG_VRH1_365);              /* 1.25 x 3.65 = 4.5625 V */
  LCD_Data(FLAG_VRH2_365);              /* -1.25 x 3.65 = -4.5625 V */

  /* power control 2 */
  LCD_Cmd(CMD_POWER_CTRL_2);
  LCD_Data(FLAG_POWER_CTRL2 | FLAG_BT_4);    /* step-up factor */

  /* VCOM control */
  LCD_Cmd(CMD_VCOM_CTRL);
  LCD_Data(0);                          /* pseudo read */
  LCD_Data(FLAG_VCOM_146875);           /* factor -1.46875 */
  LCD_Data(FLAG_VCOM_REG);              /* use value from register */
  LCD_Data(0);                          /* pseudo read */

  #if 0
  /* display inversion control */
  LCD_Cmd(CMD_INVERS_CTRL);
  LCD_Data(FLAG_DINV_2DOT);             /* 2-dot inversion (default) */
  #endif

  /* entry mode */
  LCD_Cmd(CMD_ENTRY_MODE);
  LCD_Data(FLAG_DTE_GON_3 | FLAG_EPF_2);     /* normal display */
  /* and standard RGB565-to-RGB666 conversion method  */

  /* memory access control */
  LCD_Cmd(CMD_MEM_CTRL);
  #ifdef LCD_BGR
    /* reverse red and blue color channels */
    Bits = FLAG_COLOR_BGR;           /* color bit order: BGR */
  #else
    Bits = FLAG_COLOR_RGB;           /* color bit order: RGB */
  #endif
  #ifdef LCD_ROTATE
    Bits |= FLAG_XY_REV;             /* swap x and y */
  #endif
  #ifdef LCD_FLIP_X 
    Bits |= FLAG_COL_REV;            /* flip x */
  #endif
  #ifdef LCD_FLIP_Y
    Bits |= FLAG_PAGE_REV;           /* flip y */
  #endif
  LCD_Data(Bits);                  /* send parameter bits */

  /* address window */
  X_Start = 0;
  X_End = LCD_PIXELS_X - 1;
  Y_Start = 0;
  Y_End = LCD_PIXELS_Y - 1;
  LCD_AddressWindow();

  /* power on */
  LCD_Cmd(CMD_SLEEP_OUT);          /* exit sleep mode */
  MilliSleep(120);                 /* pause for 120ms */
  LCD_Cmd(CMD_DISPLAY_ON);         /* enable display output */


  /*
   *  init driver internals
   */

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  #ifdef SW_SYMBOLS
  UI.SymbolSize_X = LCD_SYMBOL_CHAR_X;  /* x size in chars */
  UI.SymbolSize_Y = LCD_SYMBOL_CHAR_Y;  /* y size in chars */
  #endif

  /* init character stuff */
  LineFlags = 0xffff;              /* clear all lines by default */
  LCD_CharPos(1, 1);               /* reset character position */

  #if defined (LCD_PAR_8) || defined (LCD_PAR_16)
  /* clear display only for fast interfaces */
  LCD_Clear();                     /* clear display */
  #endif
}



/*
 *  display a single character
 *
 *  requires:
 *  - Char: character to display
 */

void LCD_Char(unsigned char Char)
{
  uint8_t           *Table;        /* pointer to table */
  uint8_t           Index;         /* font index */
  uint16_t          Offset;        /* address offset */
  uint8_t           Pixels;        /* pixels in y direction */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */
  uint8_t           Bits;          /* number of bits to be sent */
  uint8_t           n;             /* bitmap bit counter */

  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /* get font index number from lookup table */
  Table = (uint8_t *)&FontTable;        /* start address */
  Table += Char;                        /* add offset for character */
  Index = pgm_read_byte(Table);         /* get index number */
  if (Index == 0xff) return;            /* no character bitmap available */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&FontData;        /* start address of font data */
  Offset = FONT_BYTES_N * Index;       /* offset for character */
  Table += Offset;                     /* address of character data */

  /* LCD's address window */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);   /* update character position */
                                             /* also updates X_Start and Y_Start */
  X_End = X_Start + FONT_SIZE_X - 1;   /* offset for end */
  Y_End = Y_Start + FONT_SIZE_Y - 1;   /* offset for end */
  LCD_AddressWindow();                 /* set address window */

  #ifdef LCD_COLOR
  /* color feature enabled */
  Offset = UI.PenColor;                 /* get pen color */
  #else
  /* color feature disabled */
  Offset = COLOR_PEN;                   /* use default pen color */
  #endif

  #ifdef COLORMODE_RGB666
  /* convert RGB565 colors to RGB666 */ 
  RGB565_2_RGB666(COLOR_BACKGROUND, &RGB666_BG[0]);
  RGB565_2_RGB666(Offset, &RGB666_FG[0]);
  #endif

  LCD_Cmd(CMD_MEM_WRITE);              /* start writing */

  /* read character bitmap and send it to display */
  while (y <= FONT_BYTES_Y)
  {
    Pixels = FONT_SIZE_X;               /* track x bits to be sent */
    x = 1;                              /* reset counter */

    /* read and send all bytes for this row */
    while (x <= FONT_BYTES_X)
    {
      /* track x bits */
      if (Pixels >= 8)             /* a byte or more left */
      {
        Bits = 8;                  /* send full byte */
      }
      else                         /* less than a byte left */
      {
        Bits = Pixels;             /* send remaining bits */
      }
      Pixels -= Bits;              /* update counter */

      Index = pgm_read_byte(Table);     /* read byte */

      /* send color for each bit */
      n = Bits;
      while (n > 0)
      {
        if (Index & 0b00000001)         /* bit set */
        {
          /* send foreground/pen color */
          #if defined (COLORMODE_RGB565)
          LCD_Data2(Offset);            /* RGB565 */
          #elif defined (COLORMODE_RGB666)
          LCD_Data(RGB666_FG[0]);       /* R6 */
          LCD_Data(RGB666_FG[1]);       /* G6 */
          LCD_Data(RGB666_FG[2]);       /* B6 */
          #endif
        }
        else                            /* bit unset */
        {
          /* send background color */
          #if defined (COLORMODE_RGB565)
          LCD_Data2(COLOR_BACKGROUND);  /* RGB565 */
          #elif defined (COLORMODE_RGB666)
          LCD_Data(RGB666_BG[0]);       /* R6 */
          LCD_Data(RGB666_BG[1]);       /* G6 */
          LCD_Data(RGB666_BG[2]);       /* B6 */
          #endif
        }

        Index >>= 1;                      /* shift byte for next bit */
        n--;                              /* next bit */
      }

      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    y++;                                /* next row */
  }

  UI.CharPos_X++;             /* update character position */
}



/*
 *  set cursor
 *
 *  required:
 *  - Mode: cursor mode
 *    0: cursor on
 *    1: cursor off
 */

void LCD_Cursor(uint8_t Mode)
{
  LCD_CharPos(LCD_CHAR_X, LCD_CHAR_Y);     /* move to bottom right */

  if (Mode)              /* cursor on */
  {
    LCD_Char('>');
  }
  else                   /* cursor off */
  {
    LCD_Char(' ');
  }
}



/* ************************************************************************
 *   fancy stuff
 * ************************************************************************ */


#ifdef SW_SYMBOLS

/*
 *  display a component symbol
 *
 *  requires:
 *  - ID: symbol to display
 */

void LCD_Symbol(uint8_t ID)
{
  uint8_t           *Table;        /* pointer to symbol table */
  uint8_t           *Table2;       /* pointer */
  uint8_t           Data;          /* symbol data */
  uint16_t          Offset;        /* address offset */
  uint8_t           Pixels;        /* pixels in y direction */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */
  uint8_t           Bits;          /* number of bits to be sent */
  uint8_t           n;             /* bitmap bit counter */
  uint8_t           Factor = SYMBOL_RESIZE;  /* resize factor */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&SymbolData;       /* start address of symbol data */
  Offset = SYMBOL_BYTES_N * ID;         /* offset for symbol */
  Table += Offset;                      /* address of symbol data */

  /* LCD's address window */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);   /* update character position */
                                             /* also updates X_Start and Y_Start */
  X_End = X_Start + (SYMBOL_SIZE_X * SYMBOL_RESIZE) - 1;  /* offset for end */
  Y_End = Y_Start + (SYMBOL_SIZE_Y * SYMBOL_RESIZE) - 1;  /* offset for end */
  LCD_AddressWindow();                  /* set address window */

  #ifdef LCD_COLOR
  /* color feature enabled */
  Offset = UI.PenColor;                 /* get pen color */
  #else
  /* color feature disabled */
  Offset = COLOR_PEN;                   /* use default pen color */
  #endif

  #ifdef COLORMODE_RGB666
  /* convert RGB565 colors to RGB666 */ 
  RGB565_2_RGB666(COLOR_BACKGROUND, &RGB666_BG[0]);
  RGB565_2_RGB666(Offset, &RGB666_FG[0]);
  #endif

  LCD_Cmd(CMD_MEM_WRITE);               /* start writing */

  /* read character bitmap and send it to display */
  while (y <= SYMBOL_BYTES_Y)
  {
    Table2 = Table;           /* save current pointer */

    while (Factor > 0)        /* resize symbol */
    {
      Table = Table2;                   /* reset start pointer */

      Pixels = SYMBOL_SIZE_X;           /* x bits to be sent */
      x = 1;                            /* reset counter */

      /* read and send all bytes for this row */
      while (x <= SYMBOL_BYTES_X)
      {
        /* track x bits */
        if (Pixels >= 8)           /* a byte or more left */
        {
          Bits = 8;                /* send full byte */
        }
        else                       /* less than a byte left */
        {
          Bits = Pixels;           /* send remaining bits */
        }
        Pixels -= Bits;            /* update counter */

        Data = pgm_read_byte(Table);    /* read byte */

        /* send color for each bit */
        n = Bits;                       /* reset counter */
        n *= SYMBOL_RESIZE;             /* and consider resize factor */

        while (n > 0)                   /* x pixels */
        {
          if (Data & 0b00000001)             /* bit set */
          {
            /* send foreground/pen color */
            #if defined (COLORMODE_RGB565)
            LCD_Data2(Offset);               /* RGB565 */
            #elif defined (COLORMODE_RGB666)
            LCD_Data(RGB666_FG[0]);          /* R6 */
            LCD_Data(RGB666_FG[1]);          /* G6 */
            LCD_Data(RGB666_FG[2]);          /* B6 */
            #endif
          }
          else                               /* bit unset */
          {
            /* send background color */
            #if defined (COLORMODE_RGB565)
            LCD_Data2(COLOR_BACKGROUND);     /* RGB565 */
            #elif defined (COLORMODE_RGB666)
            LCD_Data(RGB666_BG[0]);          /* R6 */
            LCD_Data(RGB666_BG[1]);          /* G6 */
            LCD_Data(RGB666_BG[2]);          /* B6 */
            #endif
          }

          n--;                          /* next pixel */

          if (n % SYMBOL_RESIZE == 0)   /* for every resize step */
          {
            Data >>= 1;                 /* shift byte for next bit */
          }
        }

        Table++;                        /* address for next byte */
        x++;                            /* next byte */
      }

      Factor--;                    /* one y resizing step done */
    }

    /* manage y direction */
    if (Factor == 0)               /* all y resizing steps done */
    {
      Factor = SYMBOL_RESIZE;      /* reset resize factor */
      y++;                         /* next row */
    }              
  }

  /* mark text lines as used */
  n = LCD_SYMBOL_CHAR_Y;           /* set line counter */
  x = UI.SymbolPos_Y;              /* start line */
  while (n > 1)                    /* first line already set */
  {
    x++;                           /* next line */
    LCD_CharPos(1, x);             /* mark line */
    n--;                           /* next line */
  }
}

#endif



#ifdef FUNC_COLORCODE

/*
 *  draw filled box
 *  - takes X_start, X_End, Y_Start and Y_End as coordinates 
 *
 *  requires:
 *  - Color: RGB565 color code
 */

void LCD_Box(uint16_t Color)
{
  uint16_t          x_Size;             /* x size */
  uint16_t          x;                  /* x counter */
  uint16_t          y_Size;             /* y size/counter */

  LCD_AddressWindow();             /* set address window */

  /* calculate sizes */
  x_Size = X_End - X_Start + 1;
  y_Size = Y_End - Y_Start + 1;

  #ifdef COLORMODE_RGB666
  /* convert RGB565 color to RGB666 */ 
  RGB565_2_RGB666(Color, &RGB666_FG[0]);
  #endif

  LCD_Cmd(CMD_MEM_WRITE);          /* start writing */

  while (y_Size > 0)               /* loop trough rows */
  {
    x = x_Size;                    /* reset counter */    

    while (x > 0)                  /* loop through columns */
    {
      /* send box color */
      #if defined (COLORMODE_RGB565)
      LCD_Data2(Color);            /* RGB565 */
      #elif defined (COLORMODE_RGB666)
      LCD_Data(RGB666_FG[0]);      /* R6 */
      LCD_Data(RGB666_FG[1]);      /* G6 */
      LCD_Data(RGB666_FG[2]);      /* B6 */
      #endif

      x--;                         /* next one */
    }

    y_Size--;                      /* next one */
  }
}



/*
 *  display color band of a component color code
 *  - aligned to charactor position
 *  - size: 2x1 chars
 *
 *  requires:
 *  - Color: RGB565 color code
 *  - Align: ALIGN_LEFT   align band left
 *           ALIGN_RIGHT  align band right
 */

void LCD_Band(uint16_t Color, uint8_t Align)
{
  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /* update character position, also updates X_Start and Y_Start */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);

  /*
   *  box for component body
   *  - height: nearly one char
   *    top and bottom margin: 1/8 char heigth
   *  - width: two chars
   */
 
  X_End = X_Start + (2 * FONT_SIZE_X) - 1;   /* offset for end */
  /* offset for end & bottom margin */
  Y_End = Y_Start + FONT_SIZE_Y - 1 - (FONT_SIZE_Y / 8);   
  Y_Start += (FONT_SIZE_Y / 8);              /* top margin */

  /* draw body using the component's body color */
  LCD_Box(COLOR_CODE_NONE);


  /*
   *  box for band
   *  - heigth: same as body but -1 dot at top and bottom
   *  - width: 1 char
   *    left and right margin: 1/3 char width
   */

  /* create thin outline */
  Y_Start += 1;
  Y_End -= 1;

  if (Align == ALIGN_LEFT)    /* align band left */
  {
    X_Start += (FONT_SIZE_X / 3);       /* left margin */
    X_End = X_Start + FONT_SIZE_X - 1;  /* offset for end */
  }
  else                        /* align band right */
  {
    X_End -= (FONT_SIZE_X / 3);         /* right margin */
    X_Start = X_End - FONT_SIZE_X + 1;  /* offset for start */
  }

  /* draw band */
  LCD_Box(Color);

  UI.CharPos_X += 2;          /* update character position */
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_DRIVER_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   driver functions for R&D Display
 *   - for identifying display controllers
 *   - x * y pixels
 *   - interfaces
 *     - 8 bit parallel in 8080 mode
 *     - 8 bit parallel in 6800 mode (untested)
 *     - 4 line SPI (untested)
 *
 *   (c) 2020-2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI
 *    /RES         LCD_RES (optional)
 *    /CS          LCD_CS (optional)
 *    D/C          LCD_DC
 *    SCK          LCD_SCK
 *    SDI          LCD_SDI
 *    SDO          LCD_SDO
 *    For hardware SPI LCD_SCK, LCD_SDI and LCD_SDO have to be the MCU's
 *    SCK, MOSI and MISO pins. 
 *  - max. SPI clock: 10MHz write and 6.6MHz read
 *  - pin assignment for 8 bit parallel interface in 8080 mode
 *             LCD_PORT/LCD_DDR:
 *    /RES     LCD_RES (optional)
 *    /CS      LCD_CS (optional)
 *    D/C      LCD_DC
 *    WR       LCD_WR
 *    RD       LCD_RD (optional)
 *             LCD_PORT2/LCD_DDR2/LCD_PIN2:
 *    DB0      LCD_DB0 (LCD_PORT2 pin #0)
 *    DB1      LCD_DB1 (LCD_PORT2 pin #1)
 *    DB2      LCD_DB2 (LCD_PORT2 pin #2)
 *    DB3      LCD_DB3 (LCD_PORT2 pin #3)
 *    DB4      LCD_DB4 (LCD_PORT2 pin #4)
 *    DB5      LCD_DB5 (LCD_PORT2 pin #5)
 *    DB6      LCD_DB6 (LCD_PORT2 pin #6)
 *    DB7      LCD_DB7 (LCD_PORT2 pin #7)
 *  - max. clock rate for parallel bus
 *    15MHz write
 *    6.25MHz read register data
 *    2.2MHz read frame memory
 *  - pin assignment for 8 bit parallel interface in 6800 mode
 *             LCD_PORT/LCD_DDR:
 *    /RES     LCD_RES (optional)
 *    /CS      LCD_CS (optional)
 *    D/C      LCD_DC
 *    R/W      LCD_RW
 *    E        LCD_E
 *             LCD_PORT2/LCD_DDR2/LCD_PIN2:
 *    DB0      LCD_DB0 (LCD_PORT2 pin #0)
 *    DB1      LCD_DB1 (LCD_PORT2 pin #1)
 *    DB2      LCD_DB2 (LCD_PORT2 pin #2)
 *    DB3      LCD_DB3 (LCD_PORT2 pin #3)
 *    DB4      LCD_DB4 (LCD_PORT2 pin #4)
 *    DB5      LCD_DB5 (LCD_PORT2 pin #5)
 *    DB6      LCD_DB6 (LCD_PORT2 pin #6)
 *    DB7      LCD_DB7 (LCD_PORT2 pin #7)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_RD_DISPLAY


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
//#ifndef FONT_SET
//  #error <<< No font selected! >>>
//#endif
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

#define LCD_CHAR_X            16
#define LCD_CHAR_Y            8


/* component symbols */
#ifdef SW_SYMBOLS
  /* resize symbols by a factor of 2 */
  #define SYMBOL_RESIZE       2

  /* size in relation to a character */
  #define LCD_SYMBOL_CHAR_X   SYMBOL_RESIZE
  #define LCD_SYMBOL_CHAR_Y   SYMBOL_RESIZE
#endif



/*
 *  local constants
 */

/* command mode (bitfield) */
#define CMD_MODE_8            0b00000001     /* 8 bits */
#define CMD_MODE_16           0b00000010     /* 16 bits */

/* control line management (bitfield) */
  /* chip select: /CS */
#define DISP_SELECT           0b00000001     /* select controller */
#define DISP_DESELECT         0b00000010     /* deselect controller */
  /* read/write mode and data line direction */
#define DISP_READ             0b00000100     /* read mode / lines: input */
#define DISP_WRITE            0b00001000     /* write mode / lines: output */



/*
 *  local variables
 */

/* command mode (bitfield) */
uint8_t             CmdMode;       /* 8 or 16 bits */

/* address window */
uint16_t            X_Start;       /* start position X (column) */
uint16_t            X_End;         /* end position X (column) */
uint16_t            Y_Start;       /* start position Y (page/row) */
uint16_t            Y_End;         /* end position Y (page/row) */

/* text line management */
uint16_t            LineFlags;     /* bitfield for up to 16 lines */



/* ************************************************************************
 *   low level functions for 4 line SPI interface
 * ************************************************************************ */


/*
 *  protocol:
 *  - CSX -> D/CX -> D7-0 with rising edge of SCL
 *  - D/CX: high = data / low = command
 */

#ifdef LCD_SPI

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
  Bits |= (1 << LCD_DC);                /* D/C */

  /* optional output pins */
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RES */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CS */
  #endif

  LCD_DDR = Bits;                       /* set new directions */


  /* set default levels */
  #ifdef LCD_CS
  /* disable chip */
  LCD_PORT |= (1 << LCD_CS);            /* set /CS high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  LCD_PORT |= (1 << LCD_RES);           /* set /RES high */
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
 *  manage control lines
 *
 *  requires:
 *  - Mode: control flags (bitfield)
 *    DISP_SELECT    select controller
 *    DISP_DESELECT  deselect controller
 *    DISP_WRITE     write mode
 *    DISP_READ      read mode
 */

void LCD_Control(uint8_t Mode)
{
  /*
   *  We can ignore DISP_WRITE and DISP_READ because SPI uses two separate
   *  lines for reading and writing.
   */

  #ifdef LCD_CS
  if (Mode & DISP_SELECT)          /* select controller */
  {
    /* select chip */
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CS low */
  }
  else if (Mode & DISP_DESELECT)   /* deselect controller */
  {
    /* select chip */
    LCD_PORT |= (1 << LCD_CS);     /* set /CS high */
  }
  #endif
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/C low */

  SPI_Write_Byte(Cmd);             /* write command byte */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  SPI_Write_Byte(Data);            /* write data byte */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */

  SPI_Write_Byte((uint8_t)Data);   /* write MSB of data */
  SPI_Write_Byte(Byte);            /* write LSB of data */
}



#ifdef LCD_READ

/*
 *  read byte from display
 *
 *  returns:
 *  - byte
 */

uint8_t LCD_ReadByte(void)
{
  uint8_t           Byte;     /* return value */

  Byte = SPI_WriteRead_Byte(0);    /* send dummy byte and read byte */

  return Byte;
}

#endif

#endif



/* ************************************************************************
 *   low level functions for 8 bit parallel interface in 8080 mode
 *   - LCD_PORT (LCD_DDR) for control signals
 *   - LCD_PORT2 (LCD_DDR2/LCD_PIN2) for data signals 0-7
 * ************************************************************************ */


/*
 *  protocol:
 *  - write: CS low -> D/C -> WR low to init write -> set D7-0 ->
 *           rising edge of WR triggers read
 *  - read:  CS low -> D/C -> RD low to trigger output ->
 *           read D7-0 with rising edge of RD
 *  - D/C: high = data / low = command
 *  - commands may have to be sent as 2 bytes for some controllers
 */


#ifdef LCD_PAR_8_8080

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
  Bits |= (1 << LCD_DC) | (1 << LCD_WR);     /* D/C, WR */

  /* optional output pins */
  #ifdef LCD_RD
  Bits |= (1 << LCD_RD);                /* RD */
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
  Bits |= (1 << LCD_RD);                /* set RD high */
  #endif
  #ifdef LCD_CS
  /* disable chip */
  Bits |= (1 << LCD_CS);                /* set /CS high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  Bits |= (1 << LCD_RES);               /* set /RES high */
  #endif

  LCD_PORT = Bits;                      /* set new levels */
}



/*
 *  manage control lines
 *
 *  requires:
 *  - Mode: control flags (bitfield)
 *    DISP_SELECT    select controller
 *    DISP_DESELECT  deselect controller
 *    DISP_WRITE     write mode
 *    DISP_READ      read mode
 */

void LCD_Control(uint8_t Mode)
{
  #ifdef LCD_READ
  if (Mode & DISP_WRITE)           /* start writing / end reading */
  {
    /* set data pins to output mode before writing */
    LCD_DDR2 = 0b11111111;         /* DB0-7 */
  }
  else if (Mode & DISP_READ)       /* start reading / end writing */
  {
    /* set data pins to input mode before reading */
    LCD_DDR2 = 0b00000000;         /* DB0-7 */
  }
  #endif

  #ifdef LCD_CS
  if (Mode & DISP_SELECT)          /* select controller */
  {
    /* select chip */
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CS low */
  }
  else if (Mode & DISP_DESELECT)   /* deselect controller */
  {
    /* select chip */
    LCD_PORT |= (1 << LCD_CS);     /* set /CS high */
  }
  #endif
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
  LCD_PORT &= ~(1 << LCD_WR);      /* set WR low */
                                   /* wait 15ns */
  LCD_PORT |= (1 << LCD_WR);       /* set WR high */

  /* data hold time 10ns */
  /* next write cycle after 15ns WR being high */
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
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/C low */

  if (CmdMode & CMD_MODE_8)        /* 8 bit command */
  {
    /* send command: 1 byte */
    LCD_SendByte(Cmd);             /* send command byte */
  }
  else if (CmdMode & CMD_MODE_16)  /* 16 bit command */
  {
    /* send command: 2 bytes */
    LCD_SendByte(0);               /* send dummy MSB */
    LCD_SendByte(Cmd);             /* send command byte as LSB */
  }
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
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* send data */
  LCD_SendByte(Data);              /* send data byte */
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
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* send data */
  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */
  LCD_SendByte((uint8_t)Data);     /* send MSB */
  LCD_SendByte(Byte);              /* send LSB */
}



#ifdef LCD_READ

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

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* start read cycle (RD low for min. 45ns) */
  LCD_PORT &= ~(1 << LCD_RD);      /* set RD low */

  /* wait for display to fetch data: max. 40ns */
  /* but we wait a little be longer, just in case (min. 150ns) */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    ::
  );

  /* read data: DB0-7 */
  Byte = LCD_PIN2;

  /* end read cycle */
  LCD_PORT |= (1 << LCD_RD);       /* set RD high */

  /* wait for display to release data lines: max. 80ns */
  #ifndef LCD_CS
  /* burn two clock cycles */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    ::
  );
  #endif

  /* next read cycle after 90ns RD being high */

  return Byte;
}

#endif

#endif




/* ************************************************************************
 *   low level functions for 8 bit parallel interface in 6800 mode
 *   - LCD_PORT (LCD_DDR) for control signals
 *   - LCD_PORT2 (LCD_DDR2/LCD_PIN2) for data signals 0-7
 * ************************************************************************ */


/*
 *  protocol:
 *  - write: CS low -> D/C -> R/W low -> E high -> set D7-0 ->
 *           falling edge of E triggers read
 *  - read:  CS low -> D/C -> R/W high -> E high ->
 *           read D7-0 with falling edge of E
 *  - D/C: high = data / low = command
 *  - commands may have to be sent as 2 bytes for some controllers
 */


#ifdef LCD_PAR_8_6800

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
  Bits |= (1 << LCD_DC) | (1 << LCD_RW) | (1 << LCD_E);   /* D/C, R/W, E */

  /* optional output pins */
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
  Bits &= ~(1 << LCD_E);                /* set E low */

  /* optional output pins */
  #ifdef LCD_CS
  /* disable chip */
  Bits |= (1 << LCD_CS);                /* set /CS high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  Bits |= (1 << LCD_RES);               /* set /RES high */
  #endif

  LCD_PORT = Bits;                      /* set new levels */
}



/*
 *  manage control lines
 *
 *  requires:
 *  - Mode: control flags (bitfield)
 *    DISP_SELECT    select controller
 *    DISP_DESELECT  deselect controller
 *    DISP_WRITE     write mode
 *    DISP_READ      read mode
 */

void LCD_Control(uint8_t Mode)
{
  #ifdef LCD_READ
  if (Mode & DISP_WRITE)           /* start writing / end reading */
  {
    /* set data pins to output mode before writing */
    LCD_DDR2 = 0b11111111;         /* DB0-7 */
  }
  else if (Mode & DISP_READ)       /* start reading / end writing */
  {
    /* set data pins to input mode before reading */
    LCD_DDR2 = 0b00000000;         /* DB0-7 */
  }
  #endif

  #ifdef LCD_CS
  if (Mode & DISP_SELECT)          /* select controller */
  {
    /* select chip */
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CS low */
  }
  else if (Mode & DISP_DESELECT)   /* deselect controller */
  {
    /* select chip */
    LCD_PORT |= (1 << LCD_CS);     /* set /CS high */
  }
  #endif
}



/*
 *  send a byte (data or command) to the display
 *
 *  requires:
 *  - Byte: byte value to send
 */

void LCD_SendByte(uint8_t Byte)
{
  /* claim bus */
  LCD_PORT |= (1 << LCD_E);        /* set E high */

  /* set data signals */
  LCD_PORT2 = Byte;                /* DB0-7 */

  /* trigger read (falling edge takes data in) */
  LCD_PORT &= ~(1 << LCD_E);       /* set E low */

  /* data hold time 10ns */
  /* next write cycle after 15ns E being low */
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
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/C low */

  if (CmdMode & CMD_MODE_8)        /* 8 bit command */
  {
    /* send command: 1 byte */
    LCD_SendByte(Cmd);             /* send command byte */
  }
  else if (CmdMode & CMD_MODE_16)  /* 16 bit command */
  {
    /* send command: 2 bytes */
    LCD_SendByte(0);               /* send dummy MSB */
    LCD_SendByte(Cmd);             /* send command byte as LSB */
  }
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
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* send data */
  LCD_SendByte(Data);              /* send data byte */
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
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* send data */
  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */
  LCD_SendByte((uint8_t)Data);     /* send MSB */
  LCD_SendByte(Byte);              /* send LSB */
}



#ifdef LCD_READ

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

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  /* start read cycle */
  LCD_PORT |= (1 << LCD_E);        /* set E high */

  /* wait for display to fetch data: min. 90ns */
  /* but we wait a little be longer, just in case (min. 150ns) */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    ::
  );

  /* read data: DB0-7 */
  Byte = LCD_PIN2;

  /* end read cycle */
  LCD_PORT &= ~(1 << LCD_E);       /* set E low */

  /* wait for display to release data lines: max. 80ns */
  #ifndef LCD_CS
  /* burn two clock cycles */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    ::
  );
  #endif

  return Byte;
}

#endif

#endif



/* ************************************************************************
 *   debugging support
 * ************************************************************************ */


#ifdef SW_DISPLAY_ID

/*
 *  try to read ID of display controller
 *  - sets Cfg.DisplayID: ID (hex)
 */

void Display_ID(void)
{
  uint16_t          ID;            /* controller ID */
  uint8_t           Flag = 0;      /* control flag */

  /*
   *  We try different "Read ID" commands to detect the display controller.
   *  And we'll keep non-empty data/ID if no controller is identified.
   *
   *  Do some controllers not support /CS pauses?
   */


  /*
   *  Command 0x00 #1
   *  - ILI9325, ILI9328, LGDP4535, S6D0154, SPFD5408A,
   *    ST7781?, ST7783, UC8230
   */

//  if (Flag == 0)
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0x00);                 /* command: read ID */

    /* read 2 bytes */
    LCD_Control(DISP_READ);
    ID = LCD_ReadByte();           /* byte #1: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #2: LSB */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x9325:            /* ILI9325 */
        break;

      case 0x9328:            /* ILI9328 */
        break;

      case 0x4535:            /* LGDP4535 */
        break;

      case 0x0154:            /* S6D0154 */
        break;

      case 0x5408:            /* SPFD5408A */
        break;

      case 0x7783:            /* ST7783 */
        break;

      case 0x8230:            /* UC8230 */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  #if 0
  /*
   *  Command 0x00 #2
   *  - HX8347G (0x75), HX8367A (0x67)
   *  - could be possibly moved to test above (0x7575/0x6767?)
   */

  if (Flag == 0)
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0x00);                 /* command: read ID */

    /* read 1 byte */
    LCD_Control(DISP_READ);
    ID = LCD_ReadByte();           /* byte #1: LSB */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x0075:            /* HX8347G */
        break;

      case 0x0067:            /* HX8367A */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }
  #endif


  /*
   *  Command 0x67
   *  - HX8347 A/D (0x4747)
   */

  if (Flag == 0)              /* none detected yet */
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0x67);                 /* command: read ID */

    /* read 2 bytes */
    LCD_Control(DISP_READ);
    ID = LCD_ReadByte();           /* byte #1: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #2: LSB */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x4747:            /* HX8347 A/D */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  /*
   *  Command 0xbf
   *  - HX8357B?, ILI9481, R61581 (0x1581)
   */

  if (Flag == 0)              /* none detected yet */
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0xbf);                 /* command: read ID */

    /* read 6 bytes */
    LCD_Control(DISP_READ);
    LCD_ReadByte();                /* byte #1: ignore */
    LCD_ReadByte();                /* byte #2: ignore */
    LCD_ReadByte();                /* byte #3: ignore */
    ID = LCD_ReadByte();           /* byte #4: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #5: LSB */
    LCD_ReadByte();                /* byte #6: ignore */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x8357:            /* HX8357B */
        break;

      case 0x9481:            /* ILI9481 */
        break;

      case 0x1581:            /* R61581 */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  /*
   *  Command 0xd3
   *  - ILI9341, ILI9342, ILI9486, ILI9488
   */

  if (Flag == 0)              /* none detected yet */
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0xd3);                 /* command: read ID */

    /* read 4 bytes */
    LCD_Control(DISP_READ);
    LCD_ReadByte();                /* byte #1: ignore */
    LCD_ReadByte();                /* byte #2: ignore */
    ID = LCD_ReadByte();           /* byte #3: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #4: LSB */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x9341:            /* ILI9341 */
        break;

      case 0x9342:            /* ILI9342 */
        break;

      case 0x9486:            /* ILI9486 */
        break;

      case 0x9488:            /* ILI9488 */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  /*
   *  Command 0x04
   *  - HX8357D (0x8000), ILI9340, ST7789 (0x8552)
   *  - ILI9341 (2nd time?)
   */

  if (Flag == 0)              /* none detected yet */
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0x04);                 /* command: read ID */

    /* read 4 bytes */
    LCD_Control(DISP_READ);
    LCD_ReadByte();                /* byte #1: ignore */
    LCD_ReadByte();                /* byte #2: ignore */
    ID = LCD_ReadByte();           /* byte #3: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #4: LSB */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x8000:            /* HX8357D */
        /* verify */
        /* cmd b9, send ff8357 */
        /* cmd d0, read 2 bytes: 0099 */
        break;

      case 0x9340:            /* ILI9340 */
        break;

      case 0x8552:            /* ST7789 */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  /*
   *  Command 0xef
   *  - ILI9327
   */

  if (Flag == 0)              /* none detected yet */
  {
    Flag = 1;                      /* reset flag */

    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(0xef);                 /* command: read ID */

    /* read 6 bytes */
    LCD_Control(DISP_READ);
    LCD_ReadByte();                /* byte #1: ignore */
    LCD_ReadByte();                /* byte #2: ignore */
    LCD_ReadByte();                /* byte #3: ignore */
    ID = LCD_ReadByte();           /* byte #4: MSB */
    ID <<= 8;                      /* shift to MSB */
    ID |= LCD_ReadByte();          /* byte #5: LSB */
    LCD_ReadByte();                /* byte #6: ignore */
    LCD_Control(DISP_DESELECT);

    switch (ID)                    /* check data */
    {
      case 0x0000:            /* no ID */
        Flag = 0;                  /* none detected */
        break;

      case 0x9327:            /* ILI9327 */
        break;

      default:                /* default */
        Cfg.DisplayID = ID;        /* save unknown ID */
        Flag = 0;                  /* none detected */
        break;
    }
  }


  if (Flag)                   /* controller detected */ 
  {
    Cfg.DisplayID = ID;            /* update ID */
  }
}

#endif



#ifdef SW_DISPLAY_REG

/*
 *  try to read registers of display controller and
 *  output them via TTL serial
 */

void DisplayRegisters(void)
{
  uint8_t      Run = 1;            /* loop control */
  uint8_t      Cmd = 0;            /* command/register */
  uint8_t      n;                  /* counter */
  uint8_t      Byte;               /* byte value */

  Display_Serial_Only();           /* switch output to serial */
  Serial_NewLine();                /* serial: new line */

  /* try commands/registers 0-255 */
  while (Run)
  {
    /* display command */
    Display_NextLine();            /* new line */
    Display_HexByte(Cmd);          /* display command */
    Display_Colon();               /* display: : */
    Display_Space();               /* display space */

    /* send command */
    LCD_Control(DISP_SELECT | DISP_WRITE);
    LCD_Cmd(Cmd);                  /* some command */

    /* read and display 6 bytes (MSB first) */
    LCD_Control(DISP_READ);
    n = 0;
    while (n < 6)
    {
      Byte = LCD_ReadByte();       /* read byte */
      Display_HexByte(Byte);       /* display byte */
      n++;                         /* next byte */
    }
    LCD_Control(DISP_DESELECT);

    /* loop control */
    if (Cmd == 255)                /* reached 255 */
    {
      Run = 0;                     /* end loop */
    }
    else                           /* not done yet */
    {
      Cmd++;                       /* next command */
    }
  }

  Serial_NewLine();                /* serial: new line */
  Display_LCD_Only();              /* switch output back to display */
}

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


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

  /* mark text line as used */
  if (y < 16)                 /* prevent overflow */
  {
    Mask <<= y;               /* shift to bit position for line */
    LineFlags |= Mask;        /* set bit for line */
  }

  /*
   *  no other action
   */
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


  /*
   *  no display output
   */
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
//  uint8_t           Bits;          /* register bits / counter */

  /* init command mode */
  CmdMode = CMD_MODE_8;            /* 8 bit commands */


  /*
   *  reset display controller
   */

  #ifdef LCD_RES
  /* hardware reset */
  LCD_PORT &= ~(1 << LCD_RES);          /* set /RESX low */
  wait10ms();                           /* wait 10ms */
  LCD_PORT |= (1 << LCD_RES);           /* set /RESX high */
  MilliSleep(120);                      /* wait 120ms */
  #endif

  #ifdef SW_DISPLAY_ID
  Display_ID();                         /* read controller ID */

  if (Cfg.DisplayID == 0)               /* no ID read */
  {
    /* try again using 16 bit commands */
    CmdMode = CMD_MODE_16;              /* 16 bit commands */
    Display_ID();                       /* read controller ID */
  }
  #endif


  /* 
   *  set registers of display controller
   */

  /* address window */
  X_Start = 0;
  X_End = LCD_PIXELS_X - 1;
  Y_Start = 0;
  Y_End = LCD_PIXELS_Y - 1;

  /* power on */


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

  /* clear display */
  LCD_Clear();                     /* clear display */

  #ifdef SW_DISPLAY_REG
  DisplayRegisters();              /* list registers */
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
  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /*
   *  no display output
   */

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
  /*
   *  no display output
   */
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

/* ************************************************************************
 *
 *   LCD module (HD44780 compatible)
 *
 *   (c) 2012-2013 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/* ************************************************************************
 *   constants
 * ************************************************************************ */


/*
 *  LCD commands
 */

#define CMD_CLEAR_DISPLAY     0x01    /* clear display */
#define CMD_RETURN_HOME       0x02    /* return home */
#define CMD_ENTRY_MODE_SET    0x04    /* entry mode set */
#define CMD_DISPLAY_CONTROL   0x08    /* display control */
#define CMD_SHIFT             0x10    /* shift */
#define CMD_FUNCTION_SET      0x20    /* function set */
#define CMD_SET_CG_RAM_ADDR   0x40    /* set CG RAM address (custom character) */
#define CMD_SET_DD_RAM_ADDR   0x80    /* set DD RAM address (cursor position) */


/* 
 *  LCD command flags
 */

/* entry mode set */
#define FLAG_CURSOR_DECREASE  0b00000000     /* auto-decrease cursor position */
#define FLAG_CURSOR_INCREASE  0b00000010     /* auto-increase cursor position */
#define FLAG_DISPLAY_NOSHIFT  0b00000000     /* enable display auto-shift */
#define FLAG_DISPLAY_SHIFT    0b00000001     /* disable display auto-shift */ 

/* display control */
#define FLAG_DISPLAY_OFF      0b00000000     /* display off */
#define FLAG_DISPLAY_ON       0b00000100     /* display on */
#define FLAG_CURSOR_OFF       0b00000000     /* cursor off */
#define FLAG_CURSOR_ON        0b00000010     /* cursor on */
#define FLAG_BLINK_OFF        0b00000000     /* blinking off */
#define FLAG_BLINK_ON         0b00000001     /* blinking on */

/* shift */
#define FLAG_SHIFT_CURSOR     0b00000000     /* shift cursor */
#define FLAG_SHIFT_DISPLAY    0b00001000     /* shift display */
#define FLAG_SHIFT_LEFT       0b00000000     /* shift left */
#define FLAG_SHIFT_RIGHT      0b00000100     /* shift right */

/* function set */
#define FLAG_INTERFACE_4BIT   0b00000000     /* enable 4 bit data interface */
#define FLAG_INTERFACE_8BIT   0b00010000     /* enable 8 bit data interface */
#define FLAG_LINES_1          0b00000000     /* display one line */
#define FLAG_LINES_2          0b00001000     /* display two lines */
#define FLAG_FONT_5X7         0b00000000     /* select 5x7 font */
#define FLAG_FONT_5X10        0b00000100     /* select 5x10 font */


/*
 *  custom character IDs
 */

#define LCD_CHAR_UNSET    0        /* just a place holder */
#define LCD_CHAR_DIODE1   1        /* diode icon '>|' */
#define LCD_CHAR_DIODE2   2	   /* diode icon '|<' */
#define LCD_CHAR_CAP      3        /* capacitor icon '||' */
#define LCD_CHAR_RESIS1   6        /* resistor left icon '[' */
#define LCD_CHAR_RESIS2   7        /* resistor right icon ']' */

#ifdef LCD_CYRILLIC
  #define LCD_CHAR_OMEGA  4        /* omega */ 
  #define LCD_CHAR_MICRO  5        /* µ / micro */ 
#else
  #define LCD_CHAR_OMEGA  244      /* use built-in omega */
  #define LCD_CHAR_MICRO  228      /* use built-in µ */
#endif

#define LCD_CHAR_DEGREE	  0xdf     /* use built-in degree */


/*
 *  pin assignments
 *  - change if required
 */
 
#define LCD_PORT      PORTD   /* port used: */
                              /* - lower 4 bits for data interface */
                              /* - upper 4 bits for control lines (see below) */
#define LCD_DDR       DDRD    /* data direction register for used port */
#define LCD_RS        PD4     /* port pin used for RS */
#define LCD_EN1       PD5     /* port pin used for E */



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

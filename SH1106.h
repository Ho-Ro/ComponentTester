/* ************************************************************************
 *
 *   SH1106 OLED graphic display controller
 *
 *   (c) 2020 by Markus Reschke
 *
 * ************************************************************************ */


/* ************************************************************************
 *   I2C
 * ************************************************************************ */


/*
 *  I2C slave addresses
 */

#define SH1106_I2C_ADDR_0     0b00111100     /* SA0=0 0x3c */
#define SH1106_I2C_ADDR_1     0b00111101     /* SA0=1 0x3d */


/*
 *  I2C control byte
 */

#define LCD_CONTROL_BYTE      0b00000000     /* 0x00 */
/* continuation flag */
#define FLAG_CTRL_MULTI       0b00000000     /* stream of bytes follows */
#define FLAG_CTRL_SINGLE      0b10000000     /* only 1 byte follows */
/* D/C */
#define FLAG_CTRL_CMD         0b00000000     /* command follows */
#define FLAG_CTRL_DATA        0b01000000     /* data follows */



/* ************************************************************************
 *   commands
 * ************************************************************************ */


/*
 *  set column address
 *  - 2 commands (1 byte each)
 *  - valid range 0 - 131
 *    default: 0
 */

/* set lower nibble of column address (bits 0-3): bits 0-3 */
#define CMD_COLUMN_L          0b00000000     /* set lower nibble */


/* set upper nibble column address (bits 4-7): bits 0-3 */
#define CMD_COLUMN_H          0b00010000     /* set upper nibble */



/*
 *  set charge pump voltage
 *  - 1 byte command
 *  - valid range 0-3
 *    default: 2
 */

/* byte #1: command */
#define CMD_CHARGE_PUMP       0b00110000     /* set charge pump voltage */

#define FLAG_CHARGE_PUMP_64   0b00000000     /* 6.4V */
#define FLAG_CHARGE_PUMP_74   0b00000001     /* 7.4V */
#define FLAG_CHARGE_PUMP_80   0b00000010     /* 8.0V (default) */
#define FLAG_CHARGE_PUMP_90   0b00000011     /* 9.0V */


/*
 *  set display start line
 *  - 1 byte cmd
 *  - valid range: 0 - 63
 *    default: 0
 */

#define CMD_START_LINE        0b01000000     /* set start line */



/*
 *  set contrast control register
 *  - 2 byte command
 *  - valid range: 0 - 255
 *    default: 127
 */

/* byte #1: command */
#define CMD_CONTRAST          0b10000001     /* set contrast */

/* byte #2: contrast (0-255) */


/*
 *  set segment mapping
 *  - 1 byte cmd
 */

#define CMD_SEGMENT_MAP       0b10100000     /* set segment mapping */

#define FLAG_SEG_0            0b00000000     /* SEG0 = column #0 (default) */
#define FLAG_SEG_131          0b00000001     /* SEG0 = column #131 */


/*
 *  entire display on
 *  - 1 byte cmd
 */

#define CMD_PIXEL_MODE        0b10100100     /* set pixel mode */

#define FLAG_PIXEL_RAM        0b00000000     /* based on RAM (default) */
#define FLAG_PIXEL_ALL        0b00000001     /* all pixels on */


/*
 *  set normal/inverse display
 *  - 1 byte cmd
 */

#define CMD_DISP_MODE         0b10100110     /* set display mode */

#define FLAG_DISP_NORMAL      0b00000000     /* normal display (1 = pixel on, default) */
#define FLAG_DISP_INVERSE     0b00000001     /* inversed display (0 = pixel on) */


/*
 *  set multiplex ratio (N + 1)
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_MULTIPLEX_RATIO   0b10101000     /* set multiplex ratio */

/* byte #2: ratio */
/* valid range: 0 - 63 */
/* default: 63 */


/*
 *  set DC-DC converter on/off
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_DC_DC             0b10101101     /* set DC-DC converter */

/* byte #2: mode */
#define FLAG_DC_DC_OFF        0b10001010     /* disable DC-DC converter */
#define FLAG_DC_DC_ON         0b10001011     /* enable DC-DC converter */


/*
 *  set display on/off
 *  - 1 byte cmd
 */

#define CMD_DISPLAY           0b10101110     /* enable/disable display */

#define FLAG_DISPLAY_OFF      0b00000000     /* disable display (sleep mode, default) */
#define FLAG_DISPLAY_ON       0b00000001     /* enable display */


/*
 *  set page address
 *  - 1 byte cmd
 *  - valid range: 0 - 7
 */

#define CMD_PAGE              0b10110000     /* set page */


/*
 *  set COM output scan direction
 *  - 1 byte cmd
 */

#define CMD_COM_SCAN_DIR      0b11000000     /* set COM scan direction */

#define FLAG_COM_0            0b00000000     /* from COM0 to COM[N-1] (default) */
#define FLAG_COM_63           0b00001000     /* from COM[N-1] to COM0 */


/*
 *  set display offset
 *  - vertical shift (COM)
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_DISP_OFFSET       0b11010011     /* set display offset */

/* byte #2: offset */
/* valid range: 0 - 63 */
/* default: 0 */


/*
 *  set display clock
 *  - divider ratio and oscillator frequency
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_DISP_CLOCK        0b11010101     /* set display clock */

/* byte #2: divider ratio and oscillator frequency */
/* divider ratio (n + 1) for DCLK */
#define FLAG_RATIO_1          0b00000000     /* ratio 1 (default) */
#define FLAG_RATIO_2          0b00000001     /* ratio 2 */
#define FLAG_RATIO_3          0b00000010     /* ratio 3 */
#define FLAG_RATIO_4          0b00000010     /* ratio 4 */
#define FLAG_RATIO_5          0b00000100     /* ratio 5 */
#define FLAG_RATIO_6          0b00000101     /* ratio 6 */
#define FLAG_RATIO_7          0b00000110     /* ratio 7 */
#define FLAG_RATIO_8          0b00000111     /* ratio 8 */
#define FLAG_RATIO_9          0b00001000     /* ratio 9 */
#define FLAG_RATIO_10         0b00001001     /* ratio 10 */
#define FLAG_RATIO_11         0b00001010     /* ratio 11 */
#define FLAG_RATIO_12         0b00001011     /* ratio 12 */
#define FLAG_RATIO_13         0b00001100     /* ratio 13 */
#define FLAG_RATIO_14         0b00001101     /* ratio 14 */
#define FLAG_RATIO_15         0b00001110     /* ratio 15 */
#define FLAG_RATIO_16         0b00001111     /* ratio 16 */
/* oscillator frequency */
#define FLAG_FREQ_1           0b00000000     /* -25% */
#define FLAG_FREQ_2           0b00010000     /* -20% */
#define FLAG_FREQ_3           0b00100000     /* -15% */
#define FLAG_FREQ_4           0b00100000     /* -10% */
#define FLAG_FREQ_5           0b01000000     /* -5% */
#define FLAG_FREQ_6           0b01010000     /* f_OSC (default) */
#define FLAG_FREQ_7           0b01100000     /* +5% */
#define FLAG_FREQ_8           0b01110000     /* +10% */
#define FLAG_FREQ_9           0b10000000     /* +15% */
#define FLAG_FREQ_10          0b10010000     /* +20% */
#define FLAG_FREQ_11          0b10100000     /* +25% */
#define FLAG_FREQ_12          0b10110000     /* +30% */
#define FLAG_FREQ_13          0b11000000     /* +35% */
#define FLAG_FREQ_14          0b11010000     /* +40% */
#define FLAG_FREQ_15          0b11100000     /* +45% */
#define FLAG_FREQ_16          0b11110000     /* +50% */


/*
 *  set pre-charge period
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_PRECHARGE         0b11011001     /* set pre-charge period */

/* byte #2: phase 1 and 2 periods */
/* phase 1 period */
#define FLAG_PHASE1_1         0b00000001     /* 1 DCLK */
#define FLAG_PHASE1_2         0b00000010     /* 2 DCLK (default) */
#define FLAG_PHASE1_3         0b00000010     /* 3 DCLK */
#define FLAG_PHASE1_4         0b00000100     /* 4 DCLK */
#define FLAG_PHASE1_5         0b00000101     /* 5 DCLK */
#define FLAG_PHASE1_6         0b00000110     /* 6 DCLK */
#define FLAG_PHASE1_7         0b00000111     /* 7 DCLK */
#define FLAG_PHASE1_8         0b00001000     /* 8 DCLK */
#define FLAG_PHASE1_9         0b00001001     /* 9 DCLK */
#define FLAG_PHASE1_10        0b00001010     /* 10 DCLK */
#define FLAG_PHASE1_11        0b00001011     /* 11 DCLK */
#define FLAG_PHASE1_12        0b00001100     /* 12 DCLK */
#define FLAG_PHASE1_13        0b00001101     /* 13 DCLK */
#define FLAG_PHASE1_14        0b00001110     /* 14 DCLK */
#define FLAG_PHASE1_15        0b00001111     /* 15 DCLK */
/* phase 2 period */
#define FLAG_PHASE2_1         0b00010000     /* 1 DCLK */
#define FLAG_PHASE2_2         0b00100000     /* 2 DCLK (default) */
#define FLAG_PHASE2_3         0b00100000     /* 3 DCLK */
#define FLAG_PHASE2_4         0b01000000     /* 4 DCLK */
#define FLAG_PHASE2_5         0b01010000     /* 5 DCLK */
#define FLAG_PHASE2_6         0b01100000     /* 6 DCLK */
#define FLAG_PHASE2_7         0b01110000     /* 7 DCLK */
#define FLAG_PHASE2_8         0b10000000     /* 8 DCLK */
#define FLAG_PHASE2_9         0b10010000     /* 9 DCLK */
#define FLAG_PHASE2_10        0b10100000     /* 10 DCLK */
#define FLAG_PHASE2_11        0b10110000     /* 11 DCLK */
#define FLAG_PHASE2_12        0b11000000     /* 12 DCLK */
#define FLAG_PHASE2_13        0b11010000     /* 13 DCLK */
#define FLAG_PHASE2_14        0b11100000     /* 14 DCLK */
#define FLAG_PHASE2_15        0b11110000     /* 15 DCLK */


/*
 *  set COM pins hardware configuration
 * - 2 byte command
 */

/* byte #1: command */
#define CMD_COM_CONFIG_SET    0b11011010     /* set COM pins config */

/* byte #2: config */
#define FLAG_COM_SEQ          0b00000010     /* sequential */
#define FLAG_COM_ALT          0b00010010     /* alternative (default) */


/*
 *  set V_COM_H deselect level
 *  - 2 byte command
 */

/* byte #1: command */
#define CMD_DESELECT_LEVEL    0b11011011     /* set V_COM_H deselect level */

/* byte #2: level (range 0 - 63): 0.43 + x * 0.006415 */ 
#define FLAG_LEVEL_43         0b00000000     /* 0.43 * Vcc */
/* ... */
#define FLAG_LEVEL_77         0b00110101     /* 0.77 * Vcc (default) */
/* ... */
#define FLAG_LEVEL_83         0b00111111     /* 0.83 * Vcc */


/*
 *  read-modify-write mode
 *  - read doesn't increment column address, only write
 *  - 1 byte command
 */

#define CMD_RMW               0b11100000     /* read-modify-write mode */


/*
 *  end read-modify-write mode
 *  - reset column address to former address
 *  - 1 byte command
 */

#define CMD_END               0b11101110     /* end read-modify-write mode */


/*
 *  no operation
 *  - 1 byte cmd
 */

#define CMD_NOP               0b11100011     /* no operation */



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

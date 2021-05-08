/* ************************************************************************
 *
 *   ILI9481 color graphic display controller
 *
 *   (c) 2020 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  no operation
 *  - 1 byte cmd
 */

#define CMD_NOP               0b00000000     /* no operation */


/*
 *  software reset
 *  - 1 byte cmd
 */

#define CMD_RESET             0b00000001     /* software reset */


/*
 *  get power mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_PWR_MODE     0b00001010      /* get power mode */

/* data byte #1: dummy data */

/* data byte #2: status */
  /* display: */
#define FLAG_DISPLAY_OFF      0b00000000     /* off */
#define FLAG_DISPLAY_ON       0b00000100     /* on */
  /* display normal mode: */
#define FLAG_NORMAL_OFF       0b00000000     /* off */
#define FLAG_NORMAL_ON        0b00001000     /* on */
  /* sleep mode: */
#define FLAG_SLEEP_OFF        0b00000000     /* off */
#define FLAG_SLEEP_ON         0b00010000     /* on */
  /* partial mode: */
#define FLAG_PARTIAL_OFF      0b00000000     /* off */
#define FLAG_PARTIAL_ON       0b00100000     /* on */
  /* idle mode: */
#define FLAG_IDLE_OFF         0b00000000     /* off */
#define FLAG_IDLE_ON          0b01000000     /* on */
  /* booster: */
#define FLAG_BOOSTER_OFF      0b00000000     /* off or faulty */
#define FLAG_BOOSTER_ON       0b10000000     /* on and working fine */


/*
 *  get address mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_ADDR_MODE     0b00001011     /* get address mode */

/* data byte #1: dummy data */

/* data byte #2: status */
/* same as byte #1 of CMD_MEM_CTRL */
  /* color order: */
#define RFLAG_COLOR_RGB       0b00000000     /* RGB color filter */
#define RFLAG_COLOR_BGR       0b00001000     /* BGR color filter */
  /* line address order (LCD refresh): */
#define RFLAG_VREFRESH_NORM   0b00000000     /* top to bottom */
#define RFLAG_VREFRESH_REV    0b00010000     /* bottom to top */
  /* page/column order: */
#define RFLAG_XY_NORM         0b00000000     /* normal (X & Y) */
#define RFLAG_XY_REV          0b00100000     /* reversed (X & Y swapped) */
  /* column address order: */
#define RFLAG_COL_NORM        0b00000000     /* left to right */
#define RFLAG_COL_REV         0b01000000     /* right to left */
  /* page address order: */
#define RFLAG_PAGE_NORM       0b00000000     /* top to bottom */
#define RFLAG_PAGE_REV        0b10000000     /* bottom to top */


/*
 *  get pixel format
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_PIX_FORMAT    0b00001100     /* get pixel format */

/* data byte #1: dummy data */

/* data byte #2: status */
/* same as byte #1 of CMD_PIX_FORMAT_SET */
  /* pixel format of display bus interface (MCU interface): */
#define RFLAG_DBI_3           0b00000001     /* 3 bits per pixel */
#define RFLAG_DBI_16          0b00000101     /* 16 bits per pixel */
#define RFLAG_DBI_18          0b00000110     /* 18 bits per pixel */
  /* pixel format of display pixel interface (RGB interface): */
#define RFLAG_DPI_3           0b00010000     /* 3 bits per pixel */
#define RFLAG_DPI_16          0b01010000     /* 16 bits per pixel */
#define RFLAG_DPI_18          0b01100000     /* 18 bits per pixel */


/*
 *  get display mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_DISPLAY_MODE  0b00001101     /* get display mode */

/* data byte #1: dummy data */

/* data byte #2: status */
  /* inversion */
#define FLAG_INVERSION_OFF    0b00000000     /* off */
#define FLAG_INVERSION_ON     0b00100000     /* on */
  /* vertical scrolling */
#define FLAG_VSCROLL_OFF      0b00000000     /* off */
#define FLAG_VSCROLL_ON       0b10000000     /* on */


/*
 *  get display signal mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_SIGNAL_MODE   0b00001110     /* get signal mode */

/* data byte #1: dummy data */

/* data byte #2: mode flags */
  /* tearing effect line mode: */
#define FLAG_SIG_TEAR_1       0b00000000     /* effect mode 1 */
#define FLAG_SIG_TEAR_2       0b01000000     /* effect mode 2 */
  /* tearing effect line: */
#define FLAG_SIG_TEAR_OFF     0b00000000     /* effect off */
#define FLAG_SIG_TEAR_ON      0b10000000     /* effect on */


/*
 *  get display's self-diagnostic result
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_GET_DIAG          0b00001111     /* get diagnostics */

/* data byte #1: dummy data */

/* data byte #2: diag flags */
#define FLAG_DIAG_DISP_OK     0b01000000     /* display works */
#define FLAG_DIAG_REGS_OK     0b10000000     /* register values loaded */


/*
 *  enter sleep mode (sleep in)
 *  - 1 byte cmd
 */

#define CMD_SLEEP_IN          0b00010000     /* enter sleep mode */


/*
 *  exit sleep mode (sleep out)
 *  - 1 byte cmd
 */

#define CMD_SLEEP_OUT         0b00010001     /* leave sleep mode */


/*
 *  enter partial display mode
 *  - 1 byte cmd
 */

#define CMD_PARTIAL_MODE      0b00010010     /* partial mode on */


/*
 *  enter normal display mode
 *  - 1 byte cmd
 */

#define CMD_NORMAL_MODE       0b00010011     /* normal mode on */


/*
 *  disable display inversion
 *  - 1 byte cmd
 */

#define CMD_INVERSION_OFF     0b00100000     /* display inversion off */


/*
 *  enable display inversion (invert pixels)
 *  - 1 byte cmd
 */

#define CMD_INVERSION_ON      0b00100001     /* display inversion on */


/*
 *  disable display output
 *  - 1 byte cmd
 */

#define CMD_DISPLAY_OFF       0b00101000     /* display off */


/*
 *  enable display output
 *  - 1 byte cmd
 */

#define CMD_DISPLAY_ON        0b00101001     /* display on */


/*
 *  set column address (accessable frame area)
 *  - 1 byte cmd + 4 bytes data
 */

#define CMD_COL_ADDR_SET      0b00101010     /* set column address */

/* data byte #1: start column - MSB (bit 8) */
/* data byte #2: start column - LSB (bits 7-0) */
/* data byte #3: end column - MSB (bit 8) */
/* data byte #4: end column - LSB (bits 7-0) */
/* valid range: 0x0000 - 0x013f/0x01df */


/*
 *  set page address (row, accessible frame area)
 *  - 1 byte cmd + 4 bytes data
 */

#define CMD_PAGE_ADDR_SET     0b00101011     /* set page address */

/* data byte #1: start page - MSB (bit 8) */
/* data byte #2: start page - LSB (bits 7-0) */
/* data byte #3: end page - MSB (bit 8) */
/* data byte #4: end page - LSB (bits 7-0) */
/* valid range: 0x0000 - 0x01df/0x013f */


/*
 *  write memory (starting at Start Column and Start Page)
 *  - 1 byte cmd + x bytes data
 */

#define CMD_MEM_WRITE         0b00101100     /* write memory */

/* data byte #1 - #x: image data */


/*
 *  read memory (starting at Start Column and Start Page)
 *  - 1 byte cmd + x bytes data (read mode)
 */

#define CMD_MEM_READ          0b00101110     /* read memory */

/* data byte #1: dummy data */
/* data byte #2 - #x: image data */


/*
 *  partial area
 *  - 1 byte cmd + 4 bytes data
 */

#define CMD_PARTIAL_AREA      0b00110000     /* set partial area */

/* data byte #1: start row - MSB (bit 8) */
/* data byte #2: start row - LSB (bits 7-0) */
/* data byte #3: end row - MSB (bit 8) */
/* data byte #4: end row - LSB (bits 7-0) */
/* valid range: 0x0000 - 0x01df */


/*
 *  vertical scrolling definition
 *  - 1 byte cmd + 6 bytes data
 */

#define CMD_V_SCROLL_DEF      0b00110011     /* set vertical scrolling area */

/* data byte #1: top fixed area line number - MSB (bit 8) */
/* data byte #2: top fixed area line number - LSB (bits 7-0) */
/* data byte #3: height of scrolling area - MSB (bit 8) */
/* data byte #4: height of scrolling area - LSB (bits 7-0) */
/* data byte #5: bottom fixed area line number - MSB (bit 8) */
/* data byte #6: bottom fixed area line number - LSB (bits 7-0) */


/*
 *  disable tearing effect line 
 *  - 1 byte cmd
 */

#define CMD_TEAR_OFF          0b00110100     /* tearing effect off */


/*
 *  enable tearing effect line 
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_TEAR_ON           0b00110101     /* tearing effect on */

/* data byte #1: mode */
#define FLAG_TEAR_MODE_0      0b00000000     /* V-blanking only */
#define FLAG_TEAR_MODE_1      0b00000001     /* V-blanking and H-blanking */



/*
 *  set addres mode (memory access control)
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_ADDR_MODE         0b00110110     /* set address mode */

/* data byte #1: read/write scanning direction of frame memory */
/* same as byte #2 of CMD_READ_MADCTL */
  /* vertical flip: */
#define FLAG_VFLIP_OFF        0b00000000     /* normal display */
#define FLAG_VFLIP_ON         0b00000001     /* flipped display */
  /* horizontal flip: */
#define FLAG_HFLIP_OFF        0b00000000     /* normal display */
#define FLAG_HFLIP_ON         0b00000010     /* flipped display */
  /* horizontal refereshing direction: */
#define FLAG_HREFRESH_NORM    0b00000000     /* left to right */
#define FLAG_HREFRESH_REV     0b00000100     /* right to left */
  /* color selector switch: */
#define FLAG_COLOR_RGB        0b00000000     /* RGB color filter */
#define FLAG_COLOR_BGR        0b00001000     /* BGR color filter */
  /* vertical refereshing direction: */
#define FLAG_VREFRESH_NORM    0b00000000     /* top to bottom */
#define FLAG_VREFRESH_REV     0b00010000     /* bottom to top */
  /* page/column order (exchange): */
#define FLAG_XY_NORM          0b00000000     /* normal */
#define FLAG_XY_REV           0b00100000     /* reversed (swap x and y) */
  /* column address order: */
#define FLAG_COL_NORM         0b00000000     /* left to right */
#define FLAG_COL_REV          0b01000000     /* right to left */
  /* page (row) address order: */
#define FLAG_PAGE_NORM        0b00000000     /* top to bottom */
#define FLAG_PAGE_REV         0b10000000     /* bottom to top */


/*
 *  set vertical scrolling start address 
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_VSCROLL_ADDR      0b00110111     /* set vertical scrolling start address */

/* data byte #1: line number - MSB (bit 8) */
/* data byte #2: line number - MSB (bits 7-0) */


/*
 *  exit idle mode (full color depth)
 *  - 1 byte cmd
 */

#define CMD_IDLE_OFF          0b00111000     /* idle mode off */


/*
 *  enter idle mode (8bit color depth)
 *  - 1 byte cmd
 */

#define CMD_IDLE_ON           0b00111001     /* idle mode on */


/*
 *  set pixel format for RGB image data
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_SET_PIX_FORMAT    0b00111010     /* set pixel format */

/* data byte #1: formats */
/* same as byte #2 of CMD_READ_PIX_FORMAT */
  /* pixel format of display bus interface (MCU interface): */
#define FLAG_DBI_3            0b00000001     /* 3 bits per pixel */
#define FLAG_DBI_16           0b00000101     /* 16 bits per pixel */
#define FLAG_DBI_18           0b00000110     /* 18 bits per pixel */
  /* pixel format of display pixel interface (RGB interface): */
#define FLAG_DPI_3            0b00010000     /* 3 bits per pixel */
#define FLAG_DPI_16           0b01010000     /* 16 bits per pixel */
#define FLAG_DPI_18           0b01100000     /* 18 bits per pixel */


/*
 *  write memory continue (starting at last pixel position + 1)
 *  - 1 byte cmd + x bytes data
 */

#define CMD_WRITE_MEM_CONT    0b00111100     /* write memory continue */

/* data byte #1 - #x: image data */


/*
 *  read memory continue (starting at last pixel position + 1)
 *  - 1 byte cmd + x bytes data (read mode)
 */

#define CMD_READ_MEM_CONT     0b00111110     /* read memory continue */

/* data byte #1: dummy data */
/* data byte #2 - #x: image data */


/*
 *  set tearing effect scan line 
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_SET_SCANLINE      0b01000100     /* set scan line */

/* data byte #1: line number - MSB (bit 8) */
#define FLAG_STS_HIGH_MIN     0b00000000     /* minimum value */
#define FLAG_STS_HIGH_MAX     0b00000001     /* maximum value */

/* data byte #2: line number - LSB (bits 7-0) */
#define FLAG_STS_LOW_MIN      0b00000000     /* minimum value */
#define FLAG_STS_LOW_MAX      0b11111111     /* maximum value */


/*
 *  get tearing effect scan line
 *  - 1 byte cmd + 3 bytes data (read mode)
 */

#define CMD_GET_SCANLINE      0b01000101     /* get scan line  */

/* data byte #1: dummy data */
/* data byte #2: line number - MSB (bits 8) */
/* data byte #3: line number - LSB (bits 7-0) */


/*
 *  read DDB
 *  - 1 byte cmd + 6 bytes data (read mode)
 */

#define CMD_READ_DDB          0b10100001     /* read DDB */

/* data byte #1: dummy data */
/* data byte #2: supplier ID code - MSB (bits 8-15) */
/* data byte #3: supplier ID code - LSB (bits 0-7) */
/* data byte #4: supplier elective data - MSB (bits 8-15) */
/* data byte #5: supplier elective data - LSB (bits 0-7) */
/* data byte #6: exit code (0xff) */


/*
 *  command access protect
 *  - 1 byte cmd + 1 byte data
 *
 *  user commands:         00-AF
 *  protect command:       B0
 *  manufacturer commands: B1-DF, E0-EF, F0-FF
 */

#define CMD_COMMAND_ACCESS    0b10110000     /* cmd access protect */

/* data byte #1: command access */
                                             /* protect: */
#define FLAG_CMD_ACCESS_0     0b00000000     /* none */
#define FLAG_CMD_ACCESS_1     0b00000001     /* F0-FF */
#define FLAG_CMD_ACCESS_2     0b00000010     /* E0-EF, F0-FF */
#define FLAG_CMD_ACCESS_3     0b00000011     /* B1-DF, E0-EF, F0-FF */


/*
 *  low power mode control (deep standby)
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_LOW_POWER         0b10110001     /* low power mode control */

/* data byte #1: mode */
#define FLAG_LOW_POWER_ON     0b00000001     /* enter deep standby */


/*
 *  frame memory access and interface setting
 *  - 1 byte cmd + 4 bytes data
 */

#define CMD_FRAME_MEM         0b10110011     /* frame memory access & interface */

/* data byte #1: */
  /* memory write control (exceeding number of pixels in address window): */
#define FLAG_WEMODE_IGNORE    0b00000000     /* ignore additional pixel data */
#define FLAG_WEMODE_RESET     0b00000010     /* reset and continue in next column/page */

/* data byte #2: */
  /* output interval for TE signal: */
#define FLAG_TE_INTERVAL_1    0b00000000     /* 1 frame */
#define FLAG_TE_INTERVAL_2    0b00000001     /* 2 frames */
#define FLAG_TE_INTERVAL_4    0b00000011     /* 4 frames */
#define FLAG_TE_INTERVAL_6    0b00000101     /* 6 frames */

/* data byte #3: */
  /* GRAM write cycle (frame periods): */
#define FLAG_GRAM_CYCLES_1    0b00000000     /* 1 frame */
#define FLAG_GRAM_CYCLES_2    0b00000001     /* 2 frames */
#define FLAG_GRAM_CYCLES_3    0b00000010     /* 3 frames */
#define FLAG_GRAM_CYCLES_4    0b00000011     /* 4 frames */
#define FLAG_GRAM_CYCLES_5    0b00000100     /* 5 frames */
#define FLAG_GRAM_CYCLES_6    0b00000101     /* 6 frames */
#define FLAG_GRAM_CYCLES_7    0b00000110     /* 7 frames */
#define FLAG_GRAM_CYCLES_8    0b00000111     /* 8 frames */

/* data byte #4: */
  /* image data format for frame memory read/write for */
  /* 18 bits/pixel 16 bit parallel bus and 3 bits/pixel serial bus: */
#define FLAG_DFM_1            0b00000000     /* method 1 (see datasheet) */
#define FLAG_DFM_2            0b00000001     /* method 2 (see datasheet) */
  /* data format for RGB565 to RGB666 conversion in GRAM: */
#define FLAG_EPF_1            0b00000000     /* R0 = 0, B0 = 0 */
#define FLAG_EPF_2            0b00010000     /* R0 = 1, B0 = 1 */
#define FLAG_EPF_3            0b00100000     /* DB15 -> R0, DB5 -> B0 */


/*
 *  display mode and frame memory write mode setting
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_DISP_MODE         0b10110100     /* display mode & GRAM write */

/* data byte #1: */
  /* display operation mode: */
#define FLAG_DM_INT           0b00000000     /* internal system clock */
#define FLAG_DM_RGB           0b00000001     /* DPI (RGB) interface */
  /* interface to access GRAM: */
#define FLAG_RM_DBI           0b00000000     /* DBI interface (CPU) */
#define FLAG_RM_DPI           0b00010000     /* DPI interface (RGB) */


/*
 *  read device code
 *  - 1 byte cmd + 6 bytes data (read mode)
 */

#define CMD_READ_DEVICE_CODE  0b10111111     /* read device code */

/* data byte #1: dummy data */
/* data byte #2: MIPI Alliance code - MSB (0x02) */
/* data byte #3: MIPI Alliance code - LSB (0x04) */
/* data byte #4: device ID code - MSB (0x94) */
/* data byte #5: device ID code - LSB (0x81) */
/* data byte #6: exit code (0xff) */


/*
 *  panel driving setting
 *  - 1 byte cmd + 5 bytes data
 */

#define CMD_PANEL_DRIVE       0b11000000     /* panel driving setting */

/* data byte #1: */
  /* gate driver scan direction: */
#define FLAG_GS_0             0b00000000     /* G1 -> G480 */
#define FLAG_GS_1             0b00000100     /* G480 -> G1 */
  /* gate driver pin arrangement (in combination with GS): */
#define FLAG_SM_0             0b00000000     /* in sequence */
#define FLAG_SM_1             0b00001000     /* even/odd separated */
  /* grayscale inversion of image: */
#define FLAG_REV_OFF          0b00000000     /* off */
#define FLAG_REV_ON           0b00010000     /* on */

/* data byte #2: number of lines to drive LCD at an interval of 8 lines: */
                                             /* 8 * (NL + 1) */
#define FLAG_NL_008           0b00000000     /* 8 lines */
#define FLAG_NL_016           0b00000001     /* 16 lines */
#define FLAG_NL_024           0b00000010     /* 24 lines */
#define FLAG_NL_036           0b00000011     /* 32 lines */
#define FLAG_NL_040           0b00000100     /* 40 lines */
#define FLAG_NL_048           0b00000101     /* 48 lines */
#define FLAG_NL_056           0b00000110     /* 56 lines */
#define FLAG_NL_064           0b00000111     /* 64 lines */
#define FLAG_NL_072           0b00001000     /* 72 lines */
#define FLAG_NL_080           0b00001001     /* 80 lines */
#define FLAG_NL_088           0b00001010     /* 88 lines */
#define FLAG_NL_096           0b00001011     /* 96 lines */
#define FLAG_NL_104           0b00001100     /* 104 lines */
#define FLAG_NL_112           0b00001101     /* 112 lines */
#define FLAG_NL_120           0b00001110     /* 120 lines */
#define FLAG_NL_128           0b00001111     /* 128 lines */
#define FLAG_NL_136           0b00010000     /* 136 lines */
#define FLAG_NL_144           0b00010001     /* 144 lines */
#define FLAG_NL_152           0b00010010     /* 152 lines */
#define FLAG_NL_160           0b00010011     /* 160 lines */
#define FLAG_NL_168           0b00010100     /* 168 lines */
#define FLAG_NL_176           0b00010101     /* 176 lines */
#define FLAG_NL_184           0b00010110     /* 184 lines */
#define FLAG_NL_192           0b00010111     /* 192 lines */
#define FLAG_NL_200           0b00011000     /* 200 lines */
#define FLAG_NL_208           0b00011001     /* 208 lines */
#define FLAG_NL_216           0b00011010     /* 216 lines */
#define FLAG_NL_224           0b00011011     /* 224 lines */
#define FLAG_NL_232           0b00011100     /* 232 lines */
#define FLAG_NL_240           0b00011101     /* 240 lines */
#define FLAG_NL_248           0b00011110     /* 248 lines */
#define FLAG_NL_256           0b00011111     /* 256 lines */
#define FLAG_NL_264           0b00100000     /* 264 lines */
#define FLAG_NL_272           0b00100001     /* 272 lines */
#define FLAG_NL_280           0b00100010     /* 280 lines */
#define FLAG_NL_288           0b00100011     /* 288 lines */
#define FLAG_NL_296           0b00100100     /* 296 lines */
#define FLAG_NL_304           0b00100101     /* 304 lines */
#define FLAG_NL_312           0b00100110     /* 312 lines */
#define FLAG_NL_320           0b00100111     /* 320 lines */
#define FLAG_NL_328           0b00101000     /* 328 lines */
#define FLAG_NL_336           0b00101001     /* 336 lines */
#define FLAG_NL_344           0b00101010     /* 344 lines */
#define FLAG_NL_352           0b00101011     /* 352 lines */
#define FLAG_NL_360           0b00101100     /* 360 lines */
#define FLAG_NL_368           0b00101101     /* 368 lines */
#define FLAG_NL_376           0b00101110     /* 376 lines */
#define FLAG_NL_384           0b00101111     /* 384 lines */
#define FLAG_NL_392           0b00110000     /* 392 lines */
#define FLAG_NL_400           0b00110001     /* 400 lines */
#define FLAG_NL_408           0b00110010     /* 408 lines */
#define FLAG_NL_416           0b00110011     /* 416 lines */
#define FLAG_NL_424           0b00110100     /* 424 lines */
#define FLAG_NL_432           0b00110101     /* 432 lines */
#define FLAG_NL_440           0b00110110     /* 440 lines */
#define FLAG_NL_448           0b00110111     /* 448 lines */
#define FLAG_NL_456           0b00111000     /* 456 lines */
#define FLAG_NL_464           0b00111001     /* 464 lines */
#define FLAG_NL_472           0b00111010     /* 472 lines */
#define FLAG_NL_480           0b00111011     /* 480 lines */

/* data byte #3: */
  /* scanning start position:
   * SCN         SM=0/GS=0  SM=0/GS=1  SM=1/GS=0       SM=1/GS=1
   * 0x00-0x3b   1+SCN*4    480-SCN*4  1+SCN*8         480-SCN*8
   * 0x3c-0x77   1+SCN*4    480-SCN*4  2+(SCN-0x3c)*8  479-(SCN-0x3c)*8
   */

/* data byte #4: */
  /* source output level in non-display area drive period (pos/neg) */
  /* for front/back porch period and blank area between partial displays: */
#define FLAG_PTS_0            0b00000000     /* V63 / V0 */
#define FLAG_PTS_1            0b00000001     /* V0 / V63 */
#define FLAG_PTS_2            0b00000010     /* GND / GND */
#define FLAG_PTS_3            0b00000011     /* Hi-Z / Hi-Z */
  /* source output level in non-display area (pos/neg): */
#define FLAG_NDL_0            0b00000000     /* V63 / V0 */
#define FLAG_NDL_1            0b00010000     /* V0 / V63 */

/* data byte #5: */
  /* scan cycle for interval scan in non-display area period: */
#define FLAG_ISC_03           0b00000001     /* 3 frames / 50 ms */
#define FLAG_ISC_05           0b00000010     /* 5 frames / 84 ms */
#define FLAG_ISC_07           0b00000011     /* 7 frames / 117 ms */
#define FLAG_ISC_09           0b00000100     /* 9 frames / 150 ms */
#define FLAG_ISC_11           0b00000101     /* 11 frames / 184 ms*/
#define FLAG_ISC_13           0b00000110     /* 13 frames / 217 ms */
#define FLAG_ISC_15           0b00000111     /* 15 frames / 251 ms */
#define FLAG_ISC_17           0b00001000     /* 17 frames / 284 ms */
#define FLAG_ISC_19           0b00001001     /* 19 frames / 317 ms */
#define FLAG_ISC_21           0b00001010     /* 21 frames / 351 ms */
#define FLAG_ISC_23           0b00001011     /* 23 frames / 384 ms */
#define FLAG_ISC_25           0b00001100     /* 25 frames / 418 ms */
#define FLAG_ISC_27           0b00001101     /* 27 frames / 451 ms */
#define FLAG_ISC_29           0b00001110     /* 29 frames / 484 ms */
#define FLAG_ISC_31           0b00001111     /* 31 frames / 518 ms */
  /* scan mode in non-display area: */
#define FLAG_PTG_0            0b00000000     /* normal scan */
#define FLAG_PTG_1            0b00010000     /* interval scan */


/*
 *  display timing setting for normal mode
 *  - 1 byte cmd + 3 bytes data 
 */

#define CMD_DISPTIME_NORMAL   0b11000001     /* display timing for normal mode */

/* data byte #1: */
  /* division ratio of internal clock: */
#define FLAG_DIV0_1           0b00000000     /* 1/1 */
#define FLAG_DIV0_2           0b00000001     /* 1/2 */
#define FLAG_DIV0_4           0b00000010     /* 1/4 */
#define FLAG_DIV0_8           0b00000011     /* 1/8 */
  /* VCOM liquid crystal drive waveform: */
#define FLAG_BC0_FRAME        0b00000000     /* frame inversion */
#define FLAG_BC0_LINE         0b00010000     /* line inversion */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTN0_16          0b00010000     /* 16 clocks */
#define FLAG_RTN0_17          0b00010001     /* 17 clocks */
#define FLAG_RTN0_18          0b00010010     /* 18 clocks */
#define FLAG_RTN0_19          0b00010011     /* 19 clocks */
#define FLAG_RTN0_20          0b00010100     /* 20 clocks */
#define FLAG_RTN0_21          0b00010101     /* 21 clocks */
#define FLAG_RTN0_22          0b00010110     /* 22 clocks */
#define FLAG_RTN0_23          0b00010111     /* 23 clocks */
#define FLAG_RTN0_24          0b00011000     /* 24 clocks */
#define FLAG_RTN0_25          0b00011001     /* 25 clocks */
#define FLAG_RTN0_26          0b00011010     /* 26 clocks */
#define FLAG_RTN0_27          0b00011011     /* 27 clocks */
#define FLAG_RTN0_28          0b00011100     /* 28 clocks */
#define FLAG_RTN0_29          0b00011101     /* 29 clocks */
#define FLAG_RTN0_30          0b00011110     /* 30 clocks */
#define FLAG_RTN0_31          0b00011111     /* 31 clocks */

/* data byte #3: */
  /* number of lines for back porch period: */
#define FLAG_BP0_2            0b00000010     /* 2 lines */
#define FLAG_BP0_3            0b00000011     /* 3 lines */
#define FLAG_BP0_4            0b00000100     /* 4 lines */
#define FLAG_BP0_5            0b00000101     /* 5 lines */
#define FLAG_BP0_6            0b00000110     /* 6 lines */
#define FLAG_BP0_7            0b00000111     /* 7 lines */
#define FLAG_BP0_8            0b00001000     /* 8 lines */
#define FLAG_BP0_9            0b00001001     /* 9 lines */
#define FLAG_BP0_10           0b00001010     /* 10 lines */
#define FLAG_BP0_11           0b00001011     /* 11 lines */
#define FLAG_BP0_12           0b00001100     /* 12 lines */
#define FLAG_BP0_13           0b00001101     /* 13 lines */
#define FLAG_BP0_14           0b00001110     /* 14 lines */
#define FLAG_BP0_15           0b00001111     /* 15 lines */
  /* number of lines for front porch period: */
#define FLAG_FP0_2            0b00100000     /* 2 lines */
#define FLAG_FP0_3            0b00110000     /* 3 lines */
#define FLAG_FP0_4            0b01000000     /* 4 lines */
#define FLAG_FP0_5            0b01010000     /* 5 lines */
#define FLAG_FP0_6            0b01100000     /* 6 lines */
#define FLAG_FP0_7            0b01110000     /* 7 lines */
#define FLAG_FP0_8            0b10000000     /* 8 lines */
#define FLAG_FP0_9            0b10010000     /* 9 lines */
#define FLAG_FP0_10           0b10100000     /* 10 lines */
#define FLAG_FP0_11           0b10110000     /* 11 lines */
#define FLAG_FP0_12           0b11000000     /* 12 lines */
#define FLAG_FP0_13           0b11010000     /* 13 lines */
#define FLAG_FP0_14           0b11100000     /* 14 lines */
#define FLAG_FP0_15           0b11110000     /* 15 lines */


/*
 *  display timing setting for partial mode
 *  - 1 byte cmd + 3 bytes data 
 */

#define CMD_DISPTIME_PARTIAL  0b11000010     /* display timing for partial mode */

/* data byte #1: */
  /* division ratio of internal clock: */
#define FLAG_DIV1_1           0b00000000     /* 1/1 */
#define FLAG_DIV1_2           0b00000001     /* 1/2 */
#define FLAG_DIV1_4           0b00000010     /* 1/4 */
#define FLAG_DIV1_8           0b00000011     /* 1/8 */
  /* VCOM liquid crystal drive waveform: */
#define FLAG_BC1_FRAME        0b00000000     /* frame inversion */
#define FLAG_BC1_LINE         0b00010000     /* line inversion */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTN1_16          0b00010000     /* 16 clocks */
#define FLAG_RTN1_17          0b00010001     /* 17 clocks */
#define FLAG_RTN1_18          0b00010010     /* 18 clocks */
#define FLAG_RTN1_19          0b00010011     /* 19 clocks */
#define FLAG_RTN1_20          0b00010100     /* 20 clocks */
#define FLAG_RTN1_21          0b00010101     /* 21 clocks */
#define FLAG_RTN1_22          0b00010110     /* 22 clocks */
#define FLAG_RTN1_23          0b00010111     /* 23 clocks */
#define FLAG_RTN1_24          0b00011000     /* 24 clocks */
#define FLAG_RTN1_25          0b00011001     /* 25 clocks */
#define FLAG_RTN1_26          0b00011010     /* 26 clocks */
#define FLAG_RTN1_27          0b00011011     /* 27 clocks */
#define FLAG_RTN1_28          0b00011100     /* 28 clocks */
#define FLAG_RTN1_29          0b00011101     /* 29 clocks */
#define FLAG_RTN1_30          0b00011110     /* 30 clocks */
#define FLAG_RTN1_31          0b00011111     /* 31 clocks */

/* data byte #3: */
  /* number of lines for back porch period: */
#define FLAG_BP1_2            0b00000010     /* 2 lines */
#define FLAG_BP1_3            0b00000011     /* 3 lines */
#define FLAG_BP1_4            0b00000100     /* 4 lines */
#define FLAG_BP1_5            0b00000101     /* 5 lines */
#define FLAG_BP1_6            0b00000110     /* 6 lines */
#define FLAG_BP1_7            0b00000111     /* 7 lines */
#define FLAG_BP1_8            0b00001000     /* 8 lines */
#define FLAG_BP1_9            0b00001001     /* 9 lines */
#define FLAG_BP1_10           0b00001010     /* 10 lines */
#define FLAG_BP1_11           0b00001011     /* 11 lines */
#define FLAG_BP1_12           0b00001100     /* 12 lines */
#define FLAG_BP1_13           0b00001101     /* 13 lines */
#define FLAG_BP1_14           0b00001110     /* 14 lines */
#define FLAG_BP1_15           0b00001111     /* 15 lines */
  /* number of lines for front porch period: */
#define FLAG_FP1_2            0b00100000     /* 2 lines */
#define FLAG_FP1_3            0b00110000     /* 3 lines */
#define FLAG_FP1_4            0b01000000     /* 4 lines */
#define FLAG_FP1_5            0b01010000     /* 5 lines */
#define FLAG_FP1_6            0b01100000     /* 6 lines */
#define FLAG_FP1_7            0b01110000     /* 7 lines */
#define FLAG_FP1_8            0b10000000     /* 8 lines */
#define FLAG_FP1_9            0b10010000     /* 9 lines */
#define FLAG_FP1_10           0b10100000     /* 10 lines */
#define FLAG_FP1_11           0b10110000     /* 11 lines */
#define FLAG_FP1_12           0b11000000     /* 12 lines */
#define FLAG_FP1_13           0b11010000     /* 13 lines */
#define FLAG_FP1_14           0b11100000     /* 14 lines */
#define FLAG_FP1_15           0b11110000     /* 15 lines */


/*
 *  display timing setting for idle mode
 *  - 1 byte cmd + 3 bytes data 
 */

#define CMD_DISPTIME_IDLE     0b11000010     /* display timing for idle mode */

/* data byte #1: */
  /* division ratio of internal clock: */
#define FLAG_DIV2_1           0b00000000     /* 1/1 */
#define FLAG_DIV2_2           0b00000001     /* 1/2 */
#define FLAG_DIV2_4           0b00000010     /* 1/4 */
#define FLAG_DIV2_8           0b00000011     /* 1/8 */
  /* VCOM liquid crystal drive waveform: */
#define FLAG_BC2_FRAME        0b00000000     /* frame inversion */
#define FLAG_BC2_LINE         0b00010000     /* line inversion */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTN2_16          0b00010000     /* 16 clocks */
#define FLAG_RTN2_17          0b00010001     /* 17 clocks */
#define FLAG_RTN2_18          0b00010010     /* 18 clocks */
#define FLAG_RTN2_19          0b00010011     /* 19 clocks */
#define FLAG_RTN2_20          0b00010100     /* 20 clocks */
#define FLAG_RTN2_21          0b00010101     /* 21 clocks */
#define FLAG_RTN2_22          0b00010110     /* 22 clocks */
#define FLAG_RTN2_23          0b00010111     /* 23 clocks */
#define FLAG_RTN2_24          0b00011000     /* 24 clocks */
#define FLAG_RTN2_25          0b00011001     /* 25 clocks */
#define FLAG_RTN2_26          0b00011010     /* 26 clocks */
#define FLAG_RTN2_27          0b00011011     /* 27 clocks */
#define FLAG_RTN2_28          0b00011100     /* 28 clocks */
#define FLAG_RTN2_29          0b00011101     /* 29 clocks */
#define FLAG_RTN2_30          0b00011110     /* 30 clocks */
#define FLAG_RTN2_31          0b00011111     /* 31 clocks */

/* data byte #3: */
  /* number of lines for back porch period: */
#define FLAG_BP2_2            0b00000010     /* 2 lines */
#define FLAG_BP2_3            0b00000011     /* 3 lines */
#define FLAG_BP2_4            0b00000100     /* 4 lines */
#define FLAG_BP2_5            0b00000101     /* 5 lines */
#define FLAG_BP2_6            0b00000110     /* 6 lines */
#define FLAG_BP2_7            0b00000111     /* 7 lines */
#define FLAG_BP2_8            0b00001000     /* 8 lines */
#define FLAG_BP2_9            0b00001001     /* 9 lines */
#define FLAG_BP2_10           0b00001010     /* 10 lines */
#define FLAG_BP2_11           0b00001011     /* 11 lines */
#define FLAG_BP2_12           0b00001100     /* 12 lines */
#define FLAG_BP2_13           0b00001101     /* 13 lines */
#define FLAG_BP2_14           0b00001110     /* 14 lines */
#define FLAG_BP2_15           0b00001111     /* 15 lines */
  /* number of lines for front porch period: */
#define FLAG_FP2_2            0b00100000     /* 2 lines */
#define FLAG_FP2_3            0b00110000     /* 3 lines */
#define FLAG_FP2_4            0b01000000     /* 4 lines */
#define FLAG_FP2_5            0b01010000     /* 5 lines */
#define FLAG_FP2_6            0b01100000     /* 6 lines */
#define FLAG_FP2_7            0b01110000     /* 7 lines */
#define FLAG_FP2_8            0b10000000     /* 8 lines */
#define FLAG_FP2_9            0b10010000     /* 9 lines */
#define FLAG_FP2_10           0b10100000     /* 10 lines */
#define FLAG_FP2_11           0b10110000     /* 11 lines */
#define FLAG_FP2_12           0b11000000     /* 12 lines */
#define FLAG_FP2_13           0b11010000     /* 13 lines */
#define FLAG_FP2_14           0b11100000     /* 14 lines */
#define FLAG_FP2_15           0b11110000     /* 15 lines */


/*
 *  frame rate and inversion control
 *  - 1 byte cmd + 1 byte data 
 */

#define CMD_FRAME_RATE        0b11000101     /* frame rate */

/* data byte #1: frame frequency for normal mode (full colors) */
#define FLAG_FRA_125          0b00000000     /* 125 Hz */
#define FLAG_FRA_100          0b00000001     /* 100 Hz */
#define FLAG_FRA_85           0b00000010     /* 85 Hz */
#define FLAG_FRA_72           0b00000011     /* 72 Hz */
#define FLAG_FRA_56           0b00000100     /* 56 Hz */
#define FLAG_FRA_50           0b00000101     /* 50 Hz */
#define FLAG_FRA_45           0b00000110     /* 45 Hz */
#define FLAG_FRA_42           0b00000111     /* 42 Hz */


/*
 *  interface control
 *  - 1 byte cmd + 1 byte data 
 */

#define CMD_INTERFACE_CTRL    0b11000110     /* interface control */

/* data byte #1: */
  /* signal polarity of PCLK pin: data input on */
#define FLAG_DPL_RISING       0b00000000     /* rising edge of PCLK */
#define FLAG_DPL_FALLING      0b00000001     /* falling edge of PCLK */
  /* signal polarity of ENABLE pin: data is written when */
#define FLAG_EPL_LOW          0b00000000     /* ENABLE is low */
#define FLAG_EPL_HIGH         0b00000010     /* ENABLE is high */
  /* signal polarity of HSYNC pin: */
#define FLAG_HSPL_LOW         0b00000000     /* low active */
#define FLAG_HSPL_HIGH        0b00001000     /* high active */
  /* signal polarity of VSYNC pin: */
#define FLAG_VSPL_LOW         0b00000000     /* low active */
#define FLAG_VSPL_HIGH        0b00010000     /* high active */
  /* serial interface selection: */
#define FLAG_SDA_OFF          0b00000000     /* use DIN and DOUT */
#define FLAG_SDA_ON           0b10000000     /* use DIN/SDA only */


/*
 *  gamma setting
 *  - 1 byte cmd + 12 bytes data 
 */

#define CMD_SET_GAMMA         0b11001000     /* gamma setting */

/* data bytes: please see dataheet */


/*
 *  power setting
 *  - 1 byte cmd + 3 byteb data 
 */

#define CMD_SET_POWER         0b11010000     /* power setting */

/* data byte #1: */
  /* VCI ratio factor for generating VCI1: */
#define FLAG_VC_095           0b00000000     /* 0.95 */
#define FLAG_VC_090           0b00000001     /* 0.90 */
#define FLAG_VC_085           0b00000010     /* 0.85 */
#define FLAG_VC_080           0b00000011     /* 0.80 */
#define FLAG_VC_075           0b00000100     /* 0.75 */
#define FLAG_VC_070           0b00000101     /* 0.70 */
#define FLAG_VC_000           0b00000110     /* disabled */
#define FLAG_VC_100           0b00000111     /* 1.00 */

/* data byte #2: */
  /* step-up factor and output voltage: DDVDH=2*VCI1, VCL=-VCI1 */
#define FLAG_BT_0             0b00000000     /* VGH=6*VCI1, VGL=5*VCI1 */
#define FLAG_BT_1             0b00000001     /* VGH=6*VCI1, VGL=4*VCI1 */
#define FLAG_BT_2             0b00000010     /* VGH=6*VCI1, VGL=3*VCI1 */
#define FLAG_BT_3             0b00000011     /* VGH=5*VCI1, VGL=5*VCI1 */
#define FLAG_BT_4             0b00000100     /* VGH=5*VCI1, VGL=4*VCI1 */
#define FLAG_BT_5             0b00000101     /* VGH=5*VCI1, VGL=3*VCI1 */
#define FLAG_BT_6             0b00000110     /* VGH=4*VCI1, VGL=4*VCI1 */
#define FLAG_BT_7             0b00000111     /* VGH=4*VCI1, VGL=3*VCI1 */
  /* step-up converter for VGL: */
#define FLAG_VGL_OFF          0b00000000     /* off */
#define FLAG_VGL_ON           0b01000000     /* on */

/* data byte #3: */
  /* VCI factor for generating VREG1OUT */
#define FLAG_VRH_000          0b00000000     /* halt */
#define FLAG_VRH_200          0b00000001     /* 2.00 */
#define FLAG_VRH_205          0b00000010     /* 2.05 */
#define FLAG_VRH_210          0b00000011     /* 2.10 */
#define FLAG_VRH_220          0b00000100     /* 2.20 */
#define FLAG_VRH_230          0b00000101     /* 2.30 */
#define FLAG_VRH_240          0b00000110     /* 2.40 */
#define FLAG_VRH_245          0b00000111     /* 2.45 */
#define FLAG_VRH_160          0b00001000     /* 1.60 */
#define FLAG_VRH_165          0b00001001     /* 1.65 */
#define FLAG_VRH_170          0b00001010     /* 1.70 */
#define FLAG_VRH_175          0b00001011     /* 1.75 */
#define FLAG_VRH_180          0b00001100     /* 1.80 */
#define FLAG_VRH_185          0b00001101     /* 1.85 */
#define FLAG_VRH_190          0b00001110     /* 1.90 */
#define FLAG_VRH_195          0b00001111     /* 1.95 */
  /* voltage reference: */
#define FLAG_VCIRE_EXT        0b00000000     /* use external voltage VCI */
#define FLAG_VCIRE_INT        0b00010000     /* use internal 2.5V reference */


/*
 *  VCOM control
 *  - 1 byte cmd + 3 bytes data
 */

#define CMD_VCOM_CTRL         0b11010001     /* VCOM control */

/* data byte #1: */
  /* Vcom value selection: */
#define FLAG_VCOM_REG         0b00000000     /* from register (see byte #2) */
#define FLAG_VCOM_NV          0b00000001     /* from NV memory */

/* data byte #2: */
  /* VREG1OUT factor to generate VHOMH: */
#define FLAG_VCOM_0685        0b00000000     /* 0.685 */
#define FLAG_VCOM_0690        0b00000001     /* 0.690 */
#define FLAG_VCOM_0695        0b00000010     /* 0.695 */
#define FLAG_VCOM_0700        0b00000011     /* 0.700 */
#define FLAG_VCOM_0705        0b00000100     /* 0.705 */
#define FLAG_VCOM_0710        0b00000101     /* 0.710 */
#define FLAG_VCOM_0715        0b00000110     /* 0.715 */
#define FLAG_VCOM_0720        0b00000111     /* 0.720 */
#define FLAG_VCOM_0725        0b00001000     /* 0.725 */
#define FLAG_VCOM_0730        0b00001001     /* 0.730 */
#define FLAG_VCOM_0735        0b00001010     /* 0.735 */
#define FLAG_VCOM_0740        0b00001011     /* 0.740 */
#define FLAG_VCOM_0745        0b00001100     /* 0.745 */
#define FLAG_VCOM_0750        0b00001101     /* 0.750 */
#define FLAG_VCOM_0755        0b00001110     /* 0.755 */
#define FLAG_VCOM_0760        0b00001111     /* 0.760 */
#define FLAG_VCOM_0765        0b00010000     /* 0.765 */
#define FLAG_VCOM_0770        0b00010001     /* 0.770 */
#define FLAG_VCOM_0775        0b00010010     /* 0.775 */
#define FLAG_VCOM_0780        0b00010011     /* 0.780 */
#define FLAG_VCOM_0785        0b00010100     /* 0.785 */
#define FLAG_VCOM_0790        0b00010101     /* 0.790 */
#define FLAG_VCOM_0795        0b00010110     /* 0.795 */
#define FLAG_VCOM_0800        0b00010111     /* 0.800 */
#define FLAG_VCOM_0805        0b00011000     /* 0.805 */
#define FLAG_VCOM_0810        0b00011001     /* 0.810 */
#define FLAG_VCOM_0815        0b00011010     /* 0.815 */
#define FLAG_VCOM_0820        0b00011011     /* 0.820 */
#define FLAG_VCOM_0825        0b00011100     /* 0.825 */
#define FLAG_VCOM_0830        0b00011101     /* 0.830 */
#define FLAG_VCOM_0835        0b00011110     /* 0.835 */
#define FLAG_VCOM_0840        0b00011111     /* 0.840 */
#define FLAG_VCOM_0845        0b00100000     /* 0.845 */
#define FLAG_VCOM_0850        0b00100001     /* 0.850 */
#define FLAG_VCOM_0855        0b00100010     /* 0.855 */
#define FLAG_VCOM_0860        0b00100011     /* 0.860 */
#define FLAG_VCOM_0865        0b00100100     /* 0.865 */
#define FLAG_VCOM_0870        0b00100101     /* 0.870 */
#define FLAG_VCOM_0875        0b00100110     /* 0.875 */
#define FLAG_VCOM_0880        0b00100111     /* 0.880 */
#define FLAG_VCOM_0885        0b00101000     /* 0.885 */
#define FLAG_VCOM_0890        0b00101001     /* 0.890 */
#define FLAG_VCOM_0895        0b00101010     /* 0.895 */
#define FLAG_VCOM_0900        0b00101011     /* 0.900 */
#define FLAG_VCOM_0905        0b00101100     /* 0.905 */
#define FLAG_VCOM_0910        0b00101101     /* 0.910 */
#define FLAG_VCOM_0915        0b00101110     /* 0.915 */
#define FLAG_VCOM_0920        0b00101111     /* 0.920 */
#define FLAG_VCOM_0925        0b00110000     /* 0.925 */
#define FLAG_VCOM_0930        0b00110001     /* 0.930 */
#define FLAG_VCOM_0935        0b00110010     /* 0.935 */
#define FLAG_VCOM_0940        0b00110011     /* 0.940 */
#define FLAG_VCOM_0945        0b00110100     /* 0.945 */
#define FLAG_VCOM_0950        0b00110101     /* 0.950 */
#define FLAG_VCOM_0955        0b00110110     /* 0.955 */
#define FLAG_VCOM_0960        0b00110111     /* 0.960 */
#define FLAG_VCOM_0965        0b00111000     /* 0.965 */
#define FLAG_VCOM_0970        0b00111001     /* 0.970 */
#define FLAG_VCOM_0975        0b00111010     /* 0.975 */
#define FLAG_VCOM_0980        0b00111011     /* 0.980 */
#define FLAG_VCOM_0985        0b00111100     /* 0.985 */
#define FLAG_VCOM_0990        0b00111101     /* 0.990 */
#define FLAG_VCOM_0995        0b00111110     /* 0.995 */
#define FLAG_VCOM_1000        0b00111111     /* 1.000 */

/* data byte #3: */
  /* VREG1OUT factor to generate VCOM AC: */
#define FLAG_VCOM_AC_070      0b00000000     /* 0.70 */
#define FLAG_VCOM_AC_072      0b00000001     /* 0.72 */
#define FLAG_VCOM_AC_074      0b00000010     /* 0.74 */
#define FLAG_VCOM_AC_076      0b00000011     /* 0.76 */
#define FLAG_VCOM_AC_078      0b00000100     /* 0.78 */
#define FLAG_VCOM_AC_080      0b00000101     /* 0.80 */
#define FLAG_VCOM_AC_082      0b00000110     /* 0.82 */
#define FLAG_VCOM_AC_084      0b00000111     /* 0.84 */
#define FLAG_VCOM_AC_086      0b00001000     /* 0.86 */
#define FLAG_VCOM_AC_088      0b00001001     /* 0.88 */
#define FLAG_VCOM_AC_090      0b00001010     /* 0.90 */
#define FLAG_VCOM_AC_092      0b00001011     /* 0.92 */
#define FLAG_VCOM_AC_094      0b00001100     /* 0.94 */
#define FLAG_VCOM_AC_096      0b00001101     /* 0.96 */
#define FLAG_VCOM_AC_098      0b00001110     /* 0.98 */
#define FLAG_VCOM_AC_100      0b00001111     /* 1.00 */
#define FLAG_VCOM_AC_102      0b00010000     /* 1.02 */
#define FLAG_VCOM_AC_104      0b00010001     /* 1.04 */
#define FLAG_VCOM_AC_106      0b00010010     /* 1.06 */
#define FLAG_VCOM_AC_108      0b00010011     /* 1.08 */
#define FLAG_VCOM_AC_110      0b00010100     /* 1.10 */
#define FLAG_VCOM_AC_112      0b00010101     /* 1.12 */
#define FLAG_VCOM_AC_114      0b00010110     /* 1.14 */
#define FLAG_VCOM_AC_116      0b00010111     /* 1.16 */
#define FLAG_VCOM_AC_118      0b00011000     /* 1.18 */
#define FLAG_VCOM_AC_120      0b00011001     /* 1.20 */
#define FLAG_VCOM_AC_122      0b00011010     /* 1.22 */
#define FLAG_VCOM_AC_124      0b00011011     /* 1.24 */
#define FLAG_VCOM_AC_126      0b00011100     /* 1.26 */
#define FLAG_VCOM_AC_128      0b00011101     /* 1.28 */
#define FLAG_VCOM_AC_130      0b00011110     /* 1.30 */
#define FLAG_VCOM_AC_132      0b00011111     /* 1.32 */


/*
 *  power setting for normal mode
 *  - 1 byte cmd + 2 byte data
 */

#define CMD_POWER_NORMAL      0b11010010     /* power control normal mode */

/* data byte #1: */
  /* constant current of power supply OPAMP: gamma driver / source driver */
#define FLAG_AP0_0            0b00000000     /* halt / halt */
#define FLAG_AP0_1            0b00000001     /* 1.00 / 1.00 */
#define FLAG_AP0_2            0b00000010     /* 1.00 / 0.75 */
#define FLAG_AP0_3            0b00000011     /* 1.00 / 0.50 */
#define FLAG_AP0_4            0b00000100     /* 0.75 / 1.00 */
#define FLAG_AP0_5            0b00000101     /* 0.75 / 0.75 */
#define FLAG_AP0_6            0b00000110     /* 0.75 / 0.50 */
#define FLAG_AP0_7            0b00000111     /* 0.50 / 0.50 */

/* data byte #2: */
  /* charge-pump frequency for step-up circuit 1: f_OSC */
#define FLAG_DC00_1           0b00000000     /* 1/1 */
#define FLAG_DC00_2           0b00000001     /* 1/2 */
#define FLAG_DC00_4           0b00000010     /* 1/4 */
#define FLAG_DC00_8           0b00000011     /* 1/8 */
#define FLAG_DC00_16          0b00000100     /* 1/16 */
#define FLAG_DC00_32          0b00000101     /* 1/32 */
#define FLAG_DC00_64          0b00000110     /* 1/64 */
#define FLAG_DC00_HALT        0b00000111     /* halt */
  /* charge-pump frequency for step-up circuit 2: f_OSC */
#define FLAG_DC10_16          0b00000000     /* 1/16 */
#define FLAG_DC10_32          0b00010000     /* 1/32 */
#define FLAG_DC10_64          0b00100000     /* 1/64 */
#define FLAG_DC10_128         0b00110000     /* 1/128 */
#define FLAG_DC10_256         0b01000000     /* 1/256 */
#define FLAG_DC10_512         0b01010000     /* 1/512 */
#define FLAG_DC10_INH         0b01100000     /* inhibit */
#define FLAG_DC10_HALT        0b01110000     /* halt */


/*
 *  power setting for partial mode
 *  - 1 byte cmd + 2 byte data
 */

#define CMD_POWER_PARTIAL  0b11010011     /* power control partial mode */

/* data byte #1: */
  /* constant current of power supply OPAMP: gamma driver / source driver */
#define FLAG_AP1_0            0b00000000     /* halt / halt */
#define FLAG_AP1_1            0b00000001     /* 1.00 / 1.00 */
#define FLAG_AP1_2            0b00000010     /* 1.00 / 0.75 */
#define FLAG_AP1_3            0b00000011     /* 1.00 / 0.50 */
#define FLAG_AP1_4            0b00000100     /* 0.75 / 1.00 */
#define FLAG_AP1_5            0b00000101     /* 0.75 / 0.75 */
#define FLAG_AP1_6            0b00000110     /* 0.75 / 0.50 */
#define FLAG_AP1_7            0b00000111     /* 0.50 / 0.50 */

/* data byte #2: */
  /* charge-pump frequency for step-up circuit 1: f_OSC */
#define FLAG_DC01_1           0b00000000     /* 1/1 */
#define FLAG_DC01_2           0b00000001     /* 1/2 */
#define FLAG_DC01_4           0b00000010     /* 1/4 */
#define FLAG_DC01_8           0b00000011     /* 1/8 */
#define FLAG_DC01_16          0b00000100     /* 1/16 */
#define FLAG_DC01_32          0b00000101     /* 1/32 */
#define FLAG_DC01_64          0b00000110     /* 1/64 */
#define FLAG_DC01_HALT        0b00000111     /* halt */
  /* charge-pump frequency for step-up circuit 2: f_OSC */
#define FLAG_DC11_16          0b00000000     /* 1/16 */
#define FLAG_DC11_32          0b00010000     /* 1/32 */
#define FLAG_DC11_64          0b00100000     /* 1/64 */
#define FLAG_DC11_128         0b00110000     /* 1/128 */
#define FLAG_DC11_256         0b01000000     /* 1/256 */
#define FLAG_DC11_512         0b01010000     /* 1/512 */
#define FLAG_DC11_INH         0b01100000     /* inhibit */
#define FLAG_DC11_HALT        0b01110000     /* halt */


/*
 *  power setting for idle mode
 *  - 1 byte cmd + 2 byte data
 */

#define CMD_POWER_IDLE        0b11010100     /* power control idle mode */

/* data byte #1: */
  /* constant current of power supply OPAMP: gamma driver / source driver */
#define FLAG_AP2_0            0b00000000     /* halt / halt */
#define FLAG_AP2_1            0b00000001     /* 1.00 / 1.00 */
#define FLAG_AP2_2            0b00000010     /* 1.00 / 0.75 */
#define FLAG_AP2_3            0b00000011     /* 1.00 / 0.50 */
#define FLAG_AP2_4            0b00000100     /* 0.75 / 1.00 */
#define FLAG_AP2_5            0b00000101     /* 0.75 / 0.75 */
#define FLAG_AP2_6            0b00000110     /* 0.75 / 0.50 */
#define FLAG_AP2_7            0b00000111     /* 0.50 / 0.50 */

/* data byte #2: */
  /* charge-pump frequency for step-up circuit 1: f_OSC */
#define FLAG_DC02_1           0b00000000     /* 1/1 */
#define FLAG_DC02_2           0b00000001     /* 1/2 */
#define FLAG_DC02_4           0b00000010     /* 1/4 */
#define FLAG_DC02_8           0b00000011     /* 1/8 */
#define FLAG_DC02_16          0b00000100     /* 1/16 */
#define FLAG_DC02_32          0b00000101     /* 1/32 */
#define FLAG_DC02_64          0b00000110     /* 1/64 */
#define FLAG_DC02_HALT        0b00000111     /* halt */
  /* charge-pump frequency for step-up circuit 2: f_OSC */
#define FLAG_DC12_16          0b00000000     /* 1/16 */
#define FLAG_DC12_32          0b00010000     /* 1/32 */
#define FLAG_DC12_64          0b00100000     /* 1/64 */
#define FLAG_DC12_128         0b00110000     /* 1/128 */
#define FLAG_DC12_256         0b01000000     /* 1/256 */
#define FLAG_DC12_512         0b01010000     /* 1/512 */
#define FLAG_DC12_INH         0b01100000     /* inhibit */
#define FLAG_DC12_HALT        0b01110000     /* halt */


/*
 *  write NV memory
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_WRITE_NV          0b11100000     /* write NV memory */

/* data byte #1: data */


/*
 *  NV memory control
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_NV_CTRL           0b11100001     /* NV memory control */

/* data byte #1: */
  /* ID selection: */
#define FLAG_ID1_H            0b00000000     /* ID code 1 - MSB (bits 8-15) */
#define FLAG_ID1_L            0b00000001     /* ID code 1 - LSB (bits 0-7) */
#define FLAG_ID2_H            0b00000010     /* ID code 2 - MSB (bits 8-15) */
#define FLAG_ID2_L            0b00000011     /* ID code 2 - LSB (bits 0-7) */
  /* OTP control: */
#define FLAG_PROG_OFF         0b00000000     /* NV programming disabled */
#define FLAG_PROG_VCOM        0b00010000     /* VCOM programming enabled */
#define FLAG_PROG_ID          0b00100000     /* ID code programming enabled */


/*
 *  read NV memory status
 *  - 1 byte cmd + 3 bytes data (read mode)
 */

#define CMD_READ_NV_STATUS    0b11100010     /* read NV memory status  */

/* data byte #1: dummy data */

/* data byte #2: */
  /* write counter: */
#define FLAG_NV_CNT_0         0b00000000     /* not programmed */
#define FLAG_NV_CNT_1         0b00000001     /* programmed 1 time */
#define FLAG_NV_CNT_2         0b00000010     /* programmed 2 times */

/* data byte #3: */
  /* VCOM value stored in NV memory: bits 0-5 */


/*
 *  NV memory protection (for writing)
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_NV_PROTECT        0b11100011     /* NV memory protection */

/* data byte #1: key - MSB (bits 8-15) */
#define FLAG_KEY_1            0xAA           /* fixed key */

/* data byte #2: key - LSB (bits 0-7) */
#define FLAG_KEY_0            0x55           /* fixed key */



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

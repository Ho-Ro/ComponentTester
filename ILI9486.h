/* ************************************************************************
 *
 *   ILI9486 color graphic display controller
 *
 *   (c) 2020 by Markus Reschke
 *
 * ************************************************************************ */


/* ************************************************************************
 *   regular command set
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
 *  read display identification information
 *  - 1 byte cmd + 4 bytes data (read mode)
 */

#define CMD_READ_DISP_ID      0b00000100     /* read display ID */

/* data byte #1: dummy byte */
/* data byte #2: manufacturer ID */
/* data byte #3: module/driver version ID */
/* data byte #4: module/driver ID */


/*
 *  read number of MIPI DSI errors (MIPI Display Serial Interface)
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_ERRORS       0b00000101     /* read number of errors */

/* data byte #1: dummy data */

/* data byte #2: number of errors */
  /* bits 0-6: number of errors */
  /* bit 7:    overflow indicator */


/*
 *  read display status
 *  - 1 byte cmd + 5 bytes data (read mode)
 */

#define CMD_READ_STATUS       0b00001001     /* read display status */

/* data byte #1: dummy data */

/* data byte #2: status */
  /* horizontal refresh: */
#define FLAG_STAT_HOR_NORM    0b00000000     /* left to right */
#define FLAG_STAT_HOR_REV     0b00000010     /* right to left */
  /* RGB/BGR order */
#define FLAG_STAT_COLOR_RGB   0b00000000     /* RGB */
#define FLAG_STAT_COLOR_BGR   0b00000100     /* BGR */
  /* vertical refresh: */
#define FLAG_STAT_VER_NORM    0b00000000     /* top to bottom */
#define FLAG_STAT_VER_REV     0b00001000     /* bottom to top */
  /* row/column exchange: */
#define FLAG_STAT_XY_NORM     0b00000000     /* normal */
#define FLAG_STAT_XY_REV      0b00010000     /* reversed (X & Y swapped) */
  /* column address order: */
#define FLAG_STAT_COL_NORM    0b00000000     /* left to right */
#define FLAG_STAT_COL_REV     0b00100000     /* right to left */
  /* row address order: */
#define FLAG_STAT_ROW_NORM    0b00000000     /* top to bottom */
#define FLAG_STAT_ROW_REV     0b01000000     /* bottom to top */
  /* booster voltage: */
#define FLAG_STAT_BOOST_OFF   0b00000000     /* booster on */
#define FLAG_STAT_BOOST_ON    0b10000000     /* booster off */

/* data byte #3: status */
  /* display normal mode: */
#define FLAG_STAT_NORM_OFF    0b00000000     /* off */
#define FLAG_STAT_NORM_ON     0b00000001     /* on */
  /* sleep mode: */
#define FLAG_STAT_SLEEP_OFF   0b00000000     /* off */
#define FLAG_STAT_SLEEP_ON    0b00000010     /* on */
  /* partial mode: */
#define FLAG_STAT_PART_OFF    0b00000000     /* off */
#define FLAG_STAT_PART_ON     0b00000100     /* on */
  /* idle mode: */
#define FLAG_STAT_IDLE_OFF    0b00000000     /* off */
#define FLAG_STAT_IDLE_ON     0b00001000     /* on */
  /* interface color pixel format: */
#define FLAG_STAT_PIX_12      0b00110000     /* 12 bits per pixel */
#define FLAG_STAT_PIX_16      0b01010000     /* 16 bits per pixel */
#define FLAG_STAT_PIX_18      0b01100000     /* 18 bits per pixel */

/* data byte #4: status */
  /* gamma curve - bit 2: bit 0 (always 0) */
  /* tearing effect line: */
#define FLAG_STAT_TEAR_OFF    0b00000000     /* off */
#define FLAG_STAT_TEAR_ON     0b00000010     /* on */
  /* display: */
#define FLAG_STAT_DISP_OFF    0b00000000     /* off */
#define FLAG_STAT_DISP_ON     0b00000100     /* on */
  /* inversion: */
#define FLAG_STAT_INVERS_OFF  0b00000000     /* off */
#define FLAG_STAT_INVERS_ON   0b00100000     /* on */
  /* vertical scrolling: */
#define FLAG_STAT_VSCROLL_OFF 0b00000000     /* off */
#define FLAG_STAT_VSCROLL_ON  0b10000000     /* on */

/* data byte #5: status */
  /* tearing effect line mode: */
#define FLAG_STAT_TEAR_MODE1  0b00000000     /* mode 1: V-blanking */
#define FLAG_STAT_TEAR_MODE2  0b00100000     /* mode 2: V-blanking and H-blanking */
  /* gamma curve - bits 1 and 0: */
#define FLAG_STAT_GAMMA0      0b00000000     /* gamma curve 0 */
#define FLAG_STAT_GAMMA1      0b01000000     /* gamma curve 1 */
#define FLAG_STAT_GAMMA2      0b10000000     /* gamma curve 2 */
#define FLAG_STAT_GAMMA3      0b11000000     /* gamma curve 3 */


/*
 *  read display power mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_PWR_MODE     0b00001010     /* read display power mode */

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
 *  read display MADCTL (memory acccess control)
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_MADCTL       0b00001011     /* read display MADCTL */

/* data byte #1: dummy data */

/* data byte #2: status */
/* same as byte #1 of CMD_MEM_CTRL */
  /* display data latch data order (horizontal refereshing): */
#define RFLAG_HREFRESH_NORM   0b00000000     /* left to right */
#define RFLAG_HREFRESH_REV    0b00000100     /* right to left */
  /* color order: */
#define RFLAG_COLOR_RGB       0b00000000     /* RGB */
#define RFLAG_COLOR_BGR       0b00001000     /* BGR */
  /* line address order (vertical refereshing): */
#define RFLAG_VREFRESH_NORM   0b00000000     /* top to bottom */
#define RFLAG_VREFRESH_REV    0b00010000     /* bottom to top */
  /* page/column order: */
#define RFLAG_XY_NORM         0b00000000     /* normal */
#define RFLAG_XY_REV          0b00100000     /* reversed (X & Y swapped) */
  /* column address order: */
#define RFLAG_COL_NORM        0b00000000     /* left to right */
#define RFLAG_COL_REV         0b01000000     /* right to left */
  /* page (row) address order: */
#define RFLAG_PAGE_NORM       0b00000000     /* top to bottom */
#define RFLAG_PAGE_REV        0b10000000     /* bottom to top */


/*
 *  read display pixel format
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_PIX_FORMAT   0b00001100     /* read display pixel format */

/* data byte #1: dummy data */

/* data byte #2: status */
/* same as byte #1 of CMD_SET_PIX_FORMAT */
  /* pixel format of display bus interface (MCU interface): */
#define RFLAG_DBI_16          0b00000101     /* 16 bits per pixel */
#define RFLAG_DBI_18          0b00000110     /* 18 bits per pixel */
  /* pixel format of display pixel interface (RGB interface): */
#define RFLAG_DPI_16          0b01010000     /* 16 bits per pixel */
#define RFLAG_DPI_18          0b01100000     /* 18 bits per pixel */


/*
 *  read display image mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_IMAGE_MODE   0b00001101     /* read display image mode */

/* data byte #1: dummy data */

/* data byte #2: status */
  /* inversion */
#define FLAG_INVERSION_OFF    0b00000000     /* off */
#define FLAG_INVERSION_ON     0b00100000     /* on */
  /* vertical scrolling */
#define FLAG_VSCROLL_OFF      0b00000000     /* off */
#define FLAG_VSCROLL_ON       0b10000000     /* on */


/*
 *  read display signal mode
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_SIGNAL_MODE  0b00001110     /* read display signal mode */

/* data byte #1: dummy data */

/* data byte #2: mode flags */
  /* MIPI DSI errors: */
#define FLAG_SIG_ERR_OFF      0b00000000     /* no errors */
#define FLAG_SIG_ERR_ON       0b00000001     /* errors detected */
  /* data enable (DE, RGB interface): */
#define FLAG_SIG_DE_OFF       0b00000000     /* data enable off */
#define FLAG_SIG_DE_ON        0b00000100     /* data enable on */
  /* pixel clock (DOTCLK, RGB interface): */
#define FLAG_SIG_CLK_OFF      0b00000000     /* pixel clock off */
#define FLAG_SIG_CLK_ON       0b00001000     /* pixel clock on */
  /* vertical sync (RGB interface): */
#define FLAG_SIG_VSYNC_OFF    0b00000000     /* V-sync off */
#define FLAG_SIG_VSYNC_ON     0b00010000     /* V-sync on */
  /* horizontal sync (RGB interface): */
#define FLAG_SIG_HSYNC_OFF    0b00000000     /* H-sync off */
#define FLAG_SIG_HSYNC_ON     0b00100000     /* H-sync on */
  /* tearing effect line mode: */
#define FLAG_SIG_TEAR_1       0b00000000     /* effect mode 1 */
#define FLAG_SIG_TEAR_2       0b01000000     /* effect mode 2 */
  /* tearing effect line: */
#define FLAG_SIG_TEAR_OFF     0b00000000     /* effect off */
#define FLAG_SIG_TEAR_ON      0b10000000     /* effect on */


/*
 *  read display self-diagnostic result
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_DIAG         0b00001111     /* read diagnostics */

/* data byte #1: dummy data */

/* data byte #2: diag flags */
#define FLAG_DIAG_SUM_ERR     0b00000001     /* checksums don't match */
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

/* data byte #1: start column - MSB (bits 15-8) */
/* data byte #2: start column - LSB (bits 7-0) */
/* data byte #3: end column - MSB (bits 15-8) */
/* data byte #4: end column - LSB (bits 7-0) */
/* valid range: 0x0000 - 0x013f/0x01df */


/*
 *  set page address (row, accessible frame area)
 *  - 1 byte cmd + 4 bytes data
 */

#define CMD_PAGE_ADDR_SET     0b00101011     /* set page address */

/* data byte #1: start page - MSB (bits 15-8) */
/* data byte #2: start page - LSB (bits 7-0) */
/* data byte #3: end page - MSB (bits 15-8) */
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

/* data byte #1: start row - MSB (bits 15-8) */
/* data byte #2: start row - LSB (bits 7-0) */
/* data byte #3: end row - MSB (bits 15-8) */
/* data byte #4: end row - LSB (bits 7-0) */
/* valid range: 0x0000 - 0x01df */


/*
 *  vertical scrolling definition
 *  - 1 byte cmd + 6 bytes data
 */

#define CMD_V_SCROLL_DEF      0b00110011     /* set vertical scrolling area */

/* data byte #1: top fixed area line number - MSB (bits 15-8) */
/* data byte #2: top fixed area line number - LSB (bits 7-0) */
/* data byte #3: height of scrolling area - MSB (bits 15-8) */
/* data byte #4: height of scrolling area - LSB (bits 7-0) */
/* data byte #5: bottom fixed area line number - MSB (bits 15-8) */
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
 *  memory access control
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_MEM_CTRL          0b00110110     /* memory access control */

/* data byte #1: read/write scanning direction of frame memory */
/* same as byte #2 of CMD_READ_MADCTL */
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

/* data byte #1: line number - MSB (bits 15-8) */
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
#define FLAG_DBI_16           0b00000101     /* 16 bits per pixel */
#define FLAG_DBI_18           0b00000110     /* 18 bits per pixel */
  /* pixel format of display pixel interface (RGB interface): */
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

/* data byte #1: line number - MSB (bits 15-8) */
#define FLAG_STS_HIGH_MIN     0b00000000     /* minimum value */
#define FLAG_STS_HIGH_MAX     0b00000001     /* maximum value */

/* data byte #2: line number - LSB (bits 7-0) */
#define FLAG_STS_LOW_MIN      0b00000000     /* minimum value */
#define FLAG_STS_LOW_MAX      0b11111111     /* maximum value */


/*
 *  get tearing effect scan line
 *  - 1 byte cmd + 3 bytes data (read mode)
 */

#define CMD_READ_SCANLINE     0b01000101     /* read scan line  */

/* data byte #1: dummy data */
/* data byte #2: line number - MSB (bits 15-8) */
/* data byte #3: line number - LSB (bits 7-0) */


/*
 *  write display brightness
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_WRITE_BRIGHT      0b01010001     /* write display brightness */

/* data byte #1: brightness value */
#define FLAG_BRIGHT_MIN       0b00000000     /* minimum value */
#define FLAG_BRIGHT_MAX       0b11111111     /* maximum value */


/*
 *  read display brightness 
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_BRIGHT       0b01010010     /* read display brightness */

/* data byte #1: dummy data */
/* data byte #2: brightness value, please see above */


/*
 *  write CTRL
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_WRITE_CTRL        0b01010011     /* write CTRL */

/* data byte #1: settings */
  /* backlight: */
#define FLAG_BACKLIGHT_OFF    0b00000000     /* off */
#define FLAG_BACKLIGHT_ON     0b00000100     /* on */
  /* display dimming for manual brightness setting: */
#define FLAG_DIMMING_OFF      0b00000000     /* off */
#define FLAG_DIMMING_ON       0b00001000     /* on */
  /* brightness control block: */
#define FLAG_BCTRL_OFF        0b00000000     /* off (registers are 0x00) */
#define FLAG_BCTRL_ON         0b00100000     /* on (registers are active) */


/*
 *  read CTRL 
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_CTRL         0b01010100     /* read CTRL */

/* data byte #1: dummy data */
/* data byte #2: settings, please see above */


/*
 *  write content adaptive brightness control (CABC)
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_WRITE_CABC        0b01010101     /* write content adaptive brightness control */

/* data byte #1: mode */
#define FLAG_CABC_OFF         0b00000000     /* off */
#define FLAG_CABC_INTERFACE   0b00000001     /* user interface image */
#define FLAG_CABC_STILL       0b00000010     /* still picture */
#define FLAG_CABC_MOVING      0b00000011     /* moving image */


/*
 *  read content adaptive brightness control (CABC) 
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_CABC         0b01010110     /* read CABC */

/* data byte #1: dummy data */
/* data byte #2: mode, please see FLAG_CABC_* above */


/*
 *  write CABC minimum brightness
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_WRITE_MIN_CABC    0b01011110     /* write CABC minimum brightness */

/* data byte #1: minimum brightness level of CABC function */
#define FLAG_CABC_MIN         0b00000000     /* minimum value */
#define FLAG_CABC_MAX         0b11111111     /* maximum value */


/*
 *  read CABC minimum brightness
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_MIN_CABC     0b01011111     /* read CABC minimum brightness */

/* data byte #1: dummy data */
/* data byte #2: minimum brightness level of CABC function */


/*
 *  read first checksum
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_FIRST_CHECKSUM    0b10101010     /* read first checksum */

/* data byte #1: dummy data */
/* data byte #2: checksum */


/*
 *  read continue checksum
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_CONT_CHECKSUM     0b10101111     /* read continue checksum */

/* data byte #1: dummy data */
/* data byte #2: checksum */


/*
 *  read ID1 (manufacturer)
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_ID1          0b11011010     /* read ID1 */

/* data byte #1: dummy data */
/* data byte #2: LCD module's manufacturer ID */


/*
 *  read ID2 (driver version)
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_ID2          0b11011011     /* read ID2 */

/* data byte #1: dummy data */
/* data byte #2: LCD module/driver version (default: 0x80) */


/*
 *  read ID3 (module/driver)
 *  - 1 byte cmd + 2 bytes data (read mode)
 */

#define CMD_READ_ID3          0b11011100     /* read ID3 */

/* data byte #1: dummy data */
/* data byte #2: LCD module/driver (default: 0x66) */



/* ************************************************************************
 *   extended command set
 * ************************************************************************ */


/*
 *  interface mode control
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_IF_MODE_CTRL      0b10110000     /* interface mode control */

/* data byte #1: switches */
  /* DE polarity for RGB interface: */
#define FLAG_EPL_HIGH         0b00000000     /* high enable */
#define FLAG_EPL_LOW          0b00000001     /* low enable */
  /* PCLK polarity: */
#define FLAG_DPL_RISE         0b00000000     /* data fetched at rising edge */
#define FLAG_DPL_FALL         0b00000010     /* data fetched at falling edge */
  /* HSYNC polarity: */
#define FLAG_HSPL_LOW         0b00000000     /* low level sync */
#define FLAG_HSPL_HIGH        0b00000100     /* high level sync */
  /* VSYNC polarity: */
#define FLAG_VSPL_LOW         0b00000000     /* low level sync */
#define FLAG_VSPL_HIGH        0b00001000     /* high level sync */
  /* 3/4 wire serial interface: */
#define FLAG_SDA_1            0b00000000     /* use DIN & DOUT */
#define FLAG_SDA_2            0b10000000     /* use DIN/SDA only */


/*
 *  frame rate control for normal mode (full colors)
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_FRAME_CTRL_NORM   0b10110001     /* frame rate for normal mode */

/* data byte #1: */
  /* division ratio of internal clock: */
#define FLAG_DIVA_1           0b00000000     /* f_OSC */
#define FLAG_DIVA_2           0b00000001     /* f_OSC/2 */
#define FLAG_DIVA_4           0b00000010     /* f_OSC/4 */
#define FLAG_DIVA_8           0b00000011     /* f_OSC/8 */
  /* frame frequency */
#define FLAG_FRS_28           0b00000000     /* 28 Hz */
#define FLAG_FRS_30           0b00010000     /* 30 Hz */
#define FLAG_FRS_32           0b00100000     /* 32 Hz */
#define FLAG_FRS_34           0b00110000     /* 34 Hz */
#define FLAG_FRS_36           0b01000000     /* 36 Hz */
#define FLAG_FRS_39           0b01010000     /* 39 Hz */
#define FLAG_FRS_42           0b01100000     /* 42 Hz */
#define FLAG_FRS_46           0b01110000     /* 46 Hz */
#define FLAG_FRS_50           0b10000000     /* 50 Hz */
#define FLAG_FRS_56           0b10010000     /* 56 Hz */
#define FLAG_FRS_62           0b10100000     /* 62 Hz */
#define FLAG_FRS_70           0b10110000     /* 70 Hz */
#define FLAG_FRS_81           0b11000000     /* 81 Hz */
#define FLAG_FRS_96           0b11010000     /* 96 Hz */
#define FLAG_FRS_117          0b11100000     /* 117 Hz */
//#define FLAG_FRS_117          0b11110000     /* 117 Hz */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTNA_16          0b00010000     /* 16 clocks */
#define FLAG_RTNA_17          0b00010001     /* 17 clocks */
#define FLAG_RTNA_18          0b00010010     /* 18 clocks */
#define FLAG_RTNA_19          0b00010011     /* 19 clocks */
#define FLAG_RTNA_20          0b00010100     /* 20 clocks */
#define FLAG_RTNA_21          0b00010101     /* 21 clocks */
#define FLAG_RTNA_22          0b00010110     /* 22 clocks */
#define FLAG_RTNA_23          0b00010111     /* 23 clocks */
#define FLAG_RTNA_24          0b00011000     /* 24 clocks */
#define FLAG_RTNA_25          0b00011001     /* 25 clocks */
#define FLAG_RTNA_26          0b00011010     /* 26 clocks */
#define FLAG_RTNA_27          0b00011011     /* 27 clocks */
#define FLAG_RTNA_28          0b00011100     /* 28 clocks */
#define FLAG_RTNA_29          0b00011101     /* 29 clocks */
#define FLAG_RTNA_30          0b00011110     /* 30 clocks */
#define FLAG_RTNA_31          0b00011111     /* 31 clocks */


/*
 *  frame rate control for idle mode (8bit color depth)
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_FRAME_CTRL_IDLE   0b10110010     /* frame rate for idle mode */

/* data byte #1: division ratio of internal clock */
#define FLAG_DIVB_1           0b00000000     /* f_OSC */
#define FLAG_DIVB_2           0b00000001     /* f_OSC/2 */
#define FLAG_DIVB_4           0b00000010     /* f_OSC/4 */
#define FLAG_DIVB_8           0b00000011     /* f_OSC/8 */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTNB_16          0b00010000     /* 16 clocks */
#define FLAG_RTNB_17          0b00010001     /* 17 clocks */
#define FLAG_RTNB_18          0b00010010     /* 18 clocks */
#define FLAG_RTNB_19          0b00010011     /* 19 clocks */
#define FLAG_RTNB_20          0b00010100     /* 20 clocks */
#define FLAG_RTNB_21          0b00010101     /* 21 clocks */
#define FLAG_RTNB_22          0b00010110     /* 22 clocks */
#define FLAG_RTNB_23          0b00010111     /* 23 clocks */
#define FLAG_RTNB_24          0b00011000     /* 24 clocks */
#define FLAG_RTNB_25          0b00011001     /* 25 clocks */
#define FLAG_RTNB_26          0b00011010     /* 26 clocks */
#define FLAG_RTNB_27          0b00011011     /* 27 clocks */
#define FLAG_RTNB_28          0b00011100     /* 28 clocks */
#define FLAG_RTNB_29          0b00011101     /* 29 clocks */
#define FLAG_RTNB_30          0b00011110     /* 30 clocks */
#define FLAG_RTNB_31          0b00011111     /* 31 clocks */


/*
 *  frame rate control for partial mode (full colors)
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_FRAME_CTRL_PART   0b10110011     /* frame rate for partial mode */

/* data byte #1: division ratio of internal clock */
#define FLAG_DIVC_1           0b00000000     /* f_OSC */
#define FLAG_DIVC_2           0b00000001     /* f_OSC/2 */
#define FLAG_DIVC_4           0b00000010     /* f_OSC/4 */
#define FLAG_DIVC_8           0b00000011     /* f_OSC/8 */

/* data byte #2: line period (clocks per line) */
#define FLAG_RTNC_16          0b00010000     /* 16 clocks */
#define FLAG_RTNC_17          0b00010001     /* 17 clocks */
#define FLAG_RTNC_18          0b00010010     /* 18 clocks */
#define FLAG_RTNC_19          0b00010011     /* 19 clocks */
#define FLAG_RTNC_20          0b00010100     /* 20 clocks */
#define FLAG_RTNC_21          0b00010101     /* 21 clocks */
#define FLAG_RTNC_22          0b00010110     /* 22 clocks */
#define FLAG_RTNC_23          0b00010111     /* 23 clocks */
#define FLAG_RTNC_24          0b00011000     /* 24 clocks */
#define FLAG_RTNC_25          0b00011001     /* 25 clocks */
#define FLAG_RTNC_26          0b00011010     /* 26 clocks */
#define FLAG_RTNC_27          0b00011011     /* 27 clocks */
#define FLAG_RTNC_28          0b00011100     /* 28 clocks */
#define FLAG_RTNC_29          0b00011101     /* 29 clocks */
#define FLAG_RTNC_30          0b00011110     /* 30 clocks */
#define FLAG_RTNC_31          0b00011111     /* 31 clocks */


/*
 *  display inversion control
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_INVERS_CTRL       0b10110100     /* display inversion control */

/* data byte #1: mode switches */
  /* dot inversion mode: */
#define FLAG_DINV_COL         0b00000000     /* column inversion */
#define FLAG_DINV_1DOT        0b00000001     /* 1-dot inversion */
#define FLAG_DINV_2DOT        0b00000010     /* 2-dot inversion */
  /* Z inversion mode: */
#define FLAG_ZINV_OFF         0b00000000     /* disable */
#define FLAG_ZINV_ON          0b00010000     /* enable */
 

/*
 *  blanking porch control
 *  - 1 byte cmd + 4 byte data
 */

#define CMD_BLANK_CTRL        0b10110101     /* blanking porch control */

/* data byte #1: line number of vertical front porch period */
  /* number of HSYNCs, please see datasheet: */
#define FLAG_VFP_MIN          0b00000010     /* minimum value */
#define FLAG_VFP_MAX          0b11111111     /* maximum value */

/* data byte #2: line number of vertical back porch period */
  /* number of HSYNCs, please see datasheet: */
#define FLAG_VBP_MIN          0b00000010     /* minimum value */
#define FLAG_VBP_MAX          0b11111111     /* maximum value */

/* data byte #3: line number of horizontal front porch period */
  /* number of DOTCLKs, please see datasheet: */
#define FLAG_HFP_MIN          0b00000010     /* minimum value */
#define FLAG_HFP_MAX          0b00011111     /* maximum value */

/* data byte #4: line number of horizontal back porch period */
  /* number of DOTCLKs, please see datasheet: */
#define FLAG_HBP_MIN          0b00000010     /* minimum value */
#define FLAG_HBP_MAX          0b11111111     /* maximum value */


/*
 *  display function control
 *  - 1 byte cmd + 3 bytes data
 */

#define CMD_FUNC_CTRL         0b10110110     /* display function control */

/* data byte #1: */
  /* source/VCOM output in non-display area in partial display mode: */
                                             /* source output on non-display area */
#define FLAG_PT_0             0b00000000     /* V63 */
#define FLAG_PT_1             0b00000001     /* V0 */
#define FLAG_PT_2             0b00000010     /* AGND */
#define FLAG_PT_3             0b00000011     /* Hi-Z */
  /* scan mode in non-display area: */
#define FLAG_PTG_0            0b00000000     /* normal scan */
#define FLAG_PTG_2            0b00001000     /* interval scan */
  /* display operation mode: */
#define FLAG_DM_INT           0b00000000     /* internal system clock */
#define FLAG_DM_RGB           0b00010000     /* RGB interface */
  /* interface to access GRAM: */
#define FLAG_RM_SYS           0b00000000     /* system interface */
#define FLAG_RM_RGB           0b00100000     /* RGB interface */
  /* display data path for RGB interface: */
#define FLAG_BYPASS_MEM       0b00000000     /* memory */
#define FLAG_BYPASS_REG       0b10000000     /* direct to shift register */

/* data byte #2: */
  /* scan cycle for interval scan in non-display area: */
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
  /* gate driver pin arrangement (in combination with GS): */
#define FLAG_SM_0             0b00000000     /* in sequence */
#define FLAG_SM_1             0b00010000     /* even/odd separated */
  /* output shift direction / source driver scan direction: */
#define FLAG_SS_0             0b00000000     /* S1 -> S960 */
#define FLAG_SS_1             0b00100000     /* S960 -> S1 */
  /* gate driver scan direction: */
#define FLAG_GS_0             0b00000000     /* G1 -> G480 */
#define FLAG_GS_1             0b01000000     /* G480 -> G1 */

/* data byte #3: number of lines to drive LCD at an interval of 8 lines: */
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


/*
 *  set entry mode
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_ENTRY_MODE        0b10110111     /* set entry mode */

/* data byte #1: */
  /* low voltage detection control: */
#define FLAG_GAS_ON           0b00000000     /* enable */
#define FLAG_GAS_OFF          0b00000001     /* disable */
  /* output level of gate drivers G1-G480: */
#define FLAG_DTE_GON_0        0b00000000     /* VGH */
#define FLAG_DTE_GON_1        0b00000010     /* VGH */
#define FLAG_DTE_GON_2        0b00000100     /* VGL */
#define FLAG_DTE_GON_3        0b00000110     /* normal display */
  /* Deep Standby Mode: */
#define FLAG_DSTB_ON          0b00001000     /* enter Deep Standby Mode */
  /* data format for RGB565 to RGB666 conversion in GRAM: */
#define FLAG_EPF_0            0b00000000     /* DB15 -> R0, DB4 -> B0 */
#define FLAG_EPF_1            0b01000000     /* R0 = 0, B0 = 0 */
#define FLAG_EPF_2            0b10000000     /* R0 = 0, B0 = 0 */
#define FLAG_EPF_3            0b11000000     /* DB15 -> R0, DB5 -> B0 */


/*
 *  power control 1
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_POWER_CTRL_1      0b11000000     /* power control 1 */

/* data byte #1: VREG1OUT voltage for positive gamma */
#define FLAG_VRH1_HALT        0b00000000     /* halt (HiZ) */
#define FLAG_VRH1_290         0b00000001     /* 1.25 x 2.90 = 3.6250 V */
#define FLAG_VRH1_295         0b00000010     /* 1.25 x 2.95 = 3.6875 V */
#define FLAG_VRH1_300         0b00000011     /* 1.25 x 3.00 = 3.7500 V */
#define FLAG_VRH1_305         0b00000100     /* 1.25 x 3.05 = 3.8125 V */
#define FLAG_VRH1_310         0b00000101     /* 1.25 x 3.10 = 3.8750 V */
#define FLAG_VRH1_315         0b00000110     /* 1.25 x 3.15 = 3.9375 V */
#define FLAG_VRH1_320         0b00000111     /* 1.25 x 3.20 = 4.0000 V */
#define FLAG_VRH1_325         0b00001000     /* 1.25 x 3.25 = 4.0625 V */
#define FLAG_VRH1_330         0b00001001     /* 1.25 x 3.30 = 4.1250 V */
#define FLAG_VRH1_335         0b00001010     /* 1.25 x 3.35 = 4.1875 V */
#define FLAG_VRH1_340         0b00001011     /* 1.25 x 3.40 = 4.2500 V */
#define FLAG_VRH1_345         0b00001100     /* 1.25 x 3.45 = 4.3125 V */
#define FLAG_VRH1_350         0b00001101     /* 1.25 x 3.50 = 4.3750 V */
#define FLAG_VRH1_355         0b00001110     /* 1.25 x 3.55 = 4.4375 V */
#define FLAG_VRH1_360         0b00001111     /* 1.25 x 3.60 = 4.5000 V */
#define FLAG_VRH1_365         0b00010000     /* 1.25 x 3.65 = 4.5625 V */
#define FLAG_VRH1_370         0b00010001     /* 1.25 x 3.70 = 4.6250 V */
#define FLAG_VRH1_375         0b00010010     /* 1.25 x 3.75 = 4.6875 V */
#define FLAG_VRH1_380         0b00010011     /* 1.25 x 3.80 = 4.7500 V */
#define FLAG_VRH1_385         0b00010100     /* 1.25 x 3.85 = 4.8125 V */
#define FLAG_VRH1_390         0b00010101     /* 1.25 x 3.90 = 4.8750 V */
#define FLAG_VRH1_395         0b00010110     /* 1.25 x 3.95 = 4.9375 V */
#define FLAG_VRH1_400         0b00010111     /* 1.25 x 4.00 = 5.0000 V */
#define FLAG_VRH1_405         0b00011000     /* 1.25 x 4.05 = 5.0625 V */
#define FLAG_VRH1_410         0b00011001     /* 1.25 x 4.10 = 5.1250 V */
#define FLAG_VRH1_415         0b00011010     /* 1.25 x 4.15 = 5.1875 V */
#define FLAG_VRH1_420         0b00011011     /* 1.25 x 4.20 = 5.2500 V */
#define FLAG_VRH1_425         0b00011100     /* 1.25 x 4.25 = 5.3125 V */
#define FLAG_VRH1_430         0b00011101     /* 1.25 x 4.30 = 5.3750 V */
#define FLAG_VRH1_435         0b00011110     /* 1.25 x 4.35 = 5.4375 V */
#define FLAG_VRH1_440         0b00011111     /* 1.25 x 4.40 = 5.5000 V */

/* data byte #2: VREG2OUT voltage for negative gamma */
#define FLAG_VRH2_HALT        0b00000000     /* halt (HiZ) */
#define FLAG_VRH2_290         0b00000001     /* -1.25 x 2.90 = -3.6250 V */
#define FLAG_VRH2_295         0b00000010     /* -1.25 x 2.95 = -3.6875 V */
#define FLAG_VRH2_300         0b00000011     /* -1.25 x 3.00 = -3.7500 V */
#define FLAG_VRH2_305         0b00000100     /* -1.25 x 3.05 = -3.8125 V */
#define FLAG_VRH2_310         0b00000101     /* -1.25 x 3.10 = -3.8750 V */
#define FLAG_VRH2_315         0b00000110     /* -1.25 x 3.15 = -3.9375 V */
#define FLAG_VRH2_320         0b00000111     /* -1.25 x 3.20 = -4.0000 V */
#define FLAG_VRH2_325         0b00001000     /* -1.25 x 3.25 = -4.0625 V */
#define FLAG_VRH2_330         0b00001001     /* -1.25 x 3.30 = -4.1250 V */
#define FLAG_VRH2_335         0b00001010     /* -1.25 x 3.35 = -4.1875 V */
#define FLAG_VRH2_340         0b00001011     /* -1.25 x 3.40 = -4.2500 V */
#define FLAG_VRH2_345         0b00001100     /* -1.25 x 3.45 = -4.3125 V */
#define FLAG_VRH2_350         0b00001101     /* -1.25 x 3.50 = -4.3750 V */
#define FLAG_VRH2_355         0b00001110     /* -1.25 x 3.55 = -4.4375 V */
#define FLAG_VRH2_360         0b00001111     /* -1.25 x 3.60 = -4.5000 V */
#define FLAG_VRH2_365         0b00010000     /* -1.25 x 3.65 = -4.5625 V */
#define FLAG_VRH2_370         0b00010001     /* -1.25 x 3.70 = -4.6250 V */
#define FLAG_VRH2_375         0b00010010     /* -1.25 x 3.75 = -4.6875 V */
#define FLAG_VRH2_380         0b00010011     /* -1.25 x 3.80 = -4.7500 V */
#define FLAG_VRH2_385         0b00010100     /* -1.25 x 3.85 = -4.8125 V */
#define FLAG_VRH2_390         0b00010101     /* -1.25 x 3.90 = -4.8750 V */
#define FLAG_VRH2_395         0b00010110     /* -1.25 x 3.95 = -4.9375 V */
#define FLAG_VRH2_400         0b00010111     /* -1.25 x 4.00 = -5.0000 V */
#define FLAG_VRH2_405         0b00011000     /* -1.25 x 4.05 = -5.0625 V */
#define FLAG_VRH2_410         0b00011001     /* -1.25 x 4.10 = -5.1250 V */
#define FLAG_VRH2_415         0b00011010     /* -1.25 x 4.15 = -5.1875 V */
#define FLAG_VRH2_420         0b00011011     /* -1.25 x 4.20 = -5.2500 V */
#define FLAG_VRH2_425         0b00011100     /* -1.25 x 4.25 = -5.3125 V */
#define FLAG_VRH2_430         0b00011101     /* -1.25 x 4.30 = -5.3750 V */
#define FLAG_VRH2_435         0b00011110     /* -1.25 x 4.35 = -5.4375 V */
#define FLAG_VRH2_440         0b00011111     /* -1.25 x 4.40 = -5.5000 V */


/*
 *  power control 2
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_POWER_CTRL_2      0b11000001     /* power control 2 */

/* data byte #1: */
  /* factor for the step-up circuits: */
#define FLAG_BT_0             0b00000000     /* VGH=6*VCI1, VGL=5*VCI1 */
#define FLAG_BT_1             0b00000001     /* VGH=6*VCI1, VGL=4*VCI1 */
#define FLAG_BT_2             0b00000010     /* VGH=6*VCI1, VGL=3*VCI1 */
#define FLAG_BT_3             0b00000011     /* VGH=5*VCI1, VGL=5*VCI1 */
#define FLAG_BT_4             0b00000100     /* VGH=5*VCI1, VGL=4*VCI1 */
#define FLAG_BT_5             0b00000101     /* VGH=5*VCI1, VGL=3*VCI1 */
#define FLAG_BT_6             0b00000110     /* VGH=4*VCI1, VGL=4*VCI1 */
#define FLAG_BT_7             0b00000111     /* VGH=4*VCI1, VGL=3*VCI1 */
  /* constant current of power supply OPAMP (gamma bias control): */
#define FLAG_SAP_0            0b00000000     /* small / 0.71 x I */
#define FLAG_SAP_1            0b00010000     /* small / 0.71 x I */
#define FLAG_SAP_2            0b00100000     /* small / 0.71 x I */
#define FLAG_SAP_3            0b00110000     /* small / 0.71 x I */
#define FLAG_SAP_4            0b01000000     /* medium / 1.00 x I */
#define FLAG_SAP_5            0b01010000     /* medium to large / 1.25 x I */
#define FLAG_SAP_6            0b01100000     /* large / 1.43 x I */
#define FLAG_SAP_7            0b01110000     /* large / 1.43 x I */

/* data byte #2: */
  /* VCI1 regulator voltage: */
#define FLAG_VC_EXT           0b00000000     /* external VCI */
#define FLAG_VC_31            0b00000001     /* 3.1 V */
#define FLAG_VC_30            0b00000010     /* 3.0 V */
#define FLAG_VC_29            0b00000011     /* 2.9 V */
#define FLAG_VC_28            0b00000100     /* 2.8 V */
#define FLAG_VC_27            0b00000101     /* 2.7 V */
#define FLAG_VC_26            0b00000110     /* 2.6 V */
#define FLAG_VC_25            0b00000111     /* 2.5 V */


/*
 *  power control 3 for normal mode
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_POWER_CTRL_3      0b11000010     /* power control 3 */

/* data byte #1: */
  /* frequency of step-up circuits 1, 4 and 5: */
#define FLAG_DCA0_0125        0b00000000     /* 1/8 Hz */
#define FLAG_DCA0_025         0b00000001     /* 1/4 Hz */
#define FLAG_DCA0_05          0b00000010     /* 1/2 Hz */
#define FLAG_DCA0_1           0b00000011     /* 1 Hz */
#define FLAG_DCA0_2           0b00000100     /* 2 Hz */
#define FLAG_DCA0_4           0b00000101     /* 4 Hz */
#define FLAG_DCA0_8           0b00000110     /* 8 Hz */
#define FLAG_DCA0_16          0b00000111     /* 16 Hz */

  /* frequency of step-up circuits 2 and 3: */
#define FLAG_DCA1_05          0b00000000     /* 1/2 Hz */
#define FLAG_DCA1_1           0b00010000     /* 1 Hz */
#define FLAG_DCA1_2           0b00100000     /* 2 Hz */
#define FLAG_DCA1_4           0b00110000     /* 4 Hz */
#define FLAG_DCA1_8           0b01000000     /* 8 Hz */
#define FLAG_DCA1_16          0b01010000     /* 16 Hz */
#define FLAG_DCA1_32          0b01100000     /* 32 Hz */
#define FLAG_DCA1_64          0b01110000     /* 64 Hz */


/*
 *  power control 4 for idle mode
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_POWER_CTRL_4      0b11000011     /* power control 4 */

/* data byte #1: */
  /* frequency of step-up circuits 1, 4 and 5: */
#define FLAG_DCB0_0125        0b00000000     /* 1/8 Hz */
#define FLAG_DCB0_025         0b00000001     /* 1/4 Hz */
#define FLAG_DCB0_05          0b00000010     /* 1/2 Hz */
#define FLAG_DCB0_1           0b00000011     /* 1 Hz */
#define FLAG_DCB0_2           0b00000100     /* 2 Hz */
#define FLAG_DCB0_4           0b00000101     /* 4 Hz */
#define FLAG_DCB0_8           0b00000110     /* 8 Hz */
#define FLAG_DCB0_16          0b00000111     /* 16 Hz */

  /* frequency of step-up circuits 2 and 3: */
#define FLAG_DCB1_05          0b00000000     /* 1/2 Hz */
#define FLAG_DCB1_1           0b00010000     /* 1 Hz */
#define FLAG_DCB1_2           0b00100000     /* 2 Hz */
#define FLAG_DCB1_4           0b00110000     /* 4 Hz */
#define FLAG_DCB1_8           0b01000000     /* 8 Hz */
#define FLAG_DCB1_16          0b01010000     /* 16 Hz */
#define FLAG_DCB1_32          0b01100000     /* 32 Hz */
#define FLAG_DCB1_64          0b01110000     /* 64 Hz */


/*
 *  power control 5 for partial mode
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_POWER_CTRL_5      0b11000100     /* power control 5 */

/* data byte #1: */
  /* frequency of step-up circuits 1, 4 and 5: */
#define FLAG_DCC0_0125        0b00000000     /* 1/8 Hz */
#define FLAG_DCC0_025         0b00000001     /* 1/4 Hz */
#define FLAG_DCC0_05          0b00000010     /* 1/2 Hz */
#define FLAG_DCC0_1           0b00000011     /* 1 Hz */
#define FLAG_DCC0_2           0b00000100     /* 2 Hz */
#define FLAG_DCC0_4           0b00000101     /* 4 Hz */
#define FLAG_DCC0_8           0b00000110     /* 8 Hz */
#define FLAG_DCC0_16          0b00000111     /* 16 Hz */

  /* frequency of step-up circuits 2 and 3: */
#define FLAG_DCC1_05          0b00000000     /* 1/2 Hz */
#define FLAG_DCC1_1           0b00010000     /* 1 Hz */
#define FLAG_DCC1_2           0b00100000     /* 2 Hz */
#define FLAG_DCC1_4           0b00110000     /* 4 Hz */
#define FLAG_DCC1_8           0b01000000     /* 8 Hz */
#define FLAG_DCC1_16          0b01010000     /* 16 Hz */
#define FLAG_DCC1_32          0b01100000     /* 32 Hz */
#define FLAG_DCC1_64          0b01110000     /* 64 Hz */


/*
 *  VCOM control
 *  - 1 byte cmd + 4 bytes data (mixed read and write)
 *  - data bytes #1 and #4 are read
 *  - data bytes #2 and #3 are written
 */

#define CMD_VCOM_CTRL         0b11000101     /* VCOM control */

/* data byte #1 (read mode): */
  /* NV memory status; */
#define FLAG_NV_PROG_NO       0b00000000     /* not programmed */
#define FLAG_NV_PROG_YES      0b00000001     /* is programmed */

/* data byte #2: */
  /* factor for VCOM (based on VREG2OUT): */
#define FLAG_VCOM_200000      0b00000000     /* -2 */
#define FLAG_VCOM_198438      0b00000001     /* -1.98438 */
#define FLAG_VCOM_196875      0b00000010     /* -1.96875 */
#define FLAG_VCOM_195313      0b00000011     /* -1.95313 */
#define FLAG_VCOM_193750      0b00000100     /* -1.9375 */
#define FLAG_VCOM_192188      0b00000101     /* -1.92188 */
#define FLAG_VCOM_190625      0b00000110     /* -1.90625 */
#define FLAG_VCOM_189063      0b00000111     /* -1.89063 */
#define FLAG_VCOM_187500      0b00001000     /* -1.875 */
#define FLAG_VCOM_185938      0b00001001     /* -1.85938 */
#define FLAG_VCOM_184375      0b00001010     /* -1.84375 */
#define FLAG_VCOM_182813      0b00001011     /* -1.82813 */
#define FLAG_VCOM_181250      0b00001100     /* -1.8125 */
#define FLAG_VCOM_179688      0b00001101     /* -1.79688 */
#define FLAG_VCOM_178125      0b00001110     /* -1.78125 */
#define FLAG_VCOM_176563      0b00001111     /* -1.76563 */
#define FLAG_VCOM_175000      0b00010000     /* -1.75 */
#define FLAG_VCOM_173438      0b00010001     /* -1.73438 */
#define FLAG_VCOM_171875      0b00010010     /* -1.71875 */
#define FLAG_VCOM_170313      0b00010011     /* -1.70313 */
#define FLAG_VCOM_168750      0b00010100     /* -1.6875 */
#define FLAG_VCOM_167188      0b00010101     /* -1.67188 */
#define FLAG_VCOM_165625      0b00010110     /* -1.65625 */
#define FLAG_VCOM_164063      0b00010111     /* -1.64063 */
#define FLAG_VCOM_162500      0b00011000     /* -1.625 */
#define FLAG_VCOM_160938      0b00011001     /* -1.60938 */
#define FLAG_VCOM_159375      0b00011010     /* -1.59375 */
#define FLAG_VCOM_157813      0b00011011     /* -1.57813 */
#define FLAG_VCOM_156250      0b00011100     /* -1.5625 */
#define FLAG_VCOM_154688      0b00011101     /* -1.54688 */
#define FLAG_VCOM_153125      0b00011110     /* -1.53125 */
#define FLAG_VCOM_151563      0b00011111     /* -1.51563 */
#define FLAG_VCOM_150000      0b00100000     /* -1.5 */
#define FLAG_VCOM_148438      0b00100001     /* -1.48438 */
#define FLAG_VCOM_146875      0b00100010     /* -1.46875 */
#define FLAG_VCOM_145313      0b00100011     /* -1.45313 */
#define FLAG_VCOM_143750      0b00100100     /* -1.4375 */
#define FLAG_VCOM_142188      0b00100101     /* -1.42188 */
#define FLAG_VCOM_140625      0b00100110     /* -1.40625 */
#define FLAG_VCOM_139063      0b00100111     /* -1.39063 */
#define FLAG_VCOM_137500      0b00101000     /* -1.375 */
#define FLAG_VCOM_135938      0b00101001     /* -1.35938 */
#define FLAG_VCOM_134375      0b00101010     /* -1.34375 */
#define FLAG_VCOM_132813      0b00101011     /* -1.32813 */
#define FLAG_VCOM_131250      0b00101100     /* -1.3125 */
#define FLAG_VCOM_129688      0b00101101     /* -1.29688 */
#define FLAG_VCOM_128125      0b00101110     /* -1.28125 */
#define FLAG_VCOM_126563      0b00101111     /* -1.26563 */
#define FLAG_VCOM_125000      0b00110000     /* -1.25 */
#define FLAG_VCOM_123438      0b00110001     /* -1.23438 */
#define FLAG_VCOM_121875      0b00110010     /* -1.21875 */
#define FLAG_VCOM_120313      0b00110011     /* -1.20313 */
#define FLAG_VCOM_118750      0b00110100     /* -1.1875 */
#define FLAG_VCOM_117188      0b00110101     /* -1.17188 */
#define FLAG_VCOM_115625      0b00110110     /* -1.15625 */
#define FLAG_VCOM_114063      0b00110111     /* -1.14063 */
#define FLAG_VCOM_112500      0b00111000     /* -1.125 */
#define FLAG_VCOM_110938      0b00111001     /* -1.10938 */
#define FLAG_VCOM_109375      0b00111010     /* -1.09375 */
#define FLAG_VCOM_107813      0b00111011     /* -1.07813 */
#define FLAG_VCOM_106250      0b00111100     /* -1.0625 */
#define FLAG_VCOM_104688      0b00111101     /* -1.04688 */
#define FLAG_VCOM_103125      0b00111110     /* -1.03125 */
#define FLAG_VCOM_101563      0b00111111     /* -1.01563 */
#define FLAG_VCOM_100000      0b01000000     /* -1 */
#define FLAG_VCOM_098438      0b01000001     /* -0.98438 */
#define FLAG_VCOM_096875      0b01000010     /* -0.96875 */
#define FLAG_VCOM_095313      0b01000011     /* -0.95313 */
#define FLAG_VCOM_093750      0b01000100     /* -0.9375 */
#define FLAG_VCOM_092188      0b01000101     /* -0.92188 */
#define FLAG_VCOM_090625      0b01000110     /* -0.90625 */
#define FLAG_VCOM_089063      0b01000111     /* -0.89063 */
#define FLAG_VCOM_087500      0b01001000     /* -0.875 */
#define FLAG_VCOM_085938      0b01001001     /* -0.85938 */
#define FLAG_VCOM_084375      0b01001010     /* -0.84375 */
#define FLAG_VCOM_082813      0b01001011     /* -0.82813 */
#define FLAG_VCOM_081250      0b01001100     /* -0.8125 */
#define FLAG_VCOM_079688      0b01001101     /* -0.79688 */
#define FLAG_VCOM_078125      0b01001110     /* -0.78125 */
#define FLAG_VCOM_076563      0b01001111     /* -0.76563 */
#define FLAG_VCOM_075000      0b01010000     /* -0.75 */
#define FLAG_VCOM_073438      0b01010001     /* -0.73438 */
#define FLAG_VCOM_071875      0b01010010     /* -0.71875 */
#define FLAG_VCOM_070313      0b01010011     /* -0.70313 */
#define FLAG_VCOM_068750      0b01010100     /* -0.6875 */
#define FLAG_VCOM_067188      0b01010101     /* -0.67188 */
#define FLAG_VCOM_065625      0b01010110     /* -0.65625 */
#define FLAG_VCOM_064063      0b01010111     /* -0.64063 */
#define FLAG_VCOM_062500      0b01011000     /* -0.625 */
#define FLAG_VCOM_060938      0b01011001     /* -0.60938 */
#define FLAG_VCOM_059375      0b01011010     /* -0.59375 */
#define FLAG_VCOM_057813      0b01011011     /* -0.57813 */
#define FLAG_VCOM_056250      0b01011100     /* -0.5625 */
#define FLAG_VCOM_054688      0b01011101     /* -0.54688 */
#define FLAG_VCOM_053125      0b01011110     /* -0.53125 */
#define FLAG_VCOM_051563      0b01011111     /* -0.51563 */
#define FLAG_VCOM_050000      0b01100000     /* -0.5 */
#define FLAG_VCOM_048438      0b01100001     /* -0.48438 */
#define FLAG_VCOM_046875      0b01100010     /* -0.46875 */
#define FLAG_VCOM_045313      0b01100011     /* -0.45313 */
#define FLAG_VCOM_043750      0b01100100     /* -0.4375 */
#define FLAG_VCOM_042188      0b01100101     /* -0.42188 */
#define FLAG_VCOM_040625      0b01100110     /* -0.40625 */
#define FLAG_VCOM_039063      0b01100111     /* -0.39063 */
#define FLAG_VCOM_037500      0b01101000     /* -0.375 */
#define FLAG_VCOM_035938      0b01101001     /* -0.35938 */
#define FLAG_VCOM_034375      0b01101010     /* -0.34375 */
#define FLAG_VCOM_032813      0b01101011     /* -0.32813 */
#define FLAG_VCOM_031250      0b01101100     /* -0.3125 */
#define FLAG_VCOM_029688      0b01101101     /* -0.29688 */
#define FLAG_VCOM_028125      0b01101110     /* -0.28125 */
#define FLAG_VCOM_026563      0b01101111     /* -0.26563 */
#define FLAG_VCOM_025000      0b01110000     /* -0.25 */
#define FLAG_VCOM_023438      0b01110001     /* -0.23438 */
#define FLAG_VCOM_021875      0b01110010     /* -0.21875 */
#define FLAG_VCOM_020313      0b01110011     /* -0.20313 */
#define FLAG_VCOM_018750      0b01110100     /* -0.1875 */
#define FLAG_VCOM_017188      0b01110101     /* -0.17188 */
#define FLAG_VCOM_015625      0b01110110     /* -0.15625 */
#define FLAG_VCOM_014063      0b01110111     /* -0.14063 */
#define FLAG_VCOM_012500      0b01111000     /* -0.125 */
#define FLAG_VCOM_010938      0b01111001     /* -0.10938 */
#define FLAG_VCOM_009375      0b01111010     /* -0.09375 */
#define FLAG_VCOM_007813      0b01111011     /* -0.07813 */
#define FLAG_VCOM_006250      0b01111100     /* -0.0625 */
#define FLAG_VCOM_004688      0b01111101     /* -0.04688 */
#define FLAG_VCOM_003125      0b01111110     /* -0.03125 */
#define FLAG_VCOM_001563      0b01111111     /* -0.01563 */
#define FLAG_VCOM_000000      0b10000000     /* 0 */
#define FLAG_VCOM_INHIBIT     0b10000001     /* inhibit */
#define FLAG_VCOM_HALT        0b11111111     /* halt */

/* data byte #3: */
  /* Vcom value selection: */
#define FLAG_VCOM_NV          0b00000000     /* from NV memory */
#define FLAG_VCOM_REG         0b10000000     /* from VCM_REG (see byte #2) */

/* data byte #4 (read mode) */
  /* NV memory programmed value: bits 0-7 */


/*
 *  CABC control 1 (display lines for partial mode)
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_CABC_CTRL_1       0b11000110     /* CABC control 1 */

/* data byte #1: */
  /* display lines for partial mode - LSB (8 bits 0-7): bits 0-7 */

/* data byte #2: */
  /* display lines for partial mode - MSB (3 bits 8-10): bits 0-2 */
  /* valid range: 1-480 */ 


/*
 *  CABC control 2 (LED backlight)
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_2       0b11001000     /* CABC control 2 */

/* data byte #1: */
  /* polarity of PWM signal on CABC_PWM pin: */
#define FLAG_PWM_NORM         0b00000000     /* original when backlight on */
                                             /* always low when backlight off */
#define FLAG_PWM_INV          0b00000001     /* inversed when backlight on */
                                             /* always high when backlight off */
  /* polarity of signal on CABC_ON pin: */
#define FLAG_LED_NORM         0b00000000     /* LEDONR when backlight on */
                                             /* always low when backlight off */
#define FLAG_LED_INV          0b00000010     /* inversed LEDONR when backlight on */
                                             /* always high when backlight off */
  /* register for CABC_ON pin (LEDONR): */
#define FLAG_LED_LOW          0b00000000     /* low */
#define FLAG_LED_HIGH         0b00000100     /* high */


/*
 *  CABC control 3
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_3       0b11001001     /* CABC control 3 */

/* data byte #1: */
  /* ratio (in %) of pixels for white image / all pixels */
  /* for still image mode; */
#define FLAG_THRES_STILL_99   0b00000000     /* 99% */
#define FLAG_THRES_STILL_98   0b00000001     /* 98% */
#define FLAG_THRES_STILL_96   0b00000010     /* 96% */
#define FLAG_THRES_STILL_94   0b00000011     /* 94% */
#define FLAG_THRES_STILL_92   0b00000100     /* 92% */
#define FLAG_THRES_STILL_90   0b00000101     /* 90% */
#define FLAG_THRES_STILL_88   0b00000110     /* 88% */
#define FLAG_THRES_STILL_86   0b00000111     /* 86% */
#define FLAG_THRES_STILL_84   0b00001000     /* 84% */
#define FLAG_THRES_STILL_82   0b00001001     /* 82% */
#define FLAG_THRES_STILL_80   0b00001010     /* 80% */
#define FLAG_THRES_STILL_78   0b00001011     /* 78% */
#define FLAG_THRES_STILL_76   0b00001100     /* 76% */
#define FLAG_THRES_STILL_74   0b00001101     /* 74% */
#define FLAG_THRES_STILL_72   0b00001110     /* 72% */
#define FLAG_THRES_STILL_70   0b00001111     /* 70% */
  /* ratio (in %) of pixels for white image / all pixels */
  /* for moving image mode: */
#define FLAG_THRES_MOV_99     0b00000000     /* 99% */
#define FLAG_THRES_MOV_98     0b00010000     /* 98% */
#define FLAG_THRES_MOV_96     0b00100000     /* 96% */
#define FLAG_THRES_MOV_94     0b00110000     /* 94% */
#define FLAG_THRES_MOV_92     0b01000000     /* 92% */
#define FLAG_THRES_MOV_90     0b01010000     /* 90% */
#define FLAG_THRES_MOV_88     0b01100000     /* 88% */
#define FLAG_THRES_MOV_86     0b01110000     /* 86% */
#define FLAG_THRES_MOV_84     0b10000000     /* 84% */
#define FLAG_THRES_MOV_82     0b10010000     /* 82% */
#define FLAG_THRES_MOV_80     0b10100000     /* 80% */
#define FLAG_THRES_MOV_78     0b10110000     /* 78% */
#define FLAG_THRES_MOV_76     0b11000000     /* 76% */
#define FLAG_THRES_MOV_74     0b11010000     /* 74% */
#define FLAG_THRES_MOV_72     0b11100000     /* 72% */
#define FLAG_THRES_MOV_70     0b11110000     /* 70% */


/*
 *  CABC control 4
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_4       0b11001010     /* CABC control 4 */

/* data byte #1: */
  /* ratio (in %) of pixels for white image / all pixels */
  /* for user interface mode: */
#define FLAG_THRES_UI_99      0b00000000     /* 99% */
#define FLAG_THRES_UI_98      0b00000001     /* 98% */
#define FLAG_THRES_UI_96      0b00000010     /* 96% */
#define FLAG_THRES_UI_94      0b00000011     /* 94% */
#define FLAG_THRES_UI_92      0b00000100     /* 92% */
#define FLAG_THRES_UI_90      0b00000101     /* 90% */
#define FLAG_THRES_UI_88      0b00000110     /* 88% */
#define FLAG_THRES_UI_86      0b00000111     /* 86% */
#define FLAG_THRES_UI_84      0b00001000     /* 84% */
#define FLAG_THRES_UI_82      0b00001001     /* 82% */
#define FLAG_THRES_UI_80      0b00001010     /* 80% */
#define FLAG_THRES_UI_78      0b00001011     /* 78% */
#define FLAG_THRES_UI_76      0b00001100     /* 76% */
#define FLAG_THRES_UI_74      0b00001101     /* 74% */
#define FLAG_THRES_UI_72      0b00001110     /* 72% */
#define FLAG_THRES_UI_70      0b00001111     /* 70% */


/*
 *  CABC control 5
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_5       0b11001011     /* CABC control 5 */

/* data byte #1: */
  /* grayscale threshold for still image mode: */
#define FLAG_DTH_STILL_224    0b00000000     /* 224 */
#define FLAG_DTH_STILL_220    0b00000001     /* 220 */
#define FLAG_DTH_STILL_216    0b00000010     /* 216 */
#define FLAG_DTH_STILL_212    0b00000011     /* 212 */
#define FLAG_DTH_STILL_208    0b00000100     /* 208 */
#define FLAG_DTH_STILL_204    0b00000101     /* 204 */
#define FLAG_DTH_STILL_200    0b00000110     /* 200 */
#define FLAG_DTH_STILL_196    0b00000111     /* 196 */
#define FLAG_DTH_STILL_192    0b00001000     /* 192 */
#define FLAG_DTH_STILL_188    0b00001001     /* 188 */
#define FLAG_DTH_STILL_184    0b00001010     /* 184 */
#define FLAG_DTH_STILL_180    0b00001011     /* 180 */
#define FLAG_DTH_STILL_176    0b00001100     /* 176 */
#define FLAG_DTH_STILL_172    0b00001101     /* 172 */
#define FLAG_DTH_STILL_168    0b00001110     /* 168 */
#define FLAG_DTH_STILL_164    0b00001111     /* 164 */
  /* grayscale threshold for moving image mode: */
#define FLAG_DTH_MOV_224      0b00000000     /* 224 */
#define FLAG_DTH_MOV_220      0b00010000     /* 220 */
#define FLAG_DTH_MOV_216      0b00100000     /* 216 */
#define FLAG_DTH_MOV_212      0b00110000     /* 212 */
#define FLAG_DTH_MOV_208      0b01000000     /* 208 */
#define FLAG_DTH_MOV_204      0b01010000     /* 204 */
#define FLAG_DTH_MOV_200      0b01100000     /* 200 */
#define FLAG_DTH_MOV_196      0b01110000     /* 196 */
#define FLAG_DTH_MOV_192      0b10000000     /* 192 */
#define FLAG_DTH_MOV_188      0b10010000     /* 188 */
#define FLAG_DTH_MOV_184      0b10100000     /* 184 */
#define FLAG_DTH_MOV_180      0b10110000     /* 180 */
#define FLAG_DTH_MOV_176      0b11000000     /* 176 */
#define FLAG_DTH_MOV_172      0b11010000     /* 172 */
#define FLAG_DTH_MOV_168      0b11100000     /* 168 */
#define FLAG_DTH_MOV_164      0b11110000     /* 164 */


/*
 *  CABC control 6
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_6       0b11001100     /* CABC control 6 */

/* data byte #1: */
  /* grayscale threshold for user interface mode: */
#define FLAG_DTH_UI_252       0b00000000     /* 252 */
#define FLAG_DTH_UI_248       0b00000001     /* 248 */
#define FLAG_DTH_UI_244       0b00000010     /* 244 */
#define FLAG_DTH_UI_240       0b00000011     /* 240 */
#define FLAG_DTH_UI_236       0b00000100     /* 236 */
#define FLAG_DTH_UI_232       0b00000101     /* 232 */
#define FLAG_DTH_UI_228       0b00000110     /* 228 */
#define FLAG_DTH_UI_224       0b00000111     /* 224 */
#define FLAG_DTH_UI_220       0b00001000     /* 220 */
#define FLAG_DTH_UI_216       0b00001001     /* 216 */
#define FLAG_DTH_UI_212       0b00001010     /* 212 */
#define FLAG_DTH_UI_208       0b00001011     /* 208 */
#define FLAG_DTH_UI_204       0b00001100     /* 204 */
#define FLAG_DTH_UI_200       0b00001101     /* 200 */
#define FLAG_DTH_UI_196       0b00001110     /* 196 */
#define FLAG_DTH_UI_192       0b00001111     /* 192 */


/*
 *  CABC control 7
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_7       0b11001101     /* CABC control 7 */

/* data byte #1: */
  /* transition time of brightness change in still image mode: */
#define FLAG_DIM_STILL_05     0b00000000     /* 1 frame */
#define FLAG_DIM_STILL_1      0b00000001     /* 1 frame */
#define FLAG_DIM_STILL_2      0b00000010     /* 2 frames */
#define FLAG_DIM_STILL_4      0b00000011     /* 4 frames */
#define FLAG_DIM_STILL_8      0b00000100     /* 8 frames */
#define FLAG_DIM_STILL_16     0b00000101     /* 16 frames */
#define FLAG_DIM_STILL_32     0b00000110     /* 32 frames */
#define FLAG_DIM_STILL_64     0b00000111     /* 64 frames */
  /* transition time of brightness change in moving image mode: */
#define FLAG_DIM_MOV_05       0b00000000     /* 1 frame */
#define FLAG_DIM_MOV_1        0b00010000     /* 1 frame */
#define FLAG_DIM_MOV_2        0b00100000     /* 2 frames */
#define FLAG_DIM_MOV_4        0b00110000     /* 4 frames */
#define FLAG_DIM_MOV_8        0b01000000     /* 8 frames */
#define FLAG_DIM_MOV_16       0b01010000     /* 16 frames */
#define FLAG_DIM_MOV_32       0b01100000     /* 32 frames */
#define FLAG_DIM_MOV_64       0b01110000     /* 64 frames */


/*
 *  CABC control 8
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_8       0b11001110     /* CABC control 8 */

/* data byte #1: */
  /* transition time of brightness change in user interface mode: */
#define FLAG_DIM_UI_05        0b00000000     /* 1 frame */
#define FLAG_DIM_UI_1         0b00000001     /* 1 frame */
#define FLAG_DIM_UI_2         0b00000010     /* 2 frames */
#define FLAG_DIM_UI_4         0b00000011     /* 4 frames */
#define FLAG_DIM_UI_8         0b00000100     /* 8 frames */
#define FLAG_DIM_UI_16        0b00000101     /* 16 frames */
#define FLAG_DIM_UI_32        0b00000110     /* 32 frames */
#define FLAG_DIM_UI_64        0b00000111     /* 64 frames */
  /* minimum brightness change: 4 bits, bits 4-7 */
#define FLAG_DIM_MIN_MASK     0b11110000     /* filtermask for DIM_MIN value */


/*
 *  CABC control 9
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_CABC_CTRL_9       0b11001111     /* CABC control 9 */

/* data byte #1: */
  /* period control for PWM_OUT: f = 18 MHz / ((PWM_DIV + 1) * 255) */
#define FLAG_PWM_DIV_MIN      0b00000000     /* 70.58 kHz */
#define FLAG_PWM_DIV_MAX      0b11111111     /* 275.8 Hz */


/*
 *  write NV memory
 *  - 1 byte cmd + 2 bytes data
 */

#define CMD_WRITE_NV          0b11010000     /* write NV memory */

/* data byte #1: address */
#define FLAG_NV_ADDR_ID2      0b00000000     /* ID2 */
#define FLAG_NV_ADDR_ID3      0b00000001     /* ID3 */
#define FLAG_NV_ADDR_VMF      0b00000010     /* VMF (7 bits) */
#define FLAG_NV_ADDR_MDDI     0b00000100     /* MDDI version (1 bit) */

/* data byte #2: data */


/*
 *  NV memory protection key (for writing)
 *  - 1 byte cmd + 3 bytes data
 */

#define CMD_NV_KEY            0b11010001     /* NV memory protection key */

/* data byte #1: key - bits 16-23 */
#define FLAG_KEY_2            0x55           /* fixed key */

/* data byte #2: key - bits 8-15 */
#define FLAG_KEY_1            0xAA           /* fixed key */

/* data byte #3: key - bits 0-7 */
#define FLAG_KEY_0            0x66           /* fixed key */


/*
 *  read NV memory status
 *  - 1 byte cmd + 5 bytes data (read mode)
 */

#define CMD_READ_NV_STATUS    0b11010010     /* read NV memory status  */

/* data byte #1: dummy byte */

/* data byte #2: */
  /* write counter for ID1: 4 bits, bits 0-3 */
  /* write counter for ID2: 4 bits, bits 4-7 */

/* data byte #3: */
  /* write counter for ID3: 4 bits, bits 0-3 */
  /* write counter for VMF: 4 bits, bits 4-7 */

/* write counter (number of 1s) */
#define FLAG_NV_CNT_0         0b00000000     /* not programmed */
#define FLAG_NV_CNT_1         0b00000001     /* 1 time */
#define FLAG_NV_CNT_2         0b00000011     /* 2 times */
#define FLAG_NV_CNT_3         0b00000111     /* 3 times */
#define FLAG_NV_CNT_4         0b00001111     /* 4 times */

/* data byte #4: */
  /* MDDI version: */
#define FLAG_MDDI_10          0b00000000     /* MDDI v1.0 */
#define FLAG_MDDI_12          0b00000001     /* MDDI v1.2 */
  /* status of NV memory programming: */
#define FLAG_NV_IDLE          0b00000000     /* idle */
#define FLAG_NV_BUSY          0b10000000     /* busy */

/* data byte #5: */
  /* VMF value */


/*
 *  read ID4 (device code)
 *  - 1 byte cmd + 4 bytes data (read mode)
 */

#define CMD_READ_ID4          0b11010011     /* read ID4 */

/* data byte #1: dummy byte */
/* data byte #2: IC version (0x00) */
/* data byte #3: IC model name - MSB (0x94) */
/* data byte #4: IC model name - LSB (0x86) */


/*
 *  positive gamma correction, gray scale voltage
 *  - 1 byte cmd + 15 bytes data
 */

#define CMD_GAMMA_CTRL_POS    0b11100000     /* positive gamma correction */

/* data bytes: please see dataheet */


/*
 *  negative gamma correction, gray scale voltage
 *  - 1 byte cmd + 15 bytes data
 */

#define CMD_GAMMA_CTRL_NEG    0b11100001     /* negative gamma correction */

/* data bytes: please see dataheet */


/*
 *  digital gamma control 1
 *  - 1 byte cmd + 16 bytes data
 */

#define CMD_GAMMA_CTRL_1      0b11100010     /* digital gamma control 1 */

/* data byte x (0-15): macro-adjustment for red and blue gamma curve */
/*   RCAx: 4 bits, BCAx: 4 bits */


/*
 *  digital gamma control 2
 *  - 1 byte cmd + 64 bytes data
 */

#define CMD_GAMMA_CTRL_2      0b11100011     /* digital gamma control 2 */

/* data byte x (0-63): micro-adjustment for red and blue gamma curve */
/*   RFAx: 4 bits, BFAx: 4 bits */


/*
 *  setting for SPI read command
 *  - 1 byte cmd + 1 byte data
 */

#define CMD_SET_SPI_READ      0b11111011     /* setting for SPI read */

/* data byte #1: */
  /* parameter number: 4 bits, bits 0-3 */
  /* enable/disable SPI read: */
#define FLAG_SPI_READ_OFF     0b00000000     /* disable */
#define FLAG_SPI_READ_ON      0b00010000     /* enable */



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

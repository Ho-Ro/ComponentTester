/* ************************************************************************
 *
 *   color definitions for LCD modules
 *
 *   (c) 2015-2017 by Markus Reschke
 *
 * ************************************************************************ */


/* source management */
#define COLORS_H


#ifdef LCD_COLOR

/* ************************************************************************
 *   LCD specific color values
 * ************************************************************************ */


/* ILI9341/ILI9342: 6 bit coded RGB */
#ifdef LCD_ILI9341
  #define COLOR_BLACK         0x0000
  #define COLOR_BLUE          0x001f
  #define COLOR_GREEN         0x07e0
  #define COLOR_CYAN          0x07ff
  #define COLOR_RED           0xf800
  #define COLOR_MAGENTA       0xf81f
  #define COLOR_YELLOW        0xffe0
  #define COLOR_ORANGE        0xfd20
  #define COLOR_GREY          0xc618
  #define COLOR_WHITE         0xffff
#endif


/* ST7735: 6 bit coded RGB */
#ifdef LCD_ST7735
  #define COLOR_BLACK         0x0000
  #define COLOR_BLUE          0x001f
  #define COLOR_GREEN         0x07e0
  #define COLOR_CYAN          0x07ff
  #define COLOR_RED           0xf800
  #define COLOR_MAGENTA       0xf81f
  #define COLOR_YELLOW        0xffe0
  #define COLOR_ORANGE        0xfd20
  #define COLOR_GREY          0xc618
  #define COLOR_WHITE         0xffff
#endif



/* ************************************************************************
 *   default colors
 * ************************************************************************ */


/* background color */
#define COLOR_BACKGROUND      COLOR_BLACK

/* standard pen color */
#define COLOR_PEN             COLOR_GREEN

/* title color */
#define COLOR_TITLE           COLOR_CYAN

/* symbol color */
#define COLOR_SYMBOL          COLOR_YELLOW

/* color codes for probe pins */
#define COLOR_PROBE_1         COLOR_YELLOW
#define COLOR_PROBE_2         COLOR_GREEN
#define COLOR_PROBE_3         COLOR_RED

#endif



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

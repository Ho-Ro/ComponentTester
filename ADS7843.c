/* ************************************************************************
 *
 *   driver functions for ADS7843 compatible touchscreen controller
 *
 *   (c) 2015 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    DCLK
 *    DIN
 *    DOUT
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef TOUCH_ADS7843


/*
 *  local constants
 */

/* source management */
#define TOUCH_DRIVER_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "ADS7843.h"          /* ADS7843 specifics */


/* ************************************************************************
 *   low level functions for SPI interface
 * ************************************************************************ */



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef TOUCH_DRIVER_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

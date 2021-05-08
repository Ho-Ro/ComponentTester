/* ************************************************************************
 *
 *   SPI (bit-bang & hardware)
 *
 *   (c) 2017-2020 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - port and pins for bit-bang SPI
 *    SPI_PORT      port data register
 *    SPI_DDR       port data direction register
 *    SPI_PIN       port input pins register
 *    SPI_SCK       pin for SCK
 *    SPI_MOSI      pin for MOSI
 *    SPI_MISO      pin for MISO
 *  - For hardware SPI the MCU specific pins are used:
 *    ATmega 328: SCK PB5, MOSI PB3, MISO PB4, /SS PB2
 *    ATmega 644: SCK PB7, MOSI PB5, MISO PB6, /SS PB4
 *    ATmega 2560: SCK PB1, MOSI PB2, MISO PB3, /SS PB0
 *  - /CS and other control signals have to be managed by the specific
 *    chip driver
 *  - we use SPI mode 0 (set MOSI before rising SCK)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_SPI


/*
 *  local constants
 */

/* source management */
#define SPI_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   functions for bit-bang SPI
 * ************************************************************************ */


#ifdef SPI_BITBANG

/*
 *  set up SPI bus
 *  - SCK, MOSI and MISO lines
 *  - don't care about clock rate
 */

void SPI_Setup(void)
{
  /* set up bus only once */
  if (Cfg.OP_Mode & OP_SPI) return;

  /*
   *  set up bitbang SPI:
   *  - master mode
   */

  #ifdef SPI_MISO
  /* set MISO line to input mode */
  SPI_DDR &= ~(1 << SPI_MISO);
  #endif

  /* set SCK and MOSI to output mode */
  SPI_DDR |= (1 << SPI_SCK) | (1 << SPI_MOSI);

  /* preset lines to low */
  SPI_PORT &= ~((1 << SPI_SCK) | (1 << SPI_MOSI));

  /* bus is set up now */
  Cfg.OP_Mode |= OP_SPI;      /* set flag */
}



#ifdef SPI_9

/*
 *  write a single bit
 *  - for displays supporting D/C control via SPI
 *    (3-line SPI with 9 bit frames)
 *
 *  requires:
 *  - Bit: bit to write
 */

void SPI_Write_Bit(uint8_t Bit)
{
  /*
   *  expected state:
   *  - SCK low
   *  - MOSI undefined
   */

  /*
   *  bitbang 1 bit:
   *  - simulate SPI mode 0 (CPOL = 0, CPHA = 0)
   *    - set MOSI before rising SCK
   */

  /* set MOSI based on bit */
  if (Bit)                    /* 1 */
  {
    SPI_PORT |= (1 << SPI_MOSI);        /* set MOSI high */
  }
  else                        /* 0 */
  {
    SPI_PORT &= ~(1 << SPI_MOSI);       /* set MOSI low */
  }

  /* start clock pulse (slave takes bit on rising edge) */
  SPI_PORT |= (1 << SPI_SCK);         /* set SCK high */

  /* end clock pulse (falling edge) */
  SPI_PORT &= ~(1 << SPI_SCK);        /* set SCK low */

  /*
   *  current state:
   *  - SCK low
   *  - MOSI undefined
   */
}

#endif



/*
 *  write a single byte
 *
 *  requires:
 *  - Byte: byte to write
 */

void SPI_Write_Byte(uint8_t Byte)
{
  uint8_t           n = 8;         /* counter */

  /*
   *  expected state:
   *  - SCK low
   *  - MOSI undefined
   */

  /*
   *  bitbang 8 bits:
   *  - simulate SPI mode 0 (CPOL = 0, CPHA = 0)
   *    - set MOSI before rising SCK
   *  - MSB first
   */

  while (n > 0)               /* for 8 bits */
  {
    /* get current MSB and set MOSI */
    if (Byte & 0b10000000)    /* 1 */
    {
      SPI_PORT |= (1 << SPI_MOSI);      /* set MOSI high */
    }
    else                      /* 0 */
    {
      SPI_PORT &= ~(1 << SPI_MOSI);     /* set MOSI low */
    }

    /* start clock pulse (slave takes bit on rising edge) */
    SPI_PORT |= (1 << SPI_SCK);         /* set SCK high */

    /* end clock pulse (falling edge) */
    SPI_PORT &= ~(1 << SPI_SCK);        /* set SCK low */

    Byte <<= 1;               /* shift bits one step left */
    n--;                      /* next bit */
  }

  /*
   *  current state:
   *  - SCK low
   *  - MOSI undefined
   */
}



#ifdef SPI_RW

/*
 *  write and read a single byte
 *
 *  requires:
 *  - Byte: byte to write
 *
 *  returns:
 *  - byte read
 */

uint8_t SPI_WriteRead_Byte(uint8_t Byte)
{
  uint8_t           Byte2 = 0;     /* return value */
  uint8_t           n = 8;         /* counter */
  uint8_t           Temp;          /* temporary value */

  /*
   *  expected state:
   *  - SCK low
   *  - MOSI undefined
   */

  /*
   *  bitbang 8 bits:
   *  - simulate SPI mode 0 (CPOL = 0, CPHA = 0)
   *    - set MOSI before rising SCK
   *    - read MISO after lowering SCK
   *  - MSB first
   */

  while (n > 0)               /* for 8 bits */
  {
    /* get current MSB and set MOSI */
    if (Byte & 0b10000000)    /* 1 */
    {
      SPI_PORT |= (1 << SPI_MOSI);      /* set MOSI high */
    }
    else                      /* 0 */
    {
      SPI_PORT &= ~(1 << SPI_MOSI);     /* set MOSI low */
    }

    /* start clock pulse (slave takes bit on rising edge) */
    SPI_PORT |= (1 << SPI_SCK);         /* set SCK high */

    /* slave needs some time for processing */
    /* wait about 200ns (half cycle for SPI clock of 2.5MHz) */
    #if CPU_FREQ == 8000000
      /* one cycle is 125ns: wait 2 cycles (250ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #elif CPU_FREQ == 16000000
      /* one cycle is 62.5ns: wait 3 cycles (187.6ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #elif CPU_FREQ == 20000000
      /* one cycle is 50ns: wait 4 cycles (200ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #else
      #error <<< SPI_WriteRead_Byte(): no supported MCU clock >>>
    #endif

    /* end clock pulse (slave shifts bit out on falling edge) */
    SPI_PORT &= ~(1 << SPI_SCK);        /* set SCK low */

    /* slave needs some time to shift out bit */
    /* wait about 200ns (half cycle for SPI clock of 2.5MHz) */
    #if CPU_FREQ == 8000000
      /* one cycle is 125ns: wait 2 cycles (250ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #elif CPU_FREQ == 16000000
      /* one cycle is 62.5ns: wait 3 cycles (187.6ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #elif CPU_FREQ == 20000000
      /* one cycle is 50ns: wait 4 cycles (200ns) */
      asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::
      );
    #else
      #error <<< SPI_WriteRead_Byte(): no supported MCU clock >>>
    #endif

    /* read MISO */
    Temp = SPI_PIN;              /* read port */
    Temp &= (1 << SPI_MISO);     /* filter MISO */
    Byte2 <<= 1;                 /* shift bits one step left */
    if (Temp)                    /* MISO set */
    {
      Byte2 |= 0b00000001;       /* set bit */
    }

    Byte <<= 1;               /* shift bits one step left */
    n--;                      /* next bit */
  }

  /*
   *  current state:
   *  - SCK low
   *  - MOSI undefined
   */

  return Byte2;
}

#endif


#endif



/* ************************************************************************
 *   functions for hardware SPI
 * ************************************************************************ */


#ifdef SPI_HARDWARE

/*
 *  set SPI clock rate
 *  - uses SPI.ClockRate for input
 */

void SPI_Clock(void)
{
  uint8_t           Flags;    /* clock rate flags */
  uint8_t           Bits;     /* bitfield */

  Flags = SPI.ClockRate;           /* get clock rate flags */


  /*
   *  update clock rate divider
   */

  Bits = SPCR;                            /* get control register */
  Bits &= ~((1 << SPR1) | (1 << SPR0));   /* clear clock rate bits */

  /* set divider bits */
  if (Flags & SPI_CLOCK_R0) Bits |= (1 << SPR0);
  if (Flags & SPI_CLOCK_R1) Bits |= (1 << SPR1);

  SPCR = Bits;                     /* set new clock rate */


  /*
   *  update double-speed mode
   */

  Bits = 0;                        /* reset variable */

  if (Flags & SPI_CLOCK_2X)        /*  */
  {
    Bits = (1 << SPI2X);           /* set bit to double SPI speed */
  }

  SPSR = Bits;                     /* update register */
}



/*
 *  set up SPI bus
 *  - clock and mode
 *  - lines are set up automatically
 */

void SPI_Setup(void)
{
  uint8_t           Bits;     /* register bits */

  /* set up bus only once */
  if (Cfg.OP_Mode & OP_SPI) return;

  /* set SCK and MOSI to output mode */
  /* also /SS to keep SPI system in master mode */
  Bits = SPI_DDR;
  Bits |= (1 << SPI_SCK) | (1 << SPI_MOSI) | (1 << SPI_SS);
  SPI_DDR = Bits;

  /* MISO is automatically set to input mode by enabling SPI */

  /*
   *  set up hardware SPI
   *  - master mode (MSTR = 1)
   *  - SPI mode 0 (CPOL = 0, CPHA = 0)
   *  - MSB first (DORD = 0)
   *  - polling mode (SPIE = 0)
   */

  /* set mode and enable SPI */
  SPCR = (1 << SPE) | (1 << MSTR);

  /* set clock rate */
  SPI_Clock();

  /* clear SPI interrupt flag, just in case */
  Bits = SPSR;           /* read flag */
  Bits = SPDR;           /* clear flag by reading data */

  /* SPI bus is set up now */
  Cfg.OP_Mode |= OP_SPI;      /* set flag */
}



/*
 *  write a single byte
 *
 *  requires:
 *  - Byte: byte to write
 */

void SPI_Write_Byte(uint8_t Byte)
{
  /* send byte */
  SPDR = Byte;                     /* start transmission */
  while (!(SPSR & (1 << SPIF)));   /* wait for flag */
  Byte = SPDR;                     /* clear flag by reading data */
}



#ifdef SPI_RW

/*
 *  write and read a single byte
 *
 *  requires:
 *  - Byte: byte to write
 *
 *  returns:
 *  - byte read
 */

uint8_t SPI_WriteRead_Byte(uint8_t Byte)
{
  uint8_t           Byte2;         /* return value */

  /* send byte */
  SPDR = Byte;                     /* start transmission */
  while (!(SPSR & (1 << SPIF)));   /* wait for flag */

  /* get received byte */
  Byte2 = SPDR;                    /* clear flag by reading data */

  return Byte2;
}

#endif

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef SPI_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   functions for INA226 (power monitor, I2C bus)
 *
 *   (c) 2025 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment
 *    SCL      I2C: SCL
 *    SDA      I2C: SDA
 *    A0       Gnd, Vcc/VS, SCL or SDA
 *    A1       Gnd, Vcc/VS, SCL or SDA
 *    ALERT    not used (open drain)
 *  - I2C clock modes: standard (100kHz) and fast (400kHz)
 *                     high-speed (up to 2.94MHz)
 *  - Vcc/VS 5V or 3.3V
 *    I2C pull-up resistors to 3.3V should be ok for a 5V ATmega
 *  - typical shunt resistors
 *    R_shunt     I_max        resolution
 *    ------------------------------------
 *    0.1 Ohms    0.8 A        25 µA
 *    0.01 Ohms   8 A          250 µA
 *    0.002 Ohms  20 A (40 A)  1.25 mA
 */



/* local includes */
#include "config.h"           /* global configuration */

#ifdef HW_INA226


/*
 *  local constants
 */

/* source management */
#define INA226_C

/* operation modes */
#define MODE_MANUAL      0    /* manual mode */
#define MODE_AUTO        1    /* automatic mode */

/* I2C addresses */
#define INA226_I2C_ADDR_00    0b01000000     /* 0x40, A0=Gnd A1=Gnd */
#define INA226_I2C_ADDR_01    0b01000001     /* 0x41, A0=VS  A1=Gnd */
#define INA226_I2C_ADDR_02    0b01000010     /* 0x42, A0=SDA A1=Gnd */
#define INA226_I2C_ADDR_03    0b01000011     /* 0x43, A0=SCL A1=Gnd */
#define INA226_I2C_ADDR_04    0b01000100     /* 0x44, A0=Gnd A1=VS */
#define INA226_I2C_ADDR_05    0b01000101     /* 0x45, A0=VS  A1=VS */
#define INA226_I2C_ADDR_06    0b01000110     /* 0x46, A0=SDA A1=VS */
#define INA226_I2C_ADDR_07    0b01000111     /* 0x47, A0=SCL A1=VS */
#define INA226_I2C_ADDR_08    0b01001000     /* 0x48, A0=Gnd A1=SDA */
#define INA226_I2C_ADDR_09    0b01001001     /* 0x49, A0=VS  A1=SDA */
#define INA226_I2C_ADDR_10    0b01001010     /* 0x4a, A0=SDA A1=SDA */
#define INA226_I2C_ADDR_11    0b01001011     /* 0x4b, A0=SCL A1=SDA */
#define INA226_I2C_ADDR_12    0b01001100     /* 0x4c, A0=Gnd A1=SCL */
#define INA226_I2C_ADDR_13    0b01001101     /* 0x4d, A0=VS  A1=SCL */
#define INA226_I2C_ADDR_14    0b01001110     /* 0x4e, A0=SDA A1=SCL */
#define INA226_I2C_ADDR_15    0b01001111     /* 0x4f, A0=SCL A1=SCL */


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */


/*
 *  INA226 specific local constants
 */


/*
 *  configuration register
 *  - 2 bytes, read/write
 */

#define INA226_REG_CONFIG          0b00000000     /* register address */

/* operating mode */
#define INA226_MODE_POWERDOWN      0b0000000000000000  /* power down */
#define INA226_MODE_I              0b0000000000000001  /* shunt voltage */
#define INA226_MODE_V              0b0000000000000010  /* bus voltage */
#define INA226_MODE_TRIG           0b0000000000000000  /* triggered */
#define INA226_MODE_CONT           0b0000000000000100  /* continuous  */
/* default: INA226_MODE_I & INA226_MODE_V & INA226_MODE_CONT */

/* shunt voltage conversion time */
#define INA226_I_TIME_0            0b0000000000000000  /* 140 µs */
#define INA226_I_TIME_1            0b0000000000001000  /* 204 µs */
#define INA226_I_TIME_2            0b0000000000010000  /* 332 µs */
#define INA226_I_TIME_3            0b0000000000011000  /* 588 µs */
#define INA226_I_TIME_4            0b0000000000100000  /* 1.1 ms (default) */
#define INA226_I_TIME_5            0b0000000000101000  /* 2.116 ms */
#define INA226_I_TIME_6            0b0000000000110000  /* 4.156 ms */
#define INA226_I_TIME_7            0b0000000000111000  /* 8.224 ms */

/* bus voltage conversion time */
#define INA226_V_TIME_0            0b0000000000000000  /* 140 µs */
#define INA226_V_TIME_1            0b0000000001000000  /* 204 µs */
#define INA226_V_TIME_2            0b0000000010000000  /* 332 µs */
#define INA226_V_TIME_3            0b0000000011000000  /* 588 µs */
#define INA226_V_TIME_4            0b0000000100000000  /* 1.1 ms (default) */
#define INA226_V_TIME_5            0b0000000101000000  /* 2.116 ms */
#define INA226_V_TIME_6            0b0000000110000000  /* 4.156 ms */
#define INA226_V_TIME_7            0b0000000111000000  /* 8.224 ms */

/* averaging mode (number of samples) */
#define INA226_AVG_1               0b0000000000000000  /* 1 (default) */
#define INA226_AVG_4               0b0000001000000000  /* 4 */
#define INA226_AVG_16              0b0000010000000000  /* 16 */
#define INA226_AVG_64              0b0000011000000000  /* 64 */
#define INA226_AVG_128             0b0000100000000000  /* 128 */
#define INA226_AVG_256             0b0000101000000000  /* 256 */
#define INA226_AVG_512             0b0000110000000000  /* 512 */
#define INA226_AVG_1024            0b0000111000000000  /* 1024 */

/* bit #14: always 1 */
#define INA226_BIT_14              0b0100000000000000  /* bit #14: high */

/* reset INA226 */
#define INA226_RESET               0b1000000000000000  /* reset */


/*
 *  shunt voltage register
 *  - 2 bytes, read-only
 */

#define INA226_REG_SHUNT_VOLTAGE   0b00000001     /* register address */

/* bits 0-15: signed integer, LSB 2.5µV, 81.92mV max */


/*
 *  bus voltage register
 *  - 2 bytes, read-only
 */

#define INA226_REG_BUS_VOLTAGE     0b00000010     /* register address */

/* bits 0-14: unsigned integer, LSB 1.25mV, 40.96V max */
/* bit 15: 0 */


/*
 *  power register
 *  - 2 bytes, read-only
 *  - is calculated: value = (current reg * bus voltage reg) / 20000
 */

#define INA226_REG_POWER           0b00000011     /* register address */

/* bits 0-15: unsigned integer, LSB is 25 * current_LSB */


/*
 *  current register
 *  - 2 bytes, read-only
 *  - is calculated: value = (shunt voltage reg * calibration reg) / 2048
 */

#define INA226_REG_CURRENT         0b00000100     /* register address */

/* bits 0-15: signed 16 bit integer, LSB is current_LSB */


/*
 *  calibration register
 *  - 2 bytes, read/write
 *  - current_LSB = I_max / 2^15 = I_max / 32768
 *    round up to a nice value to simplify I and P conversion in firmware
 *  - calibration value = 0.00512 / (current_LSB * R_shunt)
 */

#define INA226_REG_CAL             0b00000101     /* register address */

/* bits 0-14: unsigned integer */
/* bit 15: 0 */


/*
 *  mask/enable register
 *  - 2 bytes, read/write
 */

#define INA226_REG_MASK            0b00000110     /* register address */

/* alert function (only one plus CNVR) */
/* shunt voltage over-voltage (exceeds alert limit) */
#define INA226_ALERT_SOL           0b1000000000000000  /* SOL */
/* shunt voltage under-voltage (below alert limit) */
#define INA226_ALERT_SUL           0b0100000000000000  /* SUL */
/* bus voltage over-voltage (exceeds alert limit) */
#define INA226_ALERT_BOL           0b0010000000000000  /* BOL */
/* bus voltage under-voltage (below alert limit) */
#define INA226_ALERT_BUL           0b0001000000000000  /* BUL */
/* power over-limit (exceeds alert limit) */
#define INA226_ALERT_POL           0b0000100000000000  /* POL */
/* conversion ready */
#define INA226_ALERT_CNVR          0b0000010000000000  /* CNVR */

/* flags/settings */
/* alert function flag (alert triggered by over/under-limit) */
#define INA226_ALERT_AFF           0b0000000000010000  /* AFF */
/* conversion ready flag (conversion done) */
#define INA226_ALERT_CVRF          0b0000000000001000  /* CVRF */
/* math overflow flag (overflow error) */
#define INA226_ALERT_OVF           0b0000000000000100  /* OVF */
/* alert pin polarity (open collector output) */
#define INA226_ALERT_APOL_NORM     0b0000000000000000  /* active low (default) */
#define INA226_ALERT_APOL_INV      0b0000000000000010  /* active high */
/* alert latch (for pin and flag) */
#define INA226_ALERT_LATCH_TRANS   0b0000000000000000  /* transparent (default) */
#define INA226_ALERT_LATCH_ENABLED 0b0000000000000001  /* enabled */


/*
 *  alert limit register
 *  - 2 bytes, read/write
 */

#define INA226_REG_ALERT           0b00000111     /* register address */

/* bits 0-15: limit value */


/*
 *  manufacturer ID register
 *  - 2 bytes, read-only
 */

#define INA226_REG_MANUFACTURER    0b11111110     /* register address */

/* bits 0-15: manufacturer ID */
#define INA226_MANUFACTURER_ID     0b0101010001001001  /* 0x5449 */


/*
 *   die ID register
 *  - 2 bytes, read-only
 */

#define INA226_REG_DIE             0b11111111     /* register address */

/* bits 0-3: die revision ID */
/* bits 4-15: device ID */



/*
 *  derived constants
 */

/* averaging: set mode based on number of samples */
#if (INA226_SAMPLES >= 1024)
  #define INA226_AVG_MODE INA226_AVG_1024
#elif (INA226_SAMPLES >= 512)
  #define INA226_AVG_MODE INA226_AVG_512
#elif (INA226_SAMPLES >= 128)
  #define INA226_AVG_MODE INA226_AVG_128
#elif (INA226_SAMPLES >= 64)
  #define INA226_AVG_MODE INA226_AVG_64
#elif (INA226_SAMPLES >= 16)
  #define INA226_AVG_MODE INA226_AVG_16
#elif (INA226_SAMPLES >= 4)
  #define INA226_AVG_MODE INA226_AVG_4
#else
  #define INA226_AVG_MODE INA226_AVG_1
#endif


/*
 *  local variables
 */




/* ************************************************************************
 *   low level functions for I2C interface
 * ************************************************************************ */


/*
 *  write to INA226 register
 *
 *  requires:
 *  - Register: 8 bit register address
 *  - Value: 16 bit register value
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t INA226_WriteRegister(uint8_t Register, uint16_t Value)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           OldTimeout;         /* old ACK timeout */

  OldTimeout = I2C.Timeout;             /* save old timeout */
  I2C.Timeout = 1;                      /* ACK timeout 10µs */

  /*
   *  procedure to write register:
   *  - address INA226 in write mode
   *  - select register
   *  - send value byte-wise
   */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    /* send address & write bit, expect ACK from INA226 */
    I2C.Byte = INA226_I2C_ADDR << 1;              /* address (7 bits) & write (0) */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address INA226 */
    {
      /* send register address, expect ACK from INA226 */
      I2C.Byte = Register;                        /* set register address */
      if (I2C_WriteByte(I2C_DATA) == I2C_ACK)     /* send data */
      {      
        /* send MSB of register value, expect ACK from INA226 */
        I2C.Byte = Value >> 8;                    /* set MSB of Value */
        if (I2C_WriteByte(I2C_DATA) == I2C_ACK)   /* send data */
        {
          /* send LSB of register value, expect ACK from INA226 */
          I2C.Byte = Value & 0xff;                     /* set LSB of Value */
          if (I2C_WriteByte(I2C_DATA) == I2C_ACK)      /* send data */
          {
            Flag = 1;                                  /* signal success */
          }
        }
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



/*
 *  read from INA226 register
 *
 *  requires:
 *  - Register: 8 bit register address
 *  - Value: pointer to 16 bit value
 *
 *  returns:
 *  - 1 on success
 *  - 0 on any problem
 */

uint8_t INA226_ReadRegister(uint8_t Register, uint16_t *Value)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           OldTimeout;         /* old ACK timeout */
  uint8_t           HighByte;           /* high byte */
  uint16_t          RawValue;           /* raw value */

  OldTimeout = I2C.Timeout;             /* save old timeout */
  I2C.Timeout = 1;                      /* ACK timeout 10µs */

  /*
   *  procedure to read register:
   *  - address INA226 in write mode
   *  - select register
   *  - restart I2C transfer
   *  - address INA226 in read mode
   *  - read register value byte-wise
   */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    /* send address & write bit, expect ACK from INA226 */
    I2C.Byte = INA226_I2C_ADDR << 1;              /* address (7 bits) & write (0) */
    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address INA226 */
    {
      /* send register address, expect ACK from INA226 */
      I2C.Byte = Register;                        /* set register address */
      if (I2C_WriteByte(I2C_DATA) == I2C_ACK)     /* send data */
      {
        /* end transfer for selecting register */
        I2C_Stop();                               /* stop */

        /* new transfer for reading register */
        if (I2C_Start(I2C_START) == I2C_OK)       /* start */
        {
          /* send address & read bit, expect ACK from INA226 */
          I2C.Byte = (INA226_I2C_ADDR << 1) | 0b00000001;   /* address (7 bits) & read (1) */
          if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)        /* address INA226 */
          {
            /* read high byte and ACK */
            if (I2C_ReadByte(I2C_ACK) == I2C_OK)       /* read first byte */
            {
              HighByte = I2C.Byte;                     /* save byte */

              /* read low byte and NACK */
              if (I2C_ReadByte(I2C_NACK) == I2C_OK)    /* read second byte */
              {
                /* pre-process data */
                RawValue = HighByte;                   /* copy high byte */
                RawValue <<= 8;                        /* and shift to MSB */
                RawValue |= I2C.Byte;                  /* copy low byte */
                *Value = RawValue;                     /* save result */

                Flag = 1;                              /* signal success */          
              }
            }
          }
        }
      }
    }
  }

  I2C_Stop();                                     /* stop */

  I2C.Timeout = OldTimeout;             /* restore old timeout */

  return Flag;
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */




/* ************************************************************************
 *   tool
 * ************************************************************************ */


/*
 *  INA226 tool
 *  - read INA226 and display load current, voltage and power
 */

void INA226_Tool(void)
{
  uint8_t           Flag;               /* loop control */
  uint8_t           Test;               /* key / feedback */
  uint8_t           Mode = MODE_MANUAL; /* operation mode */
  uint16_t          Timeout = 0;        /* timeout for user feedback */
  uint16_t          Value;              /* register value */
  int32_t           I_Value;            /* I value */
  int8_t            I_Scale;            /* I scale / decimal places */
  uint32_t          U_Value;            /* U value */
  int8_t            U_Scale;            /* U scale / decimal places */
  int32_t           P_Value;            /* P value */
  int8_t            P_Scale;            /* P scale / decimal places */
  uint32_t          T_Value;            /* temporary value */
  int8_t            T_Scale;            /* temporary scale / decimal places */

  /* local constants for Flag (bitfield) */
  #define RUN_FLAG            0b00000001     /* run / otherwise end */
  #define SETUP_FLAG          0b00000010     /* set up INA226 */
  #define POWER_ALARM         0b00000100     /* power alarm triggered */


  /*
   *  display info
   */

  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    Display_ColoredEEString(INA226_str, COLOR_TITLE);  /* display: INA226 */
  #else
   Display_EEString(INA226_str);        /* display: INA226 */
  #endif
  LCD_CharPos(1, 2);                    /* move to line #2 */
  Display_EEString(Start_str);          /* display: Start */


  /*
   *  processing loop
   */

  /* enter loop, setup INA226 */
  Flag = RUN_FLAG | SETUP_FLAG;

  while (Flag)
  {
    /*
     *  user input
     */

    /* wait for user input */
    Test = TestKey(Timeout, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

    if (Test == KEY_LONG)          /* long key press */
    {
      /* display mode in line #1 */
      LCD_ClearLine(1);            /* clear line #1 */
      LCD_CharPos(1, 1);           /* move to line #1 */
      #ifdef UI_COLORED_TITLES
        Display_ColoredEEString_Space(INA226_str, COLOR_TITLE);
      #else
        Display_EEString_Space(INA226_str);
      #endif

      /* change mode */
      if (Mode == MODE_MANUAL)     /* manual mode */
      {
        Mode = MODE_AUTO;          /* set automatic mode */
        Timeout = 1000;            /* wait for max. 1s */

        /* indicate auto mode */
        Display_Char('*');         /* display: * */
      }
      else                         /* automatic mode */
      {
        Mode = MODE_MANUAL;        /* set manual mode again */
        Timeout = 0;               /* wait for user */
      }

      MilliSleep(500);             /* smooth UI */
    }
    else if (Test == KEY_TWICE)    /* two short key presses */
    {
      Flag = 0;                    /* end loop */
    }

    LCD_ClearLine2();                   /* clear line #2 */


    /*
     *  set up INA226
     *  - only once after start
     *  - configure INA226
     *  - this also checks if INA226 is connected and responsive
     */

    if (Flag & SETUP_FLAG)         /* setup requested */
    {
      Flag &= ~SETUP_FLAG;              /* clear flag */

      /* configure INA226 (using defaults) */
      Value = INA226_MODE_I | INA226_MODE_V | INA226_MODE_CONT |
              INA226_I_TIME_4 | INA226_V_TIME_4 | INA226_V_TIME_4 |
              INA226_AVG_MODE | INA226_BIT_14;

      Test = INA226_WriteRegister(INA226_REG_CONFIG, Value);     /* write register */
      if (Test == 0)                    /* error */
      {
        /* inform user */
        Display_EEString(Error_str);         /* display: Error */
        WaitKey();                           /* wait for user feedback */

        Flag = 0;                            /* don't proceed */
      }
      /* else: ok */
    }


    /*
     *  read INA226, calculate und display values
     */

    if (Flag)            /* ok to proceed */
    {
      /*
       *  read INA226's register values
       */

      /* read shunt voltage register */
      Test = INA226_ReadRegister(INA226_REG_SHUNT_VOLTAGE, &Value);
      if (Test)                         /* got value */
      {
        /* register value is signed 16-bit integer */
        I_Value = (int16_t)Value;       /* save value */

        #ifdef INA226_SHUNT_ZERO_THRESHOLD
        /* consider zero threshold for shunt voltage (+/- span) */
        if ((I_Value >= -INA226_SHUNT_ZERO_THRESHOLD) &&
            (I_Value <= INA226_SHUNT_ZERO_THRESHOLD))
        {
          I_Value = 0;                  /* set to zero */
        }
        #endif

        /* LSB is 2.5 µV */
        I_Value *= 25;                  /* apply MSB (in 0.1 µV) */
        I_Scale = -7;                   /* 0.1 µV */

        /* read bus voltage register */
        Test = INA226_ReadRegister(INA226_REG_BUS_VOLTAGE, &Value);
        if (Test)                       /* got value */
        {
          U_Value = Value;              /* save value */

          /* LSB is 1.25 mV */
          U_Value *= 125;               /* apply MSB (in 0.01 mV) */
          U_Scale = -5;                 /* 0.01 mV */

          #ifdef INA226_MINUS_SHUNT
          /* subtract voltage across current shunt from bus voltage */
          P_Value = I_Value;            /* copy I */
          /* rescale to match U */
          P_Value /= 100;               /* scale from -7 to -5 */
          /* take absolute value */
          if (P_Value < 0)              /* negative voltage drop */
          {
            P_Value = -P_Value;         /* make value positive */
          }
          if (P_Value < U_Value)        /* prevent underflow */
          {
            U_Value -= P_Value;         /* subtract voltage drop */
          }
          #endif

          /* calculate current (I = U_shunt / R_shunt) */
          I_Value *= 100;               /* scale to 0.001 µV (Scale -9) */
          I_Value /= INA226_R_SHUNT;    /* U_shunt / R_shunt (R in mOhms) */
          I_Scale = -6;                 /* 10^-9 / 10^-3 = 10^-6 */

          /* calculate power (P = U * I) */
          P_Value = I_Value;            /* copy I */
          P_Scale = I_Scale;

          /* rescale I to prevent overflow */
          while (P_Value > 10000)
          {
            P_Value /= 10;              /* value /10 */
            P_Scale++;                  /* scale +1 */
          }

          T_Value = U_Value;            /* copy U */
          T_Scale = U_Scale;

          /* rescale U to prevent overflow */
          while (T_Value > 10000)
          {
            T_Value /= 10;              /* value /10 */
            T_Scale++;                  /* scale +1 */
          }

          /* multiply (P = U * I) */
          P_Value *= T_Value;           /* U * I */
          P_Scale += T_Scale;

          #ifdef INA226_POWER_ALARM
          /* check power alarm threshold */

          /* take absolute value */
          if (P_Value >= 0)             /* positive */
          {
            T_Value = P_Value;          /* take positive value */
          }
          else                          /* negative */
          {
            T_Value = -P_Value;         /* make it positive */
          }

          /* compare current power value with alarm threshold (in mW) */
          if (CmpValue(T_Value, P_Scale, (uint32_t)INA226_POWER_ALARM, -3) >= 0)
          {
            Flag |= POWER_ALARM;             /* set flag */
          }
          #endif
        }
      }


      /*
       *  display results
       */

      /* clear lines #3 and #4 */
      LCD_ClearLine3();                 /* clear line #3 */
      LCD_ClearLine(4);                 /* clear line #4 */
      LCD_CharPos(1, 2);                /* pos #1 in line #2 */

      if (Test)                         /* got values */
      {
        /* display voltage in line #2 */
        Display_Char('V');
        Display_Space();
        Display_Value(U_Value, U_Scale, 'V');

        /* display current in line #3 */
        LCD_CharPos(1, 3);              /* pos #1 in line #3 */
        Display_Char('I');
        Display_Space();
        Display_SignedValue(I_Value, I_Scale, 'A');

        /* display power in line #4 */
        LCD_CharPos(1, 4);              /* pos #1 in line #4 */
        Display_Char('P');
        Display_Space();
        Display_SignedValue(P_Value, P_Scale, 'W');

        #ifdef INA226_POWER_ALARM
        /* buzzer: short beep when power threshold is exceeded */
        if (Flag & POWER_ALARM)              /* power alarm triggered */
        {
          Flag &= ~POWER_ALARM;              /* clear flag */

          #ifdef BUZZER_ACTIVE
          /* active buzzer: short beep (20ms) */
          BUZZER_PORT |= (1 << BUZZER_CTRL);      /* enable: set pin high */
          MilliSleep(20);                         /* wait for 20 ms */
          BUZZER_PORT &= ~(1 << BUZZER_CTRL);     /* disable: set pin low */
          #endif

          #ifdef BUZZER_PASSIVE
          /* passive buzzer: short beep, low freq (20ms, 2.5kHz) */
          PassiveBuzzer(BUZZER_FREQ_LOW);         /* low frequency beep */
          #endif
        }
        #endif
      }
      else                              /* error */
      {
        Display_Minus();                     /* display: - */
      }
    }
  }


  /*
   *  clean up
   */

  /* local constants for Flag */
  #undef RUN_FLAG
  #undef SETUP_FLAG
  #undef POWER_ALARM
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* local constants */
#undef MODE_MANUAL
#undef MODE_AUTO

/* source management */
#undef INA226_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

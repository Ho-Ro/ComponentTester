/* ************************************************************************
 *
 *   global functions
 *
 *   (c) 2012-2013 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  In each source file we create a local ID definition. If the ID definition
 *  is unset we may import functions of that specific source file.
 */

/* ************************************************************************
 *   functions from main.c
 * ************************************************************************ */

#ifndef MAIN_C

#endif


/* ************************************************************************
 *   functions from LCD.c
 * ************************************************************************ */

#ifndef LCD_C

  extern void lcd_enable(void);
  extern void lcd_send(unsigned char Data);
  extern void lcd_command(unsigned char Cmd);
  extern void lcd_data(unsigned char Data);

  extern void lcd_clear(void);
  extern void lcd_line(unsigned char Line);
  extern void lcd_clear_line(unsigned char Line);
  extern void lcd_init(void);
  extern void lcd_fixed_customchar(const unsigned char *CharData, uint8_t ID);

  extern void lcd_space(void);
  extern void lcd_testpin(unsigned char Probe);
//  extern void lcd_string(char *String);
  extern void lcd_fixed_string(const unsigned char *String);

#endif


/* ************************************************************************
 *   functions from pause.c
 * ************************************************************************ */

#ifndef PAUSE_C

  extern void MilliSleep(uint16_t Time);

#endif


/* ************************************************************************
 *   functions from adjust.c
 * ************************************************************************ */

#ifndef ADJUST_C

  extern uint8_t CheckSum(void);
  extern void SafeAdjust(void);
  extern void LoadAdjust(void);

  extern void ShowAdjust(void);
  extern uint8_t SelfAdjust(void);

  extern uint8_t SelfTest(void);

#endif


/* ************************************************************************
 *   functions from user.c
 * ************************************************************************ */

#ifndef USER_C

  extern int8_t CmpValue(unsigned long Value1, int8_t Scale1,
    unsigned long Value2, int8_t Scale2);

  extern void DisplayValue(unsigned long Value, int8_t Exponent, unsigned char Unit);
  extern void DisplaySignedValue(signed long Value, int8_t Exponent, unsigned char Unit);

  extern void ShortCircuit(uint8_t Mode);
  extern uint8_t TestKey(uint16_t Timeout, uint8_t Mode);
  extern void MainMenu(void);

#endif


/* ************************************************************************
 *   functions from probes.c
 * ************************************************************************ */

#ifndef PROBES_C

  extern uint8_t ShortedProbes(uint8_t Probe1, uint8_t Probe2);
  extern uint8_t AllProbesShorted(void);
  extern void DischargeProbes(void);
  extern void PullProbe_10ms(uint8_t Probe, uint8_t Mode);
  extern void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);

  extern void MeasureCap(uint8_t Probe1, uint8_t Probe2, uint8_t ID);

  extern unsigned int SmallResistor(void);
 
  extern void CheckProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);

#endif


/* ************************************************************************
 *   functions from ADC.c
 * ************************************************************************ */

#ifndef ADC_C

  extern unsigned int ReadU(uint8_t Probe);

  extern unsigned int ReadU_5ms(uint8_t Probe);
  extern unsigned int ReadU_20ms(uint8_t Probe);

#endif


/* ************************************************************************
 *   functions from wait.S
 * ************************************************************************ */

#ifndef WAIT_S

  /* clock frequency 1 MHz */
  extern void wait5s(void);
  extern void wait4s(void);
  extern void wait3s(void);
  extern void wait2s(void);
  extern void wait1s(void);
  extern void wait1000ms(void);
  extern void wait500ms(void);
  extern void wait400ms(void);
  extern void wait300ms(void);
  extern void wait200ms(void);
  extern void wait100ms(void);
  extern void wait50ms(void);
  extern void wait40ms(void);
  extern void wait30ms(void);
  extern void wait20ms(void);
  extern void wait10ms(void);
  extern void wait5ms(void);
  extern void wait4ms(void);
  extern void wait3ms(void);
  extern void wait2ms(void);
  extern void wait1ms(void);
  extern void wait500us(void);
  extern void wait400us(void);
  extern void wait300us(void);
  extern void wait200us(void);
  extern void wait100us(void);
  extern void wait50us(void);
  extern void wait40us(void);
  extern void wait30us(void);
  extern void wait20us(void);
  extern void wait10us(void);

  /* clock frequency 2 MHz */
  extern void wait5us(void);

  /* clock frequency 4 MHz */
  extern void wait4us(void);
  extern void wait3us(void);
  extern void wait2us(void);

  /* clock frequency 8 MHz */
  extern void wait1us(void);

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

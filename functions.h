/* ************************************************************************
 *
 *   global functions
 *
 *   (c) 2012-2014 by Markus Reschke
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

  extern void LCD_Enable(void);
  extern void LCD_Send(unsigned char Data);
  extern void LCD_Cmd(unsigned char Cmd);
  extern void LCD_Data(unsigned char Data);

  extern void LCD_Clear(void);
//  extern void LCD_Line(unsigned char Line);
  extern void LCD_Line2(void);
  extern void LCD_Init(void);
  extern void LCD_EELoadChar(const unsigned char *CharData, uint8_t ID);

//  extern void LCD_ClearLine(unsigned char Line);
  extern void LCD_ClearLine2(void);
  extern void LCD_Space(void);
  extern void LCD_ProbeNumber(unsigned char Probe);
//  extern void LCD_String(char *String);
  extern void LCD_EEString(const unsigned char *String);

  extern void LCD_EEString2(const unsigned char *String);

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

  #ifdef SW_INDUCTOR
    extern unsigned long RescaleValue(unsigned long Value, int8_t Scale, int8_t NewScale);
  #endif

  #ifdef SW_FREQ_GEN
    extern void DisplayFullValue(unsigned long Value, uint8_t DecPlaces, unsigned char Unit);
  #endif

  extern void DisplayValue(unsigned long Value, int8_t Exponent, unsigned char Unit);
  extern void DisplaySignedValue(signed long Value, int8_t Exponent, unsigned char Unit);

  extern uint8_t TestKey(uint16_t Timeout, uint8_t Mode);
  extern int8_t ShortCircuit(uint8_t Mode);
  extern void MainMenu(void);

#endif


/* ************************************************************************
 *   functions from extras.c
 * ************************************************************************ */

#ifndef EXTRAS_C

  #ifdef SW_PWM
    extern void PWM_Tool(uint16_t Frequency);
  #endif
  #ifdef SW_FREQ_GEN
    extern void FrequencyGenerator(void);
  #endif
  #ifdef SW_ESR
    extern void ESR_Tool(void);
  #endif
  #ifdef HW_ZENER
    extern void Zener_Tool(void);
  #endif
  #ifdef HW_FREQ_COUNTER
    extern void FrequencyCounter(void);
  #endif

#endif


/* ************************************************************************
 *   functions from semi.c
 * ************************************************************************ */

#ifndef SEMI_C

  extern void GetGateThreshold(uint8_t Type);
  extern unsigned long Get_hfe_c(uint8_t Type);
  extern uint16_t GetLeakageCurrent(void);

  extern void CheckDiode(void);

  extern void VerifyMOSFET(void);
  extern void CheckBJTorEnhModeMOSFET(uint8_t BJT_Type, unsigned int U_Rl);
  extern void CheckDepletionModeFET(void);

  extern uint8_t CheckThyristorTriac(void);

#endif


/* ************************************************************************
 *   functions from resistor.c
 * ************************************************************************ */

#ifndef RESISTOR_C

  extern unsigned int SmallResistor(uint8_t ZeroFlag);
  extern void CheckResistor(void);
  extern uint8_t CheckSingleResistor(uint8_t HighPin, uint8_t LowPin);

#endif


/* ************************************************************************
 *   functions from inductor.c
 * ************************************************************************ */

#ifndef INDUCTOR_C

  #ifdef SW_INDUCTOR
    extern uint8_t MeasureInductor(Resistor_Type *Resistor);
  #endif

#endif


/* ************************************************************************
 *   functions from cap.c
 * ************************************************************************ */

#ifndef CAP_C

  #ifdef SW_ESR
    extern unsigned int MeasureESR(Capacitor_Type *Cap);
  #endif

  extern void MeasureCap(uint8_t Probe1, uint8_t Probe2, uint8_t ID);

#endif


/* ************************************************************************
 *   functions from probes.c
 * ************************************************************************ */

#ifndef PROBES_C

  extern void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);
  extern uint8_t ShortedProbes(uint8_t Probe1, uint8_t Probe2);
  extern uint8_t AllProbesShorted(void);
  extern void DischargeProbes(void);
  extern void PullProbe(uint8_t Probe, uint8_t Mode);
  extern unsigned int GetFactor(unsigned int U_in, uint8_t ID);

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

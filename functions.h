/* ************************************************************************
 *
 *   global functions
 *
 *   (c) 2012-2021 by Markus Reschke
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

  extern void Show_SemiPinout(uint8_t A, uint8_t B, uint8_t C);
  extern void Show_SimplePinout(uint8_t ID_1, uint8_t ID_2, uint8_t ID_3);

  extern void CheckVoltageRefs(void);

  extern void PowerOff(void);

  #ifndef BAT_NONE
  extern void ShowBattery(void);
  extern void CheckBattery(void);
  #endif

#endif


/* ************************************************************************
 *   functions from LCD module specific driver
 * ************************************************************************ */

#ifndef LCD_DRIVER_C

  extern void LCD_BusSetup(void);
  extern void LCD_Init(void);
  extern void LCD_ClearLine(uint8_t Line);
  extern void LCD_Clear(void);
  extern void LCD_CharPos(uint8_t x, uint8_t y);
  extern void LCD_Cursor(uint8_t Mode);
  extern void LCD_Char(unsigned char Char);

  #if ! defined (UI_SERIAL_COPY) && ! defined (UI_SERIAL_COMMANDS)
    /* make Display_Char() an alias for LCD_Char() */
    #define Display_Char LCD_Char
  #endif

  #ifdef SW_CONTRAST
  extern void LCD_Contrast(uint8_t Contrast);
  #endif

  #ifdef SW_SYMBOLS
  extern void LCD_Symbol(uint8_t ID);
  #endif

  #ifdef FUNC_COLORCODE
  extern void LCD_Band(uint16_t Color, uint8_t Align);
  #endif

#endif


/* ************************************************************************
 *   functions from SPI.c
 * ************************************************************************ */

#ifndef SPI_C

  #ifdef HW_SPI
    #ifdef SPI_HARDWARE
    extern void SPI_Clock(void);
    #endif
  extern void SPI_Setup(void);
    #if defined (SPI_BITBANG) && defined (SPI_9)
    extern void SPI_Write_Bit(uint8_t Bit);
    #endif
  extern void SPI_Write_Byte(uint8_t Byte);
    #ifdef SPI_RW
    extern uint8_t SPI_WriteRead_Byte(uint8_t Byte);
    #endif
  #endif

#endif


/* ************************************************************************
 *   functions from I2C.c
 * ************************************************************************ */

#ifndef I2C_C

  #ifdef HW_I2C
  extern uint8_t I2C_Setup(void);
  extern uint8_t I2C_Start(uint8_t Type);
  extern uint8_t I2C_WriteByte(uint8_t Type);
  extern void I2C_Stop(void);
    #ifdef I2C_RW
    extern uint8_t I2C_ReadByte(uint8_t Type);
    #endif
  #endif

#endif


/* ************************************************************************
 *   functions from serial.c
 * ************************************************************************ */

#ifndef SERIAL_C

  #ifdef HW_SERIAL
  extern void Serial_Setup(void);
  extern void Serial_WriteByte(uint8_t Byte);
    #ifdef SERIAL_RW
    void Serial_Ctrl(uint8_t Control);
    #endif

    extern void Serial_Char(unsigned char Char);

    #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
    extern void Serial_NewLine(void);
    #endif

  #endif

#endif


/* ************************************************************************
 *   functions from OneWire.c
 * ************************************************************************ */

#ifndef ONEWIRE_C

  #ifdef ONEWIRE_IO_PIN
  extern void OneWire_Setup(void);
  #endif

  #ifdef ONEWIRE_PROBES
  extern uint8_t OneWire_Probes(const unsigned char *String);
  #endif

  #if defined (ONEWIRE_IO_PIN) || defined (ONEWIRE_PROBES)
  extern uint8_t OneWire_ResetBus(void);
  #endif

  #ifdef SW_ONEWIRE_SCAN
  extern uint8_t OneWire_Scan_Tool(void);
  #endif

  #ifdef SW_DS18B20
  extern uint8_t DS18B20_ReadTemperature(int32_t *Value, int8_t *Scale, uint8_t *Bits);
  extern uint8_t DS18B20_Tool(void);
  #endif

#endif


/* ************************************************************************
 *   functions from DHTxx.c
 * ************************************************************************ */

#ifndef DHTXX_C

  extern uint8_t DHTxx_Tool(void);

#endif


/* ************************************************************************
 *   functions from touchscreen specific driver
 * ************************************************************************ */

#ifndef TOUCH_DRIVER_C

  #ifdef HW_TOUCH
  extern void Touch_BusSetup(void);
  extern void Touch_Init(void);
  extern uint8_t Touch_Check(void);
  extern uint8_t Touch_Adjust(void);
  #endif

#endif


/* ************************************************************************
 *   functions from display.c
 * ************************************************************************ */

#ifndef DISPLAY_C

  extern void Display_NextLine(void);
  #ifdef UI_KEY_HINTS
  extern void Display_LastLine(void);
  #endif
  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  extern void Display_Char(unsigned char Char);
  #endif
  extern void Display_EEString(const unsigned char *String);

  extern void Display_ProbeNumber(uint8_t Probe);
  extern void Display_Space(void);
  extern void Display_Minus(void);
  extern void Display_Colon(void);
  extern void Display_EEString_Space(const unsigned char *String);
  extern void Display_NL_EEString(const unsigned char *String);
  extern void Display_NL_EEString_Space(const unsigned char *String);

  extern void LCD_ClearLine2(void);

  #ifdef UI_SERIAL_COPY
  extern void Display_Serial_On(void);
  extern void Display_Serial_Off(void);
  #endif

  #if defined (UI_SERIAL_COMMANDS) || defined (SW_DISPLAY_REG)
  extern void Display_Serial_Only(void);
  extern void Display_LCD_Only(void);
  #endif

  #ifdef UI_SERIAL_COMMANDS
  extern void Display_EEString_NL(const unsigned char *String);
  #endif

  #ifdef UI_COLORED_TITLES
  extern void Display_ColoredEEString(const unsigned char *String, uint16_t Color);
  extern void Display_ColoredEEString_Space(const unsigned char *String, uint16_t Color);
  extern void Display_UseTitleColor(void);
  extern void Display_UsePenColor(void);
  #endif

  #if defined (FUNC_DISPLAY_HEXBYTE) || defined (FUNC_DISPLAY_HEXVALUE)
  extern void Display_HexDigit(uint8_t Digit);
  #endif

  #ifdef FUNC_DISPLAY_HEXBYTE
  extern void Display_HexByte(uint8_t Value);
  #endif

  #ifdef FUNC_DISPLAY_HEXVALUE
  extern void Display_HexValue(uint16_t Value, uint8_t Bits);
  #endif

  #ifdef FUNC_DISPLAY_FULLVALUE
  extern void Display_FullValue(uint32_t Value, uint8_t DecPlaces, unsigned char Unit);
  #endif

  #if defined (SW_DS18B20) || defined (SW_DHTXX)
  extern void Display_SignedFullValue(int32_t Value, uint8_t DecPlaces, unsigned char Unit);
  #endif

  extern void Display_Value(uint32_t Value, int8_t Exponent, unsigned char Unit);
  extern void Display_SignedValue(int32_t Value, int8_t Exponent, unsigned char Unit);

  #ifdef FUNC_EVALUE
  extern void Display_EValue(uint16_t Value, int8_t Scale, unsigned char Unit);
  #endif

  #ifdef FUNC_EIA96
  extern void Display_EIA96(uint8_t Index, int8_t Scale);
  #endif

  #ifdef SW_SYMBOLS
  extern void LCD_FancySemiPinout(uint8_t Line);
  #endif

  #ifdef SW_CONTRAST
  extern void ChangeContrast(void);
  #endif  

  #ifdef SW_FONT_TEST
  extern void FontTest(void);
  #endif

  #ifdef FUNC_COLORCODE
  extern void Display_ColorCode(uint16_t Value, int8_t Scale, uint16_t TolBand);
  #endif

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

  #if defined (CAP_MULTIOFFSET) || defined (R_MULTIOFFSET)
  extern uint8_t GetOffsetIndex(uint8_t Probe1, uint8_t Probe2);
  #endif

  extern void SetAdjustmentDefaults(void);
  extern void ManageAdjustmentStorage(uint8_t Mode, uint8_t ID);

  extern void ShowAdjustmentValues(void);
  extern uint8_t SelfAdjustment(void);

  extern uint8_t SelfTest(void);

#endif


/* ************************************************************************
 *   functions from commands.c
 * ************************************************************************ */

#ifndef COMMANDS_C

  #ifdef UI_SERIAL_COMMANDS
  extern uint8_t GetCommand(void);
  extern uint8_t RunCommand(uint8_t ID);
  #endif

#endif


/* ************************************************************************
 *   functions from user.c
 * ************************************************************************ */

#ifndef USER_C

  extern int8_t CmpValue(uint32_t Value1, int8_t Scale1,
    uint32_t Value2, int8_t Scale2);
  extern uint32_t RescaleValue(uint32_t Value, int8_t Scale, int8_t NewScale);

  #ifdef UI_ROUND_DS18B20
  extern int32_t RoundSignedValue(int32_t Value, uint8_t Scale, uint8_t RoundScale);
  #endif

  #ifdef UI_FAHRENHEIT
    #if defined (SW_DS18B20) || defined (SW_DHTXX)
    extern int32_t Celsius2Fahrenheit(int32_t Value, uint8_t Scale);
    #endif
  #endif

  extern uint8_t TestKey(uint16_t Timeout, uint8_t Mode);
  extern void WaitKey(void);
  #ifdef FUNC_SMOOTHLONGKEYPRESS
  extern void SmoothLongKeyPress(void);
  #endif

  extern int8_t ShortCircuit(uint8_t Mode);

  extern void MarkItem(uint8_t Item, uint8_t Selected);

  extern void AdjustmentMenu(uint8_t Mode);
  extern void MainMenu(void);

#endif


/* ************************************************************************
 *   functions from IR_RX.c
 * ************************************************************************ */

#ifndef IR_RX_C

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
  extern void IR_Detector(void);
  #endif

#endif


/* ************************************************************************
 *   functions from IR_TX.c
 * ************************************************************************ */

#ifndef IR_TX_C

  #ifdef SW_IR_TRANSMITTER
  extern void IR_RemoteControl(void);
  #endif

#endif


/* ************************************************************************
 *   functions from tools_misc.c
 * ************************************************************************ */

#ifndef TOOLS_MISC_C

  extern void ProbePinout(uint8_t Mode);

  #ifdef HW_ZENER
  extern void Zener_Tool(void);
  #endif

  #ifdef HW_PROBE_ZENER
  extern void CheckZener(void);
  #endif

  #ifdef SW_ESR_TOOL
  extern void ESR_Tool(void);
  #endif

  #ifdef SW_ENCODER
  extern void Encoder_Tool(void);
  #endif

  #ifdef SW_OPTO_COUPLER
  extern void OptoCoupler_Tool(void);
  #endif

  #ifdef SW_CAP_LEAKAGE
  extern void Cap_Leakage(void);
  #endif

  #ifdef SW_MONITOR_R
  extern void Monitor_R(void);
  #endif

  #ifdef SW_MONITOR_C
  extern void Monitor_C(void);
  #endif

  #ifdef SW_MONITOR_L
  extern void Monitor_L(void);
  #endif

  #ifdef SW_MONITOR_RCL
  extern void Monitor_RCL(void);
  #endif

  #ifdef SW_MONITOR_RL
  extern void Monitor_RL(void);
  #endif

#endif


/* ************************************************************************
 *   functions from tools_signal.c
 * ************************************************************************ */

#ifndef TOOLS_SIGNAL_C

  #ifdef SW_PWM_SIMPLE
  extern void PWM_Tool(uint16_t Frequency);
  #endif

  #ifdef SW_PWM_PLUS
  extern void PWM_Tool(void);
  #endif

  #ifdef SW_SERVO
  extern void Servo_Check(void);
  #endif

  #ifdef SW_SQUAREWAVE
  extern void SquareWave_SignalGenerator(void);
  #endif

  #ifdef HW_FREQ_COUNTER
  extern void FrequencyCounter(void);
  #endif

  #ifdef HW_EVENT_COUNTER
  extern void EventCounter(void);
  #endif

#endif


/* ************************************************************************
 *   functions from tools_LC_Meter.c
 * ************************************************************************ */

#ifndef TOOLS_LC_METER_C

  #ifdef HW_LC_METER
  extern uint8_t LC_Meter(void);
  #endif

#endif


/* ************************************************************************
 *   functions from semi.c
 * ************************************************************************ */

#ifndef SEMI_C

  extern void GetGateThreshold(uint8_t Type);
  extern uint32_t Get_hfe_c(uint8_t Type);
  extern void GetLeakageCurrent(uint8_t Mode);

  extern Diode_Type *SearchDiode(uint8_t A, uint8_t C);
  extern void CheckDiode(void);

  extern void VerifyMOSFET(void);
  extern void CheckTransistor(uint8_t BJT_Type, uint16_t U_Rl);
  extern void CheckDepletionModeFET(uint16_t U_Rl);

  extern uint8_t CheckThyristorTriac(void);
  extern void CheckPUT(void);

  #ifdef SW_UJT
  extern void CheckUJT(void);
  #endif

#endif


/* ************************************************************************
 *   functions from resistor.c
 * ************************************************************************ */

#ifndef RESISTOR_C

  extern uint16_t SmallResistor(uint8_t ZeroFlag);
  extern void CheckResistor(void);
  extern uint8_t CheckSingleResistor(uint8_t HighPin, uint8_t LowPin, uint8_t Max);

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

  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  extern uint16_t MeasureESR(Capacitor_Type *Cap);
  #endif

  extern void MeasureCap(uint8_t Probe1, uint8_t Probe2, uint8_t ID);

  #ifdef HW_ADJUST_CAP
  extern uint8_t RefCap(void);
  #endif

#endif


/* ************************************************************************
 *   functions from probes.c
 * ************************************************************************ */

#ifndef PROBES_C

  extern void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);
  extern void RestoreProbes(void);
  extern void BackupProbes(void);
  extern uint8_t GetThirdProbe(uint8_t Probe1, uint8_t Probe2);
  extern uint8_t ShortedProbes(void);
  #if defined (SW_ESR) || defined (SW_OLD_ESR)
  extern void DischargeCap(uint8_t Probe1, uint8_t Probe2);
  #endif
  extern void DischargeProbes(void);
  extern void PullProbe(uint8_t Probe, uint8_t Mode);

  extern uint16_t GetFactor(uint16_t U_in, uint8_t ID);
  #if defined (FUNC_EVALUE) || defined (FUNC_COLORCODE) || defined (FUNC_EIA96)
  extern uint8_t GetENormValue(uint32_t Value, int8_t Scale, uint8_t E_Series, uint8_t Tolerance);
  #endif

  extern void CheckProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3);
  extern void CheckAlternatives(void);

#endif


/* ************************************************************************
 *   functions from ADC.c
 * ************************************************************************ */

#ifndef ADC_C

  extern uint16_t ReadU(uint8_t Channel);

  extern uint16_t ReadU_5ms(uint8_t Channel);
  extern uint16_t ReadU_20ms(uint8_t Channel);

#endif


/* ************************************************************************
 *   functions from wait.S
 * ************************************************************************ */

#ifndef WAIT_S

  /* clock frequency >= 1 MHz */
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

  /* clock frequency >= 2 MHz */
  extern void wait5us(void);

  /* clock frequency >= 4 MHz */
  extern void wait4us(void);
  extern void wait3us(void);
  extern void wait2us(void);

  /* clock frequency >= 8 MHz */
  extern void wait1us(void);

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

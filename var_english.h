/* ************************************************************************
 *
 *   language specific global variables: English (ISO 8859-1)
 *
 *   (c) 2012-2021 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  English
 */

#if defined (UI_ENGLISH)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Component Tester";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Probing...";
  const unsigned char Timeout_str[] MEM_TYPE = "Timeout";
  const unsigned char Failed1_str[] MEM_TYPE = "No component";
  const unsigned char Failed2_str[] MEM_TYPE = "found!";
  const unsigned char Done_str[] MEM_TYPE = "done!";
  const unsigned char Select_str[] MEM_TYPE = "Select";
  const unsigned char Selftest_str[] MEM_TYPE = "Test";
  const unsigned char Adjustment_str[] MEM_TYPE = "Adjustment";
  const unsigned char Save_str[] MEM_TYPE = "Save";
  const unsigned char Load_str[] MEM_TYPE = "Load";
  const unsigned char Show_str[] MEM_TYPE = "Show Values";
  const unsigned char Remove_str[] MEM_TYPE = "Remove";
  const unsigned char Create_str[] MEM_TYPE = "Create";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "Short Circuit!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Battery?";
  const unsigned char Error_str[] MEM_TYPE = "Error!";
  const unsigned char Exit_str[] MEM_TYPE = "Exit";
  const unsigned char Checksum_str[] MEM_TYPE = "Checksum";
  const unsigned char BJT_str[] MEM_TYPE = "BJT";
  const unsigned char Thyristor_str[] MEM_TYPE = "SCR";
  const unsigned char Triac_str[] MEM_TYPE = "Triac";
  const unsigned char PUT_str[] MEM_TYPE = "PUT";
  const unsigned char Bye_str[] MEM_TYPE = "Bye!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "ok";
    const unsigned char Weak_str[] MEM_TYPE = "weak";
    const unsigned char Low_str[] MEM_TYPE = "low";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "ext";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Menu Test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "Square Wave";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Zener";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Freq. Counter";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "LF crystal";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "HF crystal";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Event Counter";
    const unsigned char Count_str[] MEM_TYPE = "Count";
    const unsigned char Time_str[] MEM_TYPE = "Time";
    const unsigned char Events_str[] MEM_TYPE = "Events";
    const unsigned char Stop_str[] MEM_TYPE = "Stop";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "LC Meter";
    const unsigned char Adjusting_str[] MEM_TYPE = "adjusting...";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Rotary Encoder";
    const unsigned char TurnRight_str[] MEM_TYPE = "Turn right!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Contrast";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "IR detector";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "IR RC";
    const unsigned char IR_Send_str[] MEM_TYPE = "sending...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Opto Coupler";
    const unsigned char None_str[] MEM_TYPE = "None";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (SW_ONEWIRE_SCAN) || defined (HW_EVENT_COUNTER)
    const unsigned char Start_str[] MEM_TYPE = "Start";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Servo";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Cap Leakage";
    const unsigned char CapCharge_str[] MEM_TYPE = "Charging";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Discharging";
  #endif

  #ifdef SW_MONITOR_R
    const unsigned char Monitor_R_str[] MEM_TYPE = "R Monitor";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] MEM_TYPE = "C Monitor";
  #endif

  #ifdef SW_MONITOR_L
    const unsigned char Monitor_L_str[] MEM_TYPE = "L Monitor";
  #endif

  #ifdef SW_MONITOR_RCL
    const unsigned char Monitor_RCL_str[] MEM_TYPE = "RCL Monitor";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] MEM_TYPE = "RL Monitor";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] MEM_TYPE = "Touch Setup";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Off";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "OneWire Scan";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

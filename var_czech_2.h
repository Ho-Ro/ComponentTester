/* ************************************************************************
 *
 *   language specific global variables: Czech (ISO 8859-2)
 *
 *   (c) 2015-2021 by Markus Reschke
 *   translation by Kapa and Bohu
 *   based on code from Markus Frejek and Karl-Heinz K�bbeler
 *
 * ************************************************************************ */


/*
 *  Czech
 */

#if defined (UI_CZECH_2)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Ctester";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Prob�h� m��en�..";
  const unsigned char Timeout_str[] MEM_TYPE = "Vypr�el �as";
  const unsigned char Failed1_str[] MEM_TYPE = "?��dn� sou��tka";
  const unsigned char Failed2_str[] MEM_TYPE = "nezn�m� - vadn�?";
  const unsigned char Done_str[] MEM_TYPE = "hotovo!";
  const unsigned char Select_str[] MEM_TYPE = "V�b�r";
  const unsigned char Selftest_str[] MEM_TYPE = "Autotest";
  const unsigned char Adjustment_str[] MEM_TYPE = "Kalibrace";
  const unsigned char Save_str[] MEM_TYPE = "Ulo�it";
  const unsigned char Load_str[] MEM_TYPE = "Na��st";
  const unsigned char Show_str[] MEM_TYPE = "Zobraz hodnoty";
  const unsigned char Remove_str[] MEM_TYPE = "Odstranit";
  const unsigned char Create_str[] MEM_TYPE = "Ud�lej";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "Zkrat!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Baterie?";
  const unsigned char Error_str[] MEM_TYPE = "Chyba!";
  const unsigned char Exit_str[] MEM_TYPE = "Exit";
  const unsigned char Checksum_str[] MEM_TYPE = "Kontroln� sou�et";
  const unsigned char BJT_str[] MEM_TYPE = "Tranzistor";
  const unsigned char Thyristor_str[] MEM_TYPE = "Tyristor";
  const unsigned char Triac_str[] MEM_TYPE = "Triak";
  const unsigned char PUT_str[] MEM_TYPE = "PUT";
  const unsigned char Bye_str[] MEM_TYPE = "�au...";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "ok";
    const unsigned char Weak_str[] MEM_TYPE = "slab�!";
    const unsigned char Low_str[] MEM_TYPE =  "vybit�!";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "ext";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Menu test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "Gen. obd�ln�k�";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Zenerka a Volt";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "��ta� frekvence";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "LF krystal";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "HF krystal";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "��ta� ud�lost�";
    const unsigned char Count_str[] MEM_TYPE = "��ta�";
    const unsigned char Time_str[] MEM_TYPE = "�as";
    const unsigned char Events_str[] MEM_TYPE = "Ud�losti";
    const unsigned char Stop_str[] MEM_TYPE = "Stop";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "LC Meter";
    const unsigned char Adjusting_str[] MEM_TYPE = "adjusting...";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "IRC sn�ma�";
    const unsigned char TurnRight_str[] MEM_TYPE = "Oto� vpravo!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Kontrast";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "IR detektor";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "IR RC";
    const unsigned char IR_Send_str[] MEM_TYPE = "vys�l�m...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Optron";
    const unsigned char None_str[] MEM_TYPE = "��dn�";
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
    const unsigned char CapLeak_str[] MEM_TYPE = "Cap �nik";
    const unsigned char CapCharge_str[] MEM_TYPE = "Nab�jen�";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Vyb�jen�";
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
    const unsigned char TouchSetup_str[] MEM_TYPE = "Dotyk kalibrace";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Vypnout";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "OneWire Scan";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

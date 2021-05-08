/* ************************************************************************
 *
 *   language specific global variables: Czech (ISO 8859-2)
 *
 *   (c) 2015-2019 by Markus Reschke
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
   *  - stored in EEPROM
   */


  /* firmware */
  const unsigned char Tester_str[] EEMEM = "Ctester";


  /* common terms and texts */
  const unsigned char Probing_str[] EEMEM = "Prob�h� m��en�..";
  const unsigned char Timeout_str[] EEMEM = "Vypr�el �as";
  const unsigned char Failed1_str[] EEMEM = "?��dn� sou��tka";
  const unsigned char Failed2_str[] EEMEM = "nezn�m� - vadn�?";
  const unsigned char Done_str[] EEMEM = "hotovo!";
  const unsigned char Select_str[] EEMEM = "V�b�r";
  const unsigned char Selftest_str[] EEMEM = "Autotest";
  const unsigned char Adjustment_str[] EEMEM = "Kalibrace";
  const unsigned char Save_str[] EEMEM = "Ulo�it";
  const unsigned char Load_str[] EEMEM = "Na��st";
  const unsigned char Show_str[] EEMEM = "Zobraz hodnoty";
  const unsigned char Remove_str[] EEMEM = "Odstranit";
  const unsigned char Create_str[] EEMEM = "Ud�lej";
  const unsigned char ShortCircuit_str[] EEMEM = "Zkrat!";
  const unsigned char DischargeFailed_str[] EEMEM = "Baterie?";
  const unsigned char Error_str[] EEMEM = "Chyba!";
  const unsigned char Exit_str[] EEMEM = "Exit";
  const unsigned char Checksum_str[] EEMEM = "Kontroln� sou�et";
  const unsigned char BJT_str[] EEMEM = "Tranzistor";
  const unsigned char Thyristor_str[] EEMEM = "Tyristor";
  const unsigned char Triac_str[] EEMEM = "Triak";
  const unsigned char PUT_str[] EEMEM = "PUT";
  const unsigned char Bye_str[] EEMEM = "�au...";


  /* units */
  const unsigned char Hertz_str[] EEMEM = "Hz";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] EEMEM = "Bat";
    const unsigned char OK_str[] EEMEM = "ok";
    const unsigned char Weak_str[] EEMEM = "slab�!";
    const unsigned char Low_str[] EEMEM =  "vybit�!";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] EEMEM = "ext";
  #endif

  #ifdef UI_KEY_HINTS
  const unsigned char Menu_or_Test_str[] EEMEM = "<Menu test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] EEMEM = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] EEMEM = "Gen. obd�ln�k�";
  #endif

  #ifdef HW_ZENER
    const unsigned char Zener_str[] EEMEM = "Zenerka a Volt";
    const unsigned char Min_str[] EEMEM = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] EEMEM = "��ta� frekvence";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] EEMEM = "Ch";
    const unsigned char FreqInput_str[] EEMEM = "BNC";
    const unsigned char LF_Crystal_str[] EEMEM = "LF krystal";
    const unsigned char HF_Crystal_str[] EEMEM = "HF krystal";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] EEMEM = "��ta� ud�lost�";
    const unsigned char Count_str[] EEMEM = "��ta�";
    const unsigned char Time_str[] EEMEM = "�as";
    const unsigned char Events_str[] EEMEM = "Ud�losti";
    const unsigned char Stop_str[] EEMEM = "Stop";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] EEMEM = "IRC sn�ma�";
    const unsigned char TurnRight_str[] EEMEM = "Oto� vpravo!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] EEMEM = "Kontrast";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] EEMEM = "IR detektor";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] EEMEM = "IR RC";
    const unsigned char IR_Send_str[] EEMEM = "vys�l�m...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] EEMEM = "Optron";
    const unsigned char None_str[] EEMEM = "��dn�";
    const unsigned char CTR_str[] EEMEM = "CTR";
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (HW_EVENT_COUNTER)
    const unsigned char Start_str[] EEMEM = "Start";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] EEMEM = "UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] EEMEM = "Servo";
    const unsigned char Sweep_str[] EEMEM = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] EEMEM = "Cap �nik";
    const unsigned char CapCharge_str[] EEMEM = "Nab�jen�";
    const unsigned char CapHigh_str[] EEMEM = "Rl";
    const unsigned char CapLow_str[] EEMEM = "Rh";
    const unsigned char CapDischarge_str[] EEMEM = "Vyb�jen�";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] EEMEM = "R/L Monitor";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] EEMEM = "C Monitor";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] EEMEM = "Dotyk kalibrace";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] EEMEM = "Vypnout";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

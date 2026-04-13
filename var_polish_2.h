/* ************************************************************************
 *
 *   language specific global variables: Polish (ISO 8859-2)
 *
 *   (c) 2012-2024 by Markus Reschke
 *   translation by Jacon (zarnow@safe-mail.net)
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  Polish
 */

#if defined (UI_POLISH_2)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash

   */

  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Tester Elementów";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Testujê...";
  const unsigned char Timeout_str[] MEM_TYPE = "Czas min±³";
  const unsigned char Failed1_str[] MEM_TYPE = "Nie  wykryto";
  const unsigned char Failed2_str[] MEM_TYPE = "elementu!";
  const unsigned char Done_str[] MEM_TYPE = "Zrobione!";
  const unsigned char Select_str[] MEM_TYPE = "Wybierz";
  const unsigned char Adjustment_str[] MEM_TYPE = "Kompensacja";
  const unsigned char Save_str[] MEM_TYPE = "Zapisz";
  const unsigned char Load_str[] MEM_TYPE = "Wczytaj";
  const unsigned char Show_str[] MEM_TYPE = "Poka¿ parametry";
  const unsigned char Remove_str[] MEM_TYPE = "Usuñ";
  const unsigned char Create_str[] MEM_TYPE = "Utwórz";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "zwarcie sond!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Nieroz³adowane!";
  const unsigned char Error_str[] MEM_TYPE = "B³±d!";
  const unsigned char Exit_str[] MEM_TYPE = "Wyj¶cie";
  const unsigned char Checksum_str[] MEM_TYPE = "Suma kontrolna";
  const unsigned char BJT_str[] MEM_TYPE = "Tranzystor";
  const unsigned char Thyristor_str[] MEM_TYPE = "Tyrystor";
  const unsigned char Triac_str[] MEM_TYPE = "Triak";
  const unsigned char PUT_str[] MEM_TYPE = "Programow. UJT";
  const unsigned char Bye_str[] MEM_TYPE = "Do widzenia!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #ifdef SW_SELFTEST
    const unsigned char Selftest_str[] MEM_TYPE = "Autotest";
  #endif

  #if ! defined (BAT_NONE) && ! defined (UI_BATTERY)
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "ok";
    const unsigned char Weak_str[] MEM_TYPE = "s³aba!";
    const unsigned char Low_str[] MEM_TYPE = "pusta!!!";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "zewn.";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Menu Test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "Gen. prostok±ta";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Dioda Zenera";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Miernik czêst.";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Wej¶cie:";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "Kwarc LF";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "Kwarc HF";
  #endif

  #ifdef HW_RING_TESTER
    const unsigned char RingTester_str[] MEM_TYPE = "Test zwaræ w L";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Licznik zdarzeñ";
    const unsigned char Count_str[] MEM_TYPE = "Liczenie";
    const unsigned char Time_str[] MEM_TYPE = "Czas";
    const unsigned char Events_str[] MEM_TYPE = "Zdarzenia";
    const unsigned char Stop_str[] MEM_TYPE = "Stop";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "Miernik LC";
    const unsigned char Adjusting_str[] MEM_TYPE = "regulacja...";
  #endif

  #ifdef HW_LOGIC_PROBE
    const unsigned char LogicProbe_str[] MEM_TYPE = "Sonda logiczna";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Enkoder obrot.";
    const unsigned char TurnRight_str[] MEM_TYPE = "Obróæ w prawo!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Kontrast";
  #endif

  #if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "Dekoder IR";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "Koder IR";
    const unsigned char IR_Send_str[] MEM_TYPE = "Wysy³am...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Transoptor";
    const unsigned char None_str[] MEM_TYPE = "Brak";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #ifdef VAR_START_STR
    const unsigned char Start_str[] MEM_TYPE = "Start";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "Jednoz³±czowy";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Serwo";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Up³ywno¶æ C";
    const unsigned char CapCharge_str[] MEM_TYPE = "£adujê";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Roz³adowujê";
  #endif

  #ifdef SW_MONITOR_R
    const unsigned char Monitor_R_str[] MEM_TYPE = "Monitor R";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] MEM_TYPE = "Monitor C";
  #endif

  #ifdef SW_MONITOR_L
    const unsigned char Monitor_L_str[] MEM_TYPE = "Monitor L";
  #endif

  #ifdef SW_MONITOR_RCL
    const unsigned char Monitor_RCL_str[] MEM_TYPE = "Monitor RCL";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] MEM_TYPE = "Monitor RL";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] MEM_TYPE = "Ustawienia dotyku";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Wy³±cz";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "Skanuj 1-Wire";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

  #ifdef SW_CONTINUITY_CHECK
    const unsigned char ContinuityCheck_str[] MEM_TYPE = "Test przej¶cia";
  #endif

  #ifdef SW_FONT_TEST
    const unsigned char FontTest_str[] MEM_TYPE = "Font";
  #endif

  #ifdef SW_SYMBOL_TEST
    const unsigned char SymbolTest_str[] MEM_TYPE = "Symbole";
  #endif

  #ifdef HW_FLASHLIGHT
    const unsigned char Flashlight_str[] MEM_TYPE = "Latarka";
  #endif

  #ifdef SW_PHOTODIODE
    const unsigned char Photodiode_str[] MEM_TYPE = "Fotodioda";
    const unsigned char NoBias_str[] MEM_TYPE = "ftv";
    const unsigned char ReverseBias_str[] MEM_TYPE = "zap";
  #endif

  #ifdef SW_DIODE_LED
    const unsigned char Diode_LED_str[] MEM_TYPE = "Dioda LED";
  #endif

  #ifdef SW_METER_5VDC
    const unsigned char Meter_5VDC_str[] MEM_TYPE = "Miernik 5V";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

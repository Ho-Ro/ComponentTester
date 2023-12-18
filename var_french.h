/* ************************************************************************
 *
 *   language specific global variables: French (ISO 8859-1)
 *
 *   (c) 2016-2023 by Markus Reschke
 *   translation by moimem@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  French
 */

#if defined (UI_FRENCH)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Component Tester";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Analyze...";
  const unsigned char Timeout_str[] MEM_TYPE = "Temps ecoule";
  const unsigned char Failed1_str[] MEM_TYPE = "Aucun Composant";
  const unsigned char Failed2_str[] MEM_TYPE = "trouve!";
  const unsigned char Done_str[] MEM_TYPE = "Fini!";
  const unsigned char Select_str[] MEM_TYPE = "Choisir";
  const unsigned char Selftest_str[] MEM_TYPE = "Autotest";
  const unsigned char Adjustment_str[] MEM_TYPE = "Calibrage";
  const unsigned char Save_str[] MEM_TYPE = "Sauvegarder";
  const unsigned char Load_str[] MEM_TYPE = "Charger";
  const unsigned char Show_str[] MEM_TYPE = "Afficher";
  const unsigned char Remove_str[] MEM_TYPE = "Effacer";
  const unsigned char Create_str[] MEM_TYPE = "Creer";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "Court-Circuit!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Batterie?";
  const unsigned char Error_str[] MEM_TYPE = "Erreur!";
  const unsigned char Exit_str[] MEM_TYPE = "Quitter";
  const unsigned char Checksum_str[] MEM_TYPE = "somme de controle";
  const unsigned char BJT_str[] MEM_TYPE = "Transistor";
  const unsigned char Thyristor_str[] MEM_TYPE = "Thyristor";
  const unsigned char Triac_str[] MEM_TYPE = "Triac";
  const unsigned char PUT_str[] MEM_TYPE = "PUT";
  const unsigned char Bye_str[] MEM_TYPE = "Bye!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #if ! defined (BAT_NONE) && ! defined (UI_BATTERY)
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "ok";
    const unsigned char Weak_str[] MEM_TYPE = "faible";
    const unsigned char Low_str[] MEM_TYPE = "basse";
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
    const unsigned char SquareWave_str[] MEM_TYPE = "Onde Carree";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Zener";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Frequencemetre";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "Crystal LF";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "Crystal HF";
  #endif

  #ifdef HW_RING_TESTER
    const unsigned char RingTester_str[] MEM_TYPE = "Buzzer";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Compt. evenements";
    const unsigned char Count_str[] MEM_TYPE = "Compter";
    const unsigned char Time_str[] MEM_TYPE = "Temps";
    const unsigned char Events_str[] MEM_TYPE = "Evenements";
    const unsigned char Stop_str[] MEM_TYPE = "Arreter";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "LC-Metre";
    const unsigned char Adjusting_str[] MEM_TYPE = "ajustement...";
  #endif

  #ifdef HW_LOGIC_PROBE
    const unsigned char LogicProbe_str[] MEM_TYPE = "Sonde Logique";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Encodeur Rotatif";
    const unsigned char TurnRight_str[] MEM_TYPE = "tourner a droite";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Contraste";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "Recepteur IR";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "Transmetteur IR";
    const unsigned char IR_Send_str[] MEM_TYPE = "envoi...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Optocoupleur";
    const unsigned char None_str[] MEM_TYPE = "Aucun";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #ifdef VAR_START_STR
    const unsigned char Start_str[] MEM_TYPE = "Lancer";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Servo";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Fuite Condo.";
    const unsigned char CapCharge_str[] MEM_TYPE = "Charge";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Decharge";
  #endif

  #ifdef SW_MONITOR_R
    const unsigned char Monitor_R_str[] MEM_TYPE = "Moniteur R";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] MEM_TYPE = "Moniteur C";
  #endif

  #ifdef SW_MONITOR_L
    const unsigned char Monitor_L_str[] MEM_TYPE = "Moniteur L";
  #endif

  #ifdef SW_MONITOR_RCL
    const unsigned char Monitor_RCL_str[] MEM_TYPE = "Moniteur RCL";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] MEM_TYPE = "Moniteur RL";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] MEM_TYPE = "Config Tactile";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Off";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "OneWire Scan";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

  #ifdef SW_CONTINUITY_CHECK
    const unsigned char ContinuityCheck_str[] MEM_TYPE = "Continuite";
  #endif

  #ifdef SW_FONT_TEST
    const unsigned char FontTest_str[] MEM_TYPE = "Police";
  #endif

  #ifdef SW_SYMBOL_TEST
    const unsigned char SymbolTest_str[] MEM_TYPE = "Symboles";
  #endif

  #ifdef HW_FLASHLIGHT
    const unsigned char Flashlight_str[] MEM_TYPE = "Torche";
  #endif

  #ifdef SW_PHOTODIODE
    const unsigned char Photodiode_str[] MEM_TYPE = "Photodiode";
    const unsigned char NoBias_str[] MEM_TYPE = "non";
    const unsigned char ReverseBias_str[] MEM_TYPE = "rev";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

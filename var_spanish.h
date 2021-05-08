/* ************************************************************************
 *
 *   language specific global variables: Spanish (ISO 8859-1)
 *
 *   (c) 2016-2021 by Markus Reschke
 *   translation by pepe10000@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  Spanish
 */

#if defined (UI_SPANISH)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Tester De Piezas";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Testeando.";
  const unsigned char Timeout_str[] MEM_TYPE = "Tiempo agotado";
  const unsigned char Failed1_str[] MEM_TYPE = "*Componente no";
  const unsigned char Failed2_str[] MEM_TYPE = "encontrado*";
  const unsigned char Done_str[] MEM_TYPE = "*Hecho*";
  const unsigned char Select_str[] MEM_TYPE = "Seleccionar";
  const unsigned char Selftest_str[] MEM_TYPE = "Autotest";
  const unsigned char Adjustment_str[] MEM_TYPE = "Calibracion";
  const unsigned char Save_str[] MEM_TYPE = "Guardar";
  const unsigned char Load_str[] MEM_TYPE = "Cargar";
  const unsigned char Show_str[] MEM_TYPE = "Mostrar datos";
  const unsigned char Remove_str[] MEM_TYPE = "*Retirar";
  const unsigned char Create_str[] MEM_TYPE = "*Crear";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "cortocircuito*";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Bateria?";
  const unsigned char Error_str[] MEM_TYPE = "*Error*";
  const unsigned char Exit_str[] MEM_TYPE = "Salir";
  const unsigned char Checksum_str[] MEM_TYPE = "Suma de control";
  const unsigned char BJT_str[] MEM_TYPE = "Transistor";
  const unsigned char Thyristor_str[] MEM_TYPE = "Tiristor";
  const unsigned char Triac_str[] MEM_TYPE = "Triac";
  const unsigned char PUT_str[] MEM_TYPE = "Trans. PUT";
  const unsigned char Bye_str[] MEM_TYPE = "Adios!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "bien";
    const unsigned char Weak_str[] MEM_TYPE = "mal";
    const unsigned char Low_str[] MEM_TYPE = "baja";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "ext";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Menu O Test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "Onda cuadrada";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Zener+Voltaje";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Frecuencimetro";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "Cristal BF";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "Cristal AF";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Cont. Eventos";
    const unsigned char Count_str[] MEM_TYPE = "Contar";
    const unsigned char Time_str[] MEM_TYPE = "Tiempo";
    const unsigned char Events_str[] MEM_TYPE = "Eventos";
    const unsigned char Stop_str[] MEM_TYPE = "Parar";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "Medir LC";
    const unsigned char Adjusting_str[] MEM_TYPE = "ajustando...";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Encoder Rotat.";
    const unsigned char TurnRight_str[] MEM_TYPE = "Gira a derecha";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Contraste";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "Receptor IR";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "Emisor IR";
    const unsigned char IR_Send_str[] MEM_TYPE = "enviando...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Optoacoplador";
    const unsigned char None_str[] MEM_TYPE = "Ninguno";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (SW_ONEWIRE_SCAN) || defined (HW_EVENT_COUNTER)
    const unsigned char Start_str[] MEM_TYPE = "Iniciar";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "Trans. UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Servo";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Fugas condens.";
    const unsigned char CapCharge_str[] MEM_TYPE = "Cargando";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Descargando";
  #endif

  #ifdef SW_MONITOR_R
    const unsigned char Monitor_R_str[] MEM_TYPE = "Monitor Resis.";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] MEM_TYPE = "Monitor Capac.";
  #endif

  #ifdef SW_MONITOR_L
    const unsigned char Monitor_L_str[] MEM_TYPE = "Monitor Induc.";
  #endif

  #ifdef SW_MONITOR_RCL
    const unsigned char Monitor_RCL_str[] MEM_TYPE = "Monitor R/C/I";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] MEM_TYPE = "Monitor R/I";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] MEM_TYPE = "Config. tactil";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Apagar tester";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "Escan. OneWire";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

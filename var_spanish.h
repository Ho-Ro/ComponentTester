/* ************************************************************************
 *
 *   language specific global variables: Spanish (ISO 8859-1)
 *
 *   (c) 2016-2019 by Markus Reschke
 *   translation by pepe10000@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz K�bbeler
 *
 * ************************************************************************ */


/*
 *  Spanish
 */

#if defined (UI_SPANISH)

  /*
   *  constant strings
   *  - stored in EEPROM
   */


  /* firmware */
  const unsigned char Tester_str[] EEMEM = "Component Tester";


  /* common terms and texts */
  const unsigned char Probing_str[] EEMEM = "Testeando.";
  const unsigned char Timeout_str[] EEMEM = "Tiempo agotado";
  const unsigned char Failed1_str[] EEMEM = "*Componente no";
  const unsigned char Failed2_str[] EEMEM = "encontrado*";
  const unsigned char Done_str[] EEMEM = "*Hecho*";
  const unsigned char Select_str[] EEMEM = "Seleccionar";
  const unsigned char Selftest_str[] EEMEM = "Autotest";
  const unsigned char Adjustment_str[] EEMEM = "Calibracion";
  const unsigned char Save_str[] EEMEM = "Guardar";
  const unsigned char Load_str[] EEMEM = "Cargar";
  const unsigned char Show_str[] EEMEM = "Mostrar datos";
  const unsigned char Remove_str[] EEMEM = "*Retirar";
  const unsigned char Create_str[] EEMEM = "*Crear";
  const unsigned char ShortCircuit_str[] EEMEM = "cortocircuito*";
  const unsigned char DischargeFailed_str[] EEMEM = "Bateria?";
  const unsigned char Error_str[] EEMEM = "*Error*";
  const unsigned char Exit_str[] EEMEM = "Salir";
  const unsigned char Checksum_str[] EEMEM = "Suma de control";
  const unsigned char BJT_str[] EEMEM = "Transistor";
  const unsigned char Thyristor_str[] EEMEM = "Tiristor";
  const unsigned char Triac_str[] EEMEM = "Triac";
  const unsigned char PUT_str[] EEMEM = "Trans. PUT";
  const unsigned char Bye_str[] EEMEM = "Adios!";


  /* units */
  const unsigned char Hertz_str[] EEMEM = "Hz";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] EEMEM = "Bat";
    const unsigned char OK_str[] EEMEM = "bien";
    const unsigned char Weak_str[] EEMEM = "mal";
    const unsigned char Low_str[] EEMEM = "baja";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] EEMEM = "ext";
  #endif

  #ifdef UI_KEY_HINTS
  const unsigned char Menu_or_Test_str[] EEMEM = "<Menu Test>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] EEMEM = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] EEMEM = "Onda cuadrada";
  #endif

  #ifdef HW_ZENER
    const unsigned char Zener_str[] EEMEM = "Zener";
    const unsigned char Min_str[] EEMEM = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] EEMEM = "Frecuencimetro";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] EEMEM = "Ch";
    const unsigned char FreqInput_str[] EEMEM = "BNC";
    const unsigned char LF_Crystal_str[] EEMEM = "LF cristal";
    const unsigned char HF_Crystal_str[] EEMEM = "HF cristal";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] EEMEM = "Event Counter";
    const unsigned char Count_str[] EEMEM = "Count";
    const unsigned char Time_str[] EEMEM = "Time";
    const unsigned char Events_str[] EEMEM = "Events";
    const unsigned char Stop_str[] EEMEM = "Stop";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] EEMEM = "Encoder Rotat.";
    const unsigned char TurnRight_str[] EEMEM = "Gira a derecha";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] EEMEM = "Contraste";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] EEMEM = "Receptor IR";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] EEMEM = "Emisor IR";
    const unsigned char IR_Send_str[] EEMEM = "enviando...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] EEMEM = "Optoacoplador";
    const unsigned char None_str[] EEMEM = "Ninguno";
    const unsigned char CTR_str[] EEMEM = "CTR";
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (HW_EVENT_COUNTER)
    const unsigned char Start_str[] EEMEM = "Iniciar";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] EEMEM = "Trans. UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] EEMEM = "Servo";
    const unsigned char Sweep_str[] EEMEM = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] EEMEM = "Fugas condens.";
    const unsigned char CapCharge_str[] EEMEM = "Cargando";
    const unsigned char CapHigh_str[] EEMEM = "Rl";
    const unsigned char CapLow_str[] EEMEM = "Rh";
    const unsigned char CapDischarge_str[] EEMEM = "Descargando";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] EEMEM = "Config. tactil";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] EEMEM = "Apagar tester";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

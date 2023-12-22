/* ************************************************************************
 *
 *   language specific global variables: Brazilian Portuguese (ISO 8859-1)
 *
 *   (c) 2016-2023 by Markus Reschke
 *   translation by (°~°) Dr. Wando  --  pressaperfeicao@yahoo.com.br
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */



/*
 *  Português-Br
 */

#if defined (UI_BRAZILIAN)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Teste de Componentes";


  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Analisando...";
  const unsigned char Timeout_str[] MEM_TYPE = "Tempo esgotado";
  const unsigned char Failed1_str[] MEM_TYPE = "Componente: ausente,";
  const unsigned char Failed2_str[] MEM_TYPE = "ou danificado!";
  const unsigned char Done_str[] MEM_TYPE = "Concluido!";
  const unsigned char Select_str[] MEM_TYPE = "Selecione";
  const unsigned char Selftest_str[] MEM_TYPE = "AutoTeste";
  const unsigned char Adjustment_str[] MEM_TYPE = "AutoAjuste";
  const unsigned char Save_str[] MEM_TYPE = "Gravar";
  const unsigned char Load_str[] MEM_TYPE = "Carregar";
  const unsigned char Show_str[] MEM_TYPE = "Informacoes";
  const unsigned char Remove_str[] MEM_TYPE = "Remover";
  const unsigned char Create_str[] MEM_TYPE = "Criar";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "Curto circuito!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Bateria?";
  const unsigned char Error_str[] MEM_TYPE = "Erro!";
  const unsigned char Exit_str[] MEM_TYPE = "Sair";
  const unsigned char Checksum_str[] MEM_TYPE = "Controle de soma";
  const unsigned char BJT_str[] MEM_TYPE = "Transistor";
  const unsigned char Thyristor_str[] MEM_TYPE = "Tiristor";
  const unsigned char Triac_str[] MEM_TYPE = "Triac";
  const unsigned char PUT_str[] MEM_TYPE = "Transistor PUT";
  const unsigned char Bye_str[] MEM_TYPE = "Ate, logo!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Hz";


  /* options */
  #if ! defined (BAT_NONE) && ! defined (UI_BATTERY)
    const unsigned char Battery_str[] MEM_TYPE = "Bat";
    const unsigned char OK_str[] MEM_TYPE = "OK";
    const unsigned char Weak_str[] MEM_TYPE = "fraca";
    const unsigned char Low_str[] MEM_TYPE = "baixa";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "ext";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Menu ou Teste>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "PWM";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "F-Gerador";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Voltimetro";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Frequencimetro";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "Cristal LF";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "Cristal HF";
  #endif

  #ifdef HW_RING_TESTER
    const unsigned char RingTester_str[] MEM_TYPE = "Teste de anel";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Contar eventos";
    const unsigned char Count_str[] MEM_TYPE = "Contar";
    const unsigned char Time_str[] MEM_TYPE = "Tempo";
    const unsigned char Events_str[] MEM_TYPE = "Eventos";
    const unsigned char Stop_str[] MEM_TYPE = "Parar";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "Medir LC";
    const unsigned char Adjusting_str[] MEM_TYPE = "ajustando...";
  #endif

  #ifdef HW_LOGIC_PROBE
    const unsigned char LogicProbe_str[] MEM_TYPE = "Sonda logica";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Encoder rotativo";
    const unsigned char TurnRight_str[] MEM_TYPE = "Gire a direita!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Contraste";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "Receptor IR";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "Emisssor IR";
    const unsigned char IR_Send_str[] MEM_TYPE = "enviando...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Optoacoplador";
    const unsigned char None_str[] MEM_TYPE = "Nenhum";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #ifdef VAR_START_STR
    const unsigned char Start_str[] MEM_TYPE = "Iniciar";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "Transistor UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Servo";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Fulga capacitor";
    const unsigned char CapCharge_str[] MEM_TYPE = "Carregando";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Descarregando";
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
    const unsigned char TouchSetup_str[] MEM_TYPE = "Config. toque";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Desligar";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "Escanear OneWire";
    const unsigned char Bus_str[] MEM_TYPE = "Bus";
  #endif

  #ifdef SW_CONTINUITY_CHECK
    const unsigned char ContinuityCheck_str[] MEM_TYPE = "Continuidade";
  #endif

  #ifdef SW_FONT_TEST
    const unsigned char FontTest_str[] MEM_TYPE = "Fonte";
  #endif

  #ifdef SW_SYMBOL_TEST
    const unsigned char SymbolTest_str[] MEM_TYPE = "Simbolos";
  #endif

  #ifdef HW_FLASHLIGHT
    const unsigned char Flashlight_str[] MEM_TYPE = "Lanterna";
  #endif

  #ifdef SW_PHOTODIODE
    const unsigned char Photodiode_str[] MEM_TYPE = "Fotodiodo";
    const unsigned char NoBias_str[] MEM_TYPE = "nao";
    const unsigned char ReverseBias_str[] MEM_TYPE = "inv";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

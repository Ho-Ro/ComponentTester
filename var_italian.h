/* ************************************************************************
 *
 *   language specific global variables: Italian (ISO 8859-1)
 *
 *   (c) 2016-2017 by Markus Reschke
 *   translation by Gino_09@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  Italian
 */

#if defined (UI_ITALIAN)

  /*
   *  constant strings (stored in EEPROM)
   */

  const unsigned char Running_str[] EEMEM = "Analizzo...";
  const unsigned char Weak_str[] EEMEM = "debole";
  const unsigned char Low_str[] EEMEM = "basso";
  const unsigned char Timeout_str[] EEMEM = "Fuori Tempo";
  const unsigned char Failed1_str[] EEMEM = "Componente non";
  const unsigned char Failed2_str[] EEMEM = "trovato!";
  const unsigned char Done_str[] EEMEM = "Fatto!";
  const unsigned char Select_str[] EEMEM = "Selezionare";
  const unsigned char Selftest_str[] EEMEM = "Autotest";
  const unsigned char Adjustment_str[] EEMEM = "Calibrazione";
  const unsigned char Save_str[] EEMEM = "Salvare";
  const unsigned char Load_str[] EEMEM = "Carica";
  const unsigned char Show_str[] EEMEM = "Mostra dati";
  const unsigned char Remove_str[] EEMEM = "Rimuovere";
  const unsigned char Create_str[] EEMEM = "Creare";
  const unsigned char ShortCircuit_str[] EEMEM = "Cortocircuito!";
  const unsigned char DischargeFailed_str[] EEMEM = "Batteria?";
  const unsigned char Error_str[] EEMEM = "Errore!";
  const unsigned char Exit_str[] EEMEM = "Esci";
  const unsigned char Checksum_str[] EEMEM = "Verifica Somma";
  const unsigned char BJT_str[] EEMEM = "Transistor";
  const unsigned char Thyristor_str[] EEMEM = "Tiristore";
  const unsigned char Triac_str[] EEMEM = "Triac";
  const unsigned char PUT_str[] EEMEM = "Trans. PUT";
  const unsigned char Bye_str[] EEMEM = "Ciao!";

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] EEMEM = "Onda Quadra";
  #endif

  #ifdef HW_ZENER
    const unsigned char Zener_str[] EEMEM = "Zener";
    const unsigned char Min_str[] EEMEM = "Min";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] EEMEM = "Frequenzimetro";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] EEMEM = "Ch";
    const unsigned char FreqInput_str[] EEMEM = "BNC";
    const unsigned char LF_Crystal_str[] EEMEM = "LF crystal";
    const unsigned char HF_Crystal_str[] EEMEM = "HF crystal";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] EEMEM = "Encoder Rotativo";
    const unsigned char TurnRight_str[] EEMEM = "Gira a destra";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] EEMEM = "Contrasto";
  #endif

  #if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] EEMEM = "Rivelatore IR";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] EEMEM = "Accoppiatore opt";
    const unsigned char Start_str[] EEMEM = "Avvio";
    const unsigned char None_str[] EEMEM = "Nessuno";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] EEMEM = "Trans. UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] EEMEM = "Servo";
    const unsigned char Sweep_str[] EEMEM = "<->";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] EEMEM = "Touch Setup";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   language specific global variables: Russian (Windows-1251)
 *
 *   (c) 2017 by Markus Reschke
 *   translation by indman@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz Kьbbeler
 *
 * ************************************************************************ */


/*
 *  Russian
 */

#if defined (UI_RUSSIAN)

  /*
   *  constant strings (stored in EEPROM)
   */

  const unsigned char Running_str[] EEMEM = "Тестирование...";
  const unsigned char Weak_str[] EEMEM = "слабая";
  const unsigned char Low_str[] EEMEM = "замена";
  const unsigned char Timeout_str[] EEMEM = "Тайм-аут";
  const unsigned char Failed1_str[] EEMEM = "Компонент не";
  const unsigned char Failed2_str[] EEMEM = "найден!";
  const unsigned char Done_str[] EEMEM = "Готово!";
  const unsigned char Select_str[] EEMEM = "Выберите";
  const unsigned char Selftest_str[] EEMEM = "Самотест";
  const unsigned char Adjustment_str[] EEMEM = "Корректировка";
  const unsigned char Save_str[] EEMEM = "Сохранить";
  const unsigned char Load_str[] EEMEM = "Загрузить";
  const unsigned char Show_str[] EEMEM = "Данные";
  const unsigned char Remove_str[] EEMEM = "Раскоротите";
  const unsigned char Create_str[] EEMEM = "Закоротите";
  const unsigned char ShortCircuit_str[] EEMEM = "тест.контакты!";
  const unsigned char DischargeFailed_str[] EEMEM = "Батарея?";
  const unsigned char Error_str[] EEMEM = "Ошибка!";
  const unsigned char Exit_str[] EEMEM = "Выход";
  const unsigned char Checksum_str[] EEMEM = "Контр.сумма";
  const unsigned char BJT_str[] EEMEM = "Транз.(BJT)";
  const unsigned char Thyristor_str[] EEMEM = "Тиристор";
  const unsigned char Triac_str[] EEMEM = "Симистор";
  const unsigned char PUT_str[] EEMEM = "Транз.(PUT)";
  const unsigned char Bye_str[] EEMEM = "До свидания!";

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] EEMEM = "Генератор";
  #endif

  #ifdef HW_ZENER
    const unsigned char Zener_str[] EEMEM = "Напряжение";
    const unsigned char Min_str[] EEMEM = "Мин.";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] EEMEM = "Частотомер";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] EEMEM = "Ch";
    const unsigned char FreqInput_str[] EEMEM = "BNC";
    const unsigned char LF_Crystal_str[] EEMEM = "LF crystal";
    const unsigned char HF_Crystal_str[] EEMEM = "HF crystal";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] EEMEM = "Энкодер";
    const unsigned char TurnRight_str[] EEMEM = "Направо!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] EEMEM = "Контраст";
  #endif

  #if defined(SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] EEMEM = "ИК-приёмник";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] EEMEM = "Oптрон";
    const unsigned char Start_str[] EEMEM = "Старт";
    const unsigned char None_str[] EEMEM = "не найден";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] EEMEM = "Транз.(UJT)";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] EEMEM = "Сервопривод";
    const unsigned char Sweep_str[] EEMEM = "<->";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] EEMEM = "Touch Setup";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

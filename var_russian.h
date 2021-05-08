/* ************************************************************************
 *
 *   language specific global variables: Russian (Windows-1251)
 *
 *   (c) 2017-2021 by Markus Reschke
 *   translation by indman@EEVblog
 *   based on code from Markus Frejek and Karl-Heinz Kьbbeler
 *
 * ************************************************************************ */


/*
 *  Russian
 */

#if defined (UI_RUSSIAN)

  /*
   *  constant strings
   *  - stored in EEPROM/Flash
   */


  /* firmware */
  const unsigned char Tester_str[] MEM_TYPE = "Мультитестер";
  

  /* common terms and texts */
  const unsigned char Probing_str[] MEM_TYPE = "Тестирование...";
  const unsigned char Timeout_str[] MEM_TYPE = "Время истекло";
  const unsigned char Failed1_str[] MEM_TYPE = "Детали нет";
  const unsigned char Failed2_str[] MEM_TYPE = "или повреждена?";
  const unsigned char Done_str[] MEM_TYPE = "Готово!";
  const unsigned char Select_str[] MEM_TYPE = "Режим";
  const unsigned char Selftest_str[] MEM_TYPE = "Самотест";
  const unsigned char Adjustment_str[] MEM_TYPE = "Корректировка";
  const unsigned char Save_str[] MEM_TYPE = "Сохранить";
  const unsigned char Load_str[] MEM_TYPE = "Загрузить";
  const unsigned char Show_str[] MEM_TYPE = "Данные";
  const unsigned char Remove_str[] MEM_TYPE = "Раскоротите!";
  const unsigned char Create_str[] MEM_TYPE = "Закоротите";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "все т.контакты!";
  const unsigned char DischargeFailed_str[] MEM_TYPE = "Батарея?";
  const unsigned char Error_str[] MEM_TYPE = "Ошибка!";
  const unsigned char Exit_str[] MEM_TYPE = "Выход";
  const unsigned char Checksum_str[] MEM_TYPE = "Контр.сумма";
  const unsigned char BJT_str[] MEM_TYPE = "BJT";
  const unsigned char Thyristor_str[] MEM_TYPE = "Тиристор";
  const unsigned char Triac_str[] MEM_TYPE = "Симистор";
  const unsigned char PUT_str[] MEM_TYPE = "PUT";
  const unsigned char Bye_str[] MEM_TYPE = "До свидания!";


  /* units */
  const unsigned char Hertz_str[] MEM_TYPE = "Гц";


  /* options */
  #ifndef BAT_NONE
    const unsigned char Battery_str[] MEM_TYPE = "Бат";
    const unsigned char OK_str[] MEM_TYPE = "OK";
    const unsigned char Weak_str[] MEM_TYPE = "слабая";
    const unsigned char Low_str[] MEM_TYPE = "замена";
  #endif

  #ifdef BAT_EXT_UNMONITORED
    const unsigned char External_str[] MEM_TYPE = "внешняя";
  #endif

  #ifdef UI_KEY_HINTS
    const unsigned char Menu_or_Test_str[] MEM_TYPE = "<Меню Тест>";
  #endif

  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
    const unsigned char PWM_str[] MEM_TYPE = "10-бит ШИМ";
  #endif

  #ifdef SW_SQUAREWAVE
    const unsigned char SquareWave_str[] MEM_TYPE = "Генератор";
  #endif

  #if defined (HW_ZENER) || defined (HW_PROBE_ZENER)
    const unsigned char Zener_str[] MEM_TYPE = "Вольтметр";
  #endif

  #if defined (HW_ZENER) && ! defined (ZENER_UNSWITCHED)
    const unsigned char Min_str[] MEM_TYPE = "Мин.";
  #endif

  #ifdef HW_FREQ_COUNTER
    const unsigned char FreqCounter_str[] MEM_TYPE = "Частотомер";
  #endif

  #ifdef HW_FREQ_COUNTER_EXT
    const unsigned char CounterChannel_str[] MEM_TYPE = "Ch";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "LF кварц";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "HF кварц";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Счетчик";
    const unsigned char Count_str[] MEM_TYPE = "Подсчет";
    const unsigned char Time_str[] MEM_TYPE = "Время";
    const unsigned char Events_str[] MEM_TYPE = "События";
    const unsigned char Stop_str[] MEM_TYPE = "Стоп";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "LC Meter";
    const unsigned char Adjusting_str[] MEM_TYPE = "настройка...";
  #endif

  #ifdef SW_ENCODER
    const unsigned char Encoder_str[] MEM_TYPE = "Энкодер";
    const unsigned char TurnRight_str[] MEM_TYPE = "Направо!";
  #endif

  #ifdef SW_CONTRAST
    const unsigned char Contrast_str[] MEM_TYPE = "Контраст";
  #endif

  #if defined (SW_IR_RECEIVER) || defined (HW_IR_RECEIVER)
    const unsigned char IR_Detector_str[] MEM_TYPE = "ИК приёмник";
  #endif

  #ifdef SW_IR_TRANSMITTER
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "IR RC";
    const unsigned char IR_Send_str[] MEM_TYPE = "Код.посылка...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Oптрон";
    const unsigned char None_str[] MEM_TYPE = "не найден!";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #if defined (SW_OPTO_COUPLER) || defined (SW_DS18B20) || defined (SW_ONEWIRE_SCAN) || defined (HW_EVENT_COUNTER)
    const unsigned char Start_str[] MEM_TYPE = "Старт";
  #endif

  #ifdef SW_UJT
    const unsigned char UJT_str[] MEM_TYPE = "UJT";
  #endif

  #ifdef SW_SERVO
    const unsigned char Servo_str[] MEM_TYPE = "Сервопривод";
    const unsigned char Sweep_str[] MEM_TYPE = "<->";
  #endif

  #ifdef SW_CAP_LEAKAGE
    const unsigned char CapLeak_str[] MEM_TYPE = "Ток утечки С";
    const unsigned char CapCharge_str[] MEM_TYPE = "Заряд";
    const unsigned char CapHigh_str[] MEM_TYPE = "Rl";
    const unsigned char CapLow_str[] MEM_TYPE = "Rh";
    const unsigned char CapDischarge_str[] MEM_TYPE = "Разряд";
  #endif

  #ifdef SW_MONITOR_R
    const unsigned char Monitor_R_str[] MEM_TYPE = "R монитор";
  #endif

  #ifdef SW_MONITOR_C
    const unsigned char Monitor_C_str[] MEM_TYPE = "C монитор";
  #endif

  #ifdef SW_MONITOR_L
    const unsigned char Monitor_L_str[] MEM_TYPE = "L монитор";
  #endif

  #ifdef SW_MONITOR_RCL
    const unsigned char Monitor_RCL_str[] MEM_TYPE = "RCL монитор";
  #endif

  #ifdef SW_MONITOR_RL
    const unsigned char Monitor_RL_str[] MEM_TYPE = "RL монитор";
  #endif

  #ifdef HW_TOUCH
    const unsigned char TouchSetup_str[] MEM_TYPE = "Настр.сенсора";
  #endif

  #ifdef SW_POWER_OFF
    const unsigned char PowerOff_str[] MEM_TYPE = "Выключить";
  #endif

  #ifdef SW_ONEWIRE_SCAN
    const unsigned char OneWire_Scan_str[] MEM_TYPE = "1-Wire детект";
    const unsigned char Bus_str[] MEM_TYPE = "Шина";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

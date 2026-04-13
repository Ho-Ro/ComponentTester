/* ************************************************************************
 *
 *   language specific global variables: Russian (Windows-1251)
 *
 *   (c) 2017-2024 by Markus Reschke
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
  const unsigned char Timeout_str[] MEM_TYPE = "Время истекло!";
  const unsigned char Failed1_str[] MEM_TYPE = "Детали нет";
  const unsigned char Failed2_str[] MEM_TYPE = "или повреждена";
  const unsigned char Done_str[] MEM_TYPE = "Готово!";
  const unsigned char Select_str[] MEM_TYPE = "Режим";
  const unsigned char Adjustment_str[] MEM_TYPE = "Корректировка";
  const unsigned char Save_str[] MEM_TYPE = "Сохранить";
  const unsigned char Load_str[] MEM_TYPE = "Загрузить";
  const unsigned char Show_str[] MEM_TYPE = "Данные";
  const unsigned char Remove_str[] MEM_TYPE = "Раскоротите";
  const unsigned char Create_str[] MEM_TYPE = "Закоротите";
  const unsigned char ShortCircuit_str[] MEM_TYPE = "контакты 1-2-3!";
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
  #ifdef SW_SELFTEST
    const unsigned char Selftest_str[] MEM_TYPE = "Самотест";
  #endif

  #if ! defined (BAT_NONE) && ! defined (UI_BATTERY)
    const unsigned char Battery_str[] MEM_TYPE = "Бат";
    const unsigned char OK_str[] MEM_TYPE = "OK";
    const unsigned char Weak_str[] MEM_TYPE = "слабая";
    const unsigned char Low_str[] MEM_TYPE = "разряд";
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
    const unsigned char CounterChannel_str[] MEM_TYPE = "Вх.канал:";
    const unsigned char FreqInput_str[] MEM_TYPE = "BNC";
    const unsigned char LF_Crystal_str[] MEM_TYPE = "НЧ кварц";
    const unsigned char HF_Crystal_str[] MEM_TYPE = "ВЧ кварц";
  #endif

  #ifdef HW_RING_TESTER
    const unsigned char RingTester_str[] MEM_TYPE = "Тест LOPT/FBT";
  #endif

  #ifdef HW_EVENT_COUNTER
    const unsigned char EventCounter_str[] MEM_TYPE = "Счетчик";
    const unsigned char Count_str[] MEM_TYPE = "Подсчет";
    const unsigned char Time_str[] MEM_TYPE = "Время";
    const unsigned char Events_str[] MEM_TYPE = "События";
    const unsigned char Stop_str[] MEM_TYPE = "Стоп";
  #endif

  #ifdef HW_LC_METER
    const unsigned char LC_Meter_str[] MEM_TYPE = "LC-метр";
    const unsigned char Adjusting_str[] MEM_TYPE = "настройка...";
  #endif

  #ifdef HW_LOGIC_PROBE
    const unsigned char LogicProbe_str[] MEM_TYPE = "Лог.пробник";
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
    const unsigned char IR_Transmitter_str[] MEM_TYPE = "ИК передатчик";
    const unsigned char IR_Send_str[] MEM_TYPE = "Код.посылка...";
  #endif

  #ifdef SW_OPTO_COUPLER
    const unsigned char OptoCoupler_str[] MEM_TYPE = "Oптрон";
    const unsigned char None_str[] MEM_TYPE = "не найден!";
    const unsigned char CTR_str[] MEM_TYPE = "CTR";
  #endif

  #ifdef VAR_START_STR
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

  #ifdef SW_CONTINUITY_CHECK
    const unsigned char ContinuityCheck_str[] MEM_TYPE = "Звук.прозвонка";
  #endif

  #ifdef SW_FONT_TEST
    const unsigned char FontTest_str[] MEM_TYPE = "Шрифт";
  #endif

  #ifdef SW_SYMBOL_TEST
    const unsigned char SymbolTest_str[] MEM_TYPE = "Символы";
  #endif

  #ifdef HW_FLASHLIGHT
    const unsigned char Flashlight_str[] MEM_TYPE = "Фонарик";
  #endif

  #ifdef SW_PHOTODIODE
    const unsigned char Photodiode_str[] MEM_TYPE = "Фотодиод";
    const unsigned char NoBias_str[] MEM_TYPE = "Ip_no";
    const unsigned char ReverseBias_str[] MEM_TYPE = "Ip_rev";
  #endif

  #ifdef SW_DIODE_LED
    const unsigned char Diode_LED_str[] MEM_TYPE = "Диод/LED";
  #endif

  #ifdef SW_METER_5VDC
    const unsigned char Meter_5VDC_str[] MEM_TYPE = "5V-вольтметр";
  #endif

#endif


/* ************************************************************************
 *   EOF
 * ************************************************************************ */

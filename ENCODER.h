class ENCODER {
  public:
    ENCODER(uint8_t pin1, uint8_t pin2) 
      : _pin1(pin1), _pin2(pin2), _position(0), _stepCounter(0), _lastState(0), _lastDebounceTime(0), _debounceDelay(5) {}

    void begin() {
      pinMode(_pin1, INPUT_PULLUP);
      pinMode(_pin2, INPUT_PULLUP);
      // Сохраняем начальное состояние
      _lastState = (digitalRead(_pin1) << 1) | digitalRead(_pin2);
    }

    void update() {
      // Чтение текущих состояний пинов
      uint8_t currentState = (digitalRead(_pin1) << 1) | digitalRead(_pin2);

      // Проверка на изменение состояния (и фильтрация дребезга)
      if (currentState != _lastState && (millis() - _lastDebounceTime) > _debounceDelay) {
        _lastDebounceTime = millis();  // Сбрасываем время дребезга

        // Определяем направление вращения
        if ((_lastState == 0b00 && currentState == 0b01) ||
            (_lastState == 0b01 && currentState == 0b11) ||
            (_lastState == 0b11 && currentState == 0b10) ||
            (_lastState == 0b10 && currentState == 0b00)) {
          _stepCounter++;
        } 
        else if ((_lastState == 0b00 && currentState == 0b10) ||
                (_lastState == 0b10 && currentState == 0b11) ||
                (_lastState == 0b11 && currentState == 0b01) ||
                (_lastState == 0b01 && currentState == 0b00)) {
          _stepCounter--;
        }

        _lastState = currentState;

        // Обновляем позицию после полного цикла (4 шага)
        if (_stepCounter >= 4) {
            _position++;
            _stepCounter = 0;
        } else if (_stepCounter <= -4) {
            _position--;
            _stepCounter = 0;
        }
      }
    }

    int8_t getPosition() const {
      return _position;
    }

  private:
    uint8_t _pin1, _pin2;
    int8_t _position;
    int8_t _stepCounter;                 // Счётчик шагов внутри полного цикла
    uint8_t _lastState;
    unsigned long _lastDebounceTime;     // Последнее время изменения состояния
    const unsigned long _debounceDelay;  // Задержка для фильтрации дребезга
};

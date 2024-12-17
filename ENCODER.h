class Encoder {
  public:
    Encoder(uint8_t pin1, uint8_t pin2, int8_t minPosition, int8_t maxPosition) 
      : _pin1(pin1), _pin2(pin2), _minPosition(minPosition), _maxPosition(maxPosition), _position(0), _stepCounter(0),
        _lastState(0), _lastDebounceTime(0), _debounceDelay(5) {}

    void begin() {
      pinMode(_pin1, INPUT_PULLUP);
      pinMode(_pin2, INPUT_PULLUP);
      // Save the initial state
      _lastState = (digitalRead(_pin1) << 1) | digitalRead(_pin2);
    }

    void update() {
      // Reading current pin states
      uint8_t currentState = (digitalRead(_pin1) << 1) | digitalRead(_pin2);

      // Checking for state change (and debounce filtering)
      if (currentState != _lastState && (millis() - _lastDebounceTime) > _debounceDelay) {
        _lastDebounceTime = millis();  // Resetting the bounce time

        // Determine the direction of rotation
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

        if (_stepCounter >= 4) {
          if (_position < 2){
            _position++;
          }
          _stepCounter = 0;
        }
        else if (_stepCounter <= -4) {
          if (0 < _position){
            _position--;
          }
          _stepCounter = 0;
        }
      }
    }

    int8_t getPosition() const {
      return _position;
    }

  private:
    uint8_t _pin1, _pin2;
    int8_t _minPosition, _maxPosition;
    int8_t _position;
    int8_t _stepCounter;
    uint8_t _lastState;
    unsigned long _lastDebounceTime;     // Last time state changed
    const unsigned long _debounceDelay;  // Delay for filtering chatter
};

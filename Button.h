class Button {
  public:
    Button(uint8_t pin) 
      : _pin(pin), _lastButtonState(HIGH), _buttonState(0), _lastDebounceTime(0), _debounceDelay(50) {}

    void begin() {
      pinMode(_pin, INPUT_PULLUP);
    }    

    bool isPressed() {
      int reading = digitalRead(_pin);

      if (reading != _lastButtonState) {
        _lastDebounceTime = millis();
      }

      if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (reading != _buttonState) {
          _buttonState = reading;

          if (_buttonState == LOW) {
              Serial.println("Button pressed");
              return true;
          }
        }
      }

      _lastButtonState = reading;

      return false;
    }

  private:
    uint8_t _pin;    
    int8_t _lastButtonState;
    int8_t _buttonState;
    unsigned long _lastDebounceTime;
    const unsigned long _debounceDelay;
};

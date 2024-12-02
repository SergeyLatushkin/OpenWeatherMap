class Weather {
  public:
    Weather()
      : _temp(0.0), _feelsLike(0.0), _isDataUpdated(false), _windSpeed(0.0) {
        _description = nullptr;
        _icon = nullptr;
    }
    
    void updateData() {
      HTTPClient http;

      _isDataUpdated = false;

      http.begin("http://api.openweathermap.org/data/2.5/weather?q=Warsaw&appid=c5a08acba9e958144a4a95e10c8016c0&units=metric&lang=en");
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
          _temp = doc["main"]["temp"].as<float>();
          _feelsLike = doc["main"]["feels_like"].as<float>();

          const char* description = doc["weather"][0]["description"];
          if (description) {
            if (_description) {
              delete[] _description;
            }
            _description = createString(description);
          }

          if(!doc["weather"][0]["icon"].isNull()){
            if (_icon) {
              delete[] _icon;
            }
            _icon = createString(doc["weather"][0]["icon"]);
          }

          _windSpeed = doc["wind"]["speed"].as<float>();
          _isDataUpdated = true;

          Serial.println("CurrentWeather data is available");
        }
        else {
          Serial.print("JSON Deserialization Error: ");
          Serial.println(error.c_str());

          _isDataUpdated = false;
        }    
      }
      else {
        Serial.println("Connection failed. fetchWeather()");
        _isDataUpdated = false;
      }

      http.end();
    }

    uint8_t* fetchWeatherIcon(size_t& size) {
      HTTPClient http;
      uint8_t* buff = nullptr; // Указатель на буфер
      size = 0;

      char iconURL[100];
      snprintf(iconURL, sizeof(iconURL), "http://openweathermap.org/img/wn/%s@2x.png", _icon);

      http.begin(iconURL);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        // Получаем размер данных
        int len = http.getSize();
        if (len <= 0) {
            Serial.println("Ошибка: размер данных неизвестен или равен 0");
            http.end();

            return nullptr;
        }

        // Выделяем память под буфер
        buff = (uint8_t*)malloc(len);
        if (!buff) {
            Serial.println("Ошибка: недостаточно памяти");
            http.end();

            return nullptr;
        }

        // Читаем данные из потока
        WiFiClient* stream = http.getStreamPtr();
        size_t totalBytesRead = 0;

        while (http.connected() && totalBytesRead < len) {
          size_t availableSize = stream->available();
          if (availableSize > 0) {
            // Читаем доступные данные
            size_t bytesToRead = min(availableSize, len - totalBytesRead);
            size_t bytesRead = stream->readBytes(buff + totalBytesRead, bytesToRead);
            totalBytesRead += bytesRead;

            // Проверяем, что данные полностью считаны
            if (bytesRead == 0){
                break;
            }
          }
          delay(1); // Небольшая задержка для освобождения ресурсов
        }

        // Проверяем, успешно ли считаны все данные
        if (totalBytesRead != len) {
          Serial.println("Ошибка: данные считаны не полностью");
          free(buff);
          buff = nullptr;
        }
        else {
          size = totalBytesRead;
        }
      } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();

      return buff;
    }

    bool isDataUpdated() const {
        return _isDataUpdated;
    }

    float temp() const {
        return _temp;
    }

    const char* description() const {
        return _description;
    }

    float feelsLike() const {
        return _feelsLike;
    }

    const char* icon() const {
        return _icon;
    }

    float windSpeed() const {
        return _windSpeed;
    }
 
  private:
    char* createString(const char* input) {
      size_t length = strlen(input);
      char* str = (char*)malloc(length + 1);
      if (str == NULL) {
        Serial.println("Creating error: out of memory");
        exit(1);
      }
      strcpy(str, input);

      return str;
    }

    float _windSpeed;
    float _temp;
    char* _description;
    float _feelsLike;
    char* _icon;
    bool _isDataUpdated;
};
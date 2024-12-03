struct CurrentWeather{
  float temp;
  char* description;
  float feelsLike;
  char* iconCode;
  bool isDataUpdated;
  uint8_t* icon;
  size_t iconSize;
    
  float windSpeed;
  float deg;
  float gust;
};

class Weather {
  public:
    Weather(){ }

    void updateData(CurrentWeather* data) {
      HTTPClient http;
      char* code = nullptr;

      data->isDataUpdated = false;

      http.begin("http://api.openweathermap.org/data/2.5/weather?q=Warsaw&appid=c5a08acba9e958144a4a95e10c8016c0&units=metric&lang=en");
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
          data->temp = doc["main"]["temp"].as<float>();
          data->feelsLike = doc["main"]["feels_like"].as<float>();
          data->windSpeed = doc["wind"]["speed"].as<float>();
          data->deg = doc["wind"]["deg"].as<float>();
          data->gust = doc["wind"]["gust"].as<float>();

          const char* description = doc["weather"][0]["description"];
          if (description) {
            if (data->description) {
              delete[] data->description;
            }
            data->description = createString(description);
          }

          const char* iconCode = doc["weather"][0]["icon"];
          code = createString(iconCode);
        }
        else {
          Serial.print("JSON Deserialization Error: ");
          Serial.println(error.c_str());

          data->isDataUpdated = false;
        }    
      }
      else {
        Serial.println("Connection failed. fetchWeather()");
        data->isDataUpdated = false;
      }

      http.end();

      size_t size = 0;
      uint8_t* icon = fetchWeatherIcon(code, size);

      free(code);

      if (icon) {
        if (data->icon) {
          delete[] data->icon;
        }
        data->icon = icon;
        data->iconSize = size;
      }

      if(size == 0){
        Serial.println("Current weather data is updated partaly");
      }

      data->isDataUpdated = true;
    }    

    uint8_t* fetchWeatherIcon(char* icon, size_t& size) {
      HTTPClient http;

      uint8_t* buff = nullptr;

      char iconURL[100];
      snprintf(iconURL, sizeof(iconURL), "http://openweathermap.org/img/wn/%s@2x.png", icon);
      Serial.println(iconURL);

      http.begin(iconURL);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        int len = http.getSize();
        if (len <= 0) {
            Serial.println("Error: Data size is unknown or 0");
            http.end();

            return nullptr;
        }

        // Выделяем память под буфер
        buff = (uint8_t*)malloc(len);
        if (!buff) {
            Serial.println("Error: Not enough memory");
            http.end();

            return nullptr;
        }

        WiFiClient* stream = http.getStreamPtr();
        size_t totalBytesRead = 0;

        while (http.connected() && totalBytesRead < len) {
          size_t availableSize = stream->available();
          if (availableSize > 0) {
            // reading available data
            size_t bytesToRead = min(availableSize, len - totalBytesRead);
            size_t bytesRead = stream->readBytes(buff + totalBytesRead, bytesToRead);
            totalBytesRead += bytesRead;

            if (bytesRead == 0){
                break;
            }
          }
          delay(1); // small delay to free up resources
        }

        if (totalBytesRead != len) {
          Serial.println("Error: Data not fully read");
          free(buff);
          buff = nullptr;
        }
        else {
          size = totalBytesRead;
        }
      }
      else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      
      http.end();

      return buff;
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
};
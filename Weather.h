struct CurrentWeather{
  float temp;
  short pressure;
  short humidity;
  char* description;
  float feelsLike;
  char* iconCode;
  bool isDataUpdated;
  uint8_t* icon;
  size_t iconSize;
    
  float windSpeed;
  float deg;
  float gust;

  long sunrise;
  long sunset;
  long time;
};

class Weather {
  public:
    Weather()
      : _lat(0), _lon(0) {}

    void updateData(CurrentWeather* data) {
      HTTPClient http;
      char* code = nullptr;

      data->isDataUpdated = false;

      char url[140];
      snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?lat=%.5f&lon=%.5f&appid=c5a08acba9e958144a4a95e10c8016c0&units=metric&lang=en", _lat, _lon);

      http.begin(url);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
          data->temp = doc["main"]["temp"].as<float>();
          data->pressure = doc["main"]["pressure"].as<short>();
          data->humidity = doc["main"]["humidity"].as<short>();
          data->feelsLike = doc["main"]["feels_like"].as<float>();
          data->windSpeed = doc["wind"]["speed"].as<float>();
          data->deg = doc["wind"]["deg"].as<float>();
          data->gust = doc["wind"]["gust"].as<float>();

          data->sunrise = doc["sys"]["sunrise"].as<long>();
          data->sunset = doc["sys"]["sunset"].as<long>();
          data->time = doc["dt"].as<long>();

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

    void getLocation() {
      HTTPClient http;

      http.begin("http://ip-api.com/json/");
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error) {
          _lat = doc["lat"].as<float>();
          _lon = doc["lon"].as<float>();
        }
        else {
          Serial.print("JSON Deserialization Error: ");
          Serial.println(error.c_str());
        }
      }
      else {
        Serial.println("Connection failed. getCoordinates()");
      }

      http.end();
    }
 
  private:
    float _lat;
    float _lon;
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
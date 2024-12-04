#include <TFT_eSPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "Adafruit_GFX.h"
#include <HTTPClient.h>
#include <PNGdec.h>

#include "ENCODER.h"
#include "Weather.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#define background TFT_SKYBLUE
#define MAX_IMAGE_WIDTH 240

// Wi-Fi settings
const char* ssid = "BRI-AZ-PF4DYB33 5770";
const char* password = "Pz9527?6";

float lat;
float lon;

PNG png;
ENCODER encoder(41, 40, 0, 2);
Weather weather;
CurrentWeather currentWeather;

int oldPosition = 0;
long lastTime = 0;
const int updateWeatherTime = 60000;
const int centerY = 120;
const int centerX = 120;

// function prototypes
void connectWiFi();
void displayPNG(uint8_t* pngData, size_t size);

void setup() {
  Serial.begin(115200);

  encoder.begin();
  tft.init();  
  tft.fillScreen(background);
  
  sprite.createSprite(240,240);
  sprite.setTextDatum(4);

  connectWiFi();

  getLocation();

  weather.updateData(&currentWeather, lat, lon);
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
      lat = doc["lat"].as<float>();
      lon = doc["lon"].as<float>();
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

void loop() {
  encoder.update();

  int8_t newPosition = encoder.getPosition();
  if (newPosition != oldPosition) {
    switch (newPosition) {
        case 0:
            drawWeather();
            break;
        case 1:
            drawWind();
            break;
        case 2:
            drawSomethingElse();
            break;
        default:
            printf("4\n");
            break;
    }

    oldPosition = newPosition;
  }

  if (lastTime + updateWeatherTime >= millis() && lastTime != 0){
    return;
  }

  weather.updateData(&currentWeather, lat, lon);

  if (!currentWeather.isDataUpdated){
    delay(1000);

    return;
  }

  if (newPosition == 0 && oldPosition == 0) {
    drawWeather();
  }

  lastTime= millis();
}

void drawWind() {
  sprite.fillSprite(background);
 
  sprite.setFreeFont(&FreeSerif12pt7b);
  sprite.setTextSize(1);

  //drawing speed
  char bufferSpeed[9];
  snprintf(bufferSpeed, sizeof(bufferSpeed), "%.2f m/s", currentWeather.windSpeed);
  char* speedText = concatStrings("speed ", bufferSpeed);

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(speedText, centerX, 100);
  
  free(speedText);

  //drawing gust
  char bufferGust[8];
  snprintf(bufferGust, sizeof(bufferGust), "%.1f m/s", currentWeather.gust);
  char* gustText = concatStrings("gusts ", bufferGust);

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(gustText, centerX, 130);
  
  free(gustText);
  
  //drawing deg
  int8_t indentFromEdge = 20;
  sprite.drawString("N", 120, indentFromEdge);
  sprite.drawString("E", 240 - indentFromEdge, 120);
  sprite.drawString("S", 120, 240 - indentFromEdge);
  sprite.drawString("W", indentFromEdge, 120);

  double radius = 116;
  double  tableAngleStep = 360 / 12;
  for (int i = 1; i <= 12; i++) {
    double tableAngle = tableAngleStep * i * M_PI / 180;
    double x = centerX + radius * cos(tableAngle);
    double y = centerY + radius * sin(tableAngle);

    sprite.drawCircle((int32_t)x, (int32_t)y, 3, TFT_WHITE);
    sprite.fillCircle((int32_t)x, (int32_t)y, 3, TFT_WHITE);
  }

  float angleRadians = (currentWeather.deg - 90) * M_PI / 180; // -90 shift
  int x = centerX + (int)(radius * cos(angleRadians));
  int y = centerY + (int)(radius * sin(angleRadians));

  sprite.drawCircle((int32_t)x, (int32_t)y, 3, TFT_RED);
  sprite.fillCircle((int32_t)x, (int32_t)y, 3, TFT_RED);

  tft.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer());
}

/*void drawThickLine(int32_t x1, int32_t y1, int32_t length, float angle_radians, int thickness, uint32_t color) {
    // Смещение для "жирности"
    float dx = sin(angle_radians); // Смещение по X для каждой толщины
    float dy = cos(angle_radians); // Смещение по Y для каждой толщины

    for (int i = -thickness / 2; i <= thickness / 2; i++) {
        // Начало и конец смещенной линии
        int32_t x_start = x1 + i * dy;
        int32_t y_start = y1 - i * dx;
        int32_t x_end = x_start + length * cos(angle_radians);
        int32_t y_end = y_start + length * sin(angle_radians);

        // Рисуем одну линию
        sprite.drawLine(x_start, y_start, x_end, y_end, color);
    }
}*/

void drawSomethingElse() {
  sprite.fillSprite(background);

  sprite.setFreeFont(&FreeSerif12pt7b);
  sprite.setTextSize(1);

  //drawing pressure
  char bufferPressure[12];
  snprintf(bufferPressure, sizeof(bufferPressure), "%d hPa", currentWeather.pressure);
  char* pressureText = concatStrings("pressure ", bufferPressure); 

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(pressureText, centerX, 100);

  free(pressureText);

  //drawing humidity
  char bufferHumidity[8];
  snprintf(bufferHumidity, sizeof(bufferHumidity), "%d%%", currentWeather.humidity);
  char* humidityText = concatStrings("humidity ", bufferHumidity); 

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(humidityText, centerX, 130);

  free(humidityText);

  tft.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer());
}

void drawWeather() {
  sprite.fillSprite(background);

  //drawing icon
  int16_t rc = png.openFLASH(currentWeather.icon, currentWeather.iconSize, pngDraw);
  if (rc == PNG_SUCCESS) {
    //Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    sprite.startWrite();
    rc = png.decode(NULL, 0);
    sprite.endWrite();
  }
  else {
    Serial.println("Icon was not opened");
  }

  //drawing temp
  char temp[8];
  snprintf(temp, sizeof(temp), "%.1f", currentWeather.temp);

  sprite.setFreeFont(&FreeSerif24pt7b);
  sprite.setTextColor(TFT_WHITE, background);
  sprite.setTextSize(1.5);

  int textLengthInPixels = sprite.textWidth(temp);

  for (int i = 0; i < 2; i++) {
    sprite.drawCircle(centerX + 10 + textLengthInPixels / 2, 100, 4 - i, TFT_WHITE);
  }

  sprite.drawString(temp, centerX, 110);
  sprite.unloadFont();

  //drawing description
  sprite.setFreeFont(&FreeSerif12pt7b);
  sprite.setTextSize(1);
  sprite.drawString(currentWeather.description, centerX, 160);

  //drawing feels like
  char bufferFeelsLike[8];
  snprintf(bufferFeelsLike, sizeof(bufferFeelsLike), "%.1f", currentWeather.feelsLike);
  char* feelsLikeText = concatStrings("feels like ", bufferFeelsLike);

  int flLengthPixels = sprite.textWidth(feelsLikeText);
  sprite.drawCircle(centerX + 5 + flLengthPixels / 2, 185, 2, TFT_WHITE);

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(feelsLikeText, centerX, 190);
  sprite.unloadFont();

  free(feelsLikeText);
  
  tft.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer());
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  uint16_t transparentColor = sprite.color565(0, 0, 0);
  
  // Получаем строку с альфа-каналом
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);

  for (int x = 0; x < pDraw->iWidth; x++) {
    if (lineBuffer[x] != transparentColor) {
      sprite.drawPixel(70 + x, pDraw->y, lineBuffer[x]);
    }
  }
}

void connectWiFi(){    
  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
  }
  else {
    Serial.println("Wi-Fi failed!");
    while (true);
  }
}

char* concatStrings(const char* str1, const char* str2) {
    size_t length1 = strlen(str1);
    size_t length2 = strlen(str2);
    char* result = (char*)malloc(length1 + length2 + 1);
    if (result == NULL) {
        Serial.println("Concatting error: out of memory");
        exit(1);
    }
    strcpy(result, str1);
    strcat(result, str2);

    return result;
}
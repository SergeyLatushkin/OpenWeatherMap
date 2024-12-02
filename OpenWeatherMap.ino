#include <TFT_eSPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "Adafruit_GFX.h"
#include <stdio.h>
#include <string.h>
#include <HTTPClient.h>
#include <PNGdec.h>
#include "ENCODER.h"
#include "Weather.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#define background TFT_SKYBLUE
#define MAX_IMAGE_WIDTH 240

// Wi-Fi settings
const char* ssid = "BRI-AZ-PF4DYB33 5770";
const char* password = "Pz9527?6";

PNG png;
ENCODER encoder(41, 40);
Weather weather;

int oldPosition = 0;
long lastTime = 0;
int updateWeatherTime = 60000;
int centerX = 120;

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
}

void loop() {
  encoder.update();

  int8_t newPosition = encoder.getPosition();
  if (newPosition != oldPosition) {
    Serial.printf("____encoder newPosition: %d , oldPosition: %d \n", newPosition, oldPosition);

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

  if(lastTime + updateWeatherTime >= millis() && lastTime != 0){
    return;
  }

  weather.updateData();

  if(!weather.isDataUpdated()){
    delay(1000);

    return;
  }

  if (newPosition == oldPosition) {
    drawWeather();
  }

  lastTime= millis();
}

void drawWind() {
  sprite.fillSprite(background);

  //drawing speed
  char bufferSpped[8];
  snprintf(bufferSpped, sizeof(bufferSpped), "%.1f", weather.windSpeed());
  char* speedText = concatStrings("wind speed ", bufferSpped);

  sprite.setTextColor(TFT_WHITE, background);
  sprite.drawString(speedText, centerX, 100);
  
  free(speedText);

  tft.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer());
}

void drawSomethingElse() {
  sprite.fillSprite(background);

  //drawing
  sprite.setFreeFont(&FreeSerif12pt7b);
  sprite.setTextSize(1);
  sprite.drawString("draw something else", centerX, 160);
  
  tft.pushImage(0, 0, 240, 240, (uint16_t*)sprite.getPointer());
}

void drawWeather() {
  sprite.fillSprite(background);

  //drawing icon
  if (weather.icon() != nullptr) {
    size_t iconSize = 0;
    uint8_t* iconData = weather.fetchWeatherIcon(iconSize);

    int16_t rc = png.openFLASH(iconData, iconSize, pngDraw);
    if (rc == PNG_SUCCESS) {
      Serial.println("Successfully opened png file");
      //Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      sprite.startWrite();
      rc = png.decode(NULL, 0);
      sprite.endWrite();
    }
    else
    {
      Serial.println("Icon was not opened");
    }

    free(iconData);
  }

  //drawing temp
  char temp[8];
  snprintf(temp, sizeof(temp), "%.1f", weather.temp());

  sprite.setFreeFont(&FreeSerif24pt7b);
  sprite.setTextColor(TFT_WHITE, background);
  sprite.setTextSize(1.5);

  int textLengthInPixels = sprite.textWidth(temp);

  sprite.drawCircle(centerX + 10 + textLengthInPixels / 2, 100, 3, TFT_WHITE);

  sprite.drawString(temp, centerX, 110);
  sprite.unloadFont();

  //drawing description
  sprite.setFreeFont(&FreeSerif12pt7b);
  sprite.setTextSize(1);
  sprite.drawString(weather.description(), centerX, 160);

  //drawing feels like
  char bufferFeelsLike[8];
  snprintf(bufferFeelsLike, sizeof(bufferFeelsLike), "%.1f", weather.feelsLike());
  char* feelsLikeText = concatStrings("feels like ", bufferFeelsLike);

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
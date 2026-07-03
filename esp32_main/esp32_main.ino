#include "config.h"

void setup() {
  loggerInit();
  portsInit();
  relaysInit(); 
  wifiInit();

  // Инициализируем датчики температуры
  temperatureInit();

  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    otaInit();
  }
}

void loop() {
  checkResetButton();

  // Обрабатываем веб-запросы (включая запросы температуры)
  server.handleClient();
  
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    ElegantOTA.loop();
  }
  
  delay(10);
}
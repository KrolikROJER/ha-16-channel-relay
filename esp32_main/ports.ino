#include "config.h"

const int BUTTON_PIN = 0; // Кнопка BOOT на плате ESP32-S3

void portsInit() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  logMsg("Порты инициализированы. Кнопка BOOT (IO0) готова для сброса.");
}

void resetWifiConfig() {
  logMsg("Очистка сохраненных настроек Wi-Fi...");
  preferences.begin("wifi_store", false);
  preferences.clear();
  preferences.end();
  
  WiFi.disconnect(true, true); // Стираем настройки из SDK ESP32
  delay(500);
  logMsg("Настройки сброшены! Перезагрузка...");
  ESP.restart();
}

void checkResetButton() {
  // Если кнопка BOOT нажата (состояние LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long startTime = millis();
    logMsg("Кнопка BOOT нажата. Удерживайте для сброса Wi-Fi...");
    
    while (digitalRead(BUTTON_PIN) == LOW) {
      if (millis() - startTime > 4000) { // 4 секунды удержания
        resetWifiConfig();
      }
      delay(50);
    }
    logMsg("Кнопка отпущена, сброс отменен.");
  }
}

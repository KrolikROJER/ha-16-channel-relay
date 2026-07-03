#include "config.h"
#include <ElegantOTA.h>

// Функция, которая сработает при начале обновления
void onOTAStart() {
  logMsg("OTA: Обновление прошивки началось...");
}

// Функция, которая сработает по завершении (успех или ошибка)
void onOTAEnd(bool success) {
  if (success) {
    logMsg("OTA: Прошивка успешно загружена! Перезагрузка устройства...");
  } else {
    logMsg("OTA: Ошибка при обновлении прошивки.");
  }
}

void otaInit() {
  // Привязываем ElegantOTA к нашему существующему веб-серверу
  ElegantOTA.begin(&server);
  
  // Настраиваем функции обратного вызова (колбэки)
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onEnd(onOTAEnd);
  
  logMsg("Модуль ElegantOTA успешно запущен.");
  logMsg("Страница обновления доступна по адресу: http://" + WiFi.localIP().toString() + "/update");
}

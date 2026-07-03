#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Имя точки доступа ESP32 для первоначальной настройки сети
const char* AP_SSID = "ESP32-S3-Setup";

// Глобальные объекты управления системой
extern WebServer server;
extern Preferences preferences;

// Настройки кольцевого буфера для вывода системных логов на веб-страницу
const int LOG_HISTORY_SIZE = 10;
extern String logHistory[LOG_HISTORY_SIZE];
extern int logHistoryIndex;

// Глобальные динамические настройки устройства
extern int numRelays;
extern int numSensors;
const int MAX_RELAYS = 16;
const int RELAY_PINS[MAX_RELAYS] = {1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 38, 39, 21};
const int ONE_WIRE_BUS = 15; 

// === Прототипы функций по модулям ===

// Модуль логирования (logger.ino)
void loggerInit();
void logMsg(const String& msg);
String getLogsJson();

// Модуль портов и кнопки сброса (ports.ino)
void portsInit();
void resetWifiConfig();
void checkResetButton();

// Модуль Wi-Fi и веб-интерфейса (wifi_manager.ino)
void wifiInit();
void startAP();
void handleSaveDeviceConfig(); // Обработчик для новой кнопки

// Модуль беспроводного обновления прошивки (ota_manager.ino)
void otaInit();

// Модуль управления реле (relay_manager.ino)
void relaysInit();
void setRelayState(int relayIdx, bool state);
bool getRelayState(int relayIdx);

// Модуль температуры (temperature_manager.ino)
void temperatureInit();
void handleTemperatureApi();

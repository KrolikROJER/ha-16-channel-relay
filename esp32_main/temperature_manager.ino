#include "config.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void temperatureInit() {
  sensors.begin();
  preferences.begin("wifi_store", true);
  numSensors = preferences.getInt("num_sensors", 2);
  preferences.end();
  
  int count = sensors.getDeviceCount();
  logMsg("Модуль температуры инициализирован. Физически найдено: " + String(count) + ", программный лимит UI: " + String(numSensors));
}

float getTemperature(int index) {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(index);
  if (temp == DEVICE_DISCONNECTED_C) {
    return -999.0; 
  }
  return temp;
}

void registerTemperatureApi() {
  server.on("/api/temperature", HTTP_GET, handleTemperatureApi);
}

void handleTemperatureApi() {
  // Формируем JSON с динамическим полем count и массивом значений temps
  String json = "{\"count\":" + String(numSensors) + ",\"temps\":[";
  
  for (int i = 0; i < numSensors; i++) {
    float t = getTemperature(i);
    if (t != -999.0) {
      json += String(t, 1);
    } else {
      json += "null";
    }
    if (i < numSensors - 1) json += ",";
  }
  
  json += "]}";
  server.send(200, "application/json; charset=UTF-8", json);
}

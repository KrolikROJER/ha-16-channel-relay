#include "config.h"

bool relayStates[MAX_RELAYS];

void registerRelayApi() {
  // API для переключения одиночного реле
  server.on("/api/relay/toggle", HTTP_POST, []() {
    if (server.hasArg("id")) {
      int id = server.arg("id").toInt();
      if (id >= 0 && id < numRelays) { // Проверка по динамическому лимиту
        bool newState = !relayStates[id];
        setRelayState(id, newState);
        server.send(200, "text/plain", newState ? "1" : "0");
        return;
      }
    }
    server.send(400, "text/plain", "Bad Request");
  });

  // API для ВКЛЮЧЕНИЯ ВСЕХ реле одновременно 
  server.on("/api/relay/all-on", HTTP_POST, []() {
    logMsg("Запущена команда: ВКЛЮЧИТЬ ВСЕ АКТИВНЫЕ КАНАЛЫ");
    for (int i = 0; i < numRelays; i++) {
      relayStates[i] = true;
      pinMode(RELAY_PINS[i], OUTPUT);
      digitalWrite(RELAY_PINS[i], LOW);
      delay(25);
    }
    server.send(200, "text/plain", "OK");
  });

  // API для ВЫКЛЮЧЕНИЯ ВСЕХ реле одновременно
  server.on("/api/relay/all-off", HTTP_POST, []() {
    logMsg("Запущена команда: ВЫКЛЮЧИТЬ ВСЕ АКТИВНЫЕ КАНАЛЫ");
    for (int i = 0; i < numRelays; i++) {
      relayStates[i] = false;
      pinMode(RELAY_PINS[i], INPUT);
    }
    server.send(200, "text/plain", "OK");
  });

  // API для получения состояний всех реле (Добавлено поле count!)
  server.on("/api/relay/states", HTTP_GET, []() {
    String json = "{\"count\":" + String(numRelays) + ",\"states\":[";
    for (int i = 0; i < numRelays; i++) {
      json += relayStates[i] ? "true" : "false";
      if (i < numRelays - 1) json += ",";
    }
    json += "]}";
    server.send(200, "application/json", json);
  });
}

void relaysInit() {
  // На старте считываем лимит из памяти
  preferences.begin("wifi_store", true);
  numRelays = preferences.getInt("num_relays", 16);
  preferences.end();

  for (int i = 0; i < MAX_RELAYS; i++) {
    pinMode(RELAY_PINS[i], INPUT);
    relayStates[i] = false;
  }
  logMsg("Модуль реле аппаратно обесточен. Лимит активных каналов: " + String(numRelays));
}

void setRelayState(int relayIdx, bool state) {
  if (relayIdx < 0 || relayIdx >= numRelays) return;
  
  relayStates[relayIdx] = state;
  if (state) {
    pinMode(RELAY_PINS[relayIdx], OUTPUT);
    digitalWrite(RELAY_PINS[relayIdx], LOW);
  } else {
    pinMode(RELAY_PINS[relayIdx], INPUT);
  }
  
  String statusText = state ? "ВКЛЮЧЕНО" : "ВЫКЛЮЧЕНО";
  logMsg("Реле №" + String(relayIdx + 1) + " " + statusText);
}

bool getRelayState(int relayIdx) {
  if (relayIdx < 0 || relayIdx >= numRelays) return false;
  return relayStates[relayIdx];
}

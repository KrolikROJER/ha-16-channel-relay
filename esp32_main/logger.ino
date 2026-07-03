#include "config.h"

String logHistory[LOG_HISTORY_SIZE];
int logHistoryIndex = 0;

void loggerInit() {
  Serial.begin(115200);
  delay(1000);
  logMsg("СИСТЕМА ЗАПУЩЕНА");
}

void logMsg(const String& msg) {
  String formattedLog = "[" + String(millis() / 1000) + "s] " + msg;
  Serial.println(formattedLog);

  // Записываем в кольцевой буфер
  logHistory[logHistoryIndex] = formattedLog;
  logHistoryIndex = (logHistoryIndex + 1) % LOG_HISTORY_SIZE;
}

// Формируем JSON массив из последних 10 логов в хронологическом порядке
String getLogsJson() {
  String json = "[";
  int idx = logHistoryIndex;
  bool first = true;
  
  for (int i = 0; i < LOG_HISTORY_SIZE; i++) {
    if (logHistory[idx] != "") {
      if (!first) json += ",";
      // Экранируем кавычки на всякий случай
      String escapedLog = logHistory[idx];
      escapedLog.replace("\"", "\\\"");
      json += "\"" + escapedLog + "\"";
      first = false;
    }
    idx = (idx + 1) % LOG_HISTORY_SIZE;
  }
  json += "]";
  return json;
}

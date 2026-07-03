#include "config.h"
#include "wifi_html.h"
#include "relays_html.h"

WebServer server(80);
Preferences preferences;

int numRelays = 16;
int numSensors = 2;

void handleRoot() {
  logMsg("Запрос главной страницы (Параметры).");
  String page = String(HTML_PAGE_WIFI);

  int n = WiFi.scanNetworks();
  String networkOptions = "";
  if (n == 0) {
    networkOptions = "<option value=''>Сети не найдены (Обновите страницу)</option>";
  } else {
    for (int i = 0; i < n; ++i) {
      networkOptions += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
  }

  preferences.begin("wifi_store", true);
  String currentHost = preferences.getString("hostname", "kostya-svet");
  numRelays = preferences.getInt("num_relays", 16);
  numSensors = preferences.getInt("num_sensors", 2);
  preferences.end();
  
  String otaLinkHtml = "";
  if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
    otaLinkHtml = "<div class='card' style='text-align:center;'><h3>Обновление системы</h3><a href='/update' style='display:inline-block; width:94%; padding:12px; background:#28a745; color:white; text-decoration:none; border-radius:4px; font-weight:bold; font-size:15px;'>Перейти к обновлению по Wi-Fi</a></div>";
  }
  
  page.replace("{{NETWORKS}}", networkOptions);
  page.replace("{{HOSTNAME}}", currentHost);
  page.replace("{{NUM_RELAYS}}", String(numRelays));
  page.replace("{{NUM_SENSORS}}", String(numSensors));
  page.replace("{{OTA_LINK}}", otaLinkHtml);
  
  server.send(200, "text/html; charset=UTF-8", page);
}

void handleRelaysPage() {
  server.send(200, "text/html; charset=UTF-8", HTML_PAGE_RELAYS);
}

// Кнопка 1: Только сохранение Wi-Fi
void handleConnect() {
  logMsg("Получен POST запрос на конфигурацию Wi-Fi.");
  if (!server.hasArg("ssid")) {
    server.send(400, "text/plain", "Missing SSID");
    return;
  }

  String ssid = server.arg("ssid");
  String password = server.arg("password");

  preferences.begin("wifi_store", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();

  server.send(200, "text/plain", "OK");
  delay(2000);
  ESP.restart();
}

// Кнопка 2: Параметры устройства без перезагрузки платы!
void handleSaveDeviceConfig() {
  logMsg("Получен POST запрос параметров конфигурации устройства.");
  if (!server.hasArg("hostname") || !server.hasArg("num_relays") || !server.hasArg("num_sensors")) {
    server.send(400, "text/plain", "Bad Args");
    return;
  }

  String hostname = server.arg("hostname");
  hostname.toLowerCase();
  hostname.replace(" ", "-");
  
  int r = server.arg("num_relays").toInt();
  int s = server.arg("num_sensors").toInt();

  if (r > MAX_RELAYS) r = MAX_RELAYS;

  preferences.begin("wifi_store", false);
  preferences.putString("hostname", hostname);
  preferences.putInt("num_relays", r);
  preferences.putInt("num_sensors", s);
  preferences.end();

  numRelays = r;
  numSensors = s;
  
  // Обновляем mDNS на лету
  WiFi.setHostname(hostname.c_str());
  MDNS.begin(hostname.c_str());

  logMsg("Новые параметры сохранены: Реле=" + String(numRelays) + ", Датчиков=" + String(numSensors) + ", Имя=" + hostname);
  server.send(200, "text/plain", "OK");
}

void handleGetLogs() {
  server.send(200, "application/json", getLogsJson());
}

void startAP() {
  logMsg("Запуск встроенной точки доступа AP...");
  WiFi.disconnect(true, true);
  delay(100);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/relays", HTTP_GET, handleRelaysPage);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/api/config/save", HTTP_POST, handleSaveDeviceConfig);
  server.on("/api/logs", HTTP_GET, handleGetLogs);
  
  server.begin();
}

void wifiInit() {
  preferences.begin("wifi_store", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  String savedHostname = preferences.getString("hostname", "kostya-svet");
  numRelays = preferences.getInt("num_relays", 16);
  numSensors = preferences.getInt("num_sensors", 2);
  preferences.end();

  if (savedSSID == "") {
    startAP();
    return;
  }

  logMsg("Авторизация. Подключение к сети: " + savedSSID);
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(savedHostname.c_str());
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    logMsg("Успешное подключение! IP: " + WiFi.localIP().toString());

    if (MDNS.begin(savedHostname.c_str())) {
      MDNS.addService("http", "tcp", 80);
    }
    
    server.on("/", HTTP_GET, handleRoot);
    server.on("/relays", HTTP_GET, handleRelaysPage);
    server.on("/connect", HTTP_POST, handleConnect);
    server.on("/api/config/save", HTTP_POST, handleSaveDeviceConfig);
    server.on("/api/logs", HTTP_GET, handleGetLogs);
    
    // Подключаем динамические API из других файлов
    extern void registerRelayApi();
    extern void registerTemperatureApi();
    registerRelayApi();
    registerTemperatureApi();

    server.begin();
  } else {
    Serial.println();
    startAP();
  }
}

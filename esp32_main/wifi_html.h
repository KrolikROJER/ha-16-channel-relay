#pragma once
#include <Arduino.h>

const char HTML_PAGE_WIFI[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32-S3 Настройки</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f0f2f5; text-align: center; padding: 10px; margin: 0; }
    .container { max-width: 450px; margin: auto; }
    .nav-menu { display: flex; background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.05); margin-bottom: 15px; overflow: hidden; }
    .nav-btn { flex: 1; padding: 12px; text-decoration: none; color: #495057; font-weight: bold; font-size: 14px; background: #f8f9fa; }
    .nav-btn.active { background: #007bff; color: white; }
    .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 15px; text-align: left; }
    h2, h3 { margin-top: 0; text-align: center; color: #333; }
    label { font-weight: bold; display: block; margin-top: 10px; font-size: 14px; }
    input[type='text'], input[type='password'], input[type='number'], select { width: 100%; padding: 10px; margin: 5px 0 10px 0; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 14px; }
    .checkbox-container { display: flex; align-items: center; margin-bottom: 15px; user-select: none; }
    .checkbox-container input { margin-right: 8px; cursor: pointer; width: 16px; height: 16px; }
    .checkbox-container label { margin: 0; cursor: pointer; font-weight: normal; font-size: 14px; }
    input[type='button'] { width: 100%; padding: 12px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; font-size: 15px; margin-top: 5px; }
    input[type='button']:hover { background: #0056b3; }
    .btn-green { background: #28a745; }
    .btn-green:hover { background: #218838; }
    .log-box { background: #1e1e1e; color: #00ff00; font-family: 'Courier New', monospace; padding: 10px; border-radius: 4px; height: 150px; overflow-y: auto; font-size: 12px; line-height: 1.4; text-align: left; white-space: pre-wrap; }
    .status-msg { font-weight: bold; text-align: center; margin-top: 10px; font-size: 14px; }
  </style>
</head>
<body>
  <div class='container'>
    <div class='nav-menu'>
      <a href='/' class='nav-btn active'>Параметры</a>
      <a href='/relays' class='nav-btn'>Управление</a>
    </div>

    <!-- БЛОК 1: НАСТРОЙКИ СЕТИ WI-FI -->
    <div class='card'>
      <h3>Настройка Wi-Fi сети</h3>
      <form id='wifiForm'>
        <label>Выберите Wi-Fi сеть:</label>
        <select id='ssid' name='ssid'>{{NETWORKS}}</select>
        
        <label>Пароль сети:</label>
        <input type='password' id='password' name='password' placeholder='Введите пароль сети'>
        
        <div class='checkbox-container'>
          <input type='checkbox' id='showPassCheckbox' onclick='togglePassword()'>
          <label for='showPassCheckbox'>Показать пароль</label>
        </div>
        
        <input type='button' value='Сохранить Wi-Fi и перезагрузить' onclick='sendWifiConfig()'>
      </form>
      <div id='wifiStatus' class='status-msg'></div>
    </div>

    <!-- БЛОК 2: ОБЩИЕ НАСТРОЙКИ УСТРОЙСТВА -->
    <div class='card'>
      <h3>Конфигурация устройства</h3>
      <form id='deviceForm'>
        <label>Имя устройства (hostname):</label>
        <input type='text' id='hostname' name='hostname' value='{{HOSTNAME}}' required>

        <label>Количество каналов реле (макс. 16):</label>
        <input type='number' id='num_relays' name='num_relays' value='{{NUM_RELAYS}}' min='0' max='16' required>

        <label>Количество датчиков температуры:</label>
        <input type='number' id='num_sensors' name='num_sensors' value='{{NUM_SENSORS}}' min='0' max='10' required>

        <input type='button' class='btn-green' value='Применить конфигурацию' onclick='sendDeviceConfig()'>
      </form>
      <div id='deviceStatus' class='status-msg'></div>
    </div>

    {{OTA_LINK}}

    <div class='card'>
      <h3>Системные логи</h3>
      <div id='logConsole' class='log-box'>Загрузка логов...</div>
    </div>
  </div>

  <script>
    function togglePassword() {
      const passInput = document.getElementById('password');
      const checkbox = document.getElementById('showPassCheckbox');
      passInput.type = checkbox.checked ? 'text' : 'password';
    }

    // Отправка строго сетевых настроек
    function sendWifiConfig() {
      const statusDiv = document.getElementById('wifiStatus');
      statusDiv.style.color = '#333'; statusDiv.innerText = 'Сохранение сети...';
      
      const formData = new URLSearchParams();
      formData.append('ssid', document.getElementById('ssid').value);
      formData.append('password', document.getElementById('password').value);

      fetch('/connect', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: formData })
      .then(res => { if (res.ok) return res.text(); throw new Error('Error'); })
      .then(data => { statusDiv.style.color = '#5cb85c'; statusDiv.innerHTML = '<b>Сеть сохранена!</b><br>Плата перезагружается...'; })
      .catch(err => { statusDiv.style.color = '#d9534f'; statusDiv.innerText = 'Ошибка сохранения Wi-Fi.'; });
    }

    // Отправка параметров устройства
    function sendDeviceConfig() {
      const statusDiv = document.getElementById('deviceStatus');
      statusDiv.style.color = '#333'; statusDiv.innerText = 'Применение параметров...';
      
      const formData = new URLSearchParams();
      formData.append('hostname', document.getElementById('hostname').value);
      formData.append('num_relays', document.getElementById('num_relays').value);
      formData.append('num_sensors', document.getElementById('num_sensors').value);

      fetch('/api/config/save', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: formData })
      .then(res => { if (res.ok) return res.text(); throw new Error('Error'); })
      .then(data => { 
        statusDiv.style.color = '#5cb85c'; 
        statusDiv.innerHTML = '<b>Конфигурация успешно применена!</b>';
        setTimeout(() => { statusDiv.innerText = ''; }, 3000);
      })
      .catch(err => { statusDiv.style.color = '#d9534f'; statusDiv.innerText = 'Ошибка сохранения конфигурации.'; });
    }

    function fetchLogs() {
      fetch('/api/logs').then(res => res.json()).then(logs => {
        const consoleDiv = document.getElementById('logConsole');
        if (logs.length > 0) { consoleDiv.innerHTML = logs.join('<br>'); consoleDiv.scrollTop = consoleDiv.scrollHeight; }
      });
    }
    setInterval(fetchLogs, 2000);
    window.onload = fetchLogs;
  </script>
</body>
</html>
)rawliteral";

#pragma once
#include <Arduino.h>

const char HTML_PAGE_RELAYS[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32-S3 Управление</title>
  <style>
    body { font-family: Arial, sans-serif; background: #f0f2f5; text-align: center; padding: 10px; margin: 0; }
    .container { max-width: 450px; margin: auto; }
    .nav-menu { display: flex; background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.05); margin-bottom: 15px; overflow: hidden; }
    .nav-btn { flex: 1; padding: 12px; text-decoration: none; color: #495057; font-weight: bold; font-size: 14px; background: #f8f9fa; }
    .nav-btn.active { background: #007bff; color: white; }
    .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 15px; text-align: left; }
    h3 { margin-top: 0; text-align: center; color: #333; }
    
    .temp-container { display: flex; flex-direction: column; gap: 10px; margin-top: 10px; }
    .temp-box { background: #e9ecef; padding: 12px; border-radius: 6px; text-align: center; font-weight: bold; display: flex; justify-content: space-between; align-items: center; }
    .temp-val { font-size: 20px; color: #007bff; }

    .action-bar { display: flex; gap: 10px; margin-bottom: 15px; }
    .action-btn { flex: 1; padding: 10px; border: none; border-radius: 4px; font-weight: bold; cursor: pointer; color: white; font-size: 14px; }
    .btn-all-on { background: #28a745; }
    .btn-all-on:hover { background: #218838; }
    .btn-all-off { background: #dc3545; }
    .btn-all-off:hover { background: #c82333; }

    .relay-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; margin-top: 15px; }
    .relay-btn { padding: 15px 5px; background: #6c757d; color: white; border: none; border-radius: 6px; cursor: pointer; font-weight: bold; font-size: 14px; text-align: center; transition: background 0.2s; }
    .relay-btn.active { background: #28a745; box-shadow: 0 0 8px rgba(40,167,69,0.5); }
    .log-box { background: #1e1e1e; color: #00ff00; font-family: 'Courier New', monospace; padding: 10px; border-radius: 4px; height: 120px; overflow-y: auto; font-size: 12px; line-height: 1.4; text-align: left; white-space: pre-wrap; }
  </style>
</head>
<body>
  <div class='container'>
    <div class='nav-menu'>
      <a href='/' class='nav-btn'>Параметры</a>
      <a href='/relays' class='nav-btn active'>Управление</a>
    </div>

    <!-- Динамический блок температуры -->
    <div class='card' id='tempCard' style='display:none;'>
      <h3>Текущая температура</h3>
      <div class='temp-container' id='tempContainer'></div>
    </div>

    <!-- Динамический блок реле -->
    <div class='card' id='relayCard' style='display:none;'>
      <h3>Пульт управления реле</h3>
      <div class='action-bar'>
        <button class='action-btn btn-all-on' onclick='allOn()'>Включить всё</button>
        <button class='action-btn btn-all-off' onclick='allOff()'>Выключить всё</button>
      </div>
      <div class='relay-grid' id='relayGrid'></div>
    </div>

    <div class='card'>
      <h3>Консоль событий системы</h3>
      <div id='logConsole' class='log-box'>Загрузка консоли...</div>
    </div>
  </div>

  <script>
    let currentRelayCount = 0;
    let currentSensorCount = 0;

    // Инициализация интерфейса на базе данных от ESP32
    function initInterface() {
      fetch('/api/relay/states')
      .then(res => res.json())
      .then(data => {
        // Сервер теперь возвращает объект вида {"count": X, "states": [...]}
        currentRelayCount = data.count;
        const grid = document.getElementById('relayGrid');
        grid.innerHTML = '';
        
        if (currentRelayCount > 0) {
          document.getElementById('relayCard').style.display = 'block';
          for (let i = 0; i < currentRelayCount; i++) {
            const btn = document.createElement('button');
            btn.className = 'relay-btn';
            btn.id = 'relay-' + i;
            btn.innerText = 'IN ' + (i + 1);
            btn.onclick = () => toggleRelay(i);
            grid.appendChild(btn);
          }
          updateStates(data.states);
        }
      });

      fetch('/api/temperature')
      .then(res => res.json())
      .then(data => {
        // Сервер возвращает объект вида {"count": X, "temps": [23.1, null]}
        currentSensorCount = data.count;
        const container = document.getElementById('tempContainer');
        container.innerHTML = '';

        if (currentSensorCount > 0) {
          document.getElementById('tempCard').style.display = 'block';
          for (let i = 0; i < currentSensorCount; i++) {
            const div = document.createElement('div');
            div.className = 'temp-box';
            div.innerHTML = `Датчик ${i + 1} <span id='t-${i}-val' class='temp-val'>--.- °C</span>`;
            container.appendChild(div);
          }
          updateTemperatureData(data.temps);
        }
      });
    }

    function toggleRelay(id) {
      const formData = new URLSearchParams();
      formData.append('id', id);
      fetch('/api/relay/toggle', { method: 'POST', body: formData })
      .then(res => res.text())
      .then(state => {
        const btn = document.getElementById('relay-' + id);
        if(state === "1") btn.classList.add('active'); else btn.classList.remove('active');
      });
    }

    function allOn() { fetch('/api/relay/all-on', { method: 'POST' }).then(() => updateStatesLoop()); }
    function allOff() { fetch('/api/relay/all-off', { method: 'POST' }).then(() => updateStatesLoop()); }

    function updateStates(statesArray) {
      statesArray.forEach((state, i) => {
        const btn = document.getElementById('relay-' + i);
        if(btn) { if(state) btn.classList.add('active'); else btn.classList.remove('active'); }
      });
    }

    function updateStatesLoop() {
      if (currentRelayCount === 0) return;
      fetch('/api/relay/states').then(res => res.json()).then(data => updateStates(data.states));
    }

    function updateTemperatureData(tempsArray) {
      tempsArray.forEach((temp, i) => {
        const span = document.getElementById(`t-${i}-val`);
        if (span) {
          span.innerText = temp !== null ? temp + ' °C' : 'Ошибка';
        }
      });
    }

    function updateTemperatureLoop() {
      if (currentSensorCount === 0) return;
      fetch('/api/temperature').then(res => res.json()).then(data => updateTemperatureData(data.temps));
    }

    function fetchLogs() {
      fetch('/api/logs').then(res => res.json()).then(logs => {
        const consoleDiv = document.getElementById('logConsole');
        if (logs.length > 0) { consoleDiv.innerHTML = logs.join('<br>'); consoleDiv.scrollTop = consoleDiv.scrollHeight; }
      });
    }

    setInterval(updateStatesLoop, 2000);
    setInterval(updateTemperatureLoop, 5000);
    setInterval(fetchLogs, 2000);

    window.onload = () => { initInterface(); fetchLogs(); };
  </script>
</body>
</html>
)rawliteral";

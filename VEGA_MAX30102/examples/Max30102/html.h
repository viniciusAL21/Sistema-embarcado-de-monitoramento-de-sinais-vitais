#ifndef HTML_H
#define HTML_H

#include <Arduino.h>

// HTML da página como string
const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Monitor de Sinais Vitais</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f0f0f0;
      padding: 40px;
    }
    h1 {
      color: #2c3e50;
      margin-bottom: 30px;
    }
    .container {
      display: flex;
      justify-content: center;
      flex-wrap: wrap;
      gap: 20px;
    }
    .card {
      background-color: white;
      padding: 25px;
      border-radius: 12px;
      box-shadow: 2px 2px 12px rgba(0,0,0,0.2);
      width: 220px;
    }
    .label {
      font-size: 18px;
      color: #555;
      margin-bottom: 10px;
    }
    .value {
      font-size: 46px;
      font-weight: bold;
    }
    #bpm { color: #e74c3c; }
    #spo2 { color: #3498db; }
    #temp { color: #27ae60; }
  </style>
</head>
<body>
  <h1>Monitor de Sinais Vitais</h1>
  <div class="container">
    <div class="card">
      <p class="label">Batimentos (BPM)</p>
      <p class="value" id="bpm">--</p>
    </div>
    <div class="card">
      <p class="label">Saturação (SpO₂)</p>
      <p class="value" id="spo2">--</p>
    </div>
    <div class="card">
      <p class="label">Temperatura (°C)</p>
      <p class="value" id="temp">--</p>
    </div>
  </div>

  <script>
    function updateData() {
      fetch('/readData')
        .then(response => response.text())
        .then(data => {
          let arr = JSON.parse(data);
          document.getElementById('bpm').innerText = arr[0];
          document.getElementById('spo2').innerText = arr[1];
          document.getElementById('temp').innerText = arr[2];
        })
        .catch(err => console.error('Erro:', err));
    }

    setInterval(updateData, 1000); // Atualiza a cada 1 segundo
  </script>
</body>
</html>
)rawliteral";

#endif




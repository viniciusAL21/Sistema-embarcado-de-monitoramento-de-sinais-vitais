#include <Wire.h>
#include "MAX30102_PulseOximeter.h"
#include "ClosedCube_MAX30205.h"
#include <esp_now.h>
#include <WiFi.h>

// -------------------- CONSTANTES --------------------
#define REPORTING_PERIOD_SENSOR_MS 1000  // Período do sensor
#define REPORTING_PERIOD_SEND_MS 2000     // Período de envio (2 segundos)
#define I2C_SDA 21
#define I2C_SCL 22

const uint32_t DEVICE_CODE = 123456;

// -------------------- MAX30102 --------------------
PulseOximeter pox;
uint32_t last_sensor_read = 0;
uint32_t last_send = 0;
float HeartRate = 0, SpO = 0;

// -------------------- MAX30205 --------------------
ClosedCube_MAX30205 max30205;
float tempFiltrada = 25.0;
uint8_t enderecoMAX30205 = 0x48; // Endereço padrão

// -------------------- ESP-NOW --------------------
typedef struct struct_message {
  uint32_t code;
  float hr;
  float spo;
  float temp;
} struct_message;

struct_message data_package;

uint8_t receiverMac[] = {0x4C, 0xC3, 0x82, 0x0C, 0x41, 0x60};
esp_now_peer_info_t peerInfo;


void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {

  Serial.print("Send Status: ");

  if(status == ESP_NOW_SEND_SUCCESS)
    Serial.println("Success");
  else
    Serial.println("Fail");

}

bool espnow_init()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init failed");
    return false;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return false;
  }

  Serial.println("ESP-NOW ready");

  return true;
}

void onBeatDetected() {
  Serial.println("❤️ Beat detectado!");
}

// -------------------- SCAN I2C --------------------
void scanI2C() {
  Serial.println("\n🔍 Escaneando I2C...");
  byte count = 0;
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("✅ Dispositivo em 0x");
      Serial.print(addr, HEX);
      
      if (addr >= 0x48 && addr <= 0x4F) {
        max30205.begin(addr);
        float testTemp = max30205.readTemperature();
        if (testTemp > -50 && testTemp < 100) {
          Serial.print(" - MAX30205 confirmado! Temp: ");
          Serial.print(testTemp);
          Serial.print("°C");
          enderecoMAX30205 = addr;
        }
      }
      Serial.println();
      count++;
    }
  }
  Serial.print("Total: ");
  Serial.println(count);
}

// -------------------- TEMPERATURA --------------------
float lerTemperatura() {
  float temp = max30205.readTemperature();
  if (isnan(temp) || temp < -40 || temp > 70) {
    return tempFiltrada;
  }
  return temp;
}


bool espnow_send()
{
  struct_message msg;

  msg.code = DEVICE_CODE;
  msg.hr = HeartRate;
  msg.spo = SpO;
  msg.temp = tempFiltrada;

  esp_err_t result = esp_now_send(receiverMac,(uint8_t *)&msg,sizeof(msg));

  if(result == ESP_OK)
  {
    Serial.println("Sent");
    return true;
  }
  else
  {
    Serial.println("Send error");
    return false;
  }
}

// // -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Inicializando sistema...");
  
  // Inicia I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  delay(100);
  
  // Scan I2C
  scanI2C();
  
  // Inicializa MAX30102
  Serial.print("Inicializando MAX30102... ");
  if (!pox.begin()) {
    Serial.println("❌ FALHOU!");
  } else {
    Serial.println("✅ OK!");
    pox.setIRLedCurrent(MAX30102_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  
  // Inicializa MAX30205
  Serial.print("Inicializando MAX30205... ");
  max30205.begin(enderecoMAX30205);
  delay(100);
  
  float testTemp = max30205.readTemperature();
  if (testTemp > -50 && testTemp < 100) {
    tempFiltrada = testTemp;
    Serial.print("✅ OK! Temperatura: ");
    Serial.print(tempFiltrada, 2);
    Serial.println("°C");
  } else {
    Serial.println("⚠️ Sensor respondeu mas leitura parece incorreta");
  }
  
    if(!espnow_init())
    {
      Serial.println("ESP-NOW failed");
      while(true);
    }
  
  Serial.println("✅ Sistema pronto!\n");
}

// -------------------- LOOP --------------------
void loop() {
  // Mantém o oxímetro atualizado (chamar com frequência!)
  pox.update();
  
  // Leitura dos sensores a cada 1 segundo
  if (millis() - last_sensor_read > REPORTING_PERIOD_SENSOR_MS) {
    // Lê oxímetro
    HeartRate = pox.getHeartRate();
    SpO = pox.getSpO2();
    
    // Lê temperatura com filtro
    float leitura = lerTemperatura();
    if (leitura > -50 && leitura < 100) {
      tempFiltrada = 0.8 * tempFiltrada + 0.2 * leitura;
    }
    
    // Mostra no monitor serial
    Serial.print("❤️ HR="); Serial.print(HeartRate, 1); Serial.print(" bpm | ");
    Serial.print("🫁 SpO2="); Serial.print(SpO, 1); Serial.print("% | ");
    Serial.print("🌡️ Temp="); Serial.print(tempFiltrada, 2); Serial.println("°C");
    
    last_sensor_read = millis();
  }
  
  // Envio dos dados a cada 2 segundos
  if (millis() - last_send > REPORTING_PERIOD_SEND_MS) {
    espnow_send();
    last_send = millis();
  }
}

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// #include <esp_now.h>
// #include <WiFi.h>

// uint8_t receiverMac[] = {0x4C, 0xC3, 0x82, 0x0C, 0x41, 0x60};

// typedef struct struct_message {
//   uint32_t code;
//   float b;
//   float c;
//   float d;
// } struct_message;

// struct_message myData;

// void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
//   Serial.print("Send Status: ");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
// }

// void setup() {

//   Serial.begin(115200);

//   WiFi.mode(WIFI_STA);
//   WiFi.disconnect();

//   Serial.print("WiFi channel: ");
//   Serial.println(WiFi.channel());

//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return;
//   }

//   esp_now_register_send_cb(OnDataSent);

//   esp_now_peer_info_t peerInfo = {};
//   memcpy(peerInfo.peer_addr, receiverMac, 6);
//   peerInfo.channel = 0;
//   peerInfo.encrypt = false;

//   if (esp_now_add_peer(&peerInfo) != ESP_OK){
//     Serial.println("Failed to add peer");
//     return;
//   }
// }

// void loop() {

//   strcpy(myData.a, "HELLO");
//   myData.b = random(10);
//   myData.c = 3.14;
//   myData.d = true;

//   esp_err_t result = esp_now_send(receiverMac,(uint8_t *)&myData,sizeof(myData));

//   if(result == ESP_OK)
//     Serial.println("Sent");
//   else
//     Serial.println("Error sending");

//   delay(2000);
// }
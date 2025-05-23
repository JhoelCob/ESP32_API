#include <WiFi.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 64
#define API_URL "http://161.132.206.104:9040/Datos/"

String inputString = "";
bool stringComplete = false;
bool wifiConnected = false;
bool dataSent = false; // Variable para controlar el envío único

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  delay(1000);

  String ssid = readEEPROMString(SSID_ADDR, 32);
  String password = readEEPROMString(PASS_ADDR, 32);

  if (ssid.length() > 0 && password.length() > 0) {
    Serial.println(" Intentando conectar con credenciales guardadas...");
    WiFi.begin(ssid.c_str(), password.c_str());

    if (waitForWiFiConnection()) {
      Serial.println(" Conexión exitosa con credenciales guardadas.");
      wifiConnected = true;
    } else {
      Serial.println(" Falló la conexión. Iniciando configuración manual...");
      configureWiFi();
    }
  } else {
    Serial.println(" No se encontraron credenciales. Iniciando configuración manual...");
    configureWiFi();
  }
}

void loop() {
  if (wifiConnected && !dataSent) { 
    DynamicJsonDocument doc(512); 
    doc["IMEI"] = "test_wifi";
    doc["K"] = 1;
    doc["N"] = 1;
    doc["P"] = 1;
    doc["PH"] = 1;
    doc["humidity"] = 1;
    doc["id"] = "1";
    doc["power"] = 1;
    doc["temperature"] = 1;
    doc["time"] = "1";

    String jsonString;
    serializeJson(doc, jsonString);
    
    // Enviar los datos via HTTP POST
    sendPostRequest(jsonString);
    
    dataSent = true; // Marcar como enviado para evitar repeticiones
    
    Serial.println("Conexion  exitosa");
  }
}

// ====== Las demás funciones permanecen igual ======
bool waitForWiFiConnection() {
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void configureWiFi() {
  Serial.println(" Buscando redes disponibles...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No se encontraron redes.");
    return;
  }

  for (int i = 0; i < n; ++i) {
    Serial.printf("%d: %s (%ddBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }

  Serial.println(" Ingresa el número de red WiFi:");
  while (!stringComplete) receiveSerial();
  int index = inputString.toInt() - 1;
  inputString = ""; stringComplete = false;

  if (index >= 0 && index < n) {
    String selectedSSID = WiFi.SSID(index);
    Serial.println(" Ingresa la contraseña:");
    while (!stringComplete) receiveSerial();
    String pass = inputString;
    inputString = ""; stringComplete = false;

    Serial.println("⏳ Conectando...");
    WiFi.begin(selectedSSID.c_str(), pass.c_str());

    if (waitForWiFiConnection()) {
      Serial.println(" Conexión exitosa.");
      writeEEPROMString(SSID_ADDR, selectedSSID, 32);
      writeEEPROMString(PASS_ADDR, pass, 32);
      EEPROM.commit();
      wifiConnected = true;
    } else {
      Serial.println(" Falló la conexión. Intenta nuevamente.");
    }
  } else {
    Serial.println(" Número de red inválido.");
  }
}

void receiveSerial() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

void writeEEPROMString(int addrOffset, String data, int maxLength) {
  for (int i = 0; i < maxLength; ++i) {
    if (i < data.length()) {
      EEPROM.write(addrOffset + i, data[i]);
    } else {
      EEPROM.write(addrOffset + i, 0);
    }
  }
}

String readEEPROMString(int addrOffset, int maxLength) {
  char data[maxLength + 1];
  for (int i = 0; i < maxLength; ++i) {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[maxLength] = '\0';
  return String(data);
}

void sendPostRequest(String jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(API_URL);
    http.addHeader("Content-Type", "application/json");
    
    Serial.println("Enviando POST request...");
    Serial.println("Datos: " + jsonData);
    
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(httpResponseCode);
      Serial.print("Respuesta: ");
      Serial.println(response);
    } else {
      Serial.print("Error en la petición HTTP: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi desconectado, no se puede enviar la petición");
    wifiConnected = false;
  }
}

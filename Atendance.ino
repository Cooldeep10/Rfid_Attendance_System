#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D4  // RFID SS (SDA)
#define RST_PIN D3 // RFID RST
MFRC522 rfid(SS_PIN, RST_PIN);

const char* SSID = "your_wifi_name";
const char* PASSWORD = "your_wifi_password";
const char* GScriptURL = "your_google_script_web_app_url";

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  WiFi.begin(SSID, PASSWORD);
  client.setInsecure();  // Ignore SSL for HTTPS requests

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected!");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  Serial.println("RFID UID: " + uidStr);

  String requestURL = String(GScriptURL) + "?sts=atc&uid=" + uidStr;
  sendToGoogleSheets(requestURL);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}

void sendToGoogleSheets(String url) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("Response: " + http.getString());
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  }
}

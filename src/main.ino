#include "max6675.h"
#include <WiFi.h>
#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// ===================== WiFi Credentials =====================
const char* ssid = "YOUR_WIFI_SSID";
const char* pass = "YOUR_WIFI_PASSWORD";

// ===================== Cloud Credentials =====================
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "YOUR_ADAFRUIT_USERNAME"
#define MQTT_PASS "YOUR_ADAFRUIT_IO_KEY"

#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DataBase_URL "YOUR_FIREBASE_DATABASE_URL"

// ===================== Sensor Pins =====================
int thermoDO = 18;
int thermoCS = 19;
int thermoCLK = 21;
int UVpin = 25;
const int flowSensorPin = 4;
#define Buzzer 5

// ===================== Variables =====================
float Temp;
float UVintensity;
float UVreading;

volatile int pulseCount = 0;
float flowRate = 0.0;
unsigned long lastTime = 0;
unsigned long interval = 1000;
const float calibrationFactor = 7.5;

// ===================== Sensor Setup =====================
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
WiFiClient client;

// ===================== MQTT Setup =====================
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

Adafruit_MQTT_Publish WaterFeed =
    Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/waterflow");

Adafruit_MQTT_Publish TemperatureFeed =
    Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/temperature");

Adafruit_MQTT_Publish UVFeed =
    Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/uv");

// ===================== Firebase Setup =====================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// ===================== Interrupt =====================
void IRAM_ATTR countPulse() {
    pulseCount++;
}

// ===================== Setup =====================
void setup() {
    Serial.begin(115200);

    pinMode(flowSensorPin, INPUT);
    pinMode(Buzzer, OUTPUT);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");

    config.api_key = API_KEY;
    config.database_url = DataBase_URL;

    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Firebase signup successful");
        signupOK = true;
    } else {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    attachInterrupt(digitalPinToInterrupt(flowSensorPin),
                    countPulse, RISING);

    lastTime = millis();
}

// ===================== Main Loop =====================
void loop() {
    MQTT_connect();

    Temp = thermocouple.readCelsius();

    UVreading = analogRead(UVpin);
    UVintensity = map(UVreading, 0, 4095, 240, 360);

    if (millis() - lastTime >= interval) {
        flowRate = pulseCount / calibrationFactor;

        Serial.print("Flow rate: ");
        Serial.print(flowRate);
        Serial.println(" L/min");

        pulseCount = 0;
        lastTime = millis();
    }

    // Temperature alert
    if (Temp > 180) {
        digitalWrite(Buzzer, HIGH);
    } else {
        digitalWrite(Buzzer, LOW);
    }

    Serial.print("UV intensity = ");
    Serial.print(UVintensity);
    Serial.print(" nm | Temp = ");
    Serial.print(Temp);
    Serial.println(" °C");

    // MQTT Publish
    UVFeed.publish((int32_t)UVintensity);
    TemperatureFeed.publish((int32_t)Temp);
    WaterFeed.publish(flowRate);

    // Firebase Upload
    if (Firebase.ready() && signupOK &&
        (millis() - sendDataPrevMillis > 5000 ||
         sendDataPrevMillis == 0)) {

        sendDataPrevMillis = millis();

        Firebase.RTDB.setFloat(&fbdo,
                               "ESP32/Temperature", Temp);

        Firebase.RTDB.setFloat(&fbdo,
                               "ESP32/Flow_Rate", flowRate);

        Firebase.RTDB.setFloat(&fbdo,
                               "ESP32/UV_Intensity", UVintensity);
    }

    delay(5000);
}

// ===================== MQTT Connection =====================
void MQTT_connect() {
    if (mqtt.connected()) return;

    int8_t ret;
    uint8_t retries = 3;

    while ((ret = mqtt.connect()) != 0) {
        mqtt.disconnect();
        delay(5000);

        retries--;

        if (retries == 0) {
            while (1);
        }
    }
}
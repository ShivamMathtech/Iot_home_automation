/*
=====================================================
INDUSTRIAL HOME AUTOMATION SYSTEM
Arduino Mega 2560 Firmware
=====================================================
*/

#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;

// ================= RELAY PINS =================

int relayPins[16] = {
  22,23,24,25,
  26,27,28,29,
  30,31,32,33,
  34,35,36,37
};

// ================= SENSOR PINS =================

#define PIR_PIN 3
#define GAS_PIN A0
#define LDR_PIN A1
#define WATER_LEVEL A2

// ================= VARIABLES =================

String incomingData = "";

float temperature;
float humidity;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);

  dht.begin();

  rtc.begin();

  // Relay Setup
  for(int i=0;i<16;i++)
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  pinMode(PIR_PIN, INPUT);

  Serial.println("HOME AUTOMATION SYSTEM STARTED");
}

void loop()
{
  readSensors();

  receiveESP32Commands();

  sendSensorDataToESP32();

  delay(500);
}

// =================================================
// SENSOR READING
// =================================================

void readSensors()
{
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  int gasValue = analogRead(GAS_PIN);
  int ldrValue = analogRead(LDR_PIN);
  int waterValue = analogRead(WATER_LEVEL);

  bool motion = digitalRead(PIR_PIN);

  Serial.print("TEMP: ");
  Serial.println(temperature);

  Serial.print("HUM: ");
  Serial.println(humidity);

  Serial.print("GAS: ");
  Serial.println(gasValue);

  Serial.print("LDR: ");
  Serial.println(ldrValue);

  Serial.print("WATER: ");
  Serial.println(waterValue);

  Serial.print("MOTION: ");
  Serial.println(motion);
}

// =================================================
// RECEIVE COMMANDS FROM ESP32
// =================================================

void receiveESP32Commands()
{
  while(Serial1.available())
  {
    char c = Serial1.read();

    if(c == '\n')
    {
      processCommand(incomingData);
      incomingData = "";
    }
    else
    {
      incomingData += c;
    }
  }
}

// =================================================
// PROCESS COMMANDS
// =================================================

void processCommand(String cmd)
{
  cmd.trim();

  Serial.print("Received: ");
  Serial.println(cmd);

  // Format:
  // R1ON
  // R1OFF

  if(cmd.startsWith("R"))
  {
    int relayNumber = cmd.substring(1, cmd.indexOf("O")).toInt();

    if(cmd.endsWith("ON"))
    {
      digitalWrite(relayPins[relayNumber - 1], LOW);

      Serial.print("Relay ");
      Serial.print(relayNumber);
      Serial.println(" ON");
    }

    if(cmd.endsWith("OFF"))
    {
      digitalWrite(relayPins[relayNumber - 1], HIGH);

      Serial.print("Relay ");
      Serial.print(relayNumber);
      Serial.println(" OFF");
    }
  }
}

// =================================================
// SEND SENSOR DATA TO ESP32
// =================================================

void sendSensorDataToESP32()
{
  Serial1.print("T:");
  Serial1.println(temperature);

  Serial1.print("H:");
  Serial1.println(humidity);

  Serial1.print("G:");
  Serial1.println(analogRead(GAS_PIN));

  Serial1.print("L:");
  Serial1.println(analogRead(LDR_PIN));

  Serial1.print("W:");
  Serial1.println(analogRead(WATER_LEVEL));
}
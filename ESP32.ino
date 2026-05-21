/*
=========================================================
INDUSTRIAL IoT HOME AUTOMATION SYSTEM
ESP32 MASTER CONTROLLER
=========================================================

FEATURES:
✔ WiFi Connectivity
✔ MQTT Cloud Communication
✔ Mobile App Control
✔ UART Communication with Arduino Mega
✔ Sensor Monitoring
✔ Real-Time Appliance Control
✔ Industrial Grade Structure
=========================================================
*/

#include <WiFi.h>
#include <PubSubClient.h>

// =====================================================
// WIFI CONFIGURATION
// =====================================================

const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// =====================================================
// MQTT CONFIGURATION
// =====================================================

const char* MQTT_SERVER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;

const char* MQTT_CLIENT_ID = "IndustrialHomeAutomationESP32";

// MQTT Topics

const char* TOPIC_CONTROL      = "home/control";

const char* TOPIC_TEMPERATURE  = "home/temperature";
const char* TOPIC_HUMIDITY     = "home/humidity";
const char* TOPIC_GAS          = "home/gas";
const char* TOPIC_LIGHT        = "home/light";
const char* TOPIC_WATER        = "home/water";
const char* TOPIC_MOTION       = "home/motion";

// =====================================================
// OBJECTS
// =====================================================

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// UART2 Communication with Mega

HardwareSerial MegaSerial(2);

// =====================================================
// VARIABLES
// =====================================================

String serialBuffer = "";

unsigned long lastReconnectAttempt = 0;

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    // UART2
    MegaSerial.begin(115200, SERIAL_8N1, 16, 17);

    Serial.println("\n====================================");
    Serial.println("ESP32 INDUSTRIAL HOME AUTOMATION");
    Serial.println("====================================");

    connectWiFi();

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

    mqttClient.setCallback(mqttCallback);
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    // MQTT Reconnect
    if (!mqttClient.connected())
    {
        reconnectMQTT();
    }

    mqttClient.loop();

    // Receive data from Arduino Mega
    readMegaSerial();
}

// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi()
{
    Serial.println("\nConnecting WiFi...");

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// =====================================================
// MQTT RECONNECT
// =====================================================

void reconnectMQTT()
{
    while (!mqttClient.connected())
    {
        Serial.println("\nConnecting MQTT...");

        if (mqttClient.connect(MQTT_CLIENT_ID))
        {
            Serial.println("MQTT Connected!");

            // Subscribe control topic
            mqttClient.subscribe(TOPIC_CONTROL);

            Serial.println("Subscribed:");
            Serial.println(TOPIC_CONTROL);
        }
        else
        {
            Serial.print("Failed MQTT State: ");
            Serial.println(mqttClient.state());

            delay(3000);
        }
    }
}

// =====================================================
// MQTT CALLBACK
// =====================================================

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String message = "";

    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println("\n========== MQTT COMMAND ==========");
    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Message: ");
    Serial.println(message);

    // Forward to Arduino Mega
    MegaSerial.println(message);

    Serial.println("Forwarded to Mega");
}

// =====================================================
// READ SERIAL DATA FROM MEGA
// =====================================================

void readMegaSerial()
{
    while (MegaSerial.available())
    {
        char c = MegaSerial.read();

        if (c == '\n')
        {
            processMegaData(serialBuffer);

            serialBuffer = "";
        }
        else
        {
            serialBuffer += c;
        }
    }
}

// =====================================================
// PROCESS DATA FROM MEGA
// =====================================================

void processMegaData(String data)
{
    data.trim();

    if (data.length() == 0)
        return;

    Serial.println("\n========== SENSOR DATA ==========");
    Serial.println(data);

    // Publish to MQTT Topics

    if (data.startsWith("T:"))
    {
        mqttClient.publish(TOPIC_TEMPERATURE, data.substring(2).c_str());
    }

    else if (data.startsWith("H:"))
    {
        mqttClient.publish(TOPIC_HUMIDITY, data.substring(2).c_str());
    }

    else if (data.startsWith("G:"))
    {
        mqttClient.publish(TOPIC_GAS, data.substring(2).c_str());
    }

    else if (data.startsWith("L:"))
    {
        mqttClient.publish(TOPIC_LIGHT, data.substring(2).c_str());
    }

    else if (data.startsWith("W:"))
    {
        mqttClient.publish(TOPIC_WATER, data.substring(2).c_str());
    }

    else if (data.startsWith("M:"))
    {
        mqttClient.publish(TOPIC_MOTION, data.substring(2).c_str());
    }
}

// =====================================================
// SEND MANUAL COMMAND TO MEGA
// =====================================================

void sendRelayCommand(int relayNumber, bool state)
{
    String command = "R";

    command += String(relayNumber);

    if (state)
        command += "ON";
    else
        command += "OFF";

    MegaSerial.println(command);

    Serial.print("Sent Command: ");
    Serial.println(command);
}
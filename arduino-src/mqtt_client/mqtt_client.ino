/* #define DEBUG */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <string.h>
#include <stdlib.h>

#define SERIAL_BAUD_RATE                    9600

#define WIFI_SSID                           ""
#define WIFI_PASSWORD                       ""
#define MQTT_SERVER                         ""
#define MQTT_PORT                           1883

#define MQTT_AVAILABILITY_TOPIC             "home-assistant/smart-blinds/availability"
#define MQTT_TILT_COMMAND_TOPIC             "home-assistant/smart-blinds/tilt"
#define MQTT_TILT_STATUS_TOPIC              "home-assistant/smart-blinds/tilt-state"
#define MQTT_CALIBRATE_HIGH_COMMAND_TOPIC   "home-assistant/smart-blinds/set-upper-limit"
#define MQTT_CALIBRATE_HIGH_STATUS_TOPIC    "home-assistant/smart-blinds/upper-limit-state"
#define MQTT_CALIBRATE_LOW_COMMAND_TOPIC    "home-assistant/smart-blinds/set-lower-limit"
#define MQTT_CALIBRATE_LOW_STATUS_TOPIC     "home-assistant/smart-blinds/lower-limit-state"

#define MQTT_PAYLOAD_AVAILABLE              "online"
#define MQTT_PAYLOAD_UNAVAILABLE            "offline"

#define COMMAND_TILT                        't'
#define COMMAND_CALIBRATE_HIGH              'h'
#define COMMAND_CALIBRATE_LOW               'l'

WiFiClient wiFiClient;
PubSubClient mqttClient(wiFiClient);

void mqttCallback(char* topic, unsigned char* payload, unsigned int length) {
#ifdef DEBUG
    Serial.print("message arrived in topic ");
    Serial.println(topic);

    Serial.print("payload length: ");
    Serial.println(length);

    Serial.print("message contents: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }

    Serial.println();
    Serial.println("-----------------------");
#endif

    if (strcmp(topic, MQTT_TILT_COMMAND_TOPIC) == 0) {
        Serial.write(COMMAND_TILT);
    } else if (strcmp(topic, MQTT_CALIBRATE_HIGH_COMMAND_TOPIC) == 0) {
        Serial.write(COMMAND_CALIBRATE_HIGH);
    } else if (strcmp(topic, MQTT_CALIBRATE_LOW_COMMAND_TOPIC) == 0) {
        Serial.write(COMMAND_CALIBRATE_LOW);
    }

    for (int i = 0; i < length; i++) {
        Serial.write((char) payload[i]);
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
#ifdef DEBUG
        Serial.println("connecting to wifi");
#endif
    }
#ifdef DEBUG
    Serial.println("wifi connected");
#endif

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    while (!mqttClient.connected()) {
#ifdef DEBUG
        Serial.println("connecting to mqtt...");
#endif

        if (mqttClient.connect("smart-blinds")) {
#ifdef DEBUG
            Serial.println("mqtt connected");
#endif
            mqttClient.subscribe(MQTT_TILT_COMMAND_TOPIC);
            mqttClient.subscribe(MQTT_CALIBRATE_HIGH_COMMAND_TOPIC);
            mqttClient.subscribe(MQTT_CALIBRATE_LOW_COMMAND_TOPIC);

            mqttClient.publish(MQTT_AVAILABILITY_TOPIC, MQTT_PAYLOAD_AVAILABLE);
        } else {
#ifdef DEBUG
            Serial.print("failed with state ");
            Serial.println(mqttClient.state());
#endif
            delay(2000);
        }
    }
}

void loop() {
    mqttClient.loop();

    if (Serial.available() > 0) {
        String data;
        while (Serial.available() > 0) {
            data += (char) Serial.read();
        }

#ifdef DEBUG
        Serial.print("received data from serial: ");
        Serial.println(data);
#endif

        char command = data.charAt(0);
        data.remove(0, 1);

        switch (command) {
            case COMMAND_TILT:
#ifdef DEBUG
                Serial.print("publishing: ");
                Serial.print(data.c_str());
                Serial.println(" -> " MQTT_TILT_STATUS_TOPIC);
#endif
                mqttClient.publish(MQTT_TILT_STATUS_TOPIC, data.c_str());
                break;
            case COMMAND_CALIBRATE_HIGH:
#ifdef DEBUG
                Serial.print("publishing: ");
                Serial.print(data.c_str());
                Serial.println(" -> " MQTT_CALIBRATE_HIGH_STATUS_TOPIC);
#endif
                mqttClient.publish(MQTT_CALIBRATE_HIGH_STATUS_TOPIC, data.c_str());
                break;
            case COMMAND_CALIBRATE_LOW:
#ifdef DEBUG
                Serial.print("publishing: ");
                Serial.print(data.c_str());
                Serial.println(" -> " MQTT_CALIBRATE_LOW_STATUS_TOPIC);
#endif
                mqttClient.publish(MQTT_CALIBRATE_LOW_STATUS_TOPIC, data.c_str());
                break;
            default:
                break;
        }

        delay(100);
    }
}


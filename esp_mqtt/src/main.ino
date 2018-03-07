/*
    It's finally here!
    Edit by f0x, original code from Fastled, Sebastius, Juerd
*/

#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient/releases/tag/2.4

void onMqttMessage(char* topic, byte * payload, unsigned int length);
boolean reconnect();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define PULSE_PIN D2

// WiFi settings
char ssid[] = "revspace-pub-2.4ghz";    //    your network SSID (name)
char pass[] = "";             // your network password

// MQTT Server settings and preparations
const char* mqtt_server = "mosquitto.space.revspace.nl";
WiFiClient espClient;

PubSubClient client(mqtt_server, 1883, onMqttMessage, espClient);
long lastReconnectAttempt = 0;

void setup() {
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    Serial.println();
    Serial.println(ESP.getChipId());
    pinMode(PULSE_PIN, OUTPUT);

    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }
    Serial.println("");

    Serial.print("WiFi connected");
    Serial.println(ssid);

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

boolean reconnect() {
    if (client.connect("oreo-counter")) {
        // Once connected, publish an announcement...
        client.publish("f0x/oreo-counter", "online");
        // ... and resubscribe
        client.subscribe("revspace/revbank/sale");
        client.subscribe("revspace/revbank/track");
        client.loop();
    }
    return client.connected();
}

char productToTrack[50] = "";

void onMqttMessage(char* topic, byte * payload, unsigned int length) {
    char bericht[50] = "";

    Serial.print("received topic: ");
    Serial.println(topic);
    Serial.print("length: ");
    Serial.println(length);
    Serial.print("payload: ");
    for (uint8_t pos = 0; pos < length; pos++) {
        bericht[pos] = payload[pos];
    }
    Serial.println(bericht);
    Serial.println();

    if (strcmp(topic, "revspace/revbank/sale") == 0) {
        Serial.println("Checking for an Oreo sale");

        if (strcmp(bericht, productToTrack) == 0) {
            Serial.println("An Oreo was sold!");

            digitalWrite(PULSE_PIN, HIGH);
            delay(10);
            digitalWrite(PULSE_PIN, LOW);
        } else {
            Serial.println("It wasn't an Oreo...");
        }
    } else if (strcmp(topic, "revspace/revbank/track") == 0) {
        Serial.println("Registering a new product to track...");
        Serial.println(bericht);

        for (uint8_t pos = 0; pos < 50; pos++) {
            productToTrack[pos] = bericht[pos];
        }
    }
}

void loop() {
    if (!client.connected()) {
        long verstreken = millis();
        if (verstreken - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = verstreken;
            // Attempt to reconnect
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        // Client connected
        client.loop();
    }
}

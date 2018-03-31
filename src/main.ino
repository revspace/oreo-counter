#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient/releases/tag/2.4
#include "LedControl.h"

LedControl lc=LedControl(D7, D6, D5, 1);
void onMqttMessage(char* topic, byte * payload, unsigned int length);
boolean reconnect();

// WiFi settings
char ssid[] = "revspace-pub-2.4ghz";    //    your network SSID (name)
char pass[] = "";             // your network password

// MQTT Server settings and preparations
const char* mqtt_server = "mosquitto.space.revspace.nl";
int counter = 0;
int koekjes = 0;
int ijsjes = 0;
WiFiClient espClient;

PubSubClient client(mqtt_server, 1883, onMqttMessage, espClient);
long lastReconnectAttempt = 0;

void setup() {
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    Serial.println();
    Serial.println(ESP.getChipId());

    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, pass);

    lc.shutdown(0, false);
    lc.setScanLimit(0, 8);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);

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
        client.publish("f0x/oreo-counter", "online");
        client.subscribe("revspace/bank/841007331");
        client.subscribe("revspace/bank/40079930169221");
        client.loop();
    }
    return client.connected();
}

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

    if (strcmp(topic, "revspace/bank/4007993016922") == 0) {
      ijsjes = atoi(bericht);
    } else if (strcmp(topic, "revspace/bank/84100733") == 0) {
      koekjes = atoi(bericht);
    }

    int totaal = ijsjes + koekjes;

        //printNumber(atoi(bericht));
    lc.setDigit(0,0,totaal%10,false);
    lc.setDigit(0,1,int(totaal/10)%10,false);
    lc.setDigit(0,2,int(totaal/100)%10,false);
    lc.setDigit(0,3,int(totaal/1000)%10,false);
    Serial.println("An Oreo was sold!");
    Serial.println(totaal);
     
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

//void printNumber(int v) {  
//    int ones;  
//    int tens;  
//    int hundreds; 
//
//    boolean negative=false;
//
//    if(v < -999 || v > 999)  
//        return;  
//    if(v<0) {  
//        negative=true; 
//        v=v*-1;  
//    }
//    ones=v%10;  
//    v=v/10;  
//    tens=v%10;  
//    v=v/10; hundreds=v;  
//    if(negative) {  
//        lc.setChar(0,3,'-',false);  } 
//    else {
//        lc.setChar(0,3,' ',false);  
//    }  
//    lc.setDigit(0,2,(byte)hundreds,false);
//    lc.setDigit(0,1,(byte)tens,false); 
//    lc.setDigit(0,0,(byte)ones,false); 
//} 

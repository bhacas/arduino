#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

// ethernet interface mac address, must be unique on the LAN
static byte mac[] = { 0x74,0x69,0x69,0x2D,0x31,0x10 };
IPAddress server(192, 168, 1, 10);
EthernetClient ethClient;
PubSubClient client(ethClient);

#define SERIAL 0
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)home/gabinet

#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define WATER1PIN 5
int water1 = 0;

#define WATER2PIN 6
int water2 = 0;

unsigned long loopTime = 0;
unsigned long lastReply = 0;
unsigned long lastReconnectAttempt = 0;

void setup () {
  Serial.begin(9600);
  pinMode(13, OUTPUT); 
  digitalWrite(13, HIGH);

  pinMode(WATER1PIN, OUTPUT);
  digitalWrite(WATER1PIN, HIGH);
  pinMode(WATER2PIN, OUTPUT);
  digitalWrite(WATER2PIN, HIGH);
  
  client.setServer(server, 1883);
  client.setCallback(callback);
  
  Ethernet.begin(mac);
  delay(1500);
}     

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  
  if (topicStr == "home/kotlownia/water1/set") {
    if ((char)payload[0] == '0') {
      switchOutput("water1", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("water1", 1, true);
    }
  } else if (topicStr == "home/kotlownia/water2/set") {
    if ((char)payload[0] == '0') {
      switchOutput("water2", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("water2", 1, true);
    }
  }
}

boolean reconnect() {
  if (client.connect("kotlownia")) {
    publishFloat("home/kotlownia/water1", water1);
    publishFloat("home/kotlownia/water2", water2);
    client.subscribe("home/kotlownia/water1/set");
    client.subscribe("home/kotlownia/water2/set");
  }
  return client.connected();
}

void loop () {
  unsigned long tLoopTime = micros();
  
  if (!client.connected()) {
    if ((millis() - lastReconnectAttempt) > 10000) {
      lastReconnectAttempt = millis();
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  
  Ethernet.maintain();
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
  
  loopTime = micros() - tLoopTime;
}

void switchOutput(String type, float value, boolean doNotifyMaster) {
  #if SERIAL
    Serial.println(type);
#endif
  if (type == "water1") {
    if (value == 1) {
      water1 = 1;
      digitalWrite(WATER1PIN, LOW);
    } 
    else {
      water1 = 0;
      digitalWrite(WATER1PIN, HIGH);
    }
    publishFloat("home/kotlownia/water1", water1); 
  }
  if (type == "water2") {
    if (value == 1) {
      water2 = 1;
      digitalWrite(WATER2PIN, LOW);
    } 
    else {
      water2 = 0;
      digitalWrite(WATER2PIN, HIGH);
    }
    publishFloat("home/kotlownia/water2", water2); 
  }
}

void publishFloat(char* topic, float val)
{
  if (client.connected()) {
    char str[16];
    itoa(val, str, 10);
    client.publish(topic, str);
  }
}

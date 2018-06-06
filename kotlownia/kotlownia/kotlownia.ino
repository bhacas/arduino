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
int switchWater1 = 0;
int water1activeOutput = 1;
int water1setOutput = 1;
unsigned long water1switchTime = 0;

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
    if (water1activeOutput == water1setOutput) {
    if ((char)payload[0] == '0') {
      turnOffWater1Active();
    } else {
      water1setOutput = payload[0]-'0';
      switchWater1 = 1;
    } 
    }
  } else if (topicStr == "home/kotlownia/water2/set") {
    if ((char)payload[0] == '0') {
      turnOffWater2();
    } else {
      turnOnWater2();
    } 
  }
}

boolean reconnect() {
  if (client.connect("kotlownia")) {
    publishFloat("home/kotlownia/water1", water1 ? water1activeOutput : 0);
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
  
  water1OutputChanger();
  
  Ethernet.maintain();
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
  
  loopTime = micros() - tLoopTime;
}

void turnOnWater1() {
  water1 = 1;
  digitalWrite(WATER1PIN, LOW);
  publishFloat("home/kotlownia/water1", water1activeOutput);
}

void turnOnWater2() {
  water2 = 1;
  digitalWrite(WATER2PIN, LOW);
  publishFloat("home/kotlownia/water2", water2);
}

void turnOffWater1() {
  water1 = 0;
  digitalWrite(WATER1PIN, HIGH);
  if (water1activeOutput == 5) {
    water1activeOutput = 1;
  } else {
    water1activeOutput += 1;
  }
  publishFloat("home/kotlownia/water1", 0);
}

void turnOffWater2() {
  water2 = 0;
  digitalWrite(WATER2PIN, HIGH);
  publishFloat("home/kotlownia/water2", 0);
}

void turnOffWater1Active() {
  water1 = 0;
  digitalWrite(WATER1PIN, HIGH);
  if (water1activeOutput == 5) {
    water1activeOutput = 1;
  } else {
    water1activeOutput += 1;
  }
  water1setOutput = water1activeOutput;
  publishFloat("home/kotlownia/water1", 0);
}

void water1OutputChanger()
{
  if (water1activeOutput != water1setOutput) {
      if (((millis() - water1switchTime) > 15000) && water1 == 0) {
        turnOnWater1();
        water1switchTime = millis();
      } else if (((millis() - water1switchTime) > 3000) && water1 == 1) { {
        turnOffWater1();
        water1switchTime = millis();
      }
    }
    switchWater1 = 1;
  } else {
    if (water1 == 0 && switchWater1 == 1 && ((millis() - water1switchTime) > 15000)) {
      turnOnWater1();
      switchWater1 = 0;
    }
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

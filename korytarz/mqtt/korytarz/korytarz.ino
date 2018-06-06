#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

// ethernet interface mac addkotlowniaress, must be unique on the LAN
static byte mac[] = { 0x74,0x69,0x69,0x2D,0x31,0x07 };
IPAddress server(192, 168, 1, 10);
EthernetClient ethClient;
PubSubClient client(ethClient);

#define SERIAL 0
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)home/kotlownia

#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SWITCH1PIN 1
int switch1 = 1;
int switch1tmp = 0;
#define SWITCH2PIN 2
int switch2 = 0;
int switch2tmp = 0;
#define SWITCH3PIN 3
int switch3 = 0;
int switch3tmp = 0;
#define SWITCH4PIN 5
int switch4 = 0;
int switch4tmp = 0;
#define SWITCH5PIN 6
int switch5 = 0;
int switch5tmp = 0;
#define SWITCH6PIN 7
int switch6 = 0;
int switch6tmp = 0;
#define SWITCHBRAMAPIN A4
int switchBrama = 0;
int switchBramaTmp = 0;

#define LIGHT1PIN 8
int light1 = 0;
#define LIGHT2PIN 9
int light2 = 0;
#define LIGHT3PIN 11
int light3 = 0;
#define LIGHT4PIN 12
int light4 = 0;
#define LIGHT5PIN A5
int light5 = 0;
#define OUTBRAMAPIN A3
int outBrama = 0;
unsigned long outBramaTime = 0;
#define OUTFURTKAPIN A2

#define RESETPIN A0

int outFurtka = 0;
unsigned long outFurtkaTime = 0;

unsigned long loopTime = 0;
unsigned long switchesCheckedTime = 0;
unsigned long tempCheckedTime = 0;
unsigned long lastReply = 0;
unsigned long lastReconnectAttempt = 0;


float temp;
float humidity;

void setup () {
  Serial.begin(9600);
  dht.begin();
  pinMode(13, OUTPUT); 
  digitalWrite(13, HIGH);

pinMode(LIGHT1PIN, OUTPUT);
  pinMode(LIGHT2PIN, OUTPUT);
  pinMode(LIGHT3PIN, OUTPUT);
  pinMode(LIGHT4PIN, OUTPUT);
  pinMode(LIGHT5PIN, OUTPUT);
  pinMode(OUTBRAMAPIN, OUTPUT);
  pinMode(OUTFURTKAPIN, OUTPUT);
  pinMode(SWITCH1PIN, INPUT_PULLUP);
  pinMode(SWITCH2PIN, INPUT_PULLUP);
  pinMode(SWITCH3PIN, INPUT_PULLUP);
  pinMode(SWITCH4PIN, INPUT_PULLUP);
  pinMode(SWITCH5PIN, INPUT_PULLUP);
  pinMode(SWITCH6PIN, INPUT_PULLUP);
  pinMode(SWITCHBRAMAPIN, INPUT_PULLUP);
  digitalWrite(LIGHT1PIN, HIGH);
  digitalWrite(LIGHT2PIN, HIGH);
  digitalWrite(LIGHT3PIN, HIGH);
  digitalWrite(LIGHT4PIN, HIGH);
  digitalWrite(LIGHT5PIN, HIGH);
  digitalWrite(OUTBRAMAPIN, HIGH);
  digitalWrite(OUTFURTKAPIN, HIGH);

  switch1 = digitalRead(SWITCH1PIN);
  switch2 = digitalRead(SWITCH2PIN);
  switch3 = digitalRead(SWITCH3PIN);
  switch4 = digitalRead(SWITCH4PIN);
  switch5 = digitalRead(SWITCH5PIN);
  switch6 = digitalRead(SWITCH6PIN);
  switchBrama = digitalRead(SWITCHBRAMAPIN);
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  delay(1500);
  
  Ethernet.begin(mac);
}     

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
    
  if (topicStr == "home/kotlownia/light1/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light1", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light1", 1, true);
    } 
  } else if (topicStr == "home/kotlownia/light2/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light2", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light2", 1, true);
    }
  } else if (topicStr == "home/kotlownia/light3/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light3", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light3", 1, true);
    }
  } else if (topicStr == "home/kotlownia/light4/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light4", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light4", 1, true);
    }
  } else if (topicStr == "home/kotlownia/light5/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light5", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light5", 1, true);
    }
  } else if (topicStr == "home/kotlownia/brama/set") {
    if ((char)payload[0] == '0') {
      switchOutput("brama", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("brama", 1, true);
    }
  } else if (topicStr == "home/kotlownia/furtka/set") {
    if ((char)payload[0] == '0') {
      switchOutput("furtka", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("furtka", 1, true);
    }
  }
}

boolean reconnect() {
  if (client.connect("kotlownia")) {
    publishFloat("home/kotlownia/temp", (temp*10));
    publishFloat("home/kotlownia/humidity", humidity);
    publishFloat("home/kotlownia/light1", light1);
    publishFloat("home/kotlownia/light2", light2);
    publishFloat("home/kotlownia/light3", light3);
    publishFloat("home/kotlownia/light4", light4);
    publishFloat("home/kotlownia/light5", light5);
    publishFloat("home/kotlownia/brama", outBrama);
    publishFloat("home/kotlownia/furtka", outFurtka);
    client.subscribe("home/kotlownia/light1/set");
    client.subscribe("home/kotlownia/light2/set");
    client.subscribe("home/kotlownia/light3/set");
    client.subscribe("home/kotlownia/light4/set");
    client.subscribe("home/kotlownia/light5/set");
    client.subscribe("home/kotlownia/brama/set");
    client.subscribe("home/kotlownia/furtka/set");
  }
  return client.connected();
}

void loop () {
  unsigned long tLoopTime = micros();
  
  if (!client.connected()) {
    if (millis() - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = millis();
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  
  switchesAndLights();
  readTemp();
  
  Ethernet.maintain();
  
  if (outFurtka == 1 && (millis() - outFurtkaTime) > 1000) {
    digitalWrite(OUTFURTKAPIN, HIGH);
    outFurtka = 0;
    outFurtkaTime = 0;
    publishFloat("home/kotlownia/furtka", outFurtka); 
  }
  
  if (outBrama == 1 && (millis() - outBramaTime) > 1000) {
    digitalWrite(OUTBRAMAPIN, HIGH);
    outBrama = 0;
    outBramaTime = 0;
    publishFloat("home/kotlownia/brama", outBrama); 
  }
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
  
  loopTime = micros() - tLoopTime;
}

void readTemp()
{
  if (abs(temp - dht.readTemperature()) >= 0.2) {
      temp = dht.readTemperature();
      publishFloat("home/kotlownia/temp", (temp * 10));
  }
  if (abs(humidity - dht.readHumidity()) >= 2) {
      humidity = dht.readHumidity();
      publishFloat("home/kotlownia/humidity", humidity);
  }
}

void switchesAndLights()
{
  if (millis() - switchesCheckedTime > 100) {
    
        //switch1
    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 1) {
      toggleLight1();
      switch1 = digitalRead(SWITCH1PIN);
      switch1tmp = 0;
    }

    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 0) {
      switch1tmp = 1;
    } 
    //end switch1
    
    //switch2
    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 1) {
      toggleLight2();
      switch2 = digitalRead(SWITCH2PIN);
      switch2tmp = 0;
    }

    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 0) {
      switch2tmp = 1;
    } 
    //end switch2
    
    //switch3
    if (digitalRead(SWITCH3PIN) != switch3 && switch3tmp == 1) {
      toggleLight3();
      switch3 = digitalRead(SWITCH3PIN);
      switch3tmp = 0;
    }

    if (digitalRead(SWITCH3PIN) != switch3 && switch3tmp == 0) {
      switch3tmp = 1;
    } 
    //end switch3
    
    //switch4
    if (digitalRead(SWITCH4PIN) != switch4 && switch4tmp == 1) {
      toggleLight4();
      switch4 = digitalRead(SWITCH4PIN);
      switch4tmp = 0;
    }

    if (digitalRead(SWITCH4PIN) != switch4 && switch4tmp == 0) {
      switch4tmp = 1;
    } 
    //end switch4
    
    //switch5
    if (digitalRead(SWITCH5PIN) != switch5 && switch5tmp == 1) {
      toggleLight5();
      switch5 = digitalRead(SWITCH5PIN);
      switch5tmp = 0;
    }

    if (digitalRead(SWITCH5PIN) != switch5 && switch5tmp == 0) {
      switch5tmp = 1;
    } 
    //end switch5
    
    //switch6
    if (digitalRead(SWITCH6PIN) != switch6 && switch6tmp == 1) {
      toggleLight5();
      switch6 = digitalRead(SWITCH6PIN);
      switch6tmp = 0;
    }

    if (digitalRead(SWITCH6PIN) != switch6 && switch6tmp == 0) {
      switch6tmp = 1;
    } 
    //end switch6
    
    //brama
    if (digitalRead(SWITCHBRAMAPIN) == LOW && switchBrama == 1) {
      switchOutput("brama", 1, true);
      switchBrama = 0;
    }
    if (digitalRead(SWITCHBRAMAPIN) == HIGH && switchBrama == 0) {
      switchBrama = 1;
    }
    //end brama

    switchesCheckedTime = millis();
      #if SERIAL
    Serial.println(switchesCheckedTime);
#endif
  }
}

void toggleLight1() {
  int state = !light1;
  switchOutput("light1", state, true);
}

void toggleLight2() {
  int state = !light2;
  switchOutput("light2", state, true);
}

void toggleLight3() {
  int state = !light3;
  switchOutput("light3", state, true);
}

void toggleLight4() {
  int state = !light4;
  switchOutput("light4", state, true);
}

void toggleLight5() {
  int state = !light5;
  switchOutput("light5", state, true);
}

void switchOutput(String type, float value, boolean doNotifyMaster) {
  #if SERIAL
    Serial.println(type);
#endif
  if (type == "light1") {
    if (value == 1) {
      light1 = 1;
      digitalWrite(LIGHT1PIN, LOW);
    } 
    else {
      light1 = 0;
      digitalWrite(LIGHT1PIN, HIGH);
    }
    publishFloat("home/kotlownia/light1", light1); 
  }
  if (type == "light2") {
    if (value == 1) {
      light2 = 1;
      digitalWrite(LIGHT2PIN, LOW);
    } 
    else {
      light2 = 0;
      digitalWrite(LIGHT2PIN, HIGH);
    }
    publishFloat("home/kotlownia/light2", light2); 
  }
  if (type == "light3") {
    if (value == 1) {
      light3 = 1;
      digitalWrite(LIGHT3PIN, LOW);
    } 
    else {
      light3 = 0;
      digitalWrite(LIGHT3PIN, HIGH);
    }
    publishFloat("home/kotlownia/light3", light3); 
  }
  if (type == "light4") {
    if (value == 1) {
      light4 = 1;
      digitalWrite(LIGHT4PIN, LOW);
    } 
    else {
      light4 = 0;
      digitalWrite(LIGHT4PIN, HIGH);
    }
    publishFloat("home/kotlownia/light4", light4); 
  }
  if (type == "light5") {
    if (value == 1) {
      light5 = 1;
      digitalWrite(LIGHT5PIN, LOW);
    } 
    else {
      light5 = 0;
      digitalWrite(LIGHT5PIN, HIGH);
    }
    publishFloat("home/kotlownia/light5", light5); 
  }
  
  if (type == "brama") {
      outBrama = 1;
      outBramaTime = millis();
      digitalWrite(OUTBRAMAPIN, LOW);
      publishFloat("home/kotlownia/brama", outBrama); 
  }
  
  
  
  
  
  if (type == "furtka") {
      outFurtka = 1;
      outFurtkaTime = millis();
      digitalWrite(OUTFURTKAPIN, LOW);
      publishFloat("home/kotlownia/furtka", outFurtka);
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

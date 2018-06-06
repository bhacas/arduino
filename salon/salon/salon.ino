#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

// ethernet interface mac address, must be unique on the LAN
static byte mac[] = { 0x74,0x69,0x69,0x2D,0x31,0x09 };
IPAddress server(192, 168, 1, 10);
EthernetClient ethClient;
PubSubClient client(ethClient);

#define SERIAL 0
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)home/salon

#define DHTPIN 0
#define DHT2PIN 1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);

#define SWITCH1PIN 2
int switch1 = 1;
int switch1tmp = 0;
#define SWITCH2PIN 3
int switch2 = 0;
int switch2tmp = 0;
#define SWITCH3PIN 5
int switch3 = 0;
int switch3tmp = 0;
#define SWITCH4PIN 6
int switch4 = 0;
int switch4tmp = 0;
#define SWITCH5PIN 7
int switch5 = 0;
int switch5tmp = 0;
#define SWITCH6PIN 8
int switch6 = 0;
int switch6tmp = 0;
#define SWITCH7PIN 9
int switch7 = 0;
#define SWITCH8PIN 11
int switch8 = 0;
#define SWITCH9PIN 12
int switch9 = 0;
#define SWITCH10PIN 14
int switch10 = 0;
#define SWITCH11PIN 15
int switch11 = 0;
#define SWITCH12PIN 16
int switch12 = 0;
#define CONTACTRON1PIN 17
int contactron1 = 0;
#define CONTACTRON2PIN 18
int contactron2 = 0;
#define CONTACTRON3PIN 19
int contactron3 = 0;
#define CONTACTRON4PIN 20
int contactron4 = 0;
#define CONTACTRON5PIN 21
int contactron5 = 0;

#define LIGHT1PIN 22
int light1 = 0;
#define LIGHT2PIN 23
int light2 = 0;
#define LIGHT3PIN 24
int light3 = 0;

long currentBlind1State = 0;
long desiredBlind1State = 0;
long currentBlind2State = 0;
long desiredBlind2State = 0;
long currentBlind3State = 0;
long desiredBlind3State = 0;
unsigned long loopTime = 0;
int blind1State = 0;
#define BLIND1_UP 25
#define BLIND1_DOWN 26
#define BLIND1_MOVE_TIME 22000000 //5s
int blind2State = 0;
#define BLIND2_UP 27
#define BLIND2_DOWN 28
#define BLIND2_MOVE_TIME 22000000 //5s
int blind3State = 0;
#define BLIND3_UP 29
#define BLIND3_DOWN 30
#define BLIND3_MOVE_TIME 22000000 //5s

unsigned long switchesCheckedTime = 0;
unsigned long tempCheckedTime = 0;
unsigned long lastReply = 0;
unsigned long lastReconnectAttempt = 0;


float temp1;
float humidity1;

float temp2;
float humidity2;

void setup () {
  Serial.begin(9600);
  dht1.begin();
  dht2.begin();
  pinMode(13, OUTPUT); 
  digitalWrite(13, HIGH);

  pinMode(LIGHT1PIN, OUTPUT);
  pinMode(LIGHT2PIN, OUTPUT);
  pinMode(LIGHT3PIN, OUTPUT);
  pinMode(SWITCH1PIN, INPUT_PULLUP);
  pinMode(SWITCH2PIN, INPUT_PULLUP);
  pinMode(SWITCH3PIN, INPUT_PULLUP);
  pinMode(SWITCH4PIN, INPUT_PULLUP);
  pinMode(SWITCH5PIN, INPUT_PULLUP);
  pinMode(SWITCH6PIN, INPUT_PULLUP);
  pinMode(SWITCH7PIN, INPUT_PULLUP);
  pinMode(SWITCH8PIN, INPUT_PULLUP);
  pinMode(SWITCH9PIN, INPUT_PULLUP);
  pinMode(SWITCH10PIN, INPUT_PULLUP);
  pinMode(SWITCH11PIN, INPUT_PULLUP);
  pinMode(SWITCH12PIN, INPUT_PULLUP);
  pinMode(CONTACTRON1PIN, INPUT_PULLUP);
  pinMode(CONTACTRON2PIN, INPUT_PULLUP);
  pinMode(CONTACTRON3PIN, INPUT_PULLUP);
  pinMode(CONTACTRON4PIN, INPUT_PULLUP);
  pinMode(CONTACTRON5PIN, INPUT_PULLUP);
  pinMode(BLIND1_UP, OUTPUT);
  pinMode(BLIND1_DOWN, OUTPUT);
  pinMode(BLIND2_UP, OUTPUT);
  pinMode(BLIND2_DOWN, OUTPUT);
  pinMode(BLIND3_UP, OUTPUT);
  pinMode(BLIND3_DOWN, OUTPUT);
  digitalWrite(LIGHT1PIN, HIGH);
  digitalWrite(LIGHT2PIN, HIGH);
  digitalWrite(LIGHT3PIN, HIGH);
  digitalWrite(BLIND1_UP, HIGH);
  digitalWrite(BLIND1_DOWN, HIGH);
  digitalWrite(BLIND2_UP, HIGH);
  digitalWrite(BLIND2_DOWN, HIGH);
  digitalWrite(BLIND3_UP, HIGH);
  digitalWrite(BLIND3_DOWN, HIGH);

  switch1 = digitalRead(SWITCH1PIN);
  switch2 = digitalRead(SWITCH2PIN);
  switch3 = digitalRead(SWITCH3PIN);
  switch4 = digitalRead(SWITCH4PIN);
  switch5 = digitalRead(SWITCH5PIN);
  switch6 = digitalRead(SWITCH6PIN);
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);

  digitalWrite(BLIND1_DOWN, LOW);
  digitalWrite(BLIND2_DOWN, LOW);
  digitalWrite(BLIND3_DOWN, LOW);
  delay(round(BLIND1_MOVE_TIME / 1000));
  digitalWrite(BLIND1_DOWN, HIGH);
  digitalWrite(BLIND2_DOWN, HIGH);
  digitalWrite(BLIND3_DOWN, HIGH);
}     

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  
  if (topicStr == "home/salon/light1/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light1", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light1", 1, true);
    } 
  } else if (topicStr == "home/salon/light2/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light2", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light2", 1, true);
    }
  } else if (topicStr == "home/salon/light3/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light3", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light3", 1, true);
    }
  } else if (topicStr == "home/salon/blind1/set") {
    if ((char)payload[0] == '0') {
      switchOutput("blind1", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("blind1", 1, true);
    } else if ((char)payload[0] == '2') {
      switchOutput("blind1", 2, true);
    }
  } else if (topicStr == "home/salon/blind1/position") {
    String str = String((char *)payload);
    String substr = str.substring(0, length);
    float payloadFloat = substr.toInt();
    desiredBlind1State = BLIND1_MOVE_TIME * payloadFloat / 100;
  } else if (topicStr == "home/salon/blind2/set") {
    if ((char)payload[0] == '0') {
      switchOutput("blind2", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("blind2", 1, true);
    } else if ((char)payload[0] == '2') {
      switchOutput("blind2", 2, true);
    }
  } else if (topicStr == "home/salon/blind2/position") {
    String str = String((char *)payload);
    String substr = str.substring(0, length);
    float payloadFloat = substr.toInt();
    desiredBlind2State = BLIND2_MOVE_TIME * payloadFloat / 100;
  } else if (topicStr == "home/salon/blind1/position") {
    String str = String((char *)payload);
    String substr = str.substring(0, length);
    float payloadFloat = substr.toInt();
    desiredBlind1State = BLIND1_MOVE_TIME * payloadFloat / 100;
  } else if (topicStr == "home/salon/blind3/set") {
    if ((char)payload[0] == '0') {
      switchOutput("blind3", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("blind3", 1, true);
    } else if ((char)payload[0] == '2') {
      switchOutput("blind3", 2, true);
    }
  } else if (topicStr == "home/salon/blind3/position") {
    String str = String((char *)payload);
    String substr = str.substring(0, length);
    float payloadFloat = substr.toInt();
    desiredBlind3State = BLIND3_MOVE_TIME * payloadFloat / 100;
  }
}

boolean reconnect() {
  if (client.connect("salon")) {
    publishFloat("home/salon/temp1", temp1);
    publishFloat("home/salon/humidity1", humidity1);
    publishFloat("home/salon/temp2", temp2);
    publishFloat("home/salon/humidity2", humidity2);
    publishFloat("home/salon/light1", light1);
    publishFloat("home/salon/light2", light2);
    publishFloat("home/salon/light3", light3);
    publishFloat("home/salon/blind1", blind1State);
    publishFloat("home/salon/blind2", blind2State);
    publishFloat("home/salon/blind3", blind3State);
    publishFloat("home/salon/contactron1", contactron1);
    publishFloat("home/salon/contactron2", contactron2);
    publishFloat("home/salon/contactron3", contactron3);
    publishFloat("home/salon/contactron4", contactron4);
    publishFloat("home/salon/contactron5", contactron5);
    client.subscribe("home/salon/light1/set");
    client.subscribe("home/salon/light2/set");
    client.subscribe("home/salon/light3/set");
    client.subscribe("home/salon/blind1/set");
    client.subscribe("home/salon/blind2/set");
    client.subscribe("home/salon/blind3/set");
    client.subscribe("home/salon/blind1/position");
    client.subscribe("home/salon/blind2/position");
    client.subscribe("home/salon/blind3/position");
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
  blindMove();
  readTemp1();
  readTemp2();
  
  loopTime = micros() - tLoopTime;
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
}

void readTemp()
{
  if (abs(temp1 - dht1.readTemperature()) >= 0.2) {
      temp1 = dht1.readTemperature();
      publishFloat("home/salon/temp1", (temp1*10));
  }
  if (abs(humidity1 - dht2.readHumidity()) >= 2) {
      humidity1 = dht1.readHumidity();
      publishFloat("home/salon/humidity1", humidity1);
  }
  if (abs(temp2 - dht2.readTemperature()) >= 0.2) {
      temp2 = dht2.readTemperature();
      publishFloat("home/salon/temp2", (temp2*10));
  }
  if (abs(humidity2 - dht2.readHumidity()) >= 2) {
      humidity2 = dht2.readHumidity();
      publishFloat("home/salon/humidity2", humidity2);
  }
}

void switchesAndLights()
{
  if (millis() - switchesCheckedTime > 100) {
    
    // switch1
    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 1) {
      toggleLight1();
      switch1 = digitalRead(SWITCH1PIN);
      switch1tmp = 0;
    }

    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 0) {
      switch1tmp = 1;
    }
    // end switch1
    
    // switch2
    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 1) {
      toggleLight2();
      switch2 = digitalRead(SWITCH2PIN);
      switch2tmp = 0;
    }

    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 0) {
      switch2tmp = 1;
    }
    // end switch2
    
    // switch2
    if (digitalRead(SWITCH3PIN) != switch3 && switch3tmp == 1) {
      toggleLight3();
      switch3 = digitalRead(SWITCH3PIN);
      switch3tmp = 0;
    }

    if (digitalRead(SWITCH3PIN) != switch3 && switch3tmp == 0) {
      switch3tmp = 1;
    }
    // end switch2
    
    // blind1 switch1
    if (digitalRead(SWITCH7PIN) == HIGH && switch7 == 1) {
      if (round(currentBlind1State / 1000) == round(desiredBlind1State / 1000)) {
          switchOutput("blind1", 1, true);
      } else {
        switchOutput("blind1", 2, true);
      }
      switch7 = 0;
    }
    if (digitalRead(SWITCH7PIN) == LOW && switch7 == 0) {
      switch7 = 1;
    }
    // end blind1 switch1

    // blind1 switch2
    if (digitalRead(SWITCH8PIN) == HIGH && switch8 == 1) {
      if (round(currentBlind1State / 1000) == round(desiredBlind1State / 1000)) {
          switchOutput("blind1", 0, true);
      } else {
        switchOutput("blind1", 2, true);
      }
      switch48 = 0;
    }

    if (digitalRead(SWITCH8PIN) == LOW && switch8 == 0  ) {
      switch8 = 1;
    } 
    // end blind1 switch2
    
    // blind2 switch1
    if (digitalRead(SWITCH9PIN) == HIGH && switch9 == 1) {
      if (round(currentBlind2State / 1000) == round(desiredBlind2State / 1000)) {
          switchOutput("blind2", 1, true);
      } else {
        switchOutput("blind2", 2, true);
      }
      switch9 = 0;
    }
    if (digitalRead(SWITCH9PIN) == LOW && switch9 == 0) {
      switch7 = 1;
    }
    // end blind2 switch1

    // blind2 switch2
    if (digitalRead(SWITCH10PIN) == HIGH && switch10 == 1) {
      if (round(currentBlind2State / 1000) == round(desiredBlind2State / 1000)) {
          switchOutput("blind2", 0, true);
      } else {
        switchOutput("blind2", 2, true);
      }
      switch10 = 0;
    }

    if (digitalRead(SWITCH10PIN) == LOW && switch10 == 0  ) {
      switch10 = 1;
    } 
    // end blind2 switch2
    
    // blind3 switch1
    if (digitalRead(SWITCH11PIN) == HIGH && switch11 == 1) {
      if (round(currentBlind3State / 1000) == round(desiredBlind3State / 1000)) {
          switchOutput("blind3", 1, true);
      } else {
        switchOutput("blind3", 2, true);
      }
      switch9 = 0;
    }
    if (digitalRead(SWITCH11PIN) == LOW && switch11 == 0) {
      switch11 = 1;
    }
    // end blind3 switch1

    // blind3 switch2
    if (digitalRead(SWITCH12PIN) == HIGH && switch12 == 1) {
      if (round(currentBlind3State / 1000) == round(desiredBlind3State / 1000)) {
          switchOutput("blind3", 0, true);
      } else {
        switchOutput("blind3", 2, true);
      }
      switch12 = 0;
    }

    if (digitalRead(SWITCH12PIN) == LOW && switch12 == 0  ) {
      switch12 = 1;
    } 
    // end blind3 switch2

    if (digitalRead(CONTACTRON1PIN) == HIGH && contactron1 == 1) {
      contactron1 = 0;
      publishFloat("home/salon/contactron1", contactron1);
    }
    if (digitalRead(CONTACTRON1PIN) == LOW && contactron1 == 0) {
      contactron1 = 1; 
      publishFloat("home/salon/contactron1", contactron1);
    }

    if (digitalRead(CONTACTRON2PIN) == HIGH && contactron2 == 1) {
      contactron2 = 0;
      publishFloat("home/salon/contactron2", contactron2);
    }
    if (digitalRead(CONTACTRON2PIN) == LOW && contactron2 == 0) {
      contactron2 = 1;
      publishFloat("home/salon/contactron2", contactron2);
    }

    if (digitalRead(CONTACTRON3PIN) == HIGH && contactron3 == 1) {
      contactron3 = 0;
      publishFloat("home/salon/contactron3", contactron3);
    }
    if (digitalRead(CONTACTRON3PIN) == LOW && contactron3 == 0) {
      contactron3 = 1;
      publishFloat("home/salon/contactron3", contactron3);
    }
    
    if (digitalRead(CONTACTRON4PIN) == HIGH && contactron4 == 1) {
      contactron4 = 0;
      publishFloat("home/salon/contactron4", contactron4);
    }
    if (digitalRead(CONTACTRON4PIN) == LOW && contactron4 == 0) {
      contactron4 = 1;
      publishFloat("home/salon/contactron4", contactron4);
    }
    
    if (digitalRead(CONTACTRON5PIN) == HIGH && contactron5 == 1) {
      contactron5 = 0;
      publishFloat("home/salon/contactron5", contactron5);
    }
    if (digitalRead(CONTACTRON5PIN) == LOW && contactron5 == 0) {
      contactron5 = 1;
      publishFloat("home/salon/contactron5", contactron5);
    }
    
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
    publishFloat("home/salon/light1", light1); 
  }
  if (type == "light2") {
    if (value == 1) {
      light2 = 1;
    } 
    else {
      light2 = 0;
    }
    publishFloat("home/salon/light2", light2); 
  }

  if (type == "blind") {
    if (value == 0) {
      desiredBlindState = 0;
    } else if (value == 1) {
        desiredBlindState = BLIND_MOVE_TIME;
    } else if (value == 2) {
      desiredBlindState = currentBlindState;
    }
  }
}

void blindMove()
{

  if (round(currentBlindState / 1000) != round(desiredBlindState / 1000)) {
    if (currentBlindState < desiredBlindState) {
      digitalWrite(BLIND_UP, LOW);
      currentBlindState = currentBlindState + loopTime;
      if (currentBlindState > BLIND_MOVE_TIME) {
        currentBlindState = BLIND_MOVE_TIME;
      }
    }
    else {
      digitalWrite(BLIND_DOWN, LOW);
      currentBlindState = currentBlindState - loopTime;
      if (currentBlindState < 0) {
        currentBlindState = 0;
      }
    }
  } 
  else {
    digitalWrite(BLIND_UP, HIGH);
    digitalWrite(BLIND_DOWN, HIGH);
    currentBlindState = desiredBlindState;
  }
  
  if (digitalRead(BLIND_UP) == LOW && digitalRead(BLIND_DOWN) == LOW) {
    digitalWrite(BLIND_UP, HIGH);
    digitalWrite(BLIND_DOWN, HIGH);
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

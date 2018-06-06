#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"

// ethernet interface mac address, must be unique on the LAN
static byte mac[] = { 0x74,0x69,0x69,0x2D,0x31,0x01 };
IPAddress server(192, 168, 1, 10);
EthernetClient ethClient;
PubSubClient client(ethClient);

#define SERIAL 0
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)home/gabinet

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
#define SWITCH4PIN 5
int switch4 = 0;
#define CONTACTRON1PIN 6
int contactron1 = 0;
#define CONTACTRON2PIN 7
int contactron2 = 0;
#define CONTACTRON3PIN 8
int contactron3 = 0;

#define LIGHT1PIN 9
int light1 = 0;
int light2 = 0;

long currentBlindState = 0;
long desiredBlindState = 0;
unsigned long loopTime = 0;
int blindState = 0;
#define BLIND_UP 11
#define BLIND_DOWN 12
#define BLIND_MOVE_TIME 22000000 //5s

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
  pinMode(SWITCH1PIN, INPUT_PULLUP);
  pinMode(SWITCH2PIN, INPUT_PULLUP);
  pinMode(SWITCH3PIN, INPUT_PULLUP);
  pinMode(SWITCH4PIN, INPUT_PULLUP);
  pinMode(CONTACTRON1PIN, INPUT_PULLUP);
  pinMode(CONTACTRON2PIN, INPUT_PULLUP);
  pinMode(CONTACTRON3PIN, INPUT_PULLUP);
  pinMode(BLIND_UP, OUTPUT);
  pinMode(BLIND_DOWN, OUTPUT);
  digitalWrite(LIGHT1PIN, HIGH);
  digitalWrite(BLIND_UP, HIGH);

  switch1 = digitalRead(SWITCH1PIN);
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac);

  digitalWrite(BLIND_DOWN, LOW);
  delay(round(BLIND_MOVE_TIME / 1000));
  digitalWrite(BLIND_DOWN, HIGH);
}     

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
    
  if (topicStr == "home/gabinet/light1/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light1", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light1", 1, true);
    } 
  } else if (topicStr == "home/gabinet/light2/set") {
    if ((char)payload[0] == '0') {
      switchOutput("light2", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("light2", 1, true);
    }
  } else if (topicStr == "home/gabinet/blind/set") {
    if ((char)payload[0] == '0') {
      switchOutput("blind", 0, true);
    } else if ((char)payload[0] == '1') {
      switchOutput("blind", 1, true);
    } else if ((char)payload[0] == '2') {
      switchOutput("blind", 2, true);
    }
  } else if (topicStr == "home/gabinet/blind/position") {
    char* payloadChar = (char *)malloc(length);
    for (int i=0;i<length;i++) {
      payloadChar[i] = (char)payload[i];
    }
    String str = String(payloadChar);
    String substr = str.substring(0, length);
    float payloadFloat = substr.toInt();
    free(payloadChar);
    
    Serial.println(payloadFloat);
    desiredBlindState = BLIND_MOVE_TIME * payloadFloat / 100;
    Serial.println(desiredBlindState);
  }
}

boolean reconnect() {
  if (client.connect("gabinet")) {
    publishFloat("home/gabinet/temp", temp);
    publishFloat("home/gabinet/humidity", humidity);
    publishFloat("home/gabinet/light1", light1);
    publishFloat("home/gabinet/switch2", switch2);
    publishFloat("home/gabinet/blind", blindState);
    publishFloat("home/gabinet/contactron1", contactron1);
    publishFloat("home/gabinet/contactron2", contactron2);
    publishFloat("home/gabinet/contactron3", contactron3);
    client.subscribe("home/gabinet/light1/set");
    client.subscribe("home/gabinet/light2/set");
    client.subscribe("home/gabinet/blind/set");
    client.subscribe("home/gabinet/blind/position");
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
  readTemp();
  
  loopTime = micros() - tLoopTime;
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
}

void readTemp()
{
  if (abs(temp - dht.readTemperature()) >= 0.2) {
      temp = dht.readTemperature();
      publishFloat("home/gabinet/temp", (temp*10));
  }
  if (abs(humidity - dht.readHumidity()) >= 2) {
      humidity = dht.readHumidity();
      publishFloat("home/gabinet/humidity", humidity);
  }
}

void switchesAndLights()
{
  if (millis() - switchesCheckedTime > 100) {
    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 1) {
      toggleLight1();
      switch1 = digitalRead(SWITCH1PIN);
      switch1tmp = 0;
    }

    if (digitalRead(SWITCH1PIN) != switch1 && switch1tmp == 0) {
      switch1tmp = 1;
    } 
    
    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 1) {
      toggleLight2();
      switch2 = digitalRead(SWITCH2PIN);
      switch2tmp = 0;
    }

    if (digitalRead(SWITCH2PIN) != switch2 && switch2tmp == 0) {
      switch2tmp = 1;
    } 

    if (digitalRead(SWITCH3PIN) == HIGH && switch3 == 1) {
      if (round(currentBlindState / 1000) == round(desiredBlindState / 1000)) {
          switchOutput("blind", 1, true);
      } else {
        switchOutput("blind", 2, true);
      }
      switch3 = 0;
    }
    if (digitalRead(SWITCH3PIN) == LOW && switch3 == 0) {
      switch3 = 1;
    }

    if (digitalRead(SWITCH4PIN) == HIGH && switch4 == 1) {
      if (round(currentBlindState / 1000) == round(desiredBlindState / 1000)) {
          switchOutput("blind", 0, true);
      } else {
        switchOutput("blind", 2, true);
      }
      switch4 = 0;
    }

    if (digitalRead(SWITCH4PIN) == LOW && switch4 == 0  ) {
      switch4 = 1;
    } 

    if (digitalRead(CONTACTRON1PIN) == HIGH && contactron1 == 1) {
      contactron1 = 0;
      publishFloat("home/gabinet/contactron1", contactron1);
    }
    if (digitalRead(CONTACTRON1PIN) == LOW && contactron1 == 0) {
      contactron1 = 1; 
      publishFloat("home/gabinet/contactron1", contactron1);
    }

    if (digitalRead(CONTACTRON2PIN) == HIGH && contactron2 == 1) {
      contactron2 = 0;
      publishFloat("home/gabinet/contactron2", contactron2);
    }
    if (digitalRead(CONTACTRON2PIN) == LOW && contactron2 == 0) {
      contactron2 = 1;
      publishFloat("home/gabinet/contactron2", contactron2);
    }

    if (digitalRead(CONTACTRON3PIN) == HIGH && contactron3 == 1) {
      contactron3 = 0;
      publishFloat("home/gabinet/contactron3", contactron3);
    }
    if (digitalRead(CONTACTRON3PIN) == LOW && contactron3 == 0) {
      contactron3 = 1;
      publishFloat("home/gabinet/contactron3", contactron3);
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
    publishFloat("home/gabinet/light1", light1); 
  }
  if (type == "light2") {
    if (value == 1) {
      light2 = 1;
    } 
    else {
      light2 = 0;
    }
    publishFloat("home/gabinet/light2", light2); 
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
  
  if (currentBlindState == 0 && blindState != 0) {
    publishFloat("home/gabinet/blind", 0);
    blindState = 0;
  } else if (currentBlindState == BLIND_MOVE_TIME && blindState != 1) {
    publishFloat("home/gabinet/blind", 1);
    blindState = 1;
  } else if (currentBlindState != 0 && currentBlindState != BLIND_MOVE_TIME && blindState != 2) {
    publishFloat("home/gabinet/blind", 2);
    blindState = 2;
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

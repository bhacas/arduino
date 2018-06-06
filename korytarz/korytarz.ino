#include <EtherCard.h>
#include "DHT.h"

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x06 };
static byte hisip[] = { 
  192, 168, 1, 105 }; // remote webserver
const char website[] PROGMEM = "192.168.1.105";

#define SERIAL 1
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)

#define DHTPIN 1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SWITCH1PIN 2
int switch1 = 1;
int switch1tmp = 0;
#define SWITCH2PIN 3
int switch2 = 0;
int switch2tmp = 0;
#define SWITCH3PIN 4
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

#define LIGHT1PIN 9
int light1 = 0;
#define LIGHT2PIN 10
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

unsigned long switchesCheckedTime = 0;
unsigned long tempCheckedTime = 0;
unsigned long lastReply = 0;

float temp;
float humidity;

int resets = 0;
int replies = 0;

byte Ethernet::buffer[500];
BufferFiller bfill;
bool isDhcp = false;

void setup () {
  digitalWrite(RESETPIN, HIGH);
  pinMode(RESETPIN, OUTPUT);
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

  connect();
}   

void connect() {
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup()) {
    isDhcp = false;
  } 
  else {
    isDhcp = true;
  }
}

static word homePage() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.0 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "{\"response\": {\"light1\": $D, \"light2\": $D, \"light3\": $D, \"light4\": $D, \"light5\": $D, \"brama\": $D, \"furtka\": $D, \"switch1\": $D, \"switch2\": $D, \"switch3\": $D, \"switch4\": $D, \"switch5\": $D, \"switch6\": $D, \"switchBrama\": $D, \"temperature\": $D, \"humidity\": $D, \"resets\": $D, \"replies\": $D}}"),
  light1, light2, light3, light4, light5, outBrama, outFurtka, switch1, switch2, switch3, switch4, switch5, switch6, switchBrama, (int)(temp*10), (int)humidity, resets, replies);

  return bfill.position();
}

void loop () {
  unsigned long tLoopTime = micros();
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) doHttpReply(pos);
 
  switchesAndLights();
  readTemp();
  
  if (millis() - lastReply > 300000) {
    lastReply = millis();
    resets++;
    digitalWrite(RESETPIN, LOW);
    delay(50);
    digitalWrite(RESETPIN, HIGH);
    connect();
  }
  
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
  
  if (outFurtka == 1 && (millis() - outFurtkaTime) > 1000) {
    digitalWrite(OUTFURTKAPIN, HIGH);
    outFurtka = 0;
    outFurtkaTime = 0;
  }
  
    if (outBrama == 1 && (millis() - outBramaTime) > 1000) {
    digitalWrite(OUTBRAMAPIN, HIGH);
    outBrama = 0;
    outBramaTime = 0;
  }
}

void readTemp()
{
  if ((millis() - tempCheckedTime > 900000 /*15min*/) && (abs(temp - dht.readTemperature()) >= 0.2 || abs(humidity - dht.readHumidity()) >= 2)) {
      humidity = dht.readHumidity();
      temp = dht.readTemperature();
      tempCheckedTime = millis();
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

void doHttpReply(word pos)
{

  char* data = (char *) Ethernet::buffer + pos;
  String dataStr = String(data);

  if (dataStr.startsWith("GET /?", 0) == true) {
    //Serial.println(strlen(resp));
    String type = dataStr.substring(dataStr.indexOf("type=") + 5,
    dataStr.indexOf("&"));
    String value = dataStr.substring(dataStr.indexOf("value=") + 6,
    dataStr.indexOf(" HTTP"));

    char buffer[10];
    value.toCharArray(buffer, 10);
    float valf = atof(buffer);
    switchOutput(type, valf, false);

    //memcpy_P(ether.tcpOffset(), pageOk, sizeof pageOk);
    ether.httpServerReply(homePage());
  } 
  else {
    //memcpy_P(ether.tcpOffset(), pageNotOk, sizeof pageNotOk);
    ether.httpServerReply(homePage());
  }
  
  lastReply = millis();
  replies++;

}

void switchOutput(String type, float value, boolean doNotifyMaster) {

    //Serial.println(type);
    

  if (type == "light1") {
    if (value == 1) {
      light1 = 1;
      digitalWrite(LIGHT1PIN, LOW);
    } 
    else {
      light1 = 0;
      digitalWrite(LIGHT1PIN, HIGH);
    }
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
  }
  
  if (type == "brama") {
      outBrama = 1;
      outBramaTime = millis();
      digitalWrite(OUTBRAMAPIN, LOW);
  }
  
  if (type == "furtka") {
      outFurtka = 1;
      outFurtkaTime = millis();
      digitalWrite(OUTFURTKAPIN, LOW);
  }
}

void notifyMaster() {
  if (isDhcp == true) {
    ether.copyIp(ether.hisip, hisip);
    ether.browseUrl(PSTR("/api/api.php"), "", website, my_callback);
  }
}

static void my_callback(byte status, word off, word len) {
}




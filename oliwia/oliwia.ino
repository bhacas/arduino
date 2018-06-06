#include <EtherCard.h>
#include "DHT.h"

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 
  0x74,0x69,0x69,0x2D,0x30,0x02 };
static byte hisip[] = { 
  192, 168, 1, 105 }; // remote webserver
const char website[] PROGMEM = "192.168.1.105";

#define SERIAL 0
#define STATIC 0 // set to 1 to disable DHCP (adjust myip/gwip values below)

#define DHTPIN 1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SWITCH1PIN 2
int switch1 = 1;
int switch1tmp = 0;
#define SWITCH2PIN 3
int switch2 = 0;
#define SWITCH3PIN 4
int switch3 = 0;
#define CONTACTRON1PIN 5
int contactron1 = 0;
#define CONTACTRON2PIN 6
int contactron2 = 0;
#define CONTACTRON3PIN 7
int contactron3 = 0;

#define LIGHT1PIN 9
int light1 = 0;

long currentBlindState = 0;
long desiredBlindState = 0;
unsigned long loopTime = 0;
#define BLIND_UP 11
#define BLIND_DOWN 12
#define BLIND_MOVE_TIME 22000000 //5s

#define RESETPIN A0

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
  pinMode(SWITCH1PIN, INPUT_PULLUP);
  pinMode(SWITCH2PIN, INPUT_PULLUP);
  pinMode(SWITCH3PIN, INPUT_PULLUP);
  pinMode(CONTACTRON1PIN, INPUT_PULLUP);
  pinMode(CONTACTRON2PIN, INPUT_PULLUP);
  pinMode(CONTACTRON3PIN, INPUT_PULLUP);
  pinMode(BLIND_UP, OUTPUT);
  pinMode(BLIND_DOWN, OUTPUT);
  digitalWrite(LIGHT1PIN, HIGH);
  digitalWrite(BLIND_UP, HIGH);

  switch1 = digitalRead(SWITCH1PIN);

  digitalWrite(BLIND_DOWN, LOW);
  delay(round(BLIND_MOVE_TIME / 1000));
  digitalWrite(BLIND_DOWN, HIGH);

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
    "{\"response\": {\"light1\": $D, \"blind\": $D, \"switch1\": $D, \"switch2\": $D, \"switch3\": $D, \"contactron1\": $D, \"contactron2\": $D, \"temperature\": $D, \"humidity\": $D, \"resets\": $D, \"replies\": $D}}"),
  light1, (int)(currentBlindState * 100 / BLIND_MOVE_TIME), switch1, switch2, switch3, contactron1, contactron2, (int)(temp*10), (int)humidity, resets, replies);

  return bfill.position();
}

void loop () {
  unsigned long tLoopTime = micros();
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) doHttpReply(pos);

  digitalWrite(13, HIGH); 
  switchesAndLights();
  blindMove();
  readTemp();
  
  if (millis() - lastReply > 300000) {
    lastReply = millis();
    resets++;
    digitalWrite(RESETPIN, LOW);
    delay(50);
    digitalWrite(RESETPIN, HIGH);
    connect();
  }
  
  loopTime = micros() - tLoopTime;
  if (((int)(millis() / 1000) % 2) == 0) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
}

void readTemp()
{
  if (abs(temp - dht.readTemperature()) >= 0.2 || abs(humidity - dht.readHumidity()) >= 2) {
      humidity = dht.readHumidity();
      temp = dht.readTemperature();
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

    if (digitalRead(SWITCH2PIN) == HIGH && switch2 == 1) {
      switchOutput("blind", 1, true);
      switch2 = 0;
    }
    if (digitalRead(SWITCH2PIN) == LOW && switch2 == 0) {
      switch2 = 1;
    }

    if (digitalRead(SWITCH3PIN) == HIGH && switch3 == 1) {
      switchOutput("blind", 0, true);
      switch3 = 0;
    }

    if (digitalRead(SWITCH3PIN) == LOW && switch3 == 0  ) {
      switch3 = 1;
    } 

    if (digitalRead(CONTACTRON1PIN) == HIGH && contactron1 == 1) {
      contactron1 = 0;
    }
    if (digitalRead(CONTACTRON1PIN) == LOW && contactron1 == 0) {
      contactron1 = 1; 
    }

    if (digitalRead(CONTACTRON2PIN) == HIGH && contactron2 == 1) {
      contactron2 = 0;
    }
    if (digitalRead(CONTACTRON2PIN) == LOW && contactron2 == 0) {
      contactron2 = 1; 
    }

    if (digitalRead(CONTACTRON3PIN) == HIGH && contactron3 == 1) {
      contactron3 = 0;
    }
    if (digitalRead(CONTACTRON3PIN) == LOW && contactron3 == 0) {
      contactron3 = 1; 
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
  }

  if (type == "blind") {
    if (value == 1) {
      if (round(currentBlindState / 1000) == round(desiredBlindState / 1000)) {
        desiredBlindState = BLIND_MOVE_TIME;
      } 
      else {
        desiredBlindState = currentBlindState;
      } 
    }
    else {
      if (round(currentBlindState / 1000) == round(desiredBlindState / 1000)) {
        desiredBlindState = 0;
      } 
      else {
        desiredBlindState = currentBlindState;
      } 
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

  /*
  if (blindUp == 1 && millis() - currentBlindTime < BLIND_MOVE_TIME) {
   digitalWrite(BLIND_UP, LOW);
   blindAlreadyMove = true;
   } 
   else if(blindUp) {
   digitalWrite(BLIND_UP, HIGH);
   blindUp = 0;
   blindAlreadyMove = false;
   blindState = false;
   }
   
   if (blindDown == 1 && millis() - currentBlindTime < BLIND_MOVE_TIME) {
   digitalWrite(BLIND_DOWN, LOW);
   blindAlreadyMove = true;
   } 
   else if(blindDown) {
   digitalWrite(BLIND_DOWN, HIGH);
   blindDown = 0;
   blindAlreadyMove = false;
   blindState = true;
   }*/

}

void notifyMaster() {
  if (isDhcp == true) {
    ether.copyIp(ether.hisip, hisip);
    ether.browseUrl(PSTR("/api/api.php"), "", website, my_callback);
  }
}

static void my_callback(byte status, word off, word len) {
}




#include <MFRC522.h>
#include <coap-simple.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Dhcp.h>
#include <Dns.h>
#include <WiFiUdp.h>


const char* ssid     = "Amir";
const char* password = "90121408";

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

void def_put(CoapPacket &packet, IPAddress ip, int port); 

WiFiUDP  Udp;
Coap coap(Udp);

int temper = D0;
int desiredWater = D1;
int blink1 = D2;

int levelWater = A0;

int water = 0;

//RFID
constexpr uint8_t RST_PIN = D3;     
constexpr uint8_t SS_PIN = D4;    
MFRC522 rfid(SS_PIN, RST_PIN);
String tag;

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  //response from coap server
   if(packet.type==3 && packet.code==0){
      Serial.println("ping ok");
    }
  Serial.println(p);
  if (strstr (p, ",")){
    String message;
    for (int i = 0; i < packet.payloadlen; i++) {
      message += p[i];
    }
    Serial.println();
    int w = message.substring(0,2).toInt();
    water = w;
    Serial.println(water);
    analogWrite(desiredWater, w*1024/100);

    
    int t = message.substring(3,5).toInt();
    analogWrite(temper, t*1024/100);
  
  }
}

// CoAP client response callback
void def_put(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");

  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  //response from coap server
   if(packet.type==3 && packet.code==0){
      Serial.println("ping ok");
    }
  Serial.println(p);
  if (p != "" && p != " " && p != "wait"){
    water = 1;
    Serial.println(water);
  }
  coap.get(IPAddress(192, 168, 1, 6), 5683, "result");
}
void setup() {
   pinMode(temper, OUTPUT);
  pinMode(desiredWater, OUTPUT);
  pinMode(blink1, OUTPUT);
  pinMode(levelWater, INPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //RFID
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  rfid.PCD_DumpVersionToSerial();

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  
  // client response callback.
  // this endpoint is single callback.
  Serial.println("Setup Response Callback");
  coap.response(callback_response);

  // start coap server/client
  coap.start();

}

void loop() {
  if (rfid.PICC_IsNewCardPresent() and rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println(tag);
    char buf[15];
    for (byte i = 0; i < tag.length(); i++) {
      buf[i]= tag[i];
    }
     // send GET or PUT coap request to CoAP server.
    
    Serial.println("Send Request");
    
    delay(100);
    int msgid = coap.put(IPAddress(192, 168, 1, 5), 5683, "tag", buf);
    //Serial.println(msgid);
    delay(5000);
    coap.get(IPAddress(192, 168, 1, 5), 5683, "result");
    tag = "";
    
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
    
  }
  float r = analogRead(levelWater)*100/450;
  Serial.println(r);
  if (water-10 > r  || water+10 < r){
      digitalWrite(blink1, HIGH);
      delay(500);
  } 
  digitalWrite(blink1, LOW);
  delay(500);
  coap.loop();

}

/*
  ArduinoMqttClient - WiFi Simple Receive

  This example connects to a MQTT broker and subscribes to a single topic.
  When a message is received it prints the message to the serial monitor.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board

  This example code is in the public domain.
*/
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include <SPI.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "Amir";       // your network SSID (name)
char pass[] = "90121408";    // your network password (use for WPA, or use as key for WEP)


//get from broker
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const char broker[] = "192.168.1.6";
int        port     = 1883;
const char topic[]  = "node1";


int temper = D0;
int desiredWater = D1;
int blink1 = D2;

int levelWater = A0;

int water = 0;

//RFID
constexpr uint8_t RST_PIN = D3;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
String tag;

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

  //WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  mqttClient.setServer(broker, port);
  mqttClient.setCallback(callback);
  mqttClient.connect("arduino-1");

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();


  mqttClient.subscribe(topic);
  Serial.print("Waiting for messages on topic: ");
  Serial.println(topic);
  Serial.println();

  
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduino-1")) {
      Serial.println("connected");
      // Subscribe
      mqttClient.subscribe("node1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 0.5 second");
      // Wait 5 seconds before retrying
      delay(500);
    }
  }
}
void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  if (rfid.PICC_IsNewCardPresent() and rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    char buf[15];
    for (byte i = 0; i < tag.length(); i++) {
      buf[i]= tag[i];
    }
   
    mqttClient.publish("node2", buf);
    Serial.println("printed");
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
  
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  int w = messageTemp.substring(0,2).toInt();
  water = w;
  analogWrite(desiredWater, w*1024/100);
  

  
  int t = messageTemp.substring(3,5).toInt();
  analogWrite(temper, t*1024/100);
  

}

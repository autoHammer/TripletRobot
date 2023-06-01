// Written by William and Thomas, NTNU 2023.
// This is the main controler code for the tripet robot project

#include "EMS22.h"     // Get the library at https://github.com/autoHammer/EMS22
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <EEPROM.h>

// Wifi - variables
char ssid[] = "NTNU-IOT";
char pass[] = "";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


// MQTT - variables
const char broker[] = "broker.hivemq.com";  //"test.mosquitto.org"
int port = 1883;

String mainTopic = "triplet/fromArduino/";
String topic[] = {"base", "shoulder", "elbow", "wrist1", "wrist2", "wrist3"};

//set interval for sending messages (milliseconds)
const long interval = 200;
unsigned long previousMillis = 0;


// Encoder variables
// Check connection in the datasheet at https://www.elfadistrelec.no/Web/Downloads/_t/ds/EMS22A50-B28-LS6_eng_tds.pdf
const int CLK = 11;   // encoder pin 2
const int DO  = 10;   // encoder pin 4
const int CS  = 9;    // encoder pin 6

AbsEncoder Coder(CLK, DO, CS);
int encoderOffset[10];
int encoderRotations[10];
float encoderAngle[10];
float prevEncoderAngle[10];
float totalEncoderAngle[10];
float prevTotalEncoderAngle[10];


// switch variables
bool stopSwitch = 12;

bool ledpin = 25;


void setup() {
  // start serial communication
  Serial.begin(9600);
  Serial.println("Starting communication");

  // initialize status LED
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, HIGH);

  Serial.println("Testing encoder. . .");
  // encoder setup
  Coder.begin();
  Serial.println("Started encoder");

  // get calibration numbers from storage
  EEPROM.get(0, encoderOffset);

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(2000);
  }
  Serial.println("You're connected to the network");
  Serial.println();

  // MQTT setup
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1) {
      // status led (error)
      digitalWrite(LED_BUILTIN, HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN, LOW);
      delay(50);
    }
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  Serial.println(Coder.getEncoderCount());
}


void loop() {

  // check if mqtt is still online
  if (!mqttClient.connected()) {
    Serial.println("LOST MQTT");
    if (!mqttClient.connect(broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());

      // status led (error)
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(30);

    }
  }

  // check if WiFi is connected
  if (!wifiClient.connected()) {
    Serial.println("LOST WIFI");
    // reconnect if disconnected
    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
      // failed, retry
      Serial.print(".");
      digitalWrite(LED_BUILTIN, HIGH);
      delay(30);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  mqttClient.poll();  // keepalive the server connection

  // stop switch
  //while (digitalRead(stopSwitch));

  // Read the value of all encoders.
  Coder.read();

  for (int i = 0; i < Coder.getEncoderCount(); i++) {
    // calculate angle of one encoder
    calculateAngle(i);
    prevEncoderAngle[i] = encoderAngle[i];
  }


  // flip direction of wrist 3 and 1
  totalEncoderAngle[1] = totalEncoderAngle[1];
  totalEncoderAngle[2] = -totalEncoderAngle[2];
  totalEncoderAngle[3] = totalEncoderAngle[3];
  totalEncoderAngle[5] = -totalEncoderAngle[5];

  // quick debugging
  Serial.print(totalEncoderAngle[0]);
  Serial.print(" ");
  Serial.print(totalEncoderAngle[1]);
  Serial.print(" ");
  Serial.print(totalEncoderAngle[2]);
  Serial.print(" ");
  Serial.print(totalEncoderAngle[3]);
  Serial.print(" ");
  Serial.print(totalEncoderAngle[4]);
  Serial.print(" ");
  Serial.println(totalEncoderAngle[5]);


  // send encoder angles to MQTT for all encoders
  // continue when timer is ready
  unsigned long currentMillis = millis ();
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    //status led
    digitalWrite(ledpin, HIGH);

    // send encoder angles to MQTT for all encoders
    for (int i = 0; i < Coder.getEncoderCount(); i++) {
      // calculate angle of one encoder
      //Serial.println("");

      // info
      //Serial.print("Sending message to topic: ");
      //Serial.print(topic[i]);
      //Serial.print("  Value: ");
      //Serial.println(encoderAngle[i]);


      // send message (only when data is different)
      if (totalEncoderAngle[i] != prevTotalEncoderAngle[i]) {
        mqttClient.beginMessage(mainTopic + topic[i]);
        mqttClient.print(constrain(totalEncoderAngle[i], -360, 360));
        mqttClient.endMessage();
        prevTotalEncoderAngle[i] = totalEncoderAngle[i];
      }
    }
  }
  digitalWrite(ledpin, HIGH);
}


// calculate the angle for all encoders
// includes calibration offset and rotation count
void calculateAngle(int encoderNr) {

  // save encoder posistion with calibration offset
  int newEncoderPos = (Coder.getAnalogData(encoderNr) - encoderOffset[encoderNr]) % 1024;
  // fix arduino remainder bug
  if (newEncoderPos < 0) {
    newEncoderPos = 1024 + newEncoderPos;
  }

  float newEncoderAngle = newEncoderPos * (360. / 1024.); // convert to angle from 0 to 360

  // count rotations
  if (newEncoderAngle - prevEncoderAngle[encoderNr] > 300) {
    encoderRotations[encoderNr]--;
  }
  if (newEncoderAngle - prevEncoderAngle[encoderNr] < -300) {
    encoderRotations[encoderNr]++;
  }

  // calculate total angle
  totalEncoderAngle[encoderNr] = (newEncoderAngle + encoderRotations[encoderNr] * 360);
  encoderAngle[encoderNr] = newEncoderAngle;
}

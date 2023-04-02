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

String topic[] = {"base", "shoulder", "elbow", "wrist1", "wrist2", "wrist3"};

//set interval for sending messages (milliseconds)
const long interval = 100;
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


void setup() {
  // start serial communication
  Serial.begin(9600);

  // initialize status LED
  pinMode(LED_BUILTIN, OUTPUT);

  // encoder setup
  Coder.begin();

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

}


void loop() {
  // Read the value of all encoders.
  Coder.read();

  mqttClient.poll();  // keepalive the server connection

  // continue when timer is ready
  unsigned long currentMillis = millis ();
  if (currentMillis - previousMillis >= interval) {  
    // save the last time a message was sent
    previousMillis = currentMillis;

    // send encoder angles to MQTT for all encoders
    for (int i = 0; i < Coder.getEncoderCount(); i++) {
      // calculate angle of one encoder
      calculateAngle(i);

      // info
      Serial.print("Sending message to topic: ");
      Serial.print(topic[i]);
      Serial.print("  Value: ");
      Serial.println(Coder.getAnalogData(i));

      // send message (only when data is different)
      if (encoderAngle[i] != prevEncoderAngle[i]) {
        mqttClient.beginMessage("triplet/fromArduino/" + topic[i]);
        mqttClient.print(encoderAngle[i]);
        mqttClient.endMessage();
      }
      prevEncoderAngle[i] = encoderAngle[i];
    }
  }
}


// calculate the angle for all encoders
// includes calibration offset and rotation count
void calculateAngle(int encoderNr) {

  // save encoder posistion with calibration offset
  int newEncoderPos = (Coder.getAnalogData(encoderNr) - encoderOffset[encoderNr]) % 1024;
  float newEncoderAngle = newEncoderPos * (360. / 1024.); // convert to angle from 0 to 360

  // count rotations
  if (newEncoderAngle - prevEncoderAngle[encoderNr] > 300) {
    encoderRotations[encoderNr]++;
  }
  if (newEncoderAngle - prevEncoderAngle[encoderNr] > -300) {
    encoderRotations[encoderNr]--;
  }

  // calculate total angle
  encoderAngle[encoderNr] = newEncoderAngle + encoderRotations[encoderNr] * 360;
}

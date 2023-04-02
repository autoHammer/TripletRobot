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
//const char broker[] = "test.mosquitto.org";
const char broker[] = "broker.hivemq.com";
int port = 1883;

String topic[] = {"base", "shoulder", "elbow", "wrist1", "wrist2", "wrist3"};

    const char NTNU_TEST1[]  = "NTNU_TEST1"; //  "triplet/controller/base"
    const char NTNU_TEST2[]  = "NTNU_TEST2";



//set interval for sending messages (milliseconds)
const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

// Encoder variables
// Check connection in the datasheet at https://www.elfadistrelec.no/Web/Downloads/_t/ds/EMS22A50-B28-LS6_eng_tds.pdf
const int CLK = 11;   // encoder pin 2
const int DO  = 10;   // encoder pin 4
const int CS  = 9;    // encoder pin 6

AbsEncoder Coder(CLK, DO, CS);
int encoderPos[10];
int prevEncoderPos[10];
int encoderOffset[10];
int encoderRotations[10];


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
  calculatePos();

  //Coder.plotAngles();

  mqttClient.poll();

  unsigned long currentMillis = millis ();
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    // send data to MQTT for all encoders
    for (int i = 0; i < Coder.getEncoderCount(); i++) {
      // info
      Serial.print("Sending message to topic: ");
      Serial.print(topic[i]);
      Serial.print("  Value: ");
      Serial.println(Coder.getAnalogData(i));

      // send message
      mqttClient.beginMessage("triplet/fromArduino/" + topic[i]);
      mqttClient.print(encoderPos[i]);
      mqttClient.endMessage();
    }

    //Enkoder 1
    Serial.print("Sending message to topic: ");
    Serial.print(NTNU_TEST1);
    Serial.print("  Value: ");
    Serial.println(Coder.getAnalogData(0));

    Serial.print(Coder.getAnalogData(0));

    mqttClient.beginMessage(NTNU_TEST1);
    mqttClient.print(Coder.getAnalogData(0));
    mqttClient.endMessage();
    // Enkoder 2
    Serial.print("Sending message to topic: ");
    Serial.print(NTNU_TEST2);
    Serial.print("  Value: ");
    Serial.println(Coder.getAnalogData(1));

    mqttClient.beginMessage(NTNU_TEST2);
    mqttClient.print(Coder.getAnalogData(1));
    Serial.print(Coder.getAnalogData(1));
    mqttClient.endMessage();

    delay(1);
  }
}

void calculatePos() {
  for (int i = 0; i < Coder.getEncoderCount(); i++) {
    encoderPos[i] = Coder.getAnalogData(i) - encoderOffset[i];
    if (encoderPos[i] < 0) {
      encoderPos[i] = Coder.getAnalogData(i) - encoderOffset[i] + 1024;
    }

    if (encoderPos[i] < 0) {
      encoderRotations[i]--;
    }
    if (encoderPos[i] > 1023) {
      encoderRotations[i]++;
    }

    encoderPos[i] = encoderPos[i] + encoderRotations[i];

    prevEncoderPos[i] = encoderPos[i];
  }
}

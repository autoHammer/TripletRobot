#include "EMS22.h"
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <EEPROM.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "NTNU-IOT";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//const char broker[] = "test.mosquitto.org";
const char broker[] = "broker.hivemq.com";
int        port     = 1883;

const char 

const char NTNU_TEST1[]  = "triplet/controller/base"; //  "triplet/controller/base"
const char NTNU_TEST2[]  = "NTNU_TEST2";
//const char NTNU_TEST3[]  = "NTNU_TEST3";
//const char NTNU_TEST4[]  = "NTNU_TEST4";
//const char NTNU_TEST5[]  = "NTNU_TEST5";
//const char NTNU_TEST6[]  = "NTNU_TEST6";
//const char NTNU_TEST7[]  = "NTNU_TEST7";


//set interval for sending messages (milliseconds)
const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;


// Example code for EMS22 library - Absolute Rotary Encoder
// Written by William Hammer, NTNU 2023.

// This example supports daisy chained encoders.
// Daisy chain means connecting output from one encoder into input of another encoder.

// Get the library at https://github.com/autoHammer/EMS22

// Check connection in the datasheet at https://www.elfadistrelec.no/Web/Downloads/_t/ds/EMS22A50-B28-LS6_eng_tds.pdf
const int CLK = 11;   // encoder pin 2
const int DO  = 10;   // encoder pin 4
const int CS  = 9;    // encoder pin 6

// Encoder variables
AbsEncoder Coder(CLK, DO, CS);
int encoderPos[10];
int prevEncoderPos[10];
int encoderOffset[10];
int encoderRotations[10];


void setup() {
  Serial.begin(9600);

  // Encoder setup
  Coder.begin();
  EEPROM.get(0, encoderOffset);

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}


void loop() {
  // Read the value of all encoders. Data is stored in the object Coder.
  // Must be ran every time data needs to be updated.
  Coder.read();
  calculatePos();

  // Fetch data. Input is the encoder index.
  // The encoder connected closest to the Arduino has index 0.
  // AnalogData is a number between 0 and 1024 (one loop).
  int encoder0Data = Coder.getAnalogData(0);
  //Serial.println(encoder0Data);

  // plotAngles will format and print all angle data.
  // Use Serial plotter under "Tools" to see the result.
  //Coder.plotAngles();
  Serial.println(encoderPos[0]);

  // Store all analog data into a list.
  // First index is first connected encoder.
  int dataList[2];
  Coder.copyAllAnalog(dataList);
  //Serial.println(dataList[0]);

  mqttClient.poll();

  unsigned long currentMillis = millis ();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    //Enkoder 1
    Serial.print("Sending message to topic: ");
    Serial.println(NTNU_TEST1);
    Serial.println(Coder.getAnalogData(0));

    Serial.print(Coder.getAnalogData(0));

    mqttClient.beginMessage(NTNU_TEST1);
    mqttClient.print(Coder.getAnalogData(0));
    mqttClient.endMessage();
    // Enkoder 2
    Serial.print("Sending message to topic: ");
    Serial.println(NTNU_TEST2);
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

  //TODO: Slimplify, change mqtt code to use encoderpos.
}

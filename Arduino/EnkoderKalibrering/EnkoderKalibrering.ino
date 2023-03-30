#include <EEPROM.h>
#include "EMS22.h"

const int CLK = 11; //lilla
const int DO  = 10; //blå digital output from encoder
const int CS  = 9;  //Grønn (gul)

AbsEncoder Coder(CLK, DO, CS);

void setup() {
  // setup
  Serial.begin(9600);
  delay(100);
  Serial.println("Starting Coder");
  Coder.begin();
  Serial.println("Starting calibration");

  // get the number of connected encoders
  int encoderCount = Coder.getEncoderCount();

  // read encoder data
  Coder.read();
  int readData[10]; // empty list
  Coder.copyAllAnalog(readData);  // fills the empty list

  // store on EEPROM
  EEPROM.put(0, readData);
  delay(10);

  // read data for debug
  int check[10];
  EEPROM.get(0, check);

  // print status
  Serial.println("Finished Calibrating");
  Serial.println("Values are: ");
  for (int i = 0; i < encoderCount; i++) {
    Serial.println(readData[i]);
  }
}

void loop() {
}

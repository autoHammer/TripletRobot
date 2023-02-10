//code for EMS22A50-D28-LT6 - Absolute Rotary Encoder
// Written by William Hammer, NTNU 2023.

#include "AbsEncoder.h"

const int CLK = 11; //lilla
const int DO  = 10; //blå digital output from encoder
const int CS  = 9;  //Grønn (gul)

AbsEncoder Coder(CLK, DO, CS, 2);

//bool data[2][16];

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for serial to establish
  Serial.println("serial ready");

  Coder.begin();
}

void loop() {
  Coder.scan();

  int dataList[2];
  Coder.copyAllAnalog(dataList);

  Coder.plotAngles();
  delay(1);

  //Coder.copyRawData(data);
  //Coder.printRaw(0);

  //Serial.println(Coder.getAnalogData(0));



  //Serial.println(dataList[0]);
  //Serial.println(dataList[1]);
}

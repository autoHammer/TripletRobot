//code for EMS22A50-D28-LT6 - Absolute Rotary Encoder
// Written by William Hammer, NTNU 2023.

#include "AbsEncoder.h"

const int CLK = 11; //lilla
const int DO  = 10; //blå digital output from encoder
const int CS  = 9;  //Grønn (gul)

AbsEncoder Coder(CLK, DO, CS, 2);

bool data[2][16];

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for serial to establish
  Serial.println("serial ready");

  Coder.begin();
}

void loop() {
  Coder.scan();
  Coder.copyRawData(data);
  //Coder.printRaw(1);

  // konvertere fra decimal to binary
  float result = 0;
  for (int i = 0, j = 9; i < 10; i++, j--) {
    result += data[0][i] * pow(2, j);
  }
  //float result = Coder.rawToAnalog(0);
  Serial.print(result * (360. / 1024.));
  Serial.print(",");
  // konvertere fra decimal to binary
  result = 0;
  for (int i = 0, j = 9; i < 10; i++, j--) {
    result += data[1][i] * pow(2, j);
  }
  Serial.println(result * (360. / 1024.));

  Serial.println("");

  delay(1);

}

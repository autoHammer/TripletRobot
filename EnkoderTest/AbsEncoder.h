#ifndef AbsEncoder_h
#define AbsEncoder_h
#include "Arduino.h"

class AbsEncoder {
  public:
    AbsEncoder (int CLK_, int DO_, int CS_, int sensorCount_) {
      sensorCount = sensorCount_;
      CLK = CLK_;
      DO = DO_;
      CS = CS_;
    }

    void begin () {
      pinMode(CLK, OUTPUT);
      pinMode(DO, INPUT);
      pinMode(CS, OUTPUT);
    }

    void scan() {
      // start pulse
      digitalWrite(CLK, HIGH);
      digitalWrite(CS, HIGH);
      delayMicroseconds(1);
      digitalWrite(CS, LOW);
      delayMicroseconds(1);
      digitalWrite(CLK, LOW);

      // klokkepuls og datalesing
      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
          delayMicroseconds(1);
          digitalWrite(CLK, HIGH);
          delayMicroseconds(1);
          data[j][i] = digitalRead(DO);
          digitalWrite(CLK, LOW);
        }
        delayMicroseconds(1);
        digitalWrite(CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLK, LOW);
      }
      delayMicroseconds(1);
    }

    void copyRawData(bool list[2][16]) {
      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
          list[j][i] = data[j][i];
        }
      }
    }

    // convert raw data from one sensor
    // @return integer reprecenting angle 0 to 1023)
    //float rawToAnalog(int deviceNr) {
     // float result = 0;
     // for (int i = 0, j = 9; i < 10; i++, j--) {
     //   result += data[deviceNr][i] * pow(2, j);
     //   Serial.println(result);
     // }
     // return result;
   // }

    // print all raw data from one sensor
    void printRaw(int deviceNr) {
      Serial.print("Encoder nr. ");
      Serial.print(deviceNr);
      Serial.print(": ");
      for(int i = 0; i<16; i++){
        Serial.print(data[deviceNr][i]);
      }
      Serial.println("");
    }

    float analogToDeg(int a) {
      return a * (360. / 1024.);
    }
    
  private:
    int CLK = 0;
    int DO  = 0;
    int CS  = 0;
    int sensorCount = 1;
    bool data[2][16];
};
#endif

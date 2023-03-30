#ifndef EMS22_h
#define EMS22_h
#include "Arduino.h"

// Determines the maximum array size for declaration
#define maxEncoderCount 10

class AbsEncoder {
    /* This is a class for handling the absolute encoder EMS22A50-D28-LT6
       See example code for usage and docs.
    */
  public:
    AbsEncoder (int CLK_, int DO_, int CS_) {
      CLK = CLK_;
      DO = DO_;
      CS = CS_;
    }

    void begin() {
      /* pin setup, must be ran once in void setup()
      */
      pinMode(CLK, OUTPUT);
      pinMode(DO, INPUT);
      pinMode(CS, OUTPUT);

      // automatic detection of encoder count
      countEncoders();
    }

    void read() {
      /* read() will fetch the raw position data for all encoders.
         The data will be stored in the data 2d bool array.
         The sequence and delays are from the datasheet for the encoder.
      */
      // start pulse
      startEncoder();

      // clock pulse and data reading
      for (int j = 0; j < encoderCount; j++) {
        for (int i = 0; i < 16; i++) {
          digitalWrite(CLK, HIGH);
          delayMicroseconds(1);
          rawData[j][i] = digitalRead(DO);
          digitalWrite(CLK, LOW);
          delayMicroseconds(1);
        }
        // extra daisy chain signal
        digitalWrite(CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLK, LOW);
        delayMicroseconds(1);
      }

      updateAnalogData();
    }


    void countEncoders() {
      /* Algorithm to find how many encoders is connected.
      */
      encoderCount = 0;
      startEncoder();

      // loop until all encoders are scanned
      bool daisy = 1;
      while (daisy) {
        // look for data, daisy is 1 if any data is registrerd
        daisy = 0;
        for (int i = 0; i < 16; i++) {
          digitalWrite(CLK, HIGH);
          delayMicroseconds(1);
          if (digitalRead(DO)) daisy = 1;
          digitalWrite(CLK, LOW);
          delayMicroseconds(1);
        }
        // extra daisy chain signal
        digitalWrite(CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLK, LOW);
        // count the encoders
        if (daisy) encoderCount ++;
      }
      Serial.println(encoderCount);
      // error handling
      if (encoderCount > maxEncoderCount) {
        Serial.println("ERROR: Not connected the right amount of encoders!");
        Serial.println("       Check if encoders is connected correctly");
        Serial.print  ("       max amount of encoder = ");
        Serial.println(maxEncoderCount);
        Serial.println("       increase this in library if you have more encoders");
        encoderCount = 0;
        delay(1000);
      }
    }
    int getEncoderCount(){
      return encoderCount;
    }


    int getAnalogData(int encoderNr) {
      /* Fetch the analog data for one encoder.
      */
      return analogData[encoderNr];
    }


    void copyAllAnalog(int list[maxEncoderCount]) {
      /* Copy the date from analogData into a list declared in main.
      */
      for (int c = 0; c < encoderCount; c++) {
        list[c] = analogData[c];
      }
    }


    void plotAngles() {
      /* Plots all the angles for all encoders.
         Use Tools -> serial plotter.
      */
      for (int c = 0; c < encoderCount; c++) {
        // Convert from analog to degree.
        float angle = analogToDeg(analogData[c]);
        Serial.print("Encoder_nr_");
        Serial.print(c);
        Serial.print(":");
        Serial.print(angle);
        Serial.print(" ");
      }
      if (encoderCount != 0) Serial.println("\n");
    }


    void printRaw(int deviceNr) {
      /* Print all raw data from one encoder
         For debugging
      */
      Serial.print("Encoder nr. ");
      Serial.print(deviceNr);
      Serial.print(": ");
      for (int i = 0; i < 16; i++) {
        Serial.print(rawData[deviceNr][i]);
      }
      Serial.println("");
    }


    float analogToDeg(int a) {
      /* Converts analog 0-1023 value to degree 0-360
      */
      return a * (360. / 1024.);
    }


    void copyRawData(bool list[maxEncoderCount][16]) {
      /* Copy the raw data into the list
         make sure to declare a big enogh 2d list
      */
      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
          list[j][i] = rawData[j][i];
        }
      }
    }

    int getResolution(){
      /* Return the resolution setting
       */
      return resolution;
    }

    void changeResolution(int newR){
      resolution = newR;
    }


  private:
    int CLK = 0;
    int DO  = 0;
    int CS  = 0;
    int encoderCount = 0;              // store the amount of encoders in use
    bool rawData[maxEncoderCount][16]; // 2D list of raw data, one encoder per line, one bool per data.
    int analogData[maxEncoderCount];   // List of analog data, one int per encoder.
    int resolution = 1024;


    void startEncoder() {
      /* Start communication with encoder
         Also test if encoder is responding
      */
      digitalWrite(CLK, HIGH);
      digitalWrite(CS, HIGH);
      delayMicroseconds(1);
      digitalWrite(CS, LOW);
      delayMicroseconds(1);

      // check encoder response
      if (!digitalRead(DO)) {
        if (encoderCount != 0) Serial.println("ERROR: No response from encoder");
      }
      digitalWrite(CLK, LOW);
      delayMicroseconds(1);
    }


    void updateAnalogData() {
      /* Convert rawData  to AnalogData
      */
      for (int c = 0; c < 2; c++) {
        float result = 0;
        for (int i = 0, j = 9; i < 10; i++, j--) {  // i counts upwards, j counts down
          result += rawData[c][i] * pow(2, j);      // multiplies each bit with their respective exponent, and summs the result.
        }
        analogData[c] = result;
      }
    }
};
#endif

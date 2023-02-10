#ifndef AbsEncoder_h
#define AbsEncoder_h
#include "Arduino.h"

// Determines the maximum array size for declaration
#define maxSensorCount 10

class AbsEncoder {
  /* This is a class for handling the absolute encoder EMS22A50-D28-LT6
   * See README for usage and docs
   * Also see the examples
   */
  public:
    AbsEncoder (int CLK_, int DO_, int CS_, int sensorCount_) {
      sensorCount = sensorCount_;
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

      countSensors();
    }

    void scan() {
      /* Scan will fetch the raw position data for all encoders.
         The data will be stored in the data 2d bool array.
         The sequence and delays are from the datasheet for the sensor.
         TODO: add failcheck. (test if sensor is anwering first CLK pulse.
      */
      // start pulse
      digitalWrite(CLK, HIGH);
      digitalWrite(CS, HIGH);
      delayMicroseconds(1);
      digitalWrite(CS, LOW);
      delayMicroseconds(1);
      digitalWrite(CLK, LOW);

      // clock pulse and data reading
      for (int j = 0; j < sensorCount; j++) {
        for (int i = 0; i < 16; i++) {
          delayMicroseconds(1);
          digitalWrite(CLK, HIGH);
          delayMicroseconds(1);
          rawData[j][i] = digitalRead(DO);
          digitalWrite(CLK, LOW);
        }
        delayMicroseconds(1);
        digitalWrite(CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(CLK, LOW);
      }
      delayMicroseconds(1);

      updateAnalogData();
    }


    void countSensors(){
      // TODO: auto counting of the amount of sensors
    }


    int getAnalogData(int sensorNr){
      /* Fetch the analog data for one sensor
       */
      return analogData[sensorNr];
    }

  
    void copyAllAnalog(int list[maxSensorCount]){
      /* Copy the date from analogData into a list declared in main
       */
      for(int c = 0; c < sensorCount; c++){
        list[c] = analogData[c];
      }
    }


    void plotAngles() {
      /* Plots all the angles for all sensors.
      */
      for (int c = 0; c < sensorCount; c++) {
        // Convert from analog to degree.
        float angle = analogToDeg(analogData[c]);

        Serial.print("Sensor_nr_");
        Serial.print(c);
        Serial.print(":");
        Serial.print(angle);
        Serial.print(" ");
      }
      Serial.println("\n");
    }


    void printRaw(int deviceNr) {
      /* Frint all raw data from one sensor
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


    void copyRawData(bool list[maxSensorCount][16]) {
      /* copy the raw data into the list
         make sure to declare a big enogh 2d list
      */
      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 16; i++) {
          list[j][i] = rawData[j][i];
        }
      }
    }


  private:
    int CLK = 0;
    int DO  = 0;
    int CS  = 0;
    int sensorCount = 0;              // store the amount of sensors in use
    bool rawData[maxSensorCount][16]; // 2D list of raw data, one sensor per line, one bool per data.
    int analogData[maxSensorCount];   // List of analog data, one int per sensor.


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

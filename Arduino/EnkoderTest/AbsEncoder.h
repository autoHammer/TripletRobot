#ifndef AbsEncoder_h
#define AbsEncoder_h
#include "Arduino.h"

// Determines the maximum array size for declaration
#define maxSensorCount 10

class AbsEncoder {
    /* This is a class for handling the absolute encoder EMS22A50-D28-LT6
       See README for usage and docs
       Also see the examples
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

      countSensors();
    }

    void read() {
      /* read will fetch the raw position data for all encoders.
         The data will be stored in the data 2d bool array.
         The sequence and delays are from the datasheet for the sensor.
      */
      // start pulse
      startSensor();

      // clock pulse and data reading
      for (int j = 0; j < sensorCount; j++) {
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


    void countSensors() {
      /* algorithm to find how many sensor is connected
      */
      sensorCount = 0;
      startSensor();

      // loop until all sensors is scanned
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
        // count the sensors
        if (daisy) sensorCount ++;
      }
      Serial.println(sensorCount);
      // error handling
      if (sensorCount > maxSensorCount) {
        Serial.println("ERROR: Not connected the right amount of sensors!");
        Serial.println("       Check if sensors is connected correctly");
        Serial.print  ("       max amount of sensor = ");
        Serial.println(maxSensorCount);
        Serial.println("       increase this in library if you have more sensors");
        sensorCount = 0;
        delay(1000);
      }
    }


    int getAnalogData(int sensorNr) {
      /* Fetch the analog data for one sensor
      */
      return analogData[sensorNr];
    }


    void copyAllAnalog(int list[maxSensorCount]) {
      /* Copy the date from analogData into a list declared in main
      */
      for (int c = 0; c < sensorCount; c++) {
        list[c] = analogData[c];
      }
    }


    void plotAngles() {
      /* Plots all the angles for all sensors.
         Use Tools -> serial plotter.
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
      if (sensorCount != 0) Serial.println("\n");
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


    void startSensor() {
      /* Start communication with sensor
         Also tests if sensor is responding
      */
      digitalWrite(CLK, HIGH);
      digitalWrite(CS, HIGH);
      delayMicroseconds(1);
      digitalWrite(CS, LOW);
      delayMicroseconds(1);

      // check sensor response
      if (!digitalRead(DO)) {
        if (sensorCount != 0) Serial.println("ERROR: No response from sensor");
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

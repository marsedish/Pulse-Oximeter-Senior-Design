/*
  Optical SP02 Detection (SPK Algorithm) using the MAX30105 Breakout
  Date: November 9, 2026
  Names: Eugenio, Cindy, Sanjeev
  open source code: https://github.com/sparkfun/MAX30105_Breakout

  This code shows heart rate and SPO2 levels.

 

  This example is based on MAXREFDES117 and RD117_LILYPAD.ino from Maxim. Their example
  was modified to work with the SparkFun MAX30105 library and to compile under Arduino 1.6.11
  Please see license file for more info.

  // This code is based on the SparkFun MAX30105 Example8_SPO2 sketch as a foundation. The setup and variables were
  mostly kept the same as the base code, but the loop was coded to fit requirements of our design. The void loop was
  coded with the assistance of AI, but testing, debugging, troubleshooting, and code checking was done by us.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA) = esp pin 21
  -SCL = A5 (or SCL) = esp32 pin 22
  -INT = Not connected
 

*/

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255


uint32_t irBuffer[100];     // IR signal array
uint32_t redBuffer[100];    // Red signal array 


int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

const int BPM_AVG_SIZE = 5;
int bpmReadings[BPM_AVG_SIZE];
int bpmIndex = 0;
int bpmCount = 0;

// variables to set parameters for a "valid" BPM
const int MIN_VALID_BPM = 20;  
const int MAX_VALID_BPM = 200;

// Parameters to decide whether BPM is high or low
const int LOW_BPM = 40;
const int HIGH_BPM = 120;

const int WARNING_COUNT = 3;

int lowCount = 0;
int highCount = 0;



void setup()
{
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:

  Wire.begin(21, 22); // ESP32: SDA = GPIO21, SCL = GPIO22
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("MAX30105 connected succesfully!");



  //Configuration settings for the sensor

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  Serial.print("FIFO Configuration: 0x");
  Serial.println(particleSensor.readRegister8(0x57, 0x08), HEX);

  Serial.print("SpO2 Configuration: 0x");
  Serial.println(particleSensor.readRegister8(0x57, 0x0A), HEX);
}

void loop()
{
  bufferLength = 100; //100 samples at 25 samples/sec = 4 second

  unsigned long startTime = millis();

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) 
    {//do we have new data?
      particleSensor.check(); //Check the sensor for new data
      delay(1); //Allows ESP32 background tasks to run
    }
    redBuffer[i] = particleSensor.getFIFORed();
    irBuffer[i] = particleSensor.getFIFOIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample



  }

 

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (true)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
      {
        particleSensor.check(); //Check the sensor for new data
        delay(1);
      }  


      redBuffer[i] = particleSensor.getFIFORed();
      irBuffer[i] = particleSensor.getFIFOIR();

      particleSensor.nextSample(); //We're finished with this sample so move to next sample
    }


   

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    //Print ONLY if both are valid values
    

    // Check whether both calculations are valid
    if (validHeartRate == 1 && validSPO2 == 1)
    {
      // Accept only BPM values from 20 to 200
      if (heartRate >= MIN_VALID_BPM &&
          heartRate <= MAX_VALID_BPM)
      {
        // Add new BPM to the 5-reading moving average
        bpmReadings[bpmIndex] = heartRate;
        bpmIndex = (bpmIndex + 1) % BPM_AVG_SIZE;

        if (bpmCount < BPM_AVG_SIZE)
        {
          bpmCount++;
        }

        long bpmSum = 0;

        for (int j = 0; j < bpmCount; j++)
        {
          bpmSum += bpmReadings[j];
        }

        int smoothBPM = bpmSum / bpmCount;

        // Display calculated BPM and SpO2
        Serial.print("BPM: ");
        Serial.print(smoothBPM);
        Serial.print(" | SpO2: ");
        Serial.print(spo2);
        Serial.println("%");

        // Check for LOW heart rate
        if (smoothBPM < LOW_BPM)
        {
          lowCount++;
          highCount = 0;

          if (lowCount >= WARNING_COUNT)
          {
            Serial.println("WARNING: LOW HEART RATE!");
          }
          else
          {
            Serial.println("Confirming low heart rate...");
          }
        }

        // Check for HIGH heart rate
        else if (smoothBPM > HIGH_BPM)
        {
          highCount++;
          lowCount = 0;

          if (highCount >= WARNING_COUNT)
          {
            Serial.println("WARNING: HIGH HEART RATE!");
          }
          else
          {
            Serial.println("Confirming high heart rate...");
          }
        }

        // Heart rate is within 40-120 BPM
        else
        {
          lowCount = 0;
          highCount = 0;

          Serial.println("Heart rate within configured range");
        }
      }
      else
      {
        // BPM outside 20-200: reject the measurement
        lowCount = 0;
        highCount = 0;
        bpmCount = 0;
        bpmIndex = 0;

      }
    }
    else
    {
      // Algorithm could not obtain valid measurements
      lowCount = 0;
      highCount = 0;
      bpmCount = 0;
      bpmIndex = 0;

      Serial.println("Reading signal...");
    }


      

      delay(10);
  }
  }



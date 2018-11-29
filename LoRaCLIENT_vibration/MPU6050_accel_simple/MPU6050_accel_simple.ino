/*
    MPU6050 Triple Axis Gyroscope & Accelerometer. Simple Accelerometer Example.
    Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-zyroskop-i-akcelerometr-mpu6050.html
    GIT: https://github.com/jarzebski/Arduino-MPU6050
    Web: http://www.jarzebski.pl
    (c) 2014 by Korneliusz Jarzebski
*/

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

void setup() 
{
  Serial.begin(9600);

  Serial.println("Initialize MPU6050");

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // If you want, you can set accelerometer offsets
  // mpu.setAccelOffsetX();
  // mpu.setAccelOffsetY();
  // mpu.setAccelOffsetZ();
  
  checkSettings();
}

void checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:          ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:         ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  
  Serial.println();
}

void loop()
{
  Vector rawAccel1 = mpu.readRawAccel();
  Vector normAccel1 = mpu.readNormalizeAccel();
  delay(1000);
  Vector rawAccel2 = mpu.readRawAccel();
  Vector normAccel2 = mpu.readNormalizeAccel();

  Serial.print("Xraw1 = ");
  Serial.print(rawAccel1.XAxis);
  Serial.print(" Yraw1 = ");
  Serial.print(rawAccel1.YAxis);
  Serial.print(" Zraw1 = ");
  Serial.println(rawAccel1.ZAxis);
  Serial.print("Xraw2 = ");
  Serial.print(rawAccel2.XAxis);
  Serial.print(" Yraw2 = ");
  Serial.print(rawAccel2.YAxis);
  Serial.print(" Zraw2 = ");
  Serial.println(rawAccel2.ZAxis);
  Serial.println();

//  Serial.print(" Xnorm = ");
//  Serial.print(normAccel.XAxis);
//  Serial.print(" Ynorm = ");
//  Serial.print(normAccel.YAxis);
//  Serial.print(" Znorm = ");
//  Serial.println(normAccel.ZAxis);

  int XResult = abs(rawAccel1.XAxis - rawAccel2.XAxis);
  int YResult = abs(rawAccel1.YAxis - rawAccel2.YAxis);
  int ZResult = abs(rawAccel1.ZAxis - rawAccel2.ZAxis);

 Serial.print("XResult: ");
 Serial.println(XResult);
 Serial.print("YResult: ");
 Serial.println(YResult);
 Serial.print("ZResult: ");
 Serial.println(ZResult);
 Serial.println();

    if(XResult>5000 && YResult<5000 && ZResult<5000){
        Serial.println("X ALERT");
    }
    if(XResult<5000 && YResult>5000 && ZResult<5000){
        Serial.println("Y ALERT");
    }
    if(XResult<5000 && YResult<5000 && ZResult>5000){
        Serial.println("Z ALERT");
    }
    if(XResult>5000 && YResult>5000 && ZResult<5000){
        Serial.println("X Y ALERT");
    }
    if(XResult>5000 && YResult<5000 && ZResult>5000){
        Serial.println("X Z ALERT");
    }
    if(XResult<5000 && YResult>5000 && ZResult>5000){
        Serial.println("Y Z ALERT");
    }
    if(XResult>5000 && YResult>5000 && ZResult>5000){
        Serial.println("X Y Z ALERT");
    }
    Serial.println();
  
  delay(1000);
}



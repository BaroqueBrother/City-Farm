/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/edward/Documents/IoT/capstone/City-Farm/City_Farmer/src/City_Farmer.ino"
/*
 * Project City_Farmer
 * Description:
 * Author:
 * Date:
 */

#include <Wire.h>
void setup();
void loop();
float get03Levels();
float getCarbonLevels();
#line 9 "c:/Users/edward/Documents/IoT/capstone/City-Farm/City_Farmer/src/City_Farmer.ino"
const float argonRes = 4096.0;
const float argonVolt = 3.3;




SYSTEM_MODE(SEMI_AUTOMATIC);
void setup()
{
  // Initialise I2C communication as MASTER
  // Initialise serial communication, set baud rate = 9600
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(0x50);
  Wire.endTransmission(true);
}

void loop(){
  get03Levels();
  getCarbonLevels();
  float voltage = analogRead(A4)*0.0011224;
  Serial.printf("voltage %.1f\n",voltage);
  delay(6000);

}

float get03Levels(){
Wire.beginTransmission(0x50);
Wire.write(0x00);
Wire.endTransmission(false);
Wire.requestFrom(0x50,2,true);
unsigned int mq131High = Wire.read();
unsigned int mq131Low = Wire.read();
int rawDataMQ = ((mq131High & 0x0F) * 256) + mq131Low;
float volt131 = rawDataMQ*(argonVolt/argonRes);
float RsRoM131 = (20.0/0.18)*(argonVolt-volt131)/volt131;
float ozonePPM = pow(10.0,-log(RsRoM131)+1.48);
// Serial.printf("raw data %i \n",rawDataMQ);
// Serial.printf("voltage %.02f \n", volt131);
// Serial.printf("rsro %.02f \n",RsRoM131);
Serial.printf("ozone ppm %.02f \n",ozonePPM);
return ozonePPM;
}

float getCarbonLevels(){
  Wire.beginTransmission(0x51);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(0x51,2,true);
  unsigned int mq09High = Wire.read();
  unsigned int mq09Low = Wire.read();
  int rawDataCO = ((mq09High & 0x0F) * 256) + mq09Low;
  float volt09 = rawDataCO*(argonVolt/argonRes);
  float RsRo09 = (1/1.55)*((1.5-volt09)/volt09)*10.0;
  float coPPM = pow(10,-log(RsRo09)+1.48);
  // Serial.printf("raw data %i \n",rawDataCO);
  // Serial.printf("voltage %.02f \n", volt09);
  // Serial.printf("rsro %.02f \n",RsRo09);
  Serial.printf("carbon ppm %.02f \n",coPPM);
  return coPPM;
}

 
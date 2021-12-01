/*
 * Project City_Farmer
 * Description:
 * Author:
 * Date:
 */

#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT.h" 
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h" 
#include "credentials.h"
#include <Wire.h>
TCPClient TheClient; 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 
Adafruit_MQTT_Publish Ozone03PPM = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Ozone03PPM");
Adafruit_MQTT_Publish CO2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/CO2PPM");

const float argonRes = 4096.0;
const float argonVolt = 3.3;

unsigned long lastTime, last;
unsigned long current = millis();



SYSTEM_MODE(SEMI_AUTOMATIC);
void setup()
{
  
  Serial.begin(9600);
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  // initializing i2c
  Wire.begin();
  Wire.beginTransmission(0x50);
  Wire.endTransmission(true);
}

void loop(){
  //pinging MQTT broker every 2 minutes
   MQTT_connect();
   if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      if(! mqtt.ping()) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  // get a reading and publish every hour
  if((millis()-lastTime) >36000000){ 
  sendOzoneLevels();
  sendCarbonLevels();
  lastTime = millis();
  }
}

void sendCarbonLevels(){
  // getting mq09 sensor reading
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
  if(mqtt.Update()) {
    CO2.publish(coPPM);
  }
  // Serial.printf("raw data %i \n",rawDataCO);
  // Serial.printf("voltage %.02f \n", volt09);
  // Serial.printf("rsro %.02f \n",RsRo09);
  Serial.printf("carbon ppm %.02f \n",coPPM);
}



 void sendOzoneLevels(){
   //getting MQ131 readings 
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
    if(mqtt.Update()){
      Ozone03PPM.publish(ozonePPM);
}
// Serial.printf("raw data %i \n",rawDataMQ);
// Serial.printf("voltage %.02f \n", volt131);
// Serial.printf("rsro %.02f \n",RsRoM131);
    Serial.printf("ozone ppm %.02f \n",ozonePPM);
 }
 

void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("%s\n",(char *)mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds..\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!\n");
}
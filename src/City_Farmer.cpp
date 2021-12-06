/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/edward/Documents/IoT/City-Farm/src/City_Farmer.ino"
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
void setup();
void loop();
void sendCarbonLevels();
void sendOzoneLevels();
void MQTT_connect();
void getBatteryState();
void pingBroker();
#line 13 "c:/Users/edward/Documents/IoT/City-Farm/src/City_Farmer.ino"
TCPClient TheClient; 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 
Adafruit_MQTT_Publish Ozone03PPM = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Ozone03PPM");
Adafruit_MQTT_Publish CO2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/CO2PPM");
Adafruit_MQTT_Publish Batterystate = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/BatteryState");
Adafruit_MQTT_Publish Batteryvoltage = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/BatteryVolts");




const float argonRes = 4096.0;
const float argonVolt = 3.3;

unsigned long lastTime, last;
unsigned long current = millis();
float getSoC();
float getVCell();
FuelGauge fuel;
SYSTEM_MODE(SEMI_AUTOMATIC);

void setup(){
  Serial.begin(9600);
  Cellular.on();
  Particle.connect();
  Wire.begin();
  Wire.beginTransmission(0x50);
  Wire.endTransmission(true);
 
  }

void loop(){
  MQTT_connect();
  pingBroker();
  getBatteryState();
  Serial.printf("Battery Voltage %.02f\n",fuel.getVCell());
  if((millis()-lastTime) >25000){ 
    sendOzoneLevels();
    sendCarbonLevels();
    lastTime = millis();
    }
  
}

void sendCarbonLevels(){
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
    Serial.printf("ozone ppm %.02f \n",ozonePPM);
 }

void MQTT_connect() {
  int8_t ret;
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

void getBatteryState(){
   enum batteryState {
    BATTERY_STATE_UNKNOWN = 0,
    BATTERY_STATE_NOT_CHARGING = 1,
    BATTERY_STATE_CHARGING = 2,
    BATTERY_STATE_CHARGED = 3,
    BATTERY_STATE_DISCHARGING = 4,
    BATTERY_STATE_FAULT = 5,
    BATTERY_STATE_DISCONNECTED = 6
    };
  int batteryState = System.batteryState();
  switch(batteryState){
    case 0:
    Serial.printf("Battery State Unknown\n");
    break;
    case 1:
    Serial.printf("Battery Not Charging\n");
    break;
    case 2:
    Serial.printf("Battery Charging\n");
    break;
    case 3:
    Serial.printf("Battery Charged\n");
    break;
    case 4:
    Serial.printf("Battery Discharging\n");
    break;
    case 5:
    Serial.printf("Battery Fault\n");
    break;
    case 6:
    Serial.printf("Battery Disconnected\n");
    break;
  }
  if ((millis()-last)>25000) {
    if(mqtt.Update()){
      //Batterystate.publish(batteryState);
      Batteryvoltage.publish(fuel.getVCell());
      last = millis();
    }
  
  }
}

void pingBroker(){
  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      if(! mqtt.ping()) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
int water_ain=0;
int ad_value=0;


WiFiClient espclient;
WiFiManager wifiManager;
bool isConnected = false;


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.2.212"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""
#define AIO_TOPIC       "/homeassistant/basement/WaterLevelSensor/level"
#define DEVICE_ID       "WaterLevelSensor"

Adafruit_MQTT_Client mqtt(&espclient, AIO_SERVER, AIO_SERVERPORT);
Adafruit_MQTT_Publish waterlevel = Adafruit_MQTT_Publish(&mqtt,AIO_TOPIC);

unsigned long current_millis;
unsigned long previous_millis = 0;
unsigned long detect_interval = 100000;

const char thingSpeakAddress[] = "api.thingspeak.com";
const char thingSpeakAPIKey[] = "H1KHAGG7TIRGN6N0";

//the number the message should be sent to
const String sendNumber = "";
const String sms_key = "";

void MQTT_connect();

String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg!='\0'){
    if( ('a' <= *msg && *msg <= 'z')
      || ('A' <= *msg && *msg <= 'Z')
      || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } 
    else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

//update data to thingSpeak
void updateThingSpeak(String tsData) {

  if (!espclient.connect(thingSpeakAddress, 80)) {
     return;
  }
  espclient.print(F("GET /update?key="));
  espclient.print(thingSpeakAPIKey);
  espclient.print(F("&"));
  espclient.print(tsData);
  espclient.print(F(" HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n"));
  espclient.println();
  Serial.println("uploaded to think speak");
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(3000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


void updateMQTT(char* data){
  mqtt.disconnect();
  MQTT_connect();
  if (!waterlevel.publish(data)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  mqtt.disconnect();
}


void waterDetection(){
  current_millis = millis();
  if(current_millis - previous_millis >= detect_interval) {
    ad_value=analogRead(water_ain);
    if(ad_value>160)
    {
      Serial.println("High water level");
      //mqtt.disconnect();
      updateThingSpeak("1=1");
      delay(5000);
      updateMQTT("High");
    }
    else
    {
      Serial.println("Low water level");
      //mqtt.disconnect();
      updateThingSpeak("1=0");
      delay(5000);
      updateMQTT("Low");
    }
    previous_millis = current_millis;
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.

void callback(char* topic, byte* payload, unsigned int length){
  
}
void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.print("connecting to ");
  wifiManager.autoConnect("WaterLevelSensor");
  //setup_wifi();
  //MQTT_connect();
}
void loop(){
  waterDetection();
  //Serial.println(analogRead(water_ain));
 
}

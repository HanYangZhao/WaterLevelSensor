#include <ESP8266WiFi.h>
#include <WiFiClient.h>
int water_ain=0;
int ad_value=0;


WiFiClient client;
const char* ssid = "";
const char* password = "";
bool isConnected = false;

unsigned long current_millis;
unsigned long previous_millis = 0;
unsigned long detect_interval = 900000;

const char thingSpeakAddress[] = "api.thingspeak.com";
const char thingSpeakAPIKey[] = "";

//the number the message should be sent to
const String sendNumber = "";
const String sms_key = "";

boolean wifiConnect(){
 //wait 1 minute for wifi connection
 WiFi.begin(ssid, password);
 for (int i = 0 ; i < 60 ; i++){
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  } 
  else{
    return true;
  }
 }
 return false;
}

void sendSMS(String number,String message)
{
  // Make a TCP connection to remote host
  if (client.connect(thingSpeakAddress, 80))
  {

    //should look like this...
    //api.thingspeak.com/apps/thinghttp/send_request?api_key={api key}&number={send to number}&message={text body}

    client.print("GET /apps/thinghttp/send_request?api_key=");
    client.print(sms_key);
    client.print("&number=");
    client.print(number);
    client.print("&message=");
    client.print(message);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(thingSpeakAddress);
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println(F("Connection failed"));
  } 

  // Check for a response from the server, and route it
  // out the serial port.
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      Serial.print(c);
    }      
  }
  Serial.println();
  client.stop();
}

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

  if (!client.connect(thingSpeakAddress, 80)) {
     return;
  }
  client.print(F("GET /update?key="));
  client.print(thingSpeakAPIKey);
  client.print(F("&"));
  client.print(tsData);
  client.print(F(" HTTP/1.1\r\nHost: api.thingspeak.com\r\n\r\n"));
  client.println();
  Serial.println("uploaded to think speak");
}


void waterDetection(){
   current_millis = millis();
  if(current_millis - previous_millis >= detect_interval) {
    ad_value=analogRead(water_ain);
    if(ad_value>200)
    {
      Serial.println("High water level");
      updateThingSpeak("1=1");
      sendSMS(sendNumber, URLEncode("High water level in well"));
    }
    else
    {
      Serial.println("Low water level");
      updateThingSpeak("1=0");
    }
    previous_millis = current_millis;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  isConnected = wifiConnect();
}
void loop(){
  if(!isConnected){
    wifiConnect();
  }
  else{
    waterDetection();
  }
}

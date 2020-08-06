/*
 
*/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <Ticker.h>       //For LED status
#include <ESP8266mDNS.h>  //For OTA
#include <ArduinoOTA.h>   //For OTA
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <Wire.h>

Ticker ticker;

const char* mqttServer = "***************";
const int mqttPort = 1883;
const char* mqttUser = "***************";
const char* mqttPassword = "***************";
const char* feed = "A000";                //This is the channel where the commands come in
const char* stateFeed = "A000/state";     //This is the channel where the device talks to NORA

//Declare pins and variables
  //Numerical display
  int A[2] = {D0,D7};
  int B[2] = {D4,D8};
  int C[2] = {D5,3};
  int D[2] = {D6,1};
int IR = D3;

//Global place to store current information
bool JSONstate = false;
int JSONbrightness = 20;
float JSONhue = 0;
float JSONsat = 0;
float JSONval = 0;
int tickVar = 0;

//Set up WiFi and MQTT Clients
WiFiClient espClient;
PubSubClient client(espClient);
IRrecv irrecv(IR);
decode_results results;

//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void setup() {
  //Prepare GPIO
  pinMode(A[0],OUTPUT);
  pinMode(B[0],OUTPUT);
  pinMode(C[0],OUTPUT);
  pinMode(D[0],OUTPUT);
  pinMode(A[1],OUTPUT);
  pinMode(B[1],OUTPUT);
  pinMode(C[1],OUTPUT);
  pinMode(D[1],OUTPUT);

  //Start LED blinking 
  ticker.attach(0.6, tick); //Blink display as we do setup
  
  // Start the receiver
  irrecv.enableIRIn(); 

  //Start the I2C stuff
  Wire.begin();

  WiFiManager wifiManager;                          //Create wifi manager
  wifiManager.setAPCallback(configModeCallback);    //If connection could not be established with previous settings then run this callback
  wifiManager.autoConnect("Bridge_Audio");             //Connect 

  //WiFi connected sucessfully; Connect to MQTT server
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  //Wait until the MQTT server is connected
  while (!client.connected()) {
      //Login to server
      //clientID,mqttUsername,Password, willFeed, willQOS, willRetain, willMessage
      client.connect(feed,mqttUser,mqttPassword);  
   }
  
  //Susbcribe to correct feed
  client.subscribe(feed);
   
  //Tell the server the state of the speaker
  char json[100];
  sprintf(json,"{\"on\":true,\"brightness\":%d}",JSONbrightness);
  client.publish(stateFeed, json );  

  updateDisplay();
  setVolume(JSONbrightness);

  ticker.detach();
  //Start timer for display turn off
  ticker.attach(5, displayOff);
 
  //Set functions and stuff for OTA updates
  ArduinoOTA.setHostname(feed);
  ArduinoOTA.setPassword("***************");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } 
  });
  ArduinoOTA.begin();
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void tick()
{
  //toggle state    
  if(tickVar == 0) {
    zero(0);
    one(1);
    tickVar++;
  }
  else {
    zero(1);
    one(0);
    tickVar--;
  }
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
  ArduinoOTA.handle();
  checkConnection();
  checkIR();
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void callback(char* topic, byte* payload, unsigned int length) {
  // Parse JSON object  
  payload[length] = '\0';
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  // Test if parsing succeeds.----------------------------------------------------
  if (!error) {
    //Store values in variables
    JSONstate = doc["on"];
    JSONbrightness = doc["brightness"];

    //Turn on device---------------------------------------------------------------
    if(JSONstate) {
        //No power controls enabled
    }
    else {
       //No power controls enabled
    }
    
    //Control brightness(volume) ---------------------------------------------------
    if(JSONbrightness == 100) {
      JSONbrightness--;   //Make sure the brightness is never 100. only 2 digits on display
    }
    updateDisplay();
    setVolume(JSONbrightness);
    
    //Tell the server the state of the speaker
    char json[100];
    sprintf(json,"{\"on\":true,\"brightness\":%d}",JSONbrightness);
    client.publish(stateFeed, json );  
    
  }
  //JSON was not recieved-------------------------------------------------------------
  else if (!strcmp((char*)payload, "INFO"))
  {
    char buff[16];
    sprintf(buff, "IP: %d.%d.%d.%d   Sketch: WiFi_Audio_MQTT_NORA",WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
    client.publish(stateFeed, buff );  
  }
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void displayOff() {
  digitalWrite(A[0],1);
  digitalWrite(B[0],1);
  digitalWrite(C[0],1);
  digitalWrite(D[0],1);
  digitalWrite(A[1],1);
  digitalWrite(B[1],1);
  digitalWrite(C[1],1);
  digitalWrite(D[1],1);
  ticker.detach();
}

//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
int checkConnection()
{
  //If the MQTT client is disconnected, then the wifi might be disconnected
  if(!client.connected()) {
    
    //clientID,mqttUsername,Password, willFeed, willQOS, willRetain, willMessage
    client.connect(feed,mqttUser,mqttPassword);   
    if(client.connected()) {
      client.subscribe(feed);
    }
  }
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void setVolume(int vol) {
  byte rval = map(vol, 0, 99, 0, 127);
  Wire.beginTransmission(0x2F);
  Wire.write(rval); //
  Wire.endTransmission();
}

//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void checkIR() {
//volume up roku: E0E0E01F
//volume dn roku: E0E0D02F
//volume up RC: FFA857
//volume dn RC: FFB24D
   if (irrecv.decode(&results)) {
      if(results.value == 0xFFA857) {
        JSONbrightness++;
        if(JSONbrightness >= 100) {
          JSONbrightness = 99;
        }
      }
      else if(results.value == 0xFFB24D) {
        JSONbrightness--;
        if(JSONbrightness < 0) {
          JSONbrightness = 0;
        }
      }
      irrecv.resume();
      updateDisplay();
      setVolume(JSONbrightness);
      
      //Tell the server the state of the speaker
      char json[100];
      sprintf(json,"{\"on\":true,\"brightness\":%d}",JSONbrightness);
      client.publish(stateFeed, json );
   }
   
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void updateDisplay() {
    //tens place
    switch(JSONbrightness/10) {
      case 0:
        zero(1);
        break;
      case 1:
        one(1);
        break;
      case 2:
        two(1);
        break;
      case 3:
        three(1);
        break;
      case 4:
        four(1);
        break;
      case 5:
        five(1);
        break;
      case 6:
        six(1);
        break;
      case 7:
        seven(1);
        break;
      case 8:
        eight(1);
        break;
      case 9:
        nine(1);
    }
    //ones place
    switch(JSONbrightness%10) {
      case 0:
        zero(0);
        break;
      case 1:
        one(0);
        break;
      case 2:
        two(0);
        break;
      case 3:
        three(0);
        break;
      case 4:
        four(0);
        break;
      case 5:
        five(0);
        break;
      case 6:
        six(0);
        break;
      case 7:
        seven(0);
        break;
      case 8:
        eight(0);
        break;
      case 9:
        nine(0);
    }
    
    //Start timer for display turn off
    ticker.attach(8, displayOff);
}
//===================================================================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//===================================================================================================
void zero(int i) {
  digitalWrite(A[i],0);
  digitalWrite(B[i],0);
  digitalWrite(C[i],0);
  digitalWrite(D[i],0);
}

void one(int i) {
  digitalWrite(A[i],1);
  digitalWrite(B[i],0);
  digitalWrite(C[i],0);
  digitalWrite(D[i],0);
}
void two(int i) {
  digitalWrite(A[i],0);
  digitalWrite(B[i],1);
  digitalWrite(C[i],0);
  digitalWrite(D[i],0);
}

void three(int i) {
  digitalWrite(A[i],1);
  digitalWrite(B[i],1);
  digitalWrite(C[i],0);
  digitalWrite(D[i],0);
}

void four(int i) {
  digitalWrite(A[i],0);
  digitalWrite(B[i],0);
  digitalWrite(C[i],1);
  digitalWrite(D[i],0);
}

void five(int i) {
  digitalWrite(A[i],1);
  digitalWrite(B[i],0);
  digitalWrite(C[i],1);
  digitalWrite(D[i],0);
}

void six(int i) {
  digitalWrite(A[i],0);
  digitalWrite(B[i],1);
  digitalWrite(C[i],1);
  digitalWrite(D[i],0);
}

void seven(int i) {
  digitalWrite(A[i],1);
  digitalWrite(B[i],1);
  digitalWrite(C[i],1);
  digitalWrite(D[i],0);
}

void eight(int i) {
  digitalWrite(A[i],0);
  digitalWrite(B[i],0);
  digitalWrite(C[i],0);
  digitalWrite(D[i],1);
}

void nine(int i) {
  digitalWrite(A[i],1);
  digitalWrite(B[i],0);
  digitalWrite(C[i],0);
  digitalWrite(D[i],1);
}

/*******************************************************************
    A sketch for controlling a toy car with using a web page 
    hosted on a ESP8266
    
    Main Hardware:
    - NodeMCU Development Board cp2102 (Look for the one with the square chip beside the USB port)
    - NodeMCU Motor Shield (L2932)
    
    Written by Brian Lough
    https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA
 *******************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// These are the pins used to control the motor shield

#define MOTOR_A_POWER D8 
#define MOTOR_A_DIRECTION1 D7
#define MOTOR_A_DIRECTION2 D6

#define MOTOR_B_POWER D1 
#define MOTOR_B_DIRECTION1 D2
#define MOTOR_B_DIRECTION2 D3

// drivePower sets how fast the car goes
// Can be set between 0 and 1023 (although car problaly wont move if values are too low)
int drivePower = 1000;

// ----------------
// Set your WiFi SSID and Password here
// ----------------
const char* ssid = "CenturyLink3741";
const char* password = "Southside";

ESP8266WebServer server(80);

const char *webpage = 
#include "motorPage.h"
;

void handleRoot() {

  server.send(200, "text/html", webpage);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){

  pinMode(MOTOR_A_POWER, OUTPUT);     
  pinMode(MOTOR_A_DIRECTION1, OUTPUT);
  pinMode(MOTOR_A_DIRECTION2, OUTPUT);
  pinMode(MOTOR_B_POWER, OUTPUT);
  pinMode(MOTOR_B_DIRECTION1, OUTPUT);
  pinMode(MOTOR_B_DIRECTION2, OUTPUT);

  digitalWrite(MOTOR_A_DIRECTION1,HIGH);
  digitalWrite(MOTOR_A_DIRECTION2,LOW);
  digitalWrite(MOTOR_B_DIRECTION1,HIGH);
  digitalWrite(MOTOR_B_DIRECTION2,LOW);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("WifiCar")) {
    Serial.println("MDNS Responder Started");
  }

  server.on("/", handleRoot);

  server.on("/forward", [](){
    Serial.println("forward");
    digitalWrite(MOTOR_A_DIRECTION1,LOW);
    digitalWrite(MOTOR_A_DIRECTION2,HIGH);
    digitalWrite(MOTOR_B_DIRECTION1,LOW);
    digitalWrite(MOTOR_B_DIRECTION2,HIGH);
    analogWrite(MOTOR_A_POWER, drivePower);
    analogWrite(MOTOR_B_POWER, drivePower);
    server.send(200, "text/plain", "forward");
  });

  server.on("/driveStop", [](){
    Serial.println("driveStop");
    digitalWrite(MOTOR_A_DIRECTION1,HIGH);
    digitalWrite(MOTOR_A_DIRECTION2,LOW);
    digitalWrite(MOTOR_B_DIRECTION1,HIGH);
    digitalWrite(MOTOR_B_DIRECTION2,LOW);
    analogWrite(MOTOR_A_POWER, 0);
    analogWrite(MOTOR_B_POWER, 0);
    server.send(200, "text/plain", "driveStop");
  });

  server.on("/back", [](){
    Serial.println("back");
    digitalWrite(MOTOR_A_DIRECTION1,HIGH);
    digitalWrite(MOTOR_A_DIRECTION2,LOW);
    digitalWrite(MOTOR_B_DIRECTION1,HIGH);
    digitalWrite(MOTOR_B_DIRECTION2,LOW);
    analogWrite(MOTOR_A_POWER, drivePower);
    analogWrite(MOTOR_B_POWER, drivePower);
    server.send(200, "text/plain", "back");
  });

  server.on("/right", [](){
    Serial.println("right");
    digitalWrite(MOTOR_A_DIRECTION1,HIGH);
    digitalWrite(MOTOR_A_DIRECTION2,LOW);
    digitalWrite(MOTOR_B_DIRECTION1,LOW);
    digitalWrite(MOTOR_B_DIRECTION2,HIGH);
    analogWrite(MOTOR_A_POWER, drivePower);
    analogWrite(MOTOR_B_POWER, drivePower);
    server.send(200, "text/plain", "right");
  });

  server.on("/left", [](){
    Serial.println("left");
    digitalWrite(MOTOR_A_DIRECTION1,LOW);
    digitalWrite(MOTOR_A_DIRECTION2,HIGH);
    digitalWrite(MOTOR_B_DIRECTION1,HIGH);
    digitalWrite(MOTOR_B_DIRECTION2,LOW);
    analogWrite(MOTOR_A_POWER, drivePower);
    analogWrite(MOTOR_B_POWER, drivePower);
    server.send(200, "text/plain", "left");
  });

  server.on("/steerStop", [](){
    Serial.println("steerStop");
    analogWrite(MOTOR_A_POWER, 0);
    analogWrite(MOTOR_B_POWER, 0);
    server.send(200, "text/plain", "steerStop");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop(void){
  server.handleClient();
}

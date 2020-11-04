#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"

#define GARAGE_ONE_RELAY_PIN 5
#define GARAGE_TWO_RELAY_PIN 6

// prototypes
boolean connectWifi();

//on/off callbacks 
bool garageOneOpen();
bool garageOneClose();
bool garageTwoOpen();
bool garageTwoClose();

// Change this before you flash
const char* ssid = "5124EscambiaTerr";
const char* password = "M4v1e?Brun0";

boolean wifiConnected = false;

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *garageOne = NULL;
Switch *garageTwo = NULL;

bool isGarageOneOpen = false;
bool isGarageTwoOpen = false;

void setup()
{
  Serial.begin(9600);
   
  // Initialise wifi connection
  wifiConnected = connectWifi();
  
  if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();
    
    // Define your switches here. Max 10
    // Format: Alexa invocation name, local port no, on callback, off callback
    garageOne = new Switch("GarageOne", 80, garageOneOpen, garageOneClose);
    garageTwo = new Switch("GarageTwo", 81, garageTwoOpen, garageTwoClose);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*garageOne);
    upnpBroadcastResponder.addDevice(*garageTwo);
  }
}
 
void loop()
{
	 if(wifiConnected){
      upnpBroadcastResponder.serverLoop();
      
      garageTwo->serverLoop();
      garageOne->serverLoop();
	 }
}

bool pressButton(int relayPin)
{
  digitalWrite(relayPin, HIGH);
  digitalWrite(relayPin, LOW);
}

bool garageOneOpen() {
    Serial.println("Opening garage one ...");

    pressButton(GARAGE_ONE_RELAY_PIN);
    
    isGarageOneOpen = true;    
    return isGarageOneOpen;
}

bool garageOneClose() {
    Serial.println("Closing garage one ...");

    pressButton(GARAGE_ONE_RELAY_PIN);

    isGarageOneOpen = false;
    return isGarageOneOpen;
}

bool garageTwoOpen() {
    Serial.println("Opening garage 2 ...");

    pressButton(GARAGE_TWO_RELAY_PIN);

    isGarageTwoOpen = true;
    return isGarageTwoOpen;
}

bool garageTwoClose() {
  Serial.println("Closing garage 2 ...");

  pressButton(GARAGE_TWO_RELAY_PIN);

  isGarageTwoOpen = false;
  return isGarageTwoOpen;
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

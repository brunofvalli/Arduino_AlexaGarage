/*
 Version 0.4 - April 26 2019
*/ 

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries (use the correct version)
#include <StreamString.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#define MyApiKey "f6e22704-fa67-4938-b8bf-cca186807067" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "5124EscambiaTerr" // TODO: Change to your Wifi network SSID
#define MyWifiPassword "M4v1e?Brun0" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

const int relayPin1 = 4; // TODO: Change according to your board
const int relayPin2 = 5; // TODO: Change according to your board

String deviceId1 = "5fa41e08b1c8c45d66218555";
String deviceId2 = "5fa41f42b1c8c45d66218573";

// deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

void pressButtonRelay(int relayPin)
{
    digitalWrite(relayPin, HIGH); // turn on relay with voltage HIGH
    delay(1000);
    digitalWrite(relayPin, LOW); // turn on relay with voltage HIGH
}

void turnOn(String deviceId) {
  if (deviceId == deviceId1) // Device ID of first device
  {  
    Serial.print("Turn on device id: ");
    Serial.print(relayPin1);
    Serial.print(" - ");
    Serial.println(deviceId);
    
    digitalWrite(relayPin1, HIGH);
    pressButtonRelay(relayPin1);
  } 
  else if (deviceId == deviceId2) // Device ID of second device
  { 
    Serial.print("Turn on device id: ");
    Serial.print(relayPin2);
    Serial.print(" - ");
    Serial.println(deviceId);
    digitalWrite(relayPin2, HIGH);
    pressButtonRelay(relayPin2);
  }
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

void turnOff(String deviceId) {
   if (deviceId == deviceId1) // Device ID of first device
   {  
     Serial.print("Turn off Device ID: ");
     Serial.print(relayPin1);
     Serial.print(" - ");
     Serial.println(deviceId);

     digitalWrite(relayPin1, LOW);
     pressButtonRelay(relayPin1);
   }
   else if (deviceId == deviceId2) // Device ID of second device
   { 
     Serial.print("Turn off Device ID: ");
     Serial.print(relayPin2);
     Serial.print(" - ");
     Serial.println(deviceId);
     
     digitalWrite(relayPin2, LOW);
     pressButtonRelay(relayPin2);
  }
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "SetTargetTemperature") {
            String deviceId = json ["deviceId"];     
            String action = json ["action"];
            String value = json ["value"];
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);  

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
}

// If you want a push button: https://github.com/kakopappa/sinric/blob/master/arduino_examples/switch_with_push_button.ino  

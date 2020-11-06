/*  Viral Science www.viralsciencecreativity.com www.youtube.com/c/viralscience
 *  Corona Virus Tracker 16X2 LCD Display
 *  Corona Data: https://www.worldometers.info/coronavirus/
 *  ThingSpeak: https://thingspeak.com/
 */

#include <LiquidCrystal.h>
#include <ESP8266WiFi.h> //Use ESP8266 functions
#include <ESP8266HTTPClient.h>

#include <Wire.h> // Only needed for Arduino 1.6.5 and earlier

LiquidCrystal lcd(D0, D1, D2, D3, D4, D5);

const char *ssid = "5124EscambiaTerr";       //WIFI SSID Name
const char *password = "M4v1e?Brun0";        //WIFI Password
const char *host = "www.epochconverter.com"; //We read the data from this host
const int httpPortRead = 80;

const char *url1 = "/daynumbers";                                              //Change this URL Cases
int To_remove;                                                                 //There are some irrelevant data on the string and here's how I keep the index
                                                                               //of those characters
String Data_Raw, DayOfYear, DateEntry; //Here I keep the numbers that I got

WiFiClient client; //Create a WiFi client and http client
HTTPClient http;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(1, 0);
  lcd.print("Day Of Year"); //Change Your Country Name
  lcd.setCursor(1, 1);
  lcd.print("Bruno F Valli"); //Change Name if you want

  Serial.begin(115200);
  WiFi.disconnect(); //Disconnect and reconnect to the Wifi you set
  delay(1000);
  WiFi.begin(ssid, password);
  Serial.println("Connected to the WiFi network"); //Display feedback on the serial monitor
  Serial.println(WiFi.localIP());
}

void loop()
{
  //Reading 1: Reading of cases
  if (http.begin(host, httpPortRead, url1)) //Connect to the host and the url
  {
    int httpCode = http.GET(); //Check feedback if there's a response
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        Data_Raw = http.getString(); //Here we store the raw data string

        // <span id="td2">311</span>
        DayOfYear = Data_Raw;
        To_remove = DayOfYear.indexOf("<span id=\"td2\">"); //I look for the position of this symbol ">"
        DayOfYear.remove(0, To_remove + 17);               //I remove it and everything that's before
        To_remove = DayOfYear.indexOf("<");                 //I look for the position of this symbol ">"
        DayOfYear.remove(To_remove, (DayOfYear.length() - To_remove));   //I remove it and everything that's after

        // <span id="td1">Friday, November 6, 2020</span>
        DateEntry = Data_Raw;
        To_remove = DateEntry.indexOf("<span id=\"td1\">"); //I look for the position of this symbol ">"
        DateEntry.remove(0, To_remove + 17);               //I remove it and everything that's before
        To_remove = DateEntry.indexOf("<");                 //I look for the position of this symbol ">"
        DateEntry.remove(To_remove, (DateEntry.length() - To_remove));   //I remove it and everything that's after

      }
    }
    else //If we can't get data
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else //If we can't connect to the HTTP
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  while (WiFi.status() != WL_CONNECTED) //In case the Wifi connexion is lost
  {
    WiFi.disconnect();
    delay(1000);

    WiFi.begin(ssid, password);
    Serial.println("Reconnecting to WiFi..");
    delay(10000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(DateEntry);

  lcd.setCursor(0, 1);
  lcd.print(DayOfYear);

  delay(1000);
}

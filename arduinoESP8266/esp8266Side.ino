//Include Lib for Arduino to Nodemcu ArduinoJson 6.17.0
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "ThingSpeak.h" 

#define SECRET_SSID "XXXXXXX"    // replace MySSID with your WiFi network name
#define SECRET_PASS "XXXXXXX"  // replace MyPassword with your WiFi password
#define SECRET_CH_ID XXXXXXX     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "XXXXXXX"   // replace XYZ with your channel write API Key

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";


//Timer to run Arduino code every 5 seconds
unsigned long previousMillis = 0;
unsigned long currentMillis;
const unsigned long period = 10000;  

//D6 = Rx & D5 = Tx
SoftwareSerial nodemcu(D5, D6);

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  nodemcu.begin(9600);
  while (!Serial) continue;
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  
}

void loop() {
    // Connect or reconnect to WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(SECRET_SSID);
        while (WiFi.status() != WL_CONNECTED) {
            WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
            Serial.print(".");
            delay(5000);
        }
        Serial.println("\nConnected.");
    }

    // Get current time
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= period) {
        previousMillis = currentMillis; // Update previousMillis for accurate timing

        StaticJsonDocument<1000> doc;
        DeserializationError error;

        do {
            error = deserializeJson(doc, nodemcu);
            if (error) {
                Serial.println("Invalid JSON Object");
                delay(500);
            }
        } while (error);

        Serial.println("JSON Object Received");
        Serial.print("Received Humidity:  ");
        float hum = doc["humidity"];
        Serial.println(hum);
        Serial.print("Received Temperature:  ");
        float temp = doc["temperature"];
        Serial.println(temp);
        Serial.println("-----------------------------------------");

        // Set the fields with the values
        ThingSpeak.setField(1, hum);
        ThingSpeak.setField(2, temp);

        // Set the status
        ThingSpeak.setStatus(myStatus);

        // Write to the ThingSpeak channel
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if (x == 200) {
            Serial.println("Channel update successful.");
        } else {
            Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
    }
}

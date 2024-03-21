#include<Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "ClosedCube_HDC1080.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router
ClosedCube_HDC1080 hdc1080;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(128, 64, & Wire, -1);

const char* ssid = "XXXXX"; //--> Your wifi name or SSID.
const char* password = "XXXXX"; //--> Your wifi password.

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbzyGQgY6vD4aqOAqnm48Yz-XXXXX"; //--> spreadsheet script ID

float thi;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(2000);
  delay(500);

  //initialize sensor HDC1080
  hdc1080.begin(0x40);

  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");

  pinMode(ON_Board_LED, OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------
  Serial.println("I'm awake, but I'm going into deep sleep mode for 30 seconds");

  client.setInsecure();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  Serial.println("I'm awake, but I'm going into deep sleep mode for 30 seconds");
  //ESP.deepSleep(30e6);
  
}

void loop() {

  // change the values
  float nt = hdc1080.readTemperature();
  float nh = hdc1080.readHumidity();
  if (isnan(nt) || isnan(nh))

  {
    Serial.println("Failed to read from HDC1080 sensor!");
    return;
  }

  //final calibration based on algorithm
  float t = (((1.0081 * nt) - (0.3202)) + 1.6); //Rsquare = 0.9999999125
  float h = (((1.0305 * nh) - (0.7127)) - 16.54); // Rqsuare = 0.9996
  float tf = ((t*1.8)+32); //convertion of the temperature into Farenhiet
  thi = (tf - (0.55-(0.55*h/100))*(tf-58));

  /*
   * THI < 68 below heat stresst threshold
   * 68 - 79 Mild to moderate 
   * 80 - 89 moderate to severe
   * >90 severe
   * 
   * http:thi
   */

  //display in serial result
  String temp = "Temperature : " + String(t) + " Degree Celcius";
  String humid = "Humidity : " + String(h) + " %";
  String temphumid = "THI Index : " + String(thi) + " THI Value";
  Serial.println(temp);
  Serial.println(humid);
  Serial.println(temphumid);

  if(thi>80)
  {
    sendData2(t, h, thi); //--> Calls the sendData Subroutine
    }
  else
  {
    sendData1(t, h, thi);    
  }
  
}

// Subroutine for sending data to Google Sheets SendData1
void sendData1(float t, float h, float thi) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

/*
  //----------------------------------------Processing data and sending data
  String string_t =  String(t);
  // String string_temperature =  String(tem, DEC);
  String string_h =  String(h);
  String string_thi =  String(thi);
  String statusthi = "Poor";
  String latlong = "3.2193164975398765,101.5843277609631";
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_t +
               "&humidity=" + string_h + "&thi=" + string_thi + "&latlong=" 
               + latlong + "&status=" + statusthi;

  Serial.print("requesting URL: ");
  Serial.println(url);
  */

  String string_t =  String(t);
  // String string_temperature =  String(tem, DEC);
  String string_h =  String(h);
  String string_thi =  String(thi);
  String statusthi = "Good";
  String latlong = "3.2193164975398765,101.5843277609631";
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_t +
               "&humidity=" + string_h + "&thi=" + string_thi + "&status=" + statusthi;

  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------

  //display on OLED LCD
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Wataverse Technology");
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("====================");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Temp [C] & Hum [%]");
  display.setTextSize(1.7); //1.5
  display.setCursor(0, 30);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1);
  display.print("C ");
  display.setTextSize(1.7); //1.5
  display.setCursor(60, 30);
  display.print(h);
  display.print(" %");

  //status THI
  display.setTextSize(1.5);
  display.setCursor(0, 40);
  display.print("Status THI");
  display.setTextSize(1.5);
  display.setCursor(0, 50);
  display.print(statusthi);
  display.setTextSize(1.5);
  display.setCursor(0, 70);
  display.print(thi);

  display.display();

  delay(10000);

}

//====== Send Data 2 for option No. 2

void sendData2(float t, float h, float thi) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

/*
  //----------------------------------------Processing data and sending data
  String string_t =  String(t);
  // String string_temperature =  String(tem, DEC);
  String string_h =  String(h);
  String string_thi =  String(thi);
  String statusthi = "Poor";
  String latlong = "3.2193164975398765,101.5843277609631";
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_t +
               "&humidity=" + string_h + "&thi=" + string_thi + "&latlong=" 
               + latlong + "&status=" + statusthi;

  Serial.print("requesting URL: ");
  Serial.println(url);
  */

  String string_t =  String(t);
  // String string_temperature =  String(tem, DEC);
  String string_h =  String(h);
  String string_thi =  String(thi);
  String statusthi = "Poor";
  String latlong = "3.2193164975398765,101.5843277609631";
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_t +
               "&humidity=" + string_h + "&thi=" + string_thi + "&status=" + statusthi;

  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------

  //display on OLED LCD
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Wataverse Technology");
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("====================");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Temp [C] & Hum [%]");
  display.setTextSize(1.7); //1.5
  display.setCursor(0, 30);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(1);
  display.print("C ");
  display.setTextSize(1.7); //1.5
  display.setCursor(60, 30);
  display.print(h);
  display.print(" %");

  //status THI
  display.setTextSize(1.5);
  display.setCursor(0, 40);
  display.print("Status THI");
  display.setTextSize(1.5);
  display.setCursor(0, 50);
  display.print(statusthi);
  display.setTextSize(1.5);
  display.setCursor(0, 70);
  display.print(thi);
  
  display.display();

  delay(10000);
}

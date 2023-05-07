#include <DHT.h> // Including library for dht
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define REPORTING_PERIOD_MS 1000
String apiKey = "0SQOFM0JXTL6WNKY"; // Enter your Write API key from 
ThingSpeak
const char *ssid = "Wifi"; // replace with your wifi ssid and wpa2 key
const char *pass = "WagonR2016";
const char* server = "api.thingspeak.com";
int redLed=D5;
int greenLed=D6;
PulseOximeter pox;
uint32_t tsLastReport = 0;
// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS D3 
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS); 
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
#define DHTPIN D4 //pin where the dht11 is connected
DHT dht(DHTPIN, DHT11);
WiFiClient client;
void onBeatDetected()
{
 Serial.println("Beat!");
}
void setup() 
{
 Serial.begin(115200);
 delay(10);
 dht.begin();
 Serial.println("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) 
 {
 delay(500);
 Serial.print(".");
 digitalWrite(redLed,HIGH);
 }
 Serial.println("");
 Serial.println("WiFi connected");
digitalWrite(greenLed,HIGH);
 sensors.begin(); // Start up the library
 Serial.begin(9600);
 Serial.print("Initializing pulse oximeter..");
 // Initialize the PulseOximeter instance
 // Failures are generally due to an improper I2C wiring, missing power supply
 // or wrong target chip
 if (!pox.begin()) {
 Serial.println("FAILED");
 for(;;);
 } else {
 Serial.println("SUCCESS");
 }
 pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 // Register a callback for the beat detection
 pox.setOnBeatDetectedCallback(onBeatDetected);
}
void loop() 
{
 // Make sure to call update as fast as possible
 pox.update();
 if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
 Serial.print("Heart rate:");
 Serial.print(pox.getHeartRate());
 Serial.print("bpm / SpO2:");
 Serial.print(pox.getSpO2());
 Serial.println("%");
 sensors.requestTemperatures(); 
 tsLastReport = millis();
 delay(500);
 
 //print the temperature in Celsius
 Serial.print("\nTemperature: ");
 Serial.print(sensors.getTempCByIndex(0));
 Serial.print((char)176);//shows degrees character
 Serial.print("C | ");
 //print the temperature in Fahrenheit
 float temp_in_F=(sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0;
 Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
 Serial.print((char)176);//shows degrees character
 Serial.println("F");
 
 delay(500);
 float h = dht.readHumidity();
 float t = dht.readTemperature();
 
 if (isnan(h) || isnan(t)) 
 {
 Serial.println("Failed to read from DHT sensor!");
 return;
 }
 if (client.connect(server,80)) // "184.106.153.149" or 
api.thingspeak.com
 { 
 
 String postStr = apiKey;
 postStr +="&field1=";
 postStr += String(t);
 postStr +="&field2=";
 postStr += String(h);
 postStr +="&field3=";
 postStr += String(pox.getHeartRate());
 postStr +="&field4=";
 postStr += String(pox.getSpO2());
 postStr +="&field5=";
 postStr += String(temp_in_F);
 postStr += "\r\n\r\n";
 client.print("POST /update HTTP/1.1\n");
 client.print("Host: api.thingspeak.com\n");
 client.print("Connection: close\n");
 client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
 client.print("Content-Type: application/x-www-formurlencoded\n");
 client.print("Content-Length: ");
 client.print(postStr.length());
 client.print("\n\n");
 client.print(postStr);
 Serial.print("Temperature: ");
 Serial.print(t);
 Serial.print(" degrees Celcius, Humidity: ");
 Serial.print(h);
 Serial.println("%. Send to Thingspeak.");
 }
 client.stop();
 Serial.println("Waiting...");
 
 // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 
seconds
 delay(10000);
}
 }

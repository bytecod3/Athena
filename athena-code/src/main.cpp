#include <Arduino.h>
#include "defs.h"
#include "debug.h"
#include "WiFiManager.h"
#include <TinyGPSPlus.h>

// create GPS object
TinyGPSPlus gps;

const char* WIFI_SSID = "Athena-wardriver";
const char* WIFI_PASSWORD = "12345678";

void configWIFI();
void GPSInit();
void updateSerial();
void GPSDisplayInfo();

/**
 * Create a AP with SSID and PASSWORD to allow wifi connection to 
 * custom networks 
*/
void configWIFI() {
  debugln("Configuring WIFI");
  WiFiManager wifiManager;

 wifiManager.erase();
  if(!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD)) {
    debugln("Failed to connect");
    ESP.restart();
    delay(1000);
  }

  // at this point we are connected to WIFI
  debug("IP: ");
  debugln(WiFi.localIP());

}

/**
 * Initialize the GPS object 
*/
void GPSInit() {
  Serial2.begin(GPS_BAUDRATE);
  delay(1000);
}

/**
 * Loopback between serial1 and serial2
*/
void updateSerial() {
  delay(500);
  while (Serial.available())  {
    Serial2.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())  {
    Serial.write(Serial2.read());//Forward what Software Serial received to Serial Port
  }
}

void GPSDisplayInfo() {
  debug(F("Location: "));
  if(gps.location.isValid()) {
    Serial.print(gps.location.lat());
    debug(F(","));
    Serial.print(gps.location.lng(), 6);

  } else {
    debugln(F("Invalid coordinates"));

  }

}

void setup() {
  Serial.begin(9600);
  // configWIFI();
  GPSInit();
  
}

void loop() {

  //updateSerial();

  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      GPSDisplayInfo();
  
}


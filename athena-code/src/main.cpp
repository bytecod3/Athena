#include <Arduino.h>
#include "debug.h"
#include "WiFiManager.h"

const char* WIFI_SSID = "Athena-wardriver";
const char* WIFI_PASSWORD = "12345678";

void configWIFI();

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


void setup() {
  Serial.begin(115200);
  configWIFI();
  
}


void loop() {
  
}


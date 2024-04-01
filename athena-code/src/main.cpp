#include <Arduino.h>
#include "defs.h"
#include "debug.h"
#include "WiFiManager.h"
#include <TinyGPSPlus.h>
#include "SD.h"
#include "FS.h"
#include <freertos/FreeRTOS.h>

// create GPS object
TinyGPSPlus gps;

const char* WIFI_SSID = "Athena-wardriver";
const char* WIFI_PASSWORD = "12345678";

void configDynamicWIFI();
void GPSInit();
void updateSerial();
void GPSDisplayInfo();

/**
 * Create a AP with SSID and PASSWORD to allow wifi connection to 
 * custom networks 
*/
void configDynamicWIFI() {
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

/**
 * compare mac address to avoid logging the same network
*/
int compareMAC() {

  
}

void WiFiScanSetup() {
  // set wifi station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

/**
 * Scan all available wifi networks
*/
void scanWIFI(void* parameter) {
  while(true) {
    int n = WiFi.scanNetworks();
    Serial.println("Scan Done");

    if(n == 0) {
      debugln("No networks found");
    } else {
      debug(n);
      debugln("Networks found");

      for(int i = 0; i < n; i++) {
        uint8_t* raw_bssid = WiFi.BSSID(i);
        char bssid[18];
        sprintf(
          bssid,
          "%02X:%02X:%02X:%02X:%02X:%02X",
          raw_bssid[0],
          raw_bssid[1],
          raw_bssid[2],
          raw_bssid[3],
          raw_bssid[4],
          raw_bssid[5]
        );

        // compare the mac addresses


        // print ssid and rssi for each network 
        debug(i); debug(","); debug(WiFi.SSID(i).c_str());
        debug(","); debugf("%4d", WiFi.RSSI(i));
        debug(","); debugf("%2d", WiFi.channel(i));
        switch (WiFi.encryptionType(i))
        {
          case WIFI_AUTH_OPEN:
                debug("open");
                break;
            case WIFI_AUTH_WEP:
                debug("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                debug("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                debug("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                Serial.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                debug("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                debug("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                debug("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                debug("WAPI");
                break;
            default:
                debug("unknown");
        }

        debugln();

      }

    }
  }

  // free memory
  WiFi.scanDelete();

  // some delay before scanning again
  delay(SCAN_DELAY);
}

void setup() {
  Serial.begin(9600);
  // configDynamicWIFI();
  GPSInit();

  WiFiScanSetup();

  // print to screen
  Serial.println("Ready..");

  xTaskCreatePinnedToCore(
    scanWIFI, 
    "scanWIFI",
    10000,
    NULL, 
    1,
    NULL,
    0
  );
  
}

void loop() {

  //updateSerial();

  // while (Serial2.available() > 0) {
  //   if (gps.encode(Serial2.read())) {
  //     GPSDisplayInfo();
  //   }
  
  // }



}

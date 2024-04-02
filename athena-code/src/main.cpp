#include <Arduino.h>
#include "defs.h"
#include "debug.h"
#include "WiFiManager.h"
#include <TinyGPSPlus.h>
#include "SD.h"
#include "FS.h"
#include "U8g2lib.h"
#include "bitmaps.h"

// create GPS object
TinyGPSPlus gps;

const char* WIFI_SSID = "Athena-wardriver";
const char* WIFI_PASSWORD = "12345678";

/* struct to hold mac address of a network */
typedef struct {
  unsigned char bytes[RAW_BSSID_LENGTH];
} mac_address;

/* struct to hold latitude and longitude*/
typedef struct {
  double latitude;
  double longitude;
} Location;

Location location;
int fix_status;

/* array to hold scanned MAC addresses */
mac_address seen_macs[MAX_NETWORKS];
unsigned int seen_mac_index = 0;
char data_row[100]; // to hold the row being written to SD Card

/*OLED screen variables */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C screen(/*R2: rotation 180*/U8G2_R0, /*reset*/U8X8_PIN_NONE, /* clock */ OLED_SCL, /* data */ OLED_SDA);
unsigned int screen_counter = 0;
int font_height = 10; // change the height if you change the font
char nets[10]; // to hold number of networks found

void configDynamicWIFI();
void GPSInit();
void updateSerial();
int GPSGetLocation();
void WiFiScanSetup();
void seenMAC(mac_address);
int compareMAC(mac_address, mac_address);
void saveMAC();
void SDInit();
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
String security_int_to_string(int security_type);
void screenInit();
void displayMessage(char*, int);
void displaySplashScreen();

/**
 * @brief Initialize oled screen
 * @param none
*/
void screenInit() {
    screen.begin();
    // screen.setColorIndex(1);   
}

/**
 * show splash screen
*/
void displaySplashScreen() {
  screen.firstPage();
  do {
    screen.drawBitmap(16, 17, 32/8, 32, logo);
  } while (screen.nextPage());

  // // some delay - does not hurt for this application
  // delay(3000);

  // // default terminal theme
  // screen.setFont(u8g2_font_t0_13b_mf);
  // screen.drawStr(0, 17, "athena>");
}

/**
 * Display message on screen
*/
void displayMessage(char* message, int y) {

  if(screen_counter > 5) {
    screen_counter = 0;
  } else { 
    screen_counter++;
  }

  screen.setFont(u8g2_font_8x13_mf);
  screen.firstPage();
  do {
    screen.drawStr(0, y, message);
  } while (screen.nextPage());

}

/**
 * Initialize SD card
*/
void SDInit() {
  SD.begin(SD_CS_PIN); 
  if(!SD.begin(SD_CS_PIN)) {
    debugln("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    debugln("No SD card attached");
    return;
  }

  debugln("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    debugln("ERROR - SD card initialization failed!");
    return; // init failed
  }

  // initialize file to log networks to 
  File file = SD.open("/log.txt");
  if(!file) {
    debugln("File does not exist");
    writeFile(SD, "/log.txt", "SSID, SECURITY, CHANNEL, LAT, LONG, TIME \r\n");
  } else {
    debugln("File already exists");
  }

  file.close();

}

/**
 * Create a AP with SSID and PASSWORD to allow wifi connection to custom networks 
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

int GPSGetLocation() {
  int f;
  if(gps.location.isValid()) {
    location.latitude = gps.location.lat();
    location.longitude = gps.location.lng();
    f = 1;

  } else {
    f = 0;
  }

  return fix_status;

}

/**
 * save mac address to array
*/ 
void saveMAC(unsigned char* mac_addr) {
  
  // prevent index from falling out of range
  if(seen_mac_index > MAX_NETWORKS) {
    seen_mac_index = 0;
  }

  mac_address tmp;
  for(int i=0; i<6; i++) {
    tmp.bytes[i] = mac_addr[i];
  }

  seen_macs[seen_mac_index] = tmp;
  seen_mac_index++;
  debug("MAC LEN: "); debugln(seen_mac_index);

}

/**
 * return true if a MAC address has been seen 
*/
int seenMAC(unsigned char* mac_addr) {
  Serial.println();

  mac_address tmp;

  // copy mac_addr into tmp
  for (int i = 0; i < RAW_BSSID_LENGTH; i++) {
    tmp.bytes[i] =  mac_addr[i];
  }

  for(int i = 0; i < MAX_NETWORKS; i++) {
    if(compareMAC(tmp, seen_macs[i])) {
      return 1;
    }
  }

  return 0;
  
}

/**
 * Compare mac addresses to avoid logging sasme network twice
 * return true if 2 MAC addresses are the same
*/
int compareMAC(mac_address mac1, mac_address mac2) {

  for(int i=0; i < RAW_BSSID_LENGTH; i++) {
    if(mac1.bytes[i] != mac2.bytes[i]) {
      return 0;
    }
  }

  return 1;

}

String security_int_to_string(int security_type){
  //Provide a security type int from WiFi.encryptionType(i) to convert it to a String which Wigle CSV expects.
  String authtype = "";
  switch (security_type){
    case WIFI_AUTH_OPEN:
      authtype = "[OPEN]";
      break;
  
    case WIFI_AUTH_WEP:
      authtype = "[WEP]";
      break;
  
    case WIFI_AUTH_WPA_PSK:
      authtype = "[WPA_PSK]";
      break;
  
    case WIFI_AUTH_WPA2_PSK:
      authtype = "[WPA2_PSK]";
      break;
  
    case WIFI_AUTH_WPA_WPA2_PSK:
      authtype = "[WPA_WPA2_PSK]";
      break;
  
    case WIFI_AUTH_WPA2_ENTERPRISE:
      authtype = "[WPA2]";
      break;

    //Requires at least v2.0.0 of https://github.com/espressif/arduino-esp32/
    case WIFI_AUTH_WPA3_PSK:
      authtype = "[WPA3_PSK]";
      break;

    case WIFI_AUTH_WPA2_WPA3_PSK:
      authtype = "[WPA2_WPA3_PSK]";
      break;

    case WIFI_AUTH_WAPI_PSK:
      authtype = "[WAPI_PSK]";
      break;
        
    default:
      authtype = "[UNDEFINED]";
  }

  return authtype;
}


void WiFiScanSetup() {
  // set wifi station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}



// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void setup() {
  Serial.begin(9600);
  // configDynamicWIFI();
  GPSInit();
  SDInit();
  WiFiScanSetup();
  screenInit();

  // must wait for GPS fix first
  // while (!GPSGetLocation()) {
  //   displayMessage("ath>Fixing..", font_height);
  // }

  // at this point GPS fix has been found
  displayMessage("ath>GPS fixed", font_height*screen_counter+1);
  displayMessage("ath>Ready..", font_height*screen_counter+1);

}

void loop() {

  //updateSerial();

  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      GPSGetLocation();
    }
  
  }

  int n = WiFi.scanNetworks();
  screen.clearDisplay();
  displayMessage("ath>scanning", font_height*screen_counter+1);

  sprintf(nets, "%s>%d", "ath", n);

  if(n == 0) {
    debugln("No networks found");
    displayMessage("ath>O networks", font_height*screen_counter+1);

  } else {
    
    displayMessage(nets, font_height*screen_counter+1);

    // process the scanned networks
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

      if(seenMAC(raw_bssid)) {
        debugln("This network seen");
        continue;
      }

      saveMAC(raw_bssid);

      String ssid = WiFi.SSID(i);

      // log to file 
      // debug(WiFi.SSID(i)); 
      // debug(",");
      // debug(WiFi.RSSI(i));
      // debug(","); 
      // debug(security_int_to_string(WiFi.encryptionType(i)));
      // debug(","); 
      // debug(WiFi.channel(i));
      // debugln();

      sprintf(data_row,
              "%s, %s, %d, %d, %.4f, %.4f \n\r",
              ssid.c_str(),
              security_int_to_string(WiFi.encryptionType(i)),
              WiFi.channel(i),
              WiFi.RSSI(i),
              location.latitude,
              location.longitude          
      );

      debugln(data_row);

      // log to SD card
      appendFile(SD, "/log.txt", data_row);

    }

  }

  WiFi.scanDelete();


}

#ifndef DEFS_H
#define DEFS_H

#define GPS_BAUDRATE 9600
#define SD_CS_PIN 5

#define SCAN_DELAY 500
#define SCANNED_WIFI_LENGTH 256
#define RAW_BSSID_LENGTH 6
#define MAC_ADDR_LENGTH 18
#define RAW_MAC_LENGTH 10
#define MAX_NETWORKS 100

/*==================OLED screen variables===================== */
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define YELLOW_OFFSET 16
#define OLED_SDA 21
#define OLED_SCL 22
#define PIX_OFFSET 2

#define LED_PIN 12
#define RESET_BUTTON_PIN 13
#define BLINK_INTERVAL 400
#define DEBOUNCE_INTERVAL 150
#define SCAN_INTERVAL 2000

#endif
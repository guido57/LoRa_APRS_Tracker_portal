// ======================================
// DEFAULT VALUES
// ======================================
#define CFG_AP_SSID           "ESP32_APRS" // The Access Point SSID
#define CFG_AP_PWD            "12345678"   // The Access Point Password
#define CFG_AP_PIN            38           // The Access Point activation pin

#define CFG_IS_CLIENT_MODE    false // set to false to enable beaconing. It was true
#define CFG_USE_DISPLAY       false

#ifdef wemos_d1_mini32
  #define LORA_SCK      5
  #define LORA_MISO     19
  #define LORA_MOSI     27
  #define LORA_CS       18
  #define LORA_RST      23
  #define LORA_IRQ      26
  #define GPS_TX        34
  #define GPS_RX        12
#endif


#define CFG_LORA_PIN_SS       5 //GG it was
#define CFG_LORA_PIN_RST      14 //GG it was LORA_RST
#define CFG_LORA_PIN_DIO0     26 //GG it was LORA_IRQ
#define CFG_LORA_USE_ISR      false // set to true for incoming packet ISR usage (stream mode, e.g. speech)

#define CFG_LORA_FREQ         433775000 // 433.775e6 
#define CFG_LORA_BW           125000 // 125e3
#define CFG_LORA_SF           10
#define CFG_LORA_CR           5
#define CFG_LORA_PWR          20
#define CFG_LORA_ENABLE_CRC   true  // set to false for speech data

#define CFG_BT_NAME           "loraprs"
#define CFG_BT_USE_BLE        false // set to true to use bluetooth low energy (for ios devices)

#define CFG_APRS_LOGIN        "aprs_login"
#define CFG_APRS_PASS         "aprs_password"
#define CFG_APRS_FILTER        "r/35.60/139.80/25"
#define CFG_APRS_RAW_BKN        "IW5ALZ-7>APZMDM,WIDE1-1:!4319.15N/01120.61E-LoRA Tracker 433.775MHz/BW125/SF10/CR5" 
#define CFG_APRS_SRC_ADDR       "IW5ALZ-7"
#define CFG_APRS_ADDR_SEP       ">"
#define CFG_APRS_SW_VER         "APZMDM"
#define CFG_APRS_ADDR_SEP2      ","
#define CFG_APRS_DGPTR_CODE     "WIDE1-1"
#define CFG_APRS_MSG_TYPE       ":"
#define CFG_APRS_MSG_FMT        "!"
#define CFG_APRS_LAT            "4319.15N"
#define CFG_APRS_SYM_TABLE      "/"
#define CFG_APRS_LON            "01120.61E"
#define CFG_APRS_SYMBOL         "-"
#define CFG_APRS_MSG            "LoRA Tracker 433.775MHz/BW125/SF10/CR5"

#define CFG_WIFI_SSID         "ap-stazione"
#define CFG_WIFI_KEY          "1111111111"

#define CFG_FREQ_CORR         false   // NB! incoming interrupts may stop working on frequent corrections when enabled
#define CFG_FREQ_CORR_DELTA   1000    //      test with your module before heavy usage

#define CFG_PERSISTENT_APRS   false
#define CFG_DIGIREPEAT        false
#define CFG_RF_TO_IS          false
#define CFG_IS_TO_RF          false
#define CFG_BEACON            true   // esp32_loraprs_gps acts as a simple beacon (see the function sendPeriodicBeacon() )
#define CFG_KISS_EXTENSIONS   false  //inhibit the connection (via bluetooth) between APRSDROID and esp32_loraprs_gps 

#define CFG_PTT_ENABLE        false
#define CFG_PTT_PIN           12
#define CFG_PTT_TX_DELAY_MS   50
#define CFG_PTT_TX_TAIL_MS    10

#define CFG_USE_GPS           false  

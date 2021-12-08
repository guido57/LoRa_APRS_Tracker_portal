#include <captive_portal.h>
#include <MyAPRSMessage.h>
#include <Arduino.h>
#include <LoRa.h>
#include <OneButton.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <logger.h>
//#include <Preferences.h>
#include <config.h>
//#include "configuration.h"
#include "display.h"
#include "pins.h"
#include "power_management.h"
#include <ax25_payload.h>

//Configuration Config;

// The configuration class
Config_ns::ConfigClass cc;

PowerManagement powerManagement;

OneButton       userButton;

HardwareSerial ss(1);
TinyGPSPlus    gps;

bool AP = false;

//void load_config();
void setup_lora();
void setup_gps();

String create_lat_aprs(RawDegrees lat);
String create_long_aprs(RawDegrees lng);
String create_lat_aprs_dao(RawDegrees lat);
String create_long_aprs_dao(RawDegrees lng);
String create_dao_aprs(RawDegrees lat, RawDegrees lng);
String createDateString(time_t t);
String createTimeString(time_t t);
String getSmartBeaconState();
String getBeaconState();
String getFixedBeaconState();
String padding(unsigned int number, unsigned int width);
int DecWidePath(String & ii);
void SendLoraPacket(String data);
String GetHMS();
bool rxLoop();
int calcTXinterval();
void checkGPS();

static bool send_update = false;

static void handle_tx_click() {
  
}

unsigned long long_press_start;
int option = 0;
static void handle_long_press_start() {
  long_press_start = millis();
}
static void handle_long_press() {
  unsigned long long_press_duration = millis() - long_press_start;
  Serial.printf("I'm pressed for %lu millis untiìl now \r", long_press_duration);
  option = long_press_duration/1000;
  if(option < 2)
    show_display("TX", 1);
  else if(option <4)
    show_display("", 1);
  else 
    show_display("AP", 1);


}
static void handle_long_press_stop() {
  Serial.printf("\r\nI was pressed for %lu millis\r\n",millis() - long_press_start);
  if(option < 2)
    send_update = true;
  else if(option < 4)
    ;
  else{
    AP = true;
    CaptivePortalSetup();
  }     
}

// cppcheck-suppress unusedFunction
void setup() {
  Serial.begin(115200);

  // original configuration
  //load_config();

  // new web configuration
  cc.Init("/my-config");
  cc.printFile(); 
  //CaptivePortalSetup();


// Service button
// GPIO38 is connected to the service button on T-TTGO_T_Beam_V1_0
// but unfortunately it is unaccessible on Wemos D1 Mini32 where I used GPIO14
#ifdef wemos_d1_mini32
  userButton = OneButton(14, true, true);
  logPrintlnI("userButton on pin 14");
#else
  userButton = OneButton(BUTTON_PIN, true, true);
  log_printf("userButton on pin %d\r\n", BUTTON_PIN);
#endif


#ifdef TTGO_T_Beam_V1_0
  Wire.begin(SDA, SCL);
  if (!powerManagement.begin(Wire)) {
    logPrintlnI("AXP192 init done!");
  } else {
    logPrintlnE("AXP192 init failed!");
  }
  powerManagement.activateLoRa();
  powerManagement.activateOLED();
  if( cc["beacon_active"]->val_bool || cc["smart_beacon_active"]->val_bool){
    powerManagement.activateGPS();
    Serial.println("Activate GPS");
  }else
    powerManagement.deactivateGPS();
  powerManagement.activateMeasurement();
#endif

  delay(500);
  logPrintlnI("LoRa APRS Tracker by IW5ALZ");
  setup_display();

  show_display(cc["callsign"]->val_string, "LoRa APRS Tracker", "by IW5ALZ Guido Giorgetti", 2000);
  
  setup_gps();
  setup_lora();

  if (cc["ptt_active"]->val_bool) {
    pinMode(cc["ptt_io_pin"]->val_int, OUTPUT);
    digitalWrite(cc["ptt_io_pin"]->val_int, cc["ptt_reverse"]->val_bool ? HIGH : LOW);
  }

  // make sure wifi and bt is off as we don't need it:
  //WiFi.mode(WIFI_OFF);
  btStop();

  if (cc["beacon_button_tx"]->val_bool) {
    // attach TX action to user button (defined by BUTTON_PIN)
    userButton.attachClick(handle_tx_click);
    userButton.attachLongPressStart(handle_long_press_start);
    userButton.attachDuringLongPress(handle_long_press);
    userButton.attachLongPressStop(handle_long_press_stop);
  }

  logPrintlnI(String("Fixed Beacon is ")   + String(cc["fixed_beacon_active"]->val_bool ? "ON" : "OFF"));
  logPrintlnI(String("Normal Beacon is ")  + String(cc["beacon_active"]->val_bool ? "ON" : "OFF"));
  logPrintlnI(String("Smart Beacon is ")   + String(cc["smart_beacon_active"]->val_bool ? "ON" : "OFF"));
  show_display("INFO", 
               String("Fixed Beacon is ")  + String(cc["fixed_beacon_active"]->val_bool ? "ON" : "OFF"),
               String("Normal Beacon is ") + String(cc["beacon_active"]->val_bool ? "ON" : "OFF"), 
               String("Smart Beacon is ")   + String(cc["smart_beacon_active"]->val_bool ? "ON" : "OFF"), 
                2000);
  logPrintlnI("setup done...");
  delay(500);
}

// cppcheck-suppress unusedFunction
void loop() {
  userButton.tick();

  if(AP)
    WiFi_loop();

  // GPS is connected to Hardware Serial
  while (ss.available() > 0) {
    char c = ss.read();
    //Serial.print(c);
    gps.encode(c);
  }

  // Declare and set a few variables
  bool          gps_time_update     = gps.time.isUpdated();
  bool          gps_loc_update      = gps.location.isUpdated();
  static time_t nextBeaconTimeStamp = -1;

  static double       currentHeading          = 0;
  static double       previousHeading         = 0;
  static unsigned int rate_limit_message_text = 0;

  static String lastTXLatString   = "";
  static String lastTXLngString   = "";
  
  if (gps.time.isValid()) {
    setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
  
    // -----------------  
    //plain beacon case
    // -----------------  
    if (cc["beacon_active"]->val_bool) {

      if (nextBeaconTimeStamp <= now()){
        if(gps_loc_update  || lastTXLatString != String("")){
 
          send_update = true; // It's time to send a LORA Packet

          // enforce message text every n's Config.beacon.timeout frame
          if (cc["beacon_rate"]->val_int * rate_limit_message_text > 30)
          {
            rate_limit_message_text = 0;
          }
        }
      }
    }

    // ---------------------------------------------  
    // smart beacon case
    // ---------------------------------------------  
    if (cc["smart_beacon_active"]->val_bool) {

      if (gps_loc_update && nextBeaconTimeStamp <= now())
      {
        send_update = true; // It's time to send a LORA Packet
        currentHeading = gps.course.deg();
        // enforce message text on slowest Config.smart_beacon.slow_rate
        rate_limit_message_text = 0;
      }
    }
  }

  static double   lastTxLat       = 0.0;
  static double   lastTxLng       = 0.0;
  static double   lastTxdistance  = 0.0;
  static uint32_t txInterval      = 60000L; // Initial 60 secs internal
  static uint32_t lastTxTime      = millis();
  static int      speed_zero_sent = 0;

  static bool   BatteryIsConnected   = false;
  static String batteryVoltage       = "";
  static String batteryChargeCurrent = "";
  
  #ifdef TTGO_T_Beam_V1_0
  static unsigned int rate_limit_check_battery = 0;
  if (!(rate_limit_check_battery++ % 60))
    BatteryIsConnected = powerManagement.isBatteryConnect();
  if (BatteryIsConnected) {
    batteryVoltage       = String(powerManagement.getBatteryVoltage(), 2);
    batteryChargeCurrent = String(powerManagement.getBatteryChargeDischargeCurrent(), 0);
  }
  #endif

  // Check if a Heading variation or a position variation > 20 meters require to send_update
  if (!send_update && gps_loc_update && cc["smart_beacon_active"]->val_bool) {
    //
    uint32_t lastTx = millis() - lastTxTime;
    currentHeading  = gps.course.deg();
    lastTxdistance  = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), lastTxLat, lastTxLng);
    if (lastTx >= txInterval) {
      // Trigger Tx Tracker when Tx interval is reach
      // Will not Tx if stationary bcos speed < 5 and lastTxDistance < 20
      if (lastTxdistance > 20) {
        send_update = true;
      }
    }

    // if in "smart_beacon_min_bcn" seconds heading changed more than "smart_beacon_turn_min"
    // and distance from last TX location is more than "smart_beacon_min_tx_dist"
    // then send_update
    double headingDelta = abs(previousHeading - currentHeading);
    if (lastTx > cc["smart_beacon_min_bcn"]->val_int) {
      // Check for heading more than 25 degrees
      if (headingDelta > cc["smart_beacon_turn_min"]->val_int){
        if(lastTxdistance > cc["smart_beacon_min_tx_dist"]->val_int) {
          send_update = true;
        }
      }
    }
  }
  
  // ----------------------------------------
  // send_update section
  // ----------------------------------------
  if (send_update) {
    send_update = false;

    // set the next time the beacon will be sent
    if (cc["smart_beacon_active"]->val_bool)
      nextBeaconTimeStamp = now() + cc["smart_beacon_slow_rate"]->val_int;
    else if (cc["beacon_active"]->val_bool)
      nextBeaconTimeStamp = now() + cc["beacon_rate"]->val_int;
    else if (cc["fixed_beacon_active"]->val_bool)
      nextBeaconTimeStamp = now() + cc["fixed_beacon_rate"]->val_int;

    APRSMessage msg;
    String      lat, lng, dao;

    // set callsign, destination and path
    msg.setSource(cc["callsign"]->val_string);
    msg.setDestination(cc["destination"]->val_string);
    msg.setPath(cc["path"]->val_string);

    // Set Latitude and Longitude
    if ( (cc["beacon_active"]->val_bool && gps_loc_update) || cc["smart_beacon_active"]->val_bool) {
      if (!cc["enhance_precision"]->val_bool) {
        lat = create_lat_aprs(gps.location.rawLat());
        lng = create_long_aprs(gps.location.rawLng());
        Serial.printf("lat=%s lng=%s \r\n", lat.c_str(),lng.c_str());
      } else {
        lat = create_lat_aprs_dao(gps.location.rawLat());
        lng = create_long_aprs_dao(gps.location.rawLng());
        dao = create_dao_aprs(gps.location.rawLat(), gps.location.rawLng());
      }
    } else if (cc["fixed_beacon_active"]->val_bool){ 
        lat = cc["fixed_beacon_lat"]->val_string;
        lng = cc["fixed_beacon_lon"]->val_string;
    } else if (cc["beacon_active"]->val_bool && !gps_loc_update && (lastTXLatString != "") ){
        lat = lastTXLatString;
        lng = lastTXLngString;
    }

    // Set Altitude
    String alt     = "";
    int    alt_int = max(-99999, min(999999, (int)gps.altitude.feet()));
    if (alt_int < 0) {
      alt = "/A=-" + padding(alt_int * -1, 5);
    } else {
      alt = "/A=" + padding(alt_int, 6);
    }

    // Set course and speed
    String course_and_speed = "";
    int    speed_int        = max(0, min(999, (int)gps.speed.knots()));
    if (speed_zero_sent < 3) {
      String speed      = padding(speed_int, 3);
      int    course_int = max(0, min(360, (int)gps.course.deg()));
      /* course in between 1..360 due to aprs spec */
      if (course_int == 0) {
        course_int = 360;
      }
      String course    = padding(course_int, 3);
      course_and_speed = course + "/" + speed;
    }

    if (speed_int == 0) {
      /* speed is 0.
         we send 3 packets with speed zero (so our friends know we stand still).
         After that, we save airtime by not sending speed/course 000/000.
         Btw, even if speed we really do not move, measured course is changeing
         (-> no useful / even wrong info)
      */
      if (speed_zero_sent < 3) {
        speed_zero_sent += 1;
      }
    } else {
      speed_zero_sent = 0;
    }

    // Build the APRS message that we're sending
    String aprsmsg;
    aprsmsg = "!" + lat + cc["beacon_overlay"]->val_string + lng + cc["beacon_symbol"]->val_string + course_and_speed + alt;
    // message_text every 10's packet (i.e. if we have beacon rate 1min at high
    // speed -> every 10min). May be enforced above (at expiry of smart beacon
    // rate (i.e. every 30min), or every third packet on static rate (i.e.
    // static rate 10 -> every third packet)
    if (!(rate_limit_message_text++ % 10)) {
      aprsmsg += cc["beacon_message"]->val_string;
    }
    if (BatteryIsConnected) {
      aprsmsg +=  batteryVoltage + "V " + batteryChargeCurrent + "mA";
    }

    if (cc["enhance_precision"]->val_bool) {
      aprsmsg += " " + dao;
    }

    msg.getAPRSBody()->setData(aprsmsg);
    // Now msg has the complete APRS packet
    String data = msg.encode();
    // Now data contains the complete encoded APRS packet
    // display it
    logPrintlnD(String(millis()) + " " + data);
    String body = msg.getAPRSBody()->getData().substring(6);

    show_display("T " + msg.getSource(), 
        "> " + msg.getDestination() + ", " + msg.getPath(),
        lat + " " + lng, 
        batteryVoltage + "V " + batteryChargeCurrent + "mA",
        gps.time.isValid() ?  
            String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) :
            GetHMS(),  
                  2000);


    // "push to talk " anticipated by ptt_start_delay, if needed
    if (cc["ptt_active"]->val_bool) {
      digitalWrite(cc["ptt_io_pin"]->val_int, cc["ptt_reverse"]->val_bool ? LOW : HIGH);
      delay(cc["ptt_start_delay"]->val_long);
    }

    // -----------------------------------------------------------------
    // Clear and Encoded payload for debugging purposes
    // The clear payload is:
    // IW5ALZ-7>APZMDM,WIDE1-1:!4319.15N/01120.61E-LoRA Tracker 433.775MHz/BW125/SF10/CR5

    /*
      The encoded payload is:
    const uint8_t b1[23] = {0x82,0xa0, 0xb4,0x9a,0x88,0x9a,0x00,0x92,0xae,0x6a,0x82,0x98, 0xb4, 0x0e, 0xae,0x92,0x88,0x8a,0x62,
                  0x40,0x03,0x03,0xf0};
    const char b2[59] = "!4319.15N/01120.61E-LoRA Tracker 433.775MHz/BW125/SF10/CR5";
    */
    // -----------------------------------------------------------------
    SendLoraPacket(data);

    if (cc["smart_beacon_active"]->val_bool) {
      lastTxLat       = gps.location.lat();
      lastTxLng       = gps.location.lng();
      previousHeading = currentHeading;
      lastTxdistance  = 0.0;
      lastTxTime      = millis();
    }

    lastTXLatString = lat;
    lastTXLngString = lng;

    // release "push to talk" after ptt_end_delay if needed
    if (cc["ptt_active"]->val_bool) {
      delay(cc["ptt_end_delay"]->val_long);
      digitalWrite(cc["ptt_io_pin"]->val_int, cc["ptt_reverse"]->val_bool ? HIGH : LOW);
    }
  } // end send_update

  
  if (gps_time_update) {
    //show_display(Config.callsign, createDateString(now()) + " " + createTimeString(now()), String("Sats: ") + gps.satellites.value() + " HDOP: " + gps.hdop.hdop(), String("Nxt Bcn: ") + (Config.smart_beacon.active ? "~" : "") + createTimeString(nextBeaconTimeStamp), BatteryIsConnected ? (String("Bat: ") + batteryVoltage + "V, " + batteryChargeCurrent + "mA") : "Powered via USB", String("Smart Beacon: " + getSmartBeaconState()));
    int txi = calcTXinterval();
    txInterval = (txi == -1) ? txInterval : txi;
  } // end send_update

  // check if GPS module sends messages and if not, suggest to reset it 
  checkGPS();

  // check any received packet
  rxLoop();
}


void checkGPS(){
   
  if ((cc["debug"]->val_bool == false) && (millis() > 5000 && gps.charsProcessed() < 10)) {
    logPrintlnE("No GPS frames detected! Try to reset the GPS Chip with this "
                "firmware: https://github.com/lora-aprs/TTGO-T-Beam_GPS-reset");
  }
 

}

bool rxLoop(){

  // ----------------------------------------------------
  // RECEIVING SECTION
  // ----------------------------------------------------
  if (int packetSize = LoRa.parsePacket()) {
    // loraReceive(packetSize);
    int snr  = LoRa.packetSnr();
    int rssi = LoRa.packetRssi();
    int dbm  = snr < 0 ? rssi + snr : rssi;
    logPrintlnI("incoming packet size=" + String(packetSize) 
                + " RSSI=" + String(rssi) + 
                + " SNR=" + String(snr) + 
                + " dBm=" + String(dbm));
                
    int  rxBufIndex = 0;
    byte rxBuf[packetSize];

    while (LoRa.available()) {
      rxBuf[rxBufIndex++] = LoRa.read();
    }

    // decode the payload and print it on Serial
    AX25::Payload *pl = new AX25::Payload(rxBuf, packetSize);
    logPrintlnI(pl->ToString());

    APRSMessage raprs;
    raprs.decode(pl->ToString());
    String path = raprs.getPath();

    String body = raprs.getAPRSBody()->toString().substring(6);

    show_display("R " + String(dbm) + " dBm",
        raprs.getSource() + " > " + raprs.getDestination(),
        "Path: " + raprs.getPath(),
        raprs.getLatitude() + " " + raprs.getLongitude(),
        gps.time.isValid() ? 
            String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) :
            GetHMS(),  
        2000);

    if(cc["digipeater"]->val_bool && DecWidePath(path) >= 0){
      raprs.setPath(path);
      // repeat
      logPrintlnI("Repeating:");
      String rd = raprs.encode();
      logPrintlnI(rd);
      show_display("RP " + raprs.getSource(), 
        "> " + raprs.getDestination() + ", " + raprs.getPath(),
        raprs.getLatitude() + " " + raprs.getLongitude(),
               2000);
      SendLoraPacket(rd);
      return true;
    };
    //digitalWrite(BUILTIN_LED, LOW); // turn off the LED
  }
  return false;
 
}


/* Calculate TX interval for smart beacon

  return a value in seconds between slow_rate and fast_rate 
  and proportional to low_speed and high_speed
  e.g. slow_rate=300 secs  fast_rate=60 secs slow_speed=20Km/h fast_speed=100Km/h
       if speed < 20  Km/h return 300 secs
       if speed = 40  Km/h return 150 secs
       if speed > 100 Km/h return  60 secs 

*/
int calcTXinterval(){

    int _TXinterval = -1; 

    if (cc["smart_beacon_active"]->val_bool) {
      // Change the Tx internal based on the current speed
      int curr_speed = (int)gps.speed.kmph();
      if (curr_speed < cc["smart_beacon_slow_speed"]->val_int) {
        _TXinterval = cc["smart_beacon_slow_rate"]->val_int * 1000;
      } else if (curr_speed > cc["smart_beacon_fast_speed"]->val_int) {
        _TXinterval = cc["smart_beacon_fast_rate"]->val_int * 1000;
      } else {
        /* Interval inbetween low and high speed
           min(slow_rate, ..) because: if slow rate is 300s at slow speed <=
           10km/h and fast rate is 60s at fast speed >= 100km/h everything below
           current speed 20km/h (100*60/20 = 300) is below slow_rate.
           -> In the first check, if curr speed is 5km/h (which is < 10km/h), tx
           interval is 300s, but if speed is 6km/h, we are landing in this
           section, what leads to interval 100*60/6 = 1000s (16.6min) -> this
           would lead to decrease of beacon rate in between 5 to 20 km/h. what
           is even below the slow speed rate.
        */
        _TXinterval = min(cc["smart_beacon_slow_rate"]->val_int, 
                         cc["smart_beacon_fast_speed"]->val_int * cc["smart_beacon_fast_rate"]->val_int / curr_speed) * 1000;
      }
    }
    return _TXinterval;
} 


String GetHMS() {
    long seconds = millis()/1000;
    String s = String(seconds % 60);
    String m = String((seconds / 60) % 60);
    String h = String((seconds / (60 * 60)) % 24);
    return h + ":" + (m.length() > 1 ? m : "0" + m) + ":" + (s.length() > 1 ? s : "0" + s);
}

void SendLoraPacket(String _data){
    // Set the AX25 payload, to encode Source and Dest
    AX25::Payload payload(_data);
#define MAXBUF 200
    byte buf[MAXBUF];
    // Encode Source and Dest
    int bytesWritten = payload.ToBinary(buf, sizeof(buf));
    if (bytesWritten <= 0) {
      Serial.println("Failed to encode payload");
    } else {
      // limit to MAXBUF if necessary
      bytesWritten = MAXBUF < bytesWritten ? MAXBUF : bytesWritten;
      LoRa.beginPacket();
      for (int i = 0; i < bytesWritten; i++) {
        uint8_t b2b = buf[i];
        // Serial.printf(" %c ",b2b);
        LoRa.write(b2b);
      };
    }
    LoRa.endPacket();
}

// Decrement the WIDEM-N path
// return the decremented string. In case of error, return the input string  
// e.g. WIDE2-2 return WIDE2-1        WIDE3-1 return WIDE3-0
int DecWidePath(String &ii){
  if(! ii.startsWith("WIDE"))
    return -1;
  char * wide = strtok((char*) ii.c_str(), "-");
  if(wide == NULL) return -1;
  //Serial.printf("wide=%lu *wide=%s\r\n",(unsigned long) wide, wide);
  char * pn = strtok(NULL, "-");
  if(pn == NULL)
    return -1;
  int nn = atoi(pn);
  if(nn>0){
     nn--;
  }
  ii = String(wide) + "-" + String(nn);
  return nn;
}

void setup_lora() {
  logPrintlnI("Set SPI pins!");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  logPrintlnI("Set LoRa pins!");
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);

  long freq = cc["lora_frequency_tx"]->val_long;
  logPrintI("frequency: ");
  logPrintlnI(String(freq));
  if (!LoRa.begin(freq)) {
    logPrintlnE("Starting LoRa failed!");
    show_display("ERROR", "Starting LoRa failed!");
    while (true) {
    }
  }
  LoRa.setSyncWord((int)0x34); // GG
  LoRa.setSpreadingFactor(cc["lora_spreading_factor"]->val_int);
  LoRa.setSignalBandwidth(cc["lora_signal_bandwith"]->val_long);
  LoRa.setCodingRate4(cc["lora_coding_rate"]->val_int);
  LoRa.enableCrc();

  LoRa.setTxPower(cc["lora_power"]->val_int);
  logPrintlnI("LoRa init done!");
  show_display("INFO", "LoRa init done!", 2000);
}

void setup_gps() {
  ss.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
}

char *s_min_nn(uint32_t min_nnnnn, int high_precision) {
  /* min_nnnnn: RawDegrees billionths is uint32_t by definition and is n'telth
   * degree (-> *= 6 -> nn.mmmmmm minutes) high_precision: 0: round at decimal
   * position 2. 1: round at decimal position 4. 2: return decimal position 3-4
   * as base91 encoded char
   */

  static char buf[6];
  min_nnnnn = min_nnnnn * 0.006;

  if (high_precision) {
    if ((min_nnnnn % 10) >= 5 && min_nnnnn < 6000000 - 5) {
      // round up. Avoid overflow (59.999999 should never become 60.0 or more)
      min_nnnnn = min_nnnnn + 5;
    }
  } else {
    if ((min_nnnnn % 1000) >= 500 && min_nnnnn < (6000000 - 500)) {
      // round up. Avoid overflow (59.9999 should never become 60.0 or more)
      min_nnnnn = min_nnnnn + 500;
    }
  }

  if (high_precision < 2)
    sprintf(buf, "%02u.%02u", (unsigned int)((min_nnnnn / 100000) % 100), (unsigned int)((min_nnnnn / 1000) % 100));
  else
    sprintf(buf, "%c", (char)((min_nnnnn % 1000) / 11) + 33);
  // Like to verify? type in python for i.e. RawDegrees billions 566688333: i =
  // 566688333; "%c" % (int(((i*.0006+0.5) % 100)/1.1) +33)
  return buf;
}

String create_lat_aprs(RawDegrees lat) {
  char str[20];
  char n_s = 'N';
  if (lat.negative) {
    n_s = 'S';
  }
  // we like sprintf's float up-rounding.
  // but sprintf % may round to 60.00 -> 5360.00 (53° 60min is a wrong notation
  // ;)
  sprintf(str, "%02d%s%c", lat.deg, s_min_nn(lat.billionths, 0), n_s);
  String lat_str(str);
  return lat_str;
}

String create_lat_aprs_dao(RawDegrees lat) {
  // round to 4 digits and cut the last 2
  char str[20];
  char n_s = 'N';
  if (lat.negative) {
    n_s = 'S';
  }
  // we need sprintf's float up-rounding. Must be the same principle as in
  // aprs_dao(). We cut off the string to two decimals afterwards. but sprintf %
  // may round to 60.0000 -> 5360.0000 (53° 60min is a wrong notation ;)
  sprintf(str, "%02d%s%c", lat.deg, s_min_nn(lat.billionths, 1 /* high precision */), n_s);
  String lat_str(str);
  return lat_str;
}

String create_long_aprs(RawDegrees lng) {
  char str[20];
  char e_w = 'E';
  if (lng.negative) {
    e_w = 'W';
  }
  sprintf(str, "%03d%s%c", lng.deg, s_min_nn(lng.billionths, 0), e_w);
  String lng_str(str);
  return lng_str;
}

String create_long_aprs_dao(RawDegrees lng) {
  // round to 4 digits and cut the last 2
  char str[20];
  char e_w = 'E';
  if (lng.negative) {
    e_w = 'W';
  }
  sprintf(str, "%03d%s%c", lng.deg, s_min_nn(lng.billionths, 1 /* high precision */), e_w);
  String lng_str(str);
  return lng_str;
}

String create_dao_aprs(RawDegrees lat, RawDegrees lng) {
  // !DAO! extension, use Base91 format for best precision
  // /1.1 : scale from 0-99 to 0-90 for base91, int(... + 0.5): round to nearest
  // integer https://metacpan.org/dist/Ham-APRS-FAP/source/FAP.pm
  // http://www.aprs.org/aprs12/datum.txt
  //

  char str[10];
  sprintf(str, "!w%s", s_min_nn(lat.billionths, 2));
  sprintf(str + 3, "%s!", s_min_nn(lng.billionths, 2));
  String dao_str(str);
  return dao_str;
}

String createDateString(time_t t) {
  return String(padding(day(t), 2) + "." + padding(month(t), 2) + "." + padding(year(t), 4));
}

String createTimeString(time_t t) {
  return String(padding(hour(t), 2) + "." + padding(minute(t), 2) + "." + padding(second(t), 2));
}


String padding(unsigned int number, unsigned int width) {
  String result;
  String num(number);
  if (num.length() > width) {
    width = num.length();
  }
  for (unsigned int i = 0; i < width - num.length(); i++) {
    result.concat('0');
  }
  result.concat(num);
  return result;
}

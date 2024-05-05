#include <captive_portal.h>
#include <config.h>
#include <handleHttp.h>
#include <WebServer.h>
#include <logger.h>

// the ConfigClass 
extern Config_ns::ConfigClass cc; 

//=== CaptivePortal stuff ================
String softAP_ssid;
String softAP_password;

/* hostname for mDNS. Should work at least on windows. Try http://esp32.local */
const char *myHostname = "esp32";

// NTP settings
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600; // GMT + 1
//Change the Daylight offset in milliseconds. If your country observes Daylight saving time set it to 3600. Otherwise, set it to 0.
const int   daylightOffset_sec = 3600;

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
WebServer web_server(80);

/* Setup the Access Point */
void AccessPointSetup();

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Last time I tried to ping an external IP address (usually the gateway) */
unsigned long lastPing = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

// =====================================================
void connectWifi() {
    Serial.println("Connecting as wifi client...");
    //WiFi.forceSleepWake();
    //WiFi.disconnect();
    WiFi.mode(WIFI_MODE_AP);
    logPrintI("Connecting to ssid="); logPrintlnI(cc["WiFiSsid"]->val_string);
    WiFi.begin(cc["WiFiSsid"]->val_string.c_str(), cc["WiFiPwd"]->val_string.c_str());
    int connRes = WiFi.waitForConnectResult();
    if(connRes == WL_CONNECTED){
      logPrintI("Connected to "); logPrintI(cc["WiFiSsid"]->val_string);  
      logPrintI(" IP="); logPrintlnI(WiFi.localIP().toString());
        
    }else{
      logPrintD("connRes: ");
      logPrintlnD(String(connRes));
    }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

  tm timeinfo;
  bool got_local_time = false;

  unsigned long next_try_connect_millis = 0L;
  unsigned long try_connect_period_millis = 60000L; // 60 seconds

  unsigned long next_try_DNS_millis = 0L;
  unsigned long try_DNS_period_millis = 10; // 10 milliseconds

  bool captive_portal_setup_ok = false;
  bool access_point_setup_ok = false;
// =====================================================
/**
   1) if enabled, check WIFI conditions and try to connect to WIFI
   2) if enabled, run Access Point 
 * @return void
 */
void WiFi_loop(bool access_point_enabled){
  
  bool wifi_enabled = cc["WiFiEnabled"]->val_bool; 

  if(access_point_enabled && access_point_setup_ok == false ){
    AccessPointSetup();
    access_point_setup_ok = true;
  }


  if( (WiFi.isConnected() || access_point_enabled) && captive_portal_setup_ok == false ){
    CaptivePortalSetup();
    captive_portal_setup_ok = true;
  }


  if(wifi_enabled && ! WiFi.isConnected() && millis() > next_try_connect_millis ){
    //Serial.printf("Try to connect to WiFi\r\n");
    connectWifi();

    next_try_connect_millis = millis() + try_connect_period_millis;
  }

  if(access_point_enabled){

      //DNS
      dnsServer.processNextRequest();
  }

  if(access_point_enabled || wifi_enabled){
    
    if(millis() > next_try_DNS_millis){
      //HTTP
      web_server.handleClient();
      next_try_DNS_millis = millis() + try_DNS_period_millis;
    }
  }
}

// Set the Access Point
void AccessPointSetup(){

  //softAP_ssid =  "ESP32_" + WiFi.macAddress();
  softAP_ssid     = cc["ApSsid"]->val_string; 
  softAP_password = cc["ApPwd"]->val_string;
  delay(1000);
  // Access Point Setup
  WiFi.disconnect();   //added to start with the wifi off, avoid crashing
  WiFi.mode(WIFI_OFF); //added to start with the wifi off, avoid crashing
  WiFi.mode(WIFI_AP);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(softAP_ssid.c_str(), softAP_password.c_str());
  delay(2000);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  delay(100); // Without delay I've seen the IP address blank
  logPrintI("Access Point set! SSID: ");logPrintI(softAP_ssid);
  logPrintI(" IP address: "); logPrintlnI(WiFi.softAPIP().toString()); 
  
  /* Setup the DNS web_server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  
}

// Set the DNS Server and the Web Server
void CaptivePortalSetup(){
  
  
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  web_server.on("/", handlehttp::handleRoot);
  web_server.on("/settingssave", handlehttp::handleSettingsSave);
  web_server.on("/restoresettings", handlehttp::handleRestoreSettings);
  //web_server.on("/wifi", handlehttp::handleWifi);
  //web_server.on("/wifisave", handlehttp::handleWifiSave);
  web_server.on("/generate_204", handlehttp::handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  web_server.on("/fwlink", handlehttp::handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  web_server.onNotFound(handlehttp::handleNotFound);
  web_server.begin(); // Web server start
  logPrintlnI("HTTP server started");
  //loadCredentials(); // Load WLAN credentials from network
  //connect = strlen(espNow->settings.entries.ssid) > 0; // Request WLAN connect if there is a SSID
}
 
#include <captive_portal.h>
#include <config.h>
#include <handleHttp.h>
#include <WebServer.h>

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
    /*
    Serial.print("ssid=");Serial.println( espNow->settings.entries.ssid);
    Serial.print("password=");Serial.println(espNow->settings.entries.pwd);
    WiFi.begin(espNow->settings.entries.ssid, espNow->settings.entries.pwd);
    int connRes = WiFi.waitForConnectResult();
    Serial.print("connRes: ");
    Serial.println(connRes);
    */
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

// =====================================================
// Callback for the TaskWIFI  
/**
   check WIFI conditions and try to connect to WIFI.
 * @return void
 */
void WiFi_loop(void){

  //DNS
  dnsServer.processNextRequest();
  //HTTP
  web_server.handleClient();

}

// Set the Access Point but let the WiFi off
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
  Serial.println("Access Point set:");
  
  Serial.printf("    SSID: %s\r\n", softAP_ssid.c_str());
  Serial.print("    IP address: ");
  Serial.println(WiFi.softAPIP());
}

void CaptivePortalSetup(){

  AccessPointSetup();

  /* Setup the DNS web_server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

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
  Serial.println("HTTP server started");
  //loadCredentials(); // Load WLAN credentials from network
  //connect = strlen(espNow->settings.entries.ssid) > 0; // Request WLAN connect if there is a SSID
}
 
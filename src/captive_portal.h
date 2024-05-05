#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "handleHttp.h"


// ====== Access Point stuff ================
/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APPSK "12345678"
#endif

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
extern const char *myHostname;


// Web server
//extern ESP8266WebServer web_server;
//extern WebServer web_server;


/* Soft AP network parameters */
extern IPAddress apIP;
extern IPAddress netMsk;

/** Should I connect to WLAN asap? */
extern boolean connect;

/** Last time I tried to connect to WLAN */
extern unsigned long lastConnectTry;

/** Current WLAN status */
extern unsigned int status;

void AccessPointSetup();
void CaptivePortalSetup();
void WiFi_loop(bool access_point_enabled);


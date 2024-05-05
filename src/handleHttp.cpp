/** Handle root or redirect to captive portal */
#include <Arduino.h>
#include <handleHttp.h>
#include <captive_portal.h>
#include <config.h>
#include <tools.h>

//extern int station;
#define S String

extern Config_ns::ConfigClass cc; 

extern WebServer web_server;

namespace handlehttp {
// ==================================================================================================
void handleRoot() {
  //Serial.println("handleRoot");
 
  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }
  
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>Your settings</title></head><body>"
            "<h1>Your settings!</h1>");

  Page += F(
            "<form method='POST' action='settingssave'>"
            "<table>"
          );

  for(int i=0; i< Config_ns::Entries.size() ; i++){
    Page += F(
              "<tr>"
            );
    //Serial.printf("entry #%d\r\n",i);
    Config_ns::Entry ee = Config_ns::Entries[i];
    if(ee.value_type == "String"){
      String ss = ee.val_string;
      Page += S(
                "<td><label>" + ee.description + "</label></td>" + 
                "<td><input type=\"text\" id=\"" + ee.key + 
                "\" name=\"" + ee.key + 
                "\" value=\"" + ss + 
                "\" size=\"" + S(ss.length()) + "\"" +
                "></td>"
              );
    }
    else if(ee.value_type == "Int"){
      Page += S(
                "<td><label>" + ee.description + "</label></td>" + 
                "<td><input type=\"text\" id=\"" + ee.key + 
                "\" name=\"" + ee.key + 
                "\" value=\"" + ee.val_int + 
                "\"></td>"
              );
    }
    else if(ee.value_type == "Long"){
      Page += S(
                "<td><label>" + ee.description + "</label></td>" + 
                "<td><input type=\"text\" id=\"" + ee.key + 
                "\" name=\"" + ee.key + 
                "\" value=\"" + ee.val_long + 
                "\"></td>"
              );
    }
    else if(ee.value_type == "Bool"){
      bool bb = ee.val_bool;
      Page += S(
                "<td><label>" + ee.description + "</label></td>" + 
                "<td><input type=\"checkbox\" id=\"" + ee.key + 
                "\" name=\"" + ee.key + "\" " +
                (bb ? "checked" : "") + 
                " ></td>"
              );
    };
    Page += S(
                "</tr>"  
            )   ;
    
  }
  Page += S(
                "</table>"  
            );   

  Page += F(
            
            "<br /><input id='id_submit' type='submit' value='Save'/>"
            "<br /><br /><input id='id_restore_defaults' type='submit' value='Restore Factory Settings' formaction='restoresettings'/>"
            "</form>"
            "\r\n<br />"
            "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
            
           );
  
  
  //Serial.println("scan start");
  int n = 0;
  n = WiFi.scanNetworks();
  //Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  
  Page += F(
            "</table></body></html>"
  );


  web_server.send(200, "text/html", Page);
}
// ==================================================================================================
/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  //Serial.println("==== Captive portal verification ...");
  //Serial.print("hostHeader: "); Serial.println(web_server.hostHeader());
  if (!isIp(web_server.hostHeader()) && web_server.hostHeader() != (String(myHostname) + ".local")) {
    //Serial.print("Captive portal location: "); Serial.println(String("http://") + toStringIp(web_server.client().localIP()));
    web_server.sendHeader("Location", String("http://") + toStringIp(web_server.client().localIP()), true);
    web_server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    web_server.client().stop(); // Stop is needed because we sent no content length
    //Serial.println("==== Captive portal verification returned True");
    return true;
  }
  //Serial.println("==== Captive portal verification returned False");
  return false;
}
// ==================================================================================================
/** Handle the root save form and redirect to the root config page again */
void handleSettingsSave() {
  //Serial.println("handleSettingsSave");

  for(int i=0; i<Config_ns::Entries.size() ; i++){
    Config_ns::Entry * ee = &Config_ns::Entries[i];
    Serial.printf("ee->key is %s\r\n", ee->key.c_str());
    if(ee->value_type == "String"){
      ee->SetString( web_server.arg(ee->key).c_str());
    }
    else if(ee->value_type == "Int"){
      ee->SetInt(web_server.arg(ee->key).toInt());
    }
    else if(ee->value_type == "Long"){
      ee->SetLong(web_server.arg(ee->key).toInt());
    }
    else if(ee->value_type == "Bool"){
      Serial.printf("ee.key=%s has value from web_server of %s\r\n", 
      ee->key.c_str(),
      web_server.arg(ee->key).c_str());
      ee->SetBool( web_server.arg(ee->key) == "on");
    }
  }

  cc.SaveEntriesToSPIFFS();

  web_server.sendHeader("Location", "/", true);
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  web_server.client().stop(); // Stop is needed because we sent no content length
}
// ==================================================================================================
/** Handle the root restore settings and redirect to the root config page again */
void handleRestoreSettings() {
  //Serial.println("handleRestoreSettings");

  cc.SaveDefaultsToSPIFFS();
  cc.LoadEntriesFromSPIFFS();

  web_server.sendHeader("Location", "/", true);
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  web_server.client().stop(); // Stop is needed because we sent no content length
}
// ==================================================================================================
void handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += web_server.uri();
  message += F("\nMethod: ");
  message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += web_server.args();
  message += F("\n");

  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += String(F(" ")) + web_server.argName(i) + F(": ") + web_server.arg(i) + F("\n");
  }
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(404, "text/plain", message);
}
// ==================================================================================================
}
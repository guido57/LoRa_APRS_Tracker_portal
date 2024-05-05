#include <Arduino.h>
#include <config.h>
#include <WiFi.h>
#include <ax25_payload.h>
#include <aprs_is.h>
#include <digirepeat.h>

WiFiClient aprsisConn_;
extern Config_ns::ConfigClass cc; 
const int CfgMaxAprsInMessageSize = 255;  // maximum aprsis to LoRa message size
extern void SendLoraPacket(String _data);


bool reconnectAprsis()
{
  Serial.print("APRSIS connecting...");
  
  if (!aprsisConn_.connect(cc["APRS-IS_server"]->val_string.c_str(), 
                           cc["APRS-IS_port"]->val_long)) {
    Serial.println("Failed to connect to " + cc["APRS-IS_server"]->val_string + 
                            ":" + String(cc["APRS-IS_port"]->val_long));
    return false;
  }
  Serial.println("ok");

  String aprsLoginCommand_ = String("user ")  + cc["APRS-IS_user"]->val_string + 
                             String(" pass ") + cc["APRS-IS_pwd"]->val_string  +
                             String(" vers ") + cc["APRS-IS_version"]->val_string;
  if (cc["APRS-IS_filter"]->val_string.length() > 0) {
    aprsLoginCommand_ += String(" filter ") + cc["APRS-IS_filter"]->val_string;
  }
  
  aprsLoginCommand_ += String("\n");
  Serial.printf("APRS-IS login command is: %s\r\n", aprsLoginCommand_.c_str());
  aprsisConn_.print(aprsLoginCommand_);
  return true;
}

void sendToAprsis(const String &aprsMessage)
{
  //if (cc["WiFiEnabled"]->val_bool && WiFi.status() != WL_CONNECTED) {
  //  reconnectWifi();
  //}

  if (cc["APRS-IS_active"]->val_bool && !aprsisConn_.connected()) {
    reconnectAprsis();
  }
  aprsisConn_.println(aprsMessage);

  if (! cc["APRS-IS_persistent"]->val_bool) {
    aprsisConn_.stop();
  }
}

void onAprsisDataAvailable()
{
  String aprsisData;

  while (aprsisConn_.available() > 0) {
    char c = aprsisConn_.read();
    if (c == '\r') continue;
    //Serial.print(c);
    if (c == '\n') break;
    aprsisData += c;
    if (aprsisData.length() >= CfgMaxAprsInMessageSize   ) {
      Serial.println("APRS-IS incoming message is too long, skipping tail");
      break;
    }
  }

  if ( aprsisData.length() > 0   ) {
    AX25::Payload payload(aprsisData);
    if (payload.IsValid()) {
      Serial.printf("Received from APRS-IS:\r\n%s\r\n",aprsisData.c_str());
      repeat_to_LoRa(payload);
       
    }
  }
}

 void aprs_is_loop(){

    if (cc["APRS-IS_active"]->val_bool && 
        cc["APRS-IS_persistent"]->val_bool && 
        !aprsisConn_.connected()) {
        
        reconnectAprsis();
    }
  

    if (aprsisConn_.available() > 0) {
      onAprsisDataAvailable();
    }
}
 
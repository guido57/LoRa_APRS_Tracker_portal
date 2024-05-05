#include <ax25_payload.h>
#include <config.h>
#include <MyAPRSMessage.h>
#include <display.h>
#include <logger.h>
#include <aprs_is.h>
#include <digirepeat.h>


// The configuration class
extern Config_ns::ConfigClass cc;

void SendLoraPacket(String data);

bool need_to_repeat_to_LoRa(AX25::Payload pl){
  if( ! cc["digipeater"]->val_bool){
    Serial.printf("No need to repeat to LoRa because digipeater in config is false\r\n");
    return false;
  }

  // if it starts with # return false
  //if(pl.ToString().startsWith("#") == false ){
  //  Serial.printf("No need to repeat to LoRa because payload %s starts with #\r\n", pl.ToString().c_str());
  //  return false; 
  //}

  // if TCP* is in path, return false
  APRSMessage raprs;
  raprs.decode(pl.ToString());
  String path = raprs.getPath();
  if(path.indexOf("TCP") >=0){
    Serial.printf("No need to repeat to LoRa because path contains TCP\r\n");
    return false;
  }

  // check if WIDEN-n is in path and decrease n 
  bool need_repeat = pl.Digirepeat(cc["callsign"]->val_string);
  if(!need_repeat){
    Serial.printf("No need to repeat to LoRa because callsign is %s \r\n",cc["callsign"]->val_string.c_str());

  }
  return need_repeat;
}

bool need_to_repeat_to_APRS_IS(AX25::Payload * pl){
  if( ! cc["digipeater"]->val_bool)
    return false;

  // if TCP* is in path, return false
  APRSMessage raprs;
  raprs.decode(pl->ToString());
  String path = raprs.getPath();
  if(path.indexOf("TCP") >=0)
    return false;

  
  // check if WIDEN-n is in path and decrease n 
  bool need_repeat = pl->Digirepeat(cc["callsign"]->val_string);
  
  return need_repeat;
}


void repeat_to_LoRa(AX25::Payload pl){
    
    Serial.printf("repeat_to_LoRa\r\n");
    if(need_to_repeat_to_LoRa(pl)){
 
        APRSMessage raprs;
        raprs.decode(pl.ToString());

        show_display("RP LoRa " + raprs.getSource(), 
            "> " + raprs.getDestination() + ", " + raprs.getPath(),
            raprs.getLatitude() + " " + raprs.getLongitude(),
            2000);

        String rd = pl.ToString(); 
        logPrintlnI("Repeating to LoRa:");
        logPrintlnI(rd);
        SendLoraPacket(rd);
    }

}

void repeat_to_APRS_IS(AX25::Payload pl){
    if(need_to_repeat_to_APRS_IS(&pl)){

      APRSMessage raprs;
      raprs.decode(pl.ToString());
    
      //String body = raprs.getAPRSBody()->toString().substring(6);

      String rd = pl.ToString();
      logPrintlnI("Repeating to APRS-IS:");
      logPrintlnI(rd);
      sendToAprsis(rd);
    }
}

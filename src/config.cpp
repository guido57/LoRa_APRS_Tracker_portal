#include <Arduino.h>
#include <config.h>
#include <tools.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

namespace Config_ns{

    std::vector<Entry>  Entries = {
        // Access Point
        {"ApSsid", "ESP32_APRS", "String", "Access Point SSID e.g. ESP32_APRS "},
        {"ApPwd", "12345678", "String", "Access Point SSID e.g. 12345678 "},
        {"ApPin", "38", "Int", "Access Point Activation Pin e.g. 38"},
        // Generic        
        {"callsign", "IW0AAA-7", "String", "APRS callsign e.g. IW0AAA-7"},
        {"destination", "APZMDM", "String", "APRS destination e.g. APZMDM"},
        {"path", "WIDE1-1", "String", "APRS path e.g. WIDE1-1"},
        {"debug", String(false), "Bool", "debug mode e.g. false"},
        {"enhance_precision", String(false), "Bool", "enhanced precision mode e.g. false"},
        {"digipeater", String(false), "Bool", "digipeater repeating any incoming message e.g. false"},
        // beacon    
        {"beacon_active", String(true), "Bool", "beacon is active i.e. send a message every 'timeout' minutes  e.g. true"},
        {"beacon_message", "LoRa Tracker", "String", "beacon text message e.g. LoRa Tracker"},
        {"beacon_rate", "60", "Int", "beacon sends at beacon_rate seconds e.g. 60"},
        {"beacon_button_tx", String(true), "Bool", "beacon TX button enabled or not e.g. true"},
        {"beacon_symbol", "[", "String", "beacon symbol table to be used e.g. ["},
        {"beacon_overlay", "/", "String", "beacon symbol overlay e.g. /"},
        // smart_beacon
        {"smart_beacon_active", String(true), "Bool", "smart beacon is active i.e. send at different rates according to speed e.g. false"},
        {"smart_beacon_slow_rate", "300", "Int","smart beacon sends every 'slow_rate' seconds if speed <= slow_speed e.g. 300"},
        {"smart_beacon_slow_speed", "10", "Int","smart beacon sends at 'slow_rate' seconds when speed <= slow_speed in Km/h e.g. 10"},
        {"smart_beacon_fast_rate", "60", "Int","smart beacon sends at 'fast_rate' seconds when speed>=fast_speed e.g. 60"},
        {"smart_beacon_fast_speed", "100", "Int","smart beacon send at fast_rate seconds when speed>=fast_speed in Km/h e.g. 100"},
        {"smart_beacon_turn_min", "25", "Int","smart beacon sends at 'fast_rate' when turn > turn_min in degrees e.g. 25"},
        {"smart_beacon_min_bcn","15", "Int","smart beacon period of turn_min evaluation in seconds e.g. 15"},
        {"smart_beacon_min_tx_dist","100", "Int","smart beacon distance from the last tx position in meters e.g. 100"},
        // fixed_beacon
        {"fixed_beacon_active", String(true), "Bool", "fixed beacon i.e. send at fixed rate and fixed coordinates e.g. false"},
        {"fixed_beacon_rate", "300", "Int","fixed beacon i.e. send every 'rate' seconds e.g. 300"},
        {"fixed_beacon_lat", "4300.00N", "String", "fixed beacon i.e. fixed latitude e.g. 4300.00N"},
        {"fixed_beacon_lon", "01100.00E", "String", "fixed beacon i.e. fixed longitude e.g. 01100.00E"},
        // lora protocol parameters
        {"lora_frequency_rx", "433775000", "Long", "LoRa Rx frequency in Hertz e.g. 433775000"},
        {"lora_frequency_tx", "433775000", "Long", "LoRa Tx frequency in Hertz e.g. 433775000"},
        {"lora_power", "20", "Int", "LoRa Tx Power in dBm e.g. 20"},
        {"lora_spreading_factor", "10", "Int", "LoRa Spread Spectrum factor e.g. 10"},
        {"lora_signal_bandwith", "125000", "Long", "LoRa Signal Bandwith in Hertz e.g. 125000"},
        {"lora_coding_rate", "5", "Int", "LoRa Coding Rate in Hertz e.g. 5125000"},
        // PTT 
        {"ptt_active", String(false), "Bool", "ptt active i.e. rig transmits when io_pin goes high e.g. false"},
        {"ptt_io_pin", "4", "Int", "ptt GPIO pin e.g. 4"},
        {"ptt_start_delay", "0", "Long", "ptt start delay i.e. send after 'start_delay' msecs e.g. 0"},
        {"ptt_end_delay", "0", "Long", "ptt end delay i.e. stop transmitting after 'end_delay' msecs e.g. 0"},
        {"ptt_reverse", String(false), "Bool", "ptt reverse logic e.g. false"}
    };

    void ConfigClass::Init(String _filename){
        Serial.println("ConfigClass constructor");
        filename = _filename;
       if(!SPIFFS.begin(true)){
           Serial.println("An Error has occurred while mounting SPIFFS");
           return;
        }
        LoadEntriesFromSPIFFS();
    };

    Entry * ConfigClass::GetEntryByKey(String key){

        for (std::vector<Entry>::iterator it = Entries.begin() ; 
            it != Entries.end(); ++it){
                if(it->key == key){
                    return  &(*it);
                }
            }
        Serial.printf("Error: there's no %s key\r\n",key.c_str());
        return NULL;
    }

    Entry ConfigClass::operator [](int i) const    {
        return Entries[i];
    }
    
    Entry * ConfigClass::operator [](String key)     {
        return GetEntryByKey(key);
    }

   //Load from SPIFFS to Entries
   void ConfigClass::LoadEntriesFromSPIFFS(){
        Serial.printf("LoadEntriesFromSPIFFS\r\n");
        // Open file for reading 
        File file = SPIFFS.open(filename, FILE_READ);
        
        if(!file){
            Serial.println("There was an error opening the file for reading");
            Serial.println("Maybe the file %s doesn't esist! Load default values and save them!");
            SaveDefaultsToSPIFFS();
            // Try again opening file for reading 
            file = SPIFFS.open(filename, FILE_READ);
            if(!file){
                Serial.println("There was an error opening the file for reading even after saving defaults!");
                return;
            }
        }
        
        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/assistant to compute the capacity.
        int esize = this->size();
        Serial.printf("Entries size is %d bytes \r\n", esize);
        
        DynamicJsonDocument doc(esize) ;
        DeserializationError error = deserializeJson(doc, file);
        if (error){
            Serial.println(F("Failed to read file, using default configuration"));
            SaveDefaultsToSPIFFS();
            return;
        }    
        // Load Entries with value recovered from SPIFFS
        for(std::vector<Config_ns::Entry>::iterator it = Entries.begin(); it != Entries.end(); it++ ){
                //Serial.printf("Loading entry %s with value ",
                //    it->key.c_str());
                if(it->value_type == "Bool"){
                    it->SetBool(doc[it->key]);
                    //Serial.println(it->val_bool);
                }else if(it->value_type == "Int"){
                    it->SetInt(doc[it->key]);
                    //Serial.println(it->val_int);
                }else if(it->value_type == "Long"){
                    it->SetLong(doc[it->key]);
                    //Serial.println(it->val_long);
                }else if(it->value_type == "String"){
                    it->SetString(doc[it->key]);
                    //Serial.println(it->val_string.c_str());
                }
        }
    }

   //Save from Entries to filename in SPIFFS
   void ConfigClass::SaveEntriesToSPIFFS(){
        Serial.printf("Save EntriesToSPIFFS %s\r\n", filename.c_str());
        
        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/assistant to compute the capacity.
        int esize = this->size();
        Serial.printf("Entries size is %d bytes \r\n", esize);
        
        DynamicJsonDocument doc(esize) ;

        for(std::vector<Config_ns::Entry>::iterator it = Entries.begin(); it != Entries.end(); it++ ){
                Serial.printf("Saving entry %s with value ",
                    it->key.c_str());
                if(it->value_type == "Bool"){
                    doc[it->key] = it->val_bool;
                    Serial.println(it->val_bool);
                }else if(it->value_type == "Int"){
                    doc[it->key] = it->val_int;
                    Serial.println(it->val_int);
                }else if(it->value_type == "Long"){
                    doc[it->key] = it->val_long;
                Serial.println(it->val_long);
                }else if(it->value_type == "String"){
                    doc[it->key] = it->val_string;
                Serial.println(it->val_string.c_str());
                }
        }

        // Open file for writing 
        File file = SPIFFS.open(filename, FILE_WRITE);
        
        if(!file){
            Serial.println("There was an error opening the file for writing");
            return;
        }

        // Serialize JSON to file
        if (serializeJson(doc, file) == 0) {
           Serial.println(F("Failed to write defaults to file"));
        }

        file.close();
    }

    //Save from Entries to filename in SPIFFS
    void ConfigClass::SaveDefaultsToSPIFFS(){
        Serial.printf("SaveDefaultsToSPIFFS %s\r\n", filename.c_str());
        
        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        int esize = this->size();
        Serial.printf("Entries size is %d bytes \r\n", esize);
        DynamicJsonDocument doc(esize);

        for(std::vector<Config_ns::Entry>::iterator it = Entries.begin(); it != Entries.end(); it++ ){
                //Serial.printf("Saving entry %s with default value ",
                //    it->key.c_str());
                if(it->value_type == "Bool"){
                    bool bb = (it->value_default == "true"); 
                    doc[it->key] = bb;
                    //Serial.println(bb);
                }else if(it->value_type == "Int"){
                    int ii = atoi(it->value_default.c_str()); 
                    doc[it->key]=ii;
                    //Serial.println(ii);
                }else if(it->value_type == "Long"){
                    long ll = atol(it->value_default.c_str());
                    doc[it->key]=ll;
                    //Serial.println(ll);
                }else if(it->value_type == "String"){
                    String ss = it->value_default;
                    doc[it->key] = ss;
                    //Serial.println(ss);
                }
        }
        
        SPIFFS.remove(filename); // remove the file, if any

        // Open file for writing 
        File file = SPIFFS.open(filename, FILE_WRITE);
        
        if(!file){
            Serial.println("There was an error opening the file for writing");
            return;
        }
        
        // Serialize JSON to file
        if (serializeJson(doc, file) == 0) {
           Serial.println(F("Failed to write defaults to file"));
        }
        // Close the file
        file.close();
    }

    // Prints the content of a file to the Serial
    void ConfigClass::printFile() {
        // Open file for reading 
        File file = SPIFFS.open(filename, FILE_READ);
        if(!file){
            Serial.println("There was an error opening the file for reading");
            return;
        }

    // Extract each characters by one by one
    while (file.available()) {
        Serial.print((char)file.read());
    }
    Serial.println();

    // Close the file
    file.close();
    }

    int ConfigClass::size(){
        int size = 0;
        for(std::vector<Config_ns::Entry>::iterator it = Entries.begin(); it != Entries.end(); it++ ){
            size += it->mysize;    
        }
        // round to the higher power of 2 i.e. 1024 or 2048 or
        double aa = powf(2, int(log2(size*1.5)+0.99999)); //1.5 is a safety margin
        return (int) aa;
    }

}
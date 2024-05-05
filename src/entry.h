#include <Arduino.h>

namespace Config_ns {

    class Entry {
    public:
        String key;
        String value_default;
        String value_type;
        String description;
        Entry(String key, String value_default,String value_type,String description);
        void SetInt(int value);
        void SetLong(long value);
        void SetBool(bool value);
        void SetString(String value);
        
        void LoadValue(String value);
        void LoadValue(int value);
        void LoadValue(long value);
        void LoadValue(bool value);
    
        int val_int;
        long val_long;
        bool val_bool;
        String val_string;

        int mysize;
    };

}
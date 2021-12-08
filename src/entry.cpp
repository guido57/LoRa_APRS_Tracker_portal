#include <entry.h>

namespace Config_ns
{
    Entry::Entry(String key, String value_default,String value_type,String description){
        this->key = key;
        this->value_default = value_default;
        this->value_type = value_type;
        this->description = description;
        mysize = key.length() +  
                4 + // "":,  
                6  + // int as a String, like 32768
                10 + // long as as string, like 4096000000
                6  + // bool as a string, like false
                val_string.length() + 1
                ;
        if(value_type == "Int")
            val_int = atoi(value_default.c_str());
        else if(value_type == "Long")
            val_long = atol(value_default.c_str());
        else if(value_type == "Bool")
            val_bool = (value_default == "true");
        else if(value_type == "String")
            val_string = value_default;
    }
    void Entry::SetInt(int value){
        val_int = value;
    }
    void Entry::SetLong(long value){
        val_long = value;
    }
    void Entry::SetBool(bool value){
        val_bool = value;
    }
    void Entry::SetString(String value){
        mysize = mysize - val_string.length() + value.length();
        val_string = value;
        
    }
    void Entry::LoadValue(String value){
        mysize = mysize - val_string.length() + value.length();
        val_string = value;
    }
    void Entry::LoadValue(int value){
        val_int = value;
    }
    void Entry::LoadValue(bool value){
        val_bool = value;
    }
    void Entry::LoadValue(long value){
        val_long = value;
    }
}



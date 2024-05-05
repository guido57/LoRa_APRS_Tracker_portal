#include <Arduino.h>
#include <default.h>
#include <entry.h>
#include <vector>

namespace Config_ns {

  extern std::vector<Entry> Entries;

  class ConfigClass {
    
    public:  
      void Init(String filename);
      Entry * GetEntryByKey(String key);
      Entry operator [](int i) const;
      Entry * operator [](String key) ;
      void SaveEntriesToSPIFFS();
      void SaveDefaultsToSPIFFS();
      void LoadEntriesFromSPIFFS();
      String filename;
      void printFile();
      int size();


  };

}

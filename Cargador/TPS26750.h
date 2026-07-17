#include "TPSRegisters.h"
#include <Wire.h>

//Clase del cargador
class TPS26750 {
  private:
    float voltage;  //V
    float maxvoltage;  //V
    float minvoltage;  //V
    float maxpower;  //W
    float current;  //A
    bool success;   
  public:
    TPS26750(){voltage = 0; maxvoltage = 0; minvoltage = 0; maxpower = 0; current = 0; success=false;}

    float getvoltage(){return voltage;}
    float gecurrent(){return current;}
    bool getsuccess(){return success;}

    bool TPSnBytesRead(uint8_t DEVADDR, uint8_t REGADDR, uint8_t* data, uint8_t n);
    bool TPSnBytesWrite(uint8_t DEVADDR, uint8_t REGADDR, const uint8_t* data, uint8_t n);
    bool sendCommand4CC(const char* command);

    bool getContract();
    bool loadConfig();
};

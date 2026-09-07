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
  //Get
    float getmaxvoltage(){return maxvoltage;}
    float getminvoltage(){return minvoltage;}
    float getvoltage(){return voltage;}
    float gecurrent(){return current;}
    float getmaxpower(){return maxpower;}
    bool getsuccess(){return success;}

  //Acciones
    bool TPSnBytesRead(uint8_t DEVADDR, uint8_t REGADDR, uint8_t* data, uint8_t n);
    bool TPSnBytesWrite(uint8_t DEVADDR, uint8_t REGADDR, const uint8_t* data, uint8_t n);
    bool sendCommand4CC(const char* command);
    bool getContract();
    bool loadConfig();
    bool waitCmd1Complete(uint32_t timeoutMs);
    bool waitReadyForPatch(uint32_t timeoutMs);
    bool waitForMode(const char* targetMode, uint32_t timeoutMs);
};

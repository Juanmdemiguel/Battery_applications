#include "BQ25756.h" 
#include "TPS26750.h"

const int PIN_CHEM=2;
const int PIN_C0=3;
const int PIN_C1=4;
const int PIN_C2=5;
const int PIN_3A=6;
const int PIN_6A=7;
const int PIN_9A=8;
const int PIN_12A=9;

const int FB_A=10;
const int FB_B=11;
const int FB_C=23;

class Cargador {
private:
  bool ChargeEnable = false;
  bool USBC= false; 
  bool Config[8] = {};
  BQ25756 contCarga;
  TPS26750 contUSB;

public:
  //Funciones públicas
  bool enableBQ();
  bool enableTPS();
  bool updateConfig(); 
  bool updateChargeCurrent(); 
  bool updateChargeVoltage(); 
  bool PDManagement();

  //Comprobaciones / Debug
  void printStatus();
  void printConfig();
};
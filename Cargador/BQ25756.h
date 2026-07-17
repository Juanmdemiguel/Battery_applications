#include "BQRegisters.h"
#include "I2C.h"
#include <Wire.h>
// Estructura de configuración iniciada con los parámetros por defecto
struct BQ25756_Config {

  //Control de parámetros
  double packVoltage = 0;
  double inputCurrent = 0;
  double batCurrent = 0;

  //Límites
  uint16_t chargeCurrentLimit = 2000;        // mA; fast charge current regulation limit
  uint16_t inputCurrentDPMLimit = 3000;      // mA; input current DPM regulation limit
  uint16_t inputVoltageDPMLimit = 4200;      // mV; input voltage regulation limit
  
  //Carga
  bool chargeEnabled = true;                 // Enable or disable charging
};

//Clase del cargador
class BQ25756 {
private:
  BQ25756_Config config;

public:
  //Funciones públicas
  bool enableCharge();
  bool disableCharge();
  bool PFM_control();
  bool WD_control();

  //Setters
  bool setChargeCurrentLimit(uint16_t mA); 
  bool setInputCurrentLimit(uint16_t mA); 
  bool setInputVoltageLimit(uint16_t V);

  //Getters
  uint16_t getChargeCurrentLimit(); 
  uint16_t getInputCurrentLimit(); 
  uint16_t getTerminationCurrentLimit(); 
  uint16_t getInputVoltageLimit();
  uint8_t getChargeStatus();
  double getBatteryVoltage(); 
  double getBatteryCurrent(); 
  double getInputCurrent();
};

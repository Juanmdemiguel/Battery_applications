#pragma once
#include <Arduino.h>
//#include <cstdint>
enum BMSMode { MODE_NORMAL, MODE_IDLE, MODE_DOZE, MODE_SLEEP, MODE_POWERDOWN, MODE_UNKNOWN }; //Define los modos de operación

static const uint8_t ISLADDR = 0x28; //La librería wire.h hace 0x28<<0 || 0x28<<1
//0x28 == 00101000. <<1 == 01010001 == 0x51. <<0 == 01010000 == 0x50 -> Usar solo si ADDR Pin está conectado con Vss.

class BMS {
  //Monitorización del sistema
  float packCurrent;
  float cellVoltage[8];
  float packVoltage;
  uint8_t status[4];
  float temp[3];
  //System mode
  uint8_t cellSelected=0x83; //3 Celdas
  uint16_t currentGain = 5;  //Revisar
  float rSense = 0.005f;     // 5 mOhms

  public:
  //Update
    bool updateCellsVoltages();
    bool updatePackVoltage();
    void updatePackCurrent();
    bool updateStatus();
    bool updateTemp();
    void updateGain();  

  //Acciones
    uint16_t twoByteRead(uint8_t ADDR);
    bool oneByteWrite(uint8_t reg, uint8_t data);
    bool oneByteRead(uint8_t ADDR, uint8_t &data);
    bool setCellCount(uint16_t n);      
    float xtVoltageToTemp(float voltage);
    bool balanceCells(uint8_t cells, int ms);
    bool printStatus();

  //Get
    float getPackCurrent() const { return packCurrent; }
    float getPackVoltage() const { return packVoltage; }
    float getCellVoltage(uint8_t index) const { return (index < 8) ? cellVoltage[index] : 0.0f; }
    float getTemp(uint8_t index) const { return (index < 3) ? temp[index] : 0.0f; }     
    BMSMode getMode();   
    };
#include "BMS.h"
#include <Wire.h>

uint16_t BMS::twoByteRead(uint8_t ADDR){
  Wire.beginTransmission(ISLADDR);
  Wire.write(ADDR);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)2);
  if (Wire.available()>=2){
    uint8_t low = Wire.read();
    return (uint16_t)(Wire.read() & (uint8_t)0x0F) << 8 | low; //Last four need to be reserved
  } else return 0;
}

bool BMS::updateCellsVoltages(){
 for (uint8_t i = 0; i < 8; i++) {
   if ((cellSelected >> i) & 0x01) {
      uint16_t raw=twoByteRead(0x90+2*i);
      if (raw==0) return false;
      cellVoltage[i]=(raw*1.8f*8.0f)/(4095.0f*3.0f);
   }
  }
  return true;
}

void BMS::updatePackCurrent(){
  uint16_t raw=twoByteRead(0x8E);
  packCurrent=(raw*1.8f)/(4095.0f*currentGain*rSense);
}

bool BMS::updatePackVoltage(){
  uint16_t raw=twoByteRead(0xA6);
  if (raw==0) return false;
  packVoltage=(raw*1.8f*32.0f)/(4095.0f);
  return true;
}

bool BMS::updateStatus(){
  uint16_t raw=twoByteRead(0x80);
    status[0]= (uint8_t)(raw & 0x00FF);
    status[1]= (uint8_t)(raw >> 8);
  raw=twoByteRead(0x82);
    status[2]= (uint8_t)(raw & 0x00FF);
    status[3]= (uint8_t)(raw >> 8);
  return true;
}

bool BMS::updateTemp(){
  uint16_t raw;
  float tempV;
  for(uint8_t i = 0; i < 3; i++){
    raw=twoByteRead(0xA0+2*i);
    if (raw==0) return false;
    tempV=(raw*1.8f)/4095.0f;
    temp[i]=(tempV*1000.0f)/1.8527f-273.15f;
  }
  return true;
}

void BMS::updateGain() {
    Wire.beginTransmission(ISLADDR);
    Wire.write(0x85); 
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1);
    
    if (Wire.available()) {
        uint8_t regVal = Wire.read();
        uint8_t gainBits = (regVal >> 4) & 0x03; 
        
        switch (gainBits) {
            case 0x00: currentGain = 50;   break;
            case 0x01: currentGain = 5;  break;
            case 0x02: 
            case 0x03: currentGain = 500; break;
            default:   currentGain = 5;  break; 
        }
    }
}

bool BMS::setCellCount(uint16_t n){
if (!(n == 3 || n == 4 || n == 7 || n == 8)) return false;
  uint8_t code;
  switch(n){
    case 3: default: code = 0x83; break;  
    case 4: code = 0xC3; break;
    case 7: code = 0xEF; break;
    case 8: code = 0xFF; break;
  }
    Wire.beginTransmission(ISLADDR);
    Wire.write(0x49); // Registro de Cell Select
    Wire.write(code);
    if (Wire.endTransmission() == 0){
      cellSelected=code;
      return true;
    }
    return false;
}


// 0x87 Permite quitar funciones automáticas para controlarlas por uC. Además hace el cell balancing de las celdas seleccionadas
// 0x84 Establecelas celdas a balancear


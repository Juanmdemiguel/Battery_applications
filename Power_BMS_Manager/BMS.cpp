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

bool BMS::oneByteWrite(uint8_t reg, uint8_t data){
  Wire.beginTransmission(ISLADDR);
  Wire.write(reg);
  Wire.write(data);
  return Wire.endTransmission() == 0;
}

bool BMS::oneByteRead(uint8_t ADDR, uint8_t &data) {
  Wire.beginTransmission(ISLADDR);
  Wire.write(ADDR);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1) != 1) return false;

  data = Wire.read();
  return true;
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

  for (uint8_t i = 0; i < 3; i++) {
    raw = twoByteRead(0xA0 + 2 * i);
    tempV = (raw * 1.8f) / 4095.0f; //Igual para las tres medidas. P91.

    if (i == 0) temp[i] = (tempV * 1000.0f) / 1.8527f - 273.15f; //Temp Interna. P91.
    else temp[i] = xtVoltageToTemp(tempV);
  }
  return true;
}

//Interpolación desde la Tabla 70. P96 RENESAS ISL94202 DATASHEET.
float BMS::xtVoltageToTemp(float voltage){
    const float V[] = {0.7396f, 0.6112f, 0.4537f, 0.2887f, 0.1495f};
    const float T[] = {-40.0f, 0.0f, 25.0f, 50.0f, 80.0f};

    voltage /= 2.0f; //Gain=2. P96 ENESAS ISL94202 DATASHEET. Figure 39.
    if (voltage > V[0] || voltage < V[4]) return 0; //Comprueba la ventana posible
    for (uint8_t i = 0; i < 4; i++) {
        if (voltage <= V[i] && voltage >= V[i + 1]) {
            return T[i]+(voltage - V[i])*((T[i + 1] - T[i])/(V[i + 1] - V[i]));  // Interpolación lineal
        }
    }
    return 0;
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
if (!(n == 3 || n == 4 || n == 6 || n == 7 || n == 8)) return false;
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

bool BMS::balanceCells(uint8_t cells, int ms){
  uint8_t cb = 1 << (cells - 1);

  // Activar equilibrio por MCU
  uint8_t control2 =0; //P82 DATASHEET
  if (!oneByteRead(0x87, control2)) return false;

  control2 |= (1 << 5);
  if (!oneByteWrite(0x87, control2)) return false;

  // Seleccionar las celdas a balancear
  if (!oneByteWrite(0x84, cb)) return false; //P79 DATASHEET

  // CBAL_ON = 1 -> activar balanceo
  control2 |= (1 << 0);
  if (!oneByteWrite(0x87, control2)) return false;

  // Mantener el balanceo durante ms
  delay(ms);

  // CBAL_ON = 0 -> detener balanceo
  control2 &= ~(1 << 0);
  if (!oneByteWrite(0x87, control2)) return false; //P82 DATASHEET

  oneByteWrite(0x84, 0x00);

  // Volver al control automático
  control2 &= ~(1 << 5);
  if (!oneByteWrite(0x87, control2)) return false;
  return true;
}




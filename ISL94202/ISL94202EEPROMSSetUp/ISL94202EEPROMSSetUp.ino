#include <Wire.h>
#include "Registers.h"
#include "ISL94202Config.h"

bool check=true; //Flag que indica consecución del código

//Prototipos de las funciones utilizadas
bool writeReg(uint8_t reg, uint8_t value);
uint8_t readReg(uint8_t reg);
bool writeEEPROM(uint8_t reg, uint16_t value);
bool enableEEPROMAccess();
bool disableEEPROMAccess();
uint16_t CELLmVtoHEX(uint16_t mV);
uint16_t CELLHEXtomV(uint16_t code);
uint16_t THmVtoHEX(uint16_t mV);
uint16_t THHEXtomV(uint16_t code);
bool setOV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t CPWD=1);
bool setUV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t CPWD=1);
bool setEOCThres(uint16_t mV);
bool setOVUVTimer(uint8_t TU = 0b10, uint8_t Timing = 1);
bool setLCVL(uint16_t mV = 2300);
bool setDCOC(uint16_t A, uint8_t TU = 0b01, uint8_t Timing = 160);
bool setCOC(uint16_t A, uint8_t TU = 0b01, uint8_t Timing = 160);
bool setDCSC(uint16_t A, uint8_t TU = 0b00, uint8_t Timing = 200);
bool setCBMin(uint16_t mV);
bool setCBMax(uint16_t mV);
bool setCBMinErr(uint16_t mV);
bool setCBMaxErr(uint16_t mV);
bool setCBOnTime(uint16_t ms, uint8_t TU=0b10);
bool setCBOffTime(uint16_t ms, uint8_t TU=0b10);
bool setCBUT(uint16_t thres, uint16_t recov);
bool setCBOT(uint16_t thres, uint16_t recov);
bool setCOT(uint16_t thres, uint16_t recov);
bool setCUT(uint16_t thres, uint16_t recov);
bool setDCOT(uint16_t thres, uint16_t recov);
bool setDCUT(uint16_t thres, uint16_t recov);
bool setCellCount(uint16_t n);
bool setUp0Reg(bool PSD = 0, bool XT2M = 0, bool TGAIN = 0, bool PCFETE = 0, bool DOWD = 0, bool OWPSD = 0);
bool setUp1Reg(bool CBDD = 0, bool CBDC = 1, bool DFODUV = 0, bool CFODOV = 0, bool UVLOPD = 0, bool CB_EOC = 0);

void setup() {
 Wire.begin(); 
 Wire.setClock(400000);

 Wire.beginTransmission(0x01);
 Wire.write(00);
 Wire.endTransmission(true);

 if(enableEEPROMAccess()){
    check = check && setOV(OVThreshold, OVRecovery, OVLock, OVChargeDetectPulseWidth) //Ponia 3000 en lock?
     && setUV(UVThreshold, UVRecovery, UVLock, UVChargeDetectPulseWidth)
     && setEOCThres(EndOfChargeThreshold)
     && setLCVL(LowVoltageCharge)
     && setOVUVTimer(OVUVTimeUnits, OVUVTimer)
     && setDCOC(DischargeOCAmps, DisChargeOCUnits, DisChargeOCTime)
     && setCOC(ChargeOCAmps, ChargeOCUnits, ChargeOCTime)
     && setDCSC(DischargeSCAmps, DischargeSCUnits, DischargeSCTime)
     && setCBMin(CBMin)
     && setCBMax(CBMax)
     && setCBMaxErr(CBMaxErr)
     && setCBMinErr(CBMinErr)
     && setCBOnTime(CBOnTime, CBOnTimeUnits)
     && setCBOffTime(CBOffTime, CBOffTimeUnits)
     && setCBUT(CBUTThreshold, CBUTRecovery)
     && setCBOT(CBOTThreshold, CBOTRecovery)
     && setCOT(ChargeOTThreshold, ChargeOTRecovery)
     && setCUT(ChargeUTThreshold, ChargeUTRecovery)
     && setDCOT(DisChargeOTThreshold, DisChargeOTRecovery)
     && setDCUT(DisChargeUTThreshold, DisChargeUTRecovery)
     && setCellCount(CELLCOUNT)
     && setUp0Reg(FailShutdown,TH2Mode,TempGain,PreChargeEnable,OpenWireDisable,OpenWireShutdown)
     && setUp1Reg(DischargeCB,ChargeCB,DischargeUV,ChargeOV,UVLockPowerDown,EndOfChargeCB);
    disableEEPROMAccess();
 } else check = false;
}

void loop() {
  // No es necesario utilizar la función loop() para escribir, pero se puede usar para informar de errores. 
}

//Declaraciones de las funciones utilizadas
bool writeReg(uint8_t reg, uint8_t value){
  if (reg > 0xAB) return false;
  Wire.beginTransmission(ISLADDR);
  Wire.write(reg);
  Wire.write(value);
  if (Wire.endTransmission() != 0) return false;
  return true;
}

uint8_t readReg(uint8_t reg){ //0xFF se usa como código de error
  Wire.beginTransmission(ISLADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0xFF;  

  Wire.requestFrom(ISLADDR, 1); // 1 byte
  if (Wire.available()) return Wire.read(); //Devuelve un entero con el número de bytes disponibles
  else return 0xFF;
}

bool writeEEPROM(uint8_t reg, uint16_t value) { //Funciona hasta dos bytes
    if (reg > 0x4B) // Comprueba los límites de los registros EEPROM
        return false;

    uint8_t base = reg & 0xFC; //Redonde al byte de la primera página
    uint8_t buffer[4];

    // Lecturas de un byte
    // El primer byte recarga la página (>200µs)
    for (uint8_t i = 0; i < 4; i++) {
        Wire.beginTransmission(ISLADDR);
        Wire.write((uint8_t)(base + i));
        Wire.endTransmission(false);
        if (i == 0) delay(1); // 1ms de recargar la página
        Wire.requestFrom(ISLADDR, 1);
        if (Wire.available()) buffer[i] = Wire.read(); //Llena el buffer con las lecturas pervias de la EEPROM
    }
    buffer[reg & 0x03] = value & 0x00FF; //Cambia la posición del bit deseado, usando solo el último byte
    // 0x03=00000011 -> reg & 0xFC selecciona la página -> 0x03 selecciona la posición
    if((value & 0xFF00) > 0) buffer[(reg & 0x03) + 1] = value >> 8 ;

    // Escribe el primer byte dos veces
    for(uint8_t i = 0; i<2; i++){
      Wire.beginTransmission(ISLADDR);
      Wire.write(base);
      Wire.write(buffer[0]);
      Wire.endTransmission(true);
      delay(30);
    }

    // Escribe los bytes restantes
    for (uint8_t i = 1; i < 4; i++) {
        Wire.beginTransmission(ISLADDR);
        Wire.write((uint8_t)(base + i));
        Wire.write(buffer[i]);
        Wire.endTransmission(true);
        delay(30);
    }
    return value == readReg(reg);
}

//P148 DATASHEET ISL94202
//Fuerza el modo IDLE, deshabilita los umbrales y las lecturas y habilita los registros EEPROM
bool enableEEPROMAccess(){return writeReg(0x88, 0x01) && writeReg(0x87, 0x04) && 
  writeReg(0x44, 0x00) && writeReg(0x45, 0x00) && writeReg(0x89, 0x01);
}

bool disableEEPROMAccess(){return writeReg(0x89, 0x00) && writeReg(0x87, 0x00) &&  
  writeReg(0x44, 0x00) && writeReg(0x45, 0x00); //0x44-45 se necesitan cambiar al valor deseado
} 

uint16_t CELLmVtoHEX(uint16_t mV){return (mV*1000*3*4095)/(1.8*8);}
uint16_t CELLHEXtomV(uint16_t code){return (code*1.8*8*1000)/(4095*3);}

uint16_t THmVtoHEX(uint16_t mV){return (mV*1000*4095)/(1.8);}
uint16_t THHEXtomV(uint16_t code){return (code*1.8*1000)/(4095);}

bool setOV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t CPWD){ 
  if (recov>thres || thres>lock || CPWD>15) return false;
  uint16_t code[3];
  code[0]=CPWD<<12 | CELLmVtoHEX(thres);
  code[1]=CELLmVtoHEX(recov);
  code[2]=CELLmVtoHEX(lock);
  return writeEEPROM(OVThresADDR, code[0]) && writeEEPROM(OVRecovADDR, code[1]) && writeEEPROM(OVLockADDR, code[2]);
}

bool setUV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t CPWD){ 
  if (recov<thres || thres<lock || CPWD>15) return false;
  uint16_t code[3];
  code[0]=CPWD<<12 | CELLmVtoHEX(thres);
  code[1]=CELLmVtoHEX(recov);
  code[2]=CELLmVtoHEX(lock);
  return writeEEPROM(UVThresADDR, code[0]) && writeEEPROM(UVRecovADDR, code[1]) && writeEEPROM(UVLockADDR, code[2]);
}

bool setEOCThres(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(EOCThresADDR, code);
}

bool setOVUVTimer(uint8_t TU, uint8_t Timing){
  uint16_t code = TU << 10 | Timing ;
  return writeEEPROM(OVTimADDR, code) && writeEEPROM(UVTimADDR, code);
}

bool setLCVL(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(LVCLADDR, code);
}

//OWTimer

bool setDCOC(uint16_t A, uint8_t TU, uint8_t Timing){
  const uint16_t A_values[]  = {4, 8, 16, 24, 32, 48, 64};
  uint8_t dcoc = 0xFF;
  for(uint8_t i=0; i<7; i++){
    if(A==A_values[i]){dcoc=i; break;} 
  }
  if(dcoc==0xFF) return false;
  uint16_t code = (uint16_t)dcoc << 12| TU << 10| Timing ;
  return writeEEPROM(DCOCTIMADDR, code);
}

bool setCOC(uint16_t A, uint8_t TU, uint8_t Timing){
  const uint16_t A_values[] = {1, 2, 4, 6, 8, 12, 16, 24};
  uint8_t dcoc = 0xFF;
  for(uint8_t i=0; i<8; i++){
    if(A==A_values[i]){dcoc=i; break;} 
  }
  if(dcoc==0xFF) return false;
  uint16_t code = (uint16_t)dcoc << 12| TU << 10| Timing ;
  return writeEEPROM(COCTIMADDR, code);
}

bool setDCSC(uint16_t A, uint8_t TU, uint8_t Timing){
  const uint16_t A_values[] = {16, 24, 32, 48, 64, 96, 128};
  uint8_t dcoc = 0xFF;
  for(uint8_t i=0; i<7; i++){
    if(A==A_values[i]){dcoc=i; break;} 
  }
  if(dcoc==0xFF) return false;
  uint16_t code = (uint16_t)dcoc << 12 | TU << 10 | Timing ;
  return writeEEPROM(DCSCTIMADDR, code);
}

bool setCBMin(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMinADDR, code);
}

bool setCBMax(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMaxADDR, code);
}

bool setCBMinErr(uint16_t mV){
  if (mV<10 ||mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMinDifADDR, code);
}

bool setCBMaxErr(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMaxDifADDR, code);
}

//P53 DATSHEET ISL94202
bool setCBOnTime(uint16_t ms, uint8_t TU){ 
  if (ms>1023 || TU > 0b11) return false;
  uint16_t code = TU<<10 | ms;
  return writeEEPROM(CBOnTimADDR, code);
}

//P53 DATSHEET ISL94202
bool setCBOffTime(uint16_t ms, uint8_t TU){ 
  if (ms>1023 || TU > 0b11) return false;
  uint16_t code = TU<<10 | ms;
  return writeEEPROM(CBOnTimADDR, code);
}

bool setCBUT(uint16_t thres, uint16_t recov){ 
  if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CBUTThresADDR, code[0]) && writeEEPROM(CBUTRecovADDR, code[1]);
}

bool setCBOT(uint16_t thres, uint16_t recov){ 
  if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CBOTThresADDR, code[0]) && writeEEPROM(CBOTRecovADDR, code[1]);
}

bool setCOT(uint16_t thres, uint16_t recov){ 
  if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(COTThresADDR, code[0]) && writeEEPROM(COTRecovADDR, code[1]);
}

bool setCUT(uint16_t thres, uint16_t recov){ 
  if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CUTThresADDR, code[0]) && writeEEPROM(CUTRecovADDR, code[1]);
}

bool setDCOT(uint16_t thres, uint16_t recov){ 
  if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(DCOTThresADDR, code[0]) && writeEEPROM(DCOTRecovADDR, code[1]);
}

bool setDCUT(uint16_t thres, uint16_t recov){ 
  if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(DCUTThresADDR, code[0]) && writeEEPROM(DCUTRecovADDR, code[1]);
}

bool setCellCount(uint16_t n){
if (!(n == 3 || n == 4 || n == 7 || n == 8)) return false;
  uint8_t code;
  switch(n){
    case 3: default: code = 0x83; break;  
    case 4: code = 0xC3; break;
    case 7: code = 0xEF; break;
    case 8: code = 0xFF; break;
  }
  return writeEEPROM(CellCountADDR, code);
}

bool setUp0Reg(bool PSD, bool XT2M, bool TGAIN, bool PCFETE, bool DOWD, bool OWPSD){
  uint8_t code = PSD << 7 | XT2M << 5| TGAIN << 4 | PCFETE << 2 | DOWD << 1 | OWPSD;
return writeEEPROM(SetUp0, code);
}

bool setUp1Reg(bool CBDD, bool CBDC, bool DFODUV, bool CFODOV, bool UVLOPD, bool CB_EOC){
  uint8_t code = CBDD << 7 | CBDC << 6| DFODUV << 5 | CFODOV << 4 | UVLOPD << 3 | CB_EOC;
return writeEEPROM(SetUp1, code);
}
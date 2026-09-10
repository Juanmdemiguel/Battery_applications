#include <Wire.h>
#include "Registers.h"
#include "ISL94202Config.h"

bool check=true; //Flag que indica consecución del código
enum BMSMode { MODE_NORMAL, MODE_IDLE, MODE_DOZE, MODE_SLEEP, MODE_POWERDOWN, MODE_UNKNOWN };

//Prototipos de las funciones utilizadas
bool checkStep(bool result, const char* stepName);
bool writeReg(uint8_t reg, uint8_t value);
bool readReg(uint8_t reg, uint8_t &val);
bool waitEEPROMReady(uint16_t timeout_ms = 50);
bool writeEEPROM(uint8_t reg, uint16_t value,bool is16Bit=true);
bool writeByteEEPROM(uint8_t addr, uint8_t data);
bool readEEPROMPage(uint8_t base, uint8_t buffer[4]);
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
bool printFaults();
bool checkEEPROMWriteReady();
BMSMode getMode();
bool waitForLowPowerState(uint16_t timeout_ms = 50);

void setup() {
 Serial.begin(115200); 
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.println("Iniciando sistema BMS...");
  Wire.begin(); //Init as master -> teensy: 18 SDA0, 19 SCL0
  Wire.setClock(50000);
  Wire.beginTransmission(ISLADDR);

  uint8_t err=Wire.endTransmission();
  /* Códigos de error:
      0-> Éxito, el dispositivo esclavo ha contestado ACK (acknowledge)
      1-> Buffer de datos demasiado largo
      2-> Se envió un mensaje al esclavo, pero no contestó con ACK
      3-> El dispositivo esclavo respondió a su dirección, pero rechazó uno de los bytes de datos transmitidos posteriormente
      4-> Error genérico, probablemente no detectado
      5-> Tiempo de espera agotado */
  if (err==0) {
      Serial.println("BMS detectado correctamente.");
      Serial.println(" ");
    } else {
      Serial.print("Código de error: "); Serial.println(err);
      Serial.println("Error I2C: BMS no encontrado.");
      while (1){ // Bloquea el programa si no hay BMS con led de error
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      } 
  }

if (checkStep(enableEEPROMAccess(),"enableEEPROMAccess")) {
  check = true;
  check &= checkEEPROMWriteReady();
  check &= checkStep(setOV(OVThreshold, OVRecovery, OVLock, OVChargeDetectPulseWidth), "setOV");
  check &= checkStep(setUV(UVThreshold, UVRecovery, UVLock, UVChargeDetectPulseWidth), "setUV");
  check &= checkStep(setEOCThres(EndOfChargeThreshold), "setEOCThres");
  check &= checkStep(setLCVL(LowVoltageCharge), "setLCVL");
  check &= checkStep(setOVUVTimer(OVUVTimeUnits, OVUVTimer), "setOVUVTimer");
  check &= checkStep(setDCOC(DischargeOCAmps, DisChargeOCUnits, DisChargeOCTime), "setDCOC");
  check &= checkStep(setCOC(ChargeOCAmps, ChargeOCUnits, ChargeOCTime), "setCOC");
  check &= checkStep(setDCSC(DischargeSCAmps, DischargeSCUnits, DischargeSCTime), "setDCSC");
  check &= checkStep(setCBMin(CBMin), "setCBMin");
  check &= checkStep(setCBMax(CBMax), "setCBMax");
  check &= checkStep(setCBMaxErr(CBMaxErr), "setCBMaxErr");
  check &= checkStep(setCBMinErr(CBMinErr), "setCBMinErr");
  check &= checkStep(setCBOnTime(CBOnTime, CBOnTimeUnits), "setCBOnTime");
  check &= checkStep(setCBOffTime(CBOffTime, CBOffTimeUnits), "setCBOffTime");
  check &= checkStep(setCBUT(CBUTThreshold, CBUTRecovery), "setCBUT");
  check &= checkStep(setCBOT(CBOTThreshold, CBOTRecovery), "setCBOT");
  check &= checkStep(setCOT(ChargeOTThreshold, ChargeOTRecovery), "setCOT");
  check &= checkStep(setCUT(ChargeUTThreshold, ChargeUTRecovery), "setCUT");
  check &= checkStep(setDCOT(DisChargeOTThreshold, DisChargeOTRecovery), "setDCOT");
  check &= checkStep(setDCUT(DisChargeUTThreshold, DisChargeUTRecovery), "setDCUT");
  check &= checkStep(setCellCount(CELLCOUNT), "setCellCount");
  check &= checkStep(setUp0Reg(FailShutdown, TH2Mode, TempGain, PreChargeEnable, OpenWireDisable, OpenWireShutdown), "setUp0Reg");
  check &= checkStep(setUp1Reg(DischargeCB, ChargeCB, DischargeUV, ChargeOV, UVLockPowerDown, EndOfChargeCB), "setUp1Reg");

  checkStep(disableEEPROMAccess(),"disableEEPROMAccess");
 } else check = false;

  check ? Serial.println("EEPROM configurada correctamente") : Serial.println("Fallo en la configuración de la EEPROM");
}

void loop() {
  // No es necesario utilizar la función loop() para escribir, pero se puede usar para informar de errores. 
}

//Declaraciones de las funciones utilizadas
bool checkStep(bool result, const char* stepName) {
  if (!result) {
    Serial.print("ERROR al configurar: ");
    Serial.println(stepName);
    Serial.println(" ");
  } return result;
}

bool writeReg(uint8_t reg, uint8_t value){
  //uint8_t res;
  if (reg > 0xAB) return false;
  Wire.beginTransmission(ISLADDR);
  Wire.write(reg);
  Wire.write(value);
  if (Wire.endTransmission() != 0) return false;
  delay(5);
  //readReg(reg, res);
  //if (res!=value) return false;
  return true;
}

bool readReg(uint8_t ADDR, uint8_t &val){
    Wire.beginTransmission(ISLADDR);
    Wire.write(ADDR);
    if (Wire.endTransmission(false) != 0) return false; // Error en la transmisión I2C
    if (Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1) != 1) return false; // El ISL94202 no respondió con el byte solicitado  
    val = Wire.read(); // Solo modifica 'val' si la lectura fue exitosa
    return true;       // Lectura correcta
}

bool waitEEPROMReady(uint16_t timeout_ms) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        Wire.beginTransmission(ISLADDR);
        if (Wire.endTransmission() == 0) return true; // ACK -> listo
        delay(1);
    }
    return false; //Timeout
}

bool writeByteEEPROM(uint8_t addr, uint8_t data) {
     Wire.beginTransmission(ISLADDR);
     Wire.write(addr);
     Wire.write(data);
    if (Wire.endTransmission(true) != 0) {
        Serial.println("Fallo escritura I2C (NACK)");
        return false;
    }
    return true;
}

bool readEEPROMPage(uint8_t base, uint8_t buffer[4]) {
    for (uint8_t i = 0; i < 4; i++) {

      bool ok = false;
      for (uint8_t attempt = 0; attempt < 3 && !ok; attempt++) {
        waitForLowPowerState(); // única sincronización, ANTES de empezar la secuencia atómica

        Wire.beginTransmission(ISLADDR);
        Wire.write((uint8_t)(base + i));
        uint8_t err = Wire.endTransmission(false);
        if (err != 0) { delay(2); continue; }

        if (i == 0) {
          delay(3);
          uint8_t n = Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1); // sin waitForLowPowerState aquí
          if (n != 1) { delay(2); continue; }
          Wire.read(); // descarte del recall

          // reabrir dirección para el dato real - sigue siendo parte de la MISMA secuencia
          Wire.beginTransmission(ISLADDR);
          Wire.write((uint8_t)(base + i));
          err = Wire.endTransmission(false);
          if (err != 0) { delay(2); continue; }
        }

        uint8_t n = Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1); // sin waitForLowPowerState aquí tampoco
        if (n != 1) { delay(2); continue; }
        buffer[i] = Wire.read();
        ok = true;
      }

      if (!ok) return false;
    }
    return true;
}
//P148 DATASHEET ISL94202.
bool writeEEPROM(uint8_t reg, uint16_t value, bool is16Bit) { //Funciona hasta dos bytes
    if (reg > 0x4B){ // Comprueba los límites de los registros EEPROM
    Serial.println("Fallo 1"); return false;}

    uint8_t base = reg & 0xFC; //Redonde al byte de la primera página
    uint8_t buffer[4];

    // Lecturas de un byte
    if (!readEEPROMPage(base, buffer)) { Serial.println("Fallo 2"); return false; }
    
   /* Serial.print("reg=0x"); Serial.print(reg, HEX);
    Serial.print(" base=0x"); Serial.print(base, HEX);
    Serial.print(" buffer recién leído=[");
    for (int i=0;i<4;i++){ Serial.print(buffer[i],HEX); Serial.print(" "); }
    Serial.println("]");*/

    // Cambia la posición del bit deseado, usando solo el último byte
    // 0x03=00000011 -> reg & 0xFC selecciona la página -> 0x03 selecciona la posición
    uint8_t offset = reg & 0x03;
    buffer[offset] = value & 0xFF; // Byte bajo
    if (is16Bit && offset < 3) buffer[offset + 1] = (uint8_t)(value >> 8) & 0xFF; // Byte alto

  
    uint16_t readValue = buffer[offset];
   if (is16Bit && offset < 3) readValue |= ((uint16_t)buffer[offset + 1] << 8);
  /*  Serial.print("reg=0x"); Serial.print(reg, HEX);
    Serial.print(" base=0x"); Serial.print(base, HEX);
    Serial.print(" leido=0x"); Serial.print(readValue, HEX);
    Serial.print(" buffer pre escritura=[");
    for (int i=0;i<4;i++){ Serial.print(buffer[i],HEX); Serial.print(" "); }
    Serial.println("]");*/


    // Escribe el primer byte dos veces
   // Escribe el primer byte dos veces
    for (uint8_t i = 0; i < 2; i++) {
      if (!writeByteEEPROM(base, buffer[0])) return false;
      delay(30);
    }
    // Escribe los bytes restantes
    for (uint8_t i = 1; i < 4; i++) {
        if (!writeByteEEPROM((uint8_t)(base + i), buffer[i])) return false;
        delay(30);
    }
 // Se vuelve a realizar la lectura, para observar si se ha almacenado correctamente
    // El primer byte recarga la página (>200µs)
    if (!readEEPROMPage(base, buffer)) { Serial.println("Fallo 3"); return false; }
  
    readValue = buffer[offset];
   if (is16Bit && offset < 3) readValue |= ((uint16_t)buffer[offset + 1] << 8);
   //Comprobación del resultado
   /*Serial.print("reg=0x"); Serial.print(reg, HEX);
    Serial.print(" base=0x"); Serial.print(base, HEX);
    Serial.print(" esperado=0x"); Serial.print(value, HEX);
    Serial.print(" leido=0x"); Serial.print(readValue, HEX);
    Serial.print(" buffer=[");
    for (int i=0;i<4;i++){ Serial.print(buffer[i],HEX); Serial.print(" "); }
    Serial.println("]");*/
   if(!(readValue == value)) Serial.println("Fallo 6");
   return (readValue == value);
}

//P148 DATASHEET ISL94202
//Fuerza el modo IDLE, deshabilita los umbrales y las lecturas y habilita los registros EEPROM
bool enableEEPROMAccess(){return writeReg(0x88, 0x01) && writeReg(0x87, 0x04) && 
  writeReg(0x44, 0x00) && writeReg(0x45, 0x00) && writeReg(0x89, 0x01);
}
bool disableEEPROMAccess(){return writeReg(0x89, 0x00) && writeReg(0x87, 0x00) &&  
  writeReg(0x44, 0xAA) && writeReg(0x45, 0x06);
} 

//Para escribir en los registros es necesario convertir su contenido a binario
uint16_t CELLmVtoHEX(uint16_t mV){
  float hex = ((float)mV * 3.0 * 4095.0) / (1000.0 * 8.0 * 1.8);
  return (uint16_t)hex;
}
uint16_t CELLHEXtomV(uint16_t code){
  float mV = ((float)code * 1.8 * 8.0 * 1000.0) / (4095.0 * 3.0);
  return (uint16_t)mV;
}

uint16_t THmVtoHEX(uint16_t mV){
  float hex = ((float)mV * 4095.0) / (1000.0 * 1.8);
  return (uint16_t)hex;
}
uint16_t THHEXtomV(uint16_t code){
  float mV = ((float)code * 1.8 * 1000.0) / 4095.0;
  return (uint16_t)mV;
}

//P35-36, P38 DATASHEET ISL94202
//Contiene el umbral, la recuperación, el bloqueo de sobretensión
//Contiene además el pulso de detección de carga (charge) (CPWD)
bool setOV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t CPWD){ 
 // if (recov>thres || thres>lock || CPWD>15) return false;
  uint16_t code[3];
  code[0]=(uint16_t)CPWD<<12 | CELLmVtoHEX(thres);
  code[1]=CELLmVtoHEX(recov);
  code[2]=CELLmVtoHEX(lock);
  return writeEEPROM(OVThresADDR, code[0]) && writeEEPROM(OVRecovADDR, code[1]) && writeEEPROM(OVLockADDR, code[2]);
}

//P37-39 DATASHEET ISL94202
//Contiene el umbral, la recuperación, el bloqueo de subtensión
//Contiene además el pulso de detección de carga (load) (LDPW)
bool setUV(uint16_t thres, uint16_t recov, uint16_t lock, uint8_t LDPW){ 
 // if (recov<thres || thres<lock || LDPW>15) return false;
  uint16_t code[3];
  code[0]=(uint16_t)LDPW<<12 | CELLmVtoHEX(thres);
  code[1]=CELLmVtoHEX(recov);
  code[2]=CELLmVtoHEX(lock);
  return writeEEPROM(UVThresADDR, code[0]) && writeEEPROM(UVRecovADDR, code[1]) && writeEEPROM(UVLockADDR, code[2]);
}

//P40 DATASHEET ISL94202
//Define final de la carga
bool setEOCThres(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(EOCThresADDR, code);
}

//P41-42 DATASHEET ISL94202
//Define el tiempo en el que debe presentar OV o UV para saltar la protección
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor
bool setOVUVTimer(uint8_t TU, uint8_t Timing){
  uint16_t code = TU << 10 | Timing ;
  return writeEEPROM(OVTimADDR, code) && writeEEPROM(UVTimADDR, code);
}

//P40 DATASHEET ISL94202
//Define el voltaje bajo de la celda
bool setLCVL(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(LVCLADDR, code);
}

//OWTimer

//P43 DATASHEET ISL94202
//Define cuanta sobrecorriente en descarga dispara la protección y cuanto tiempo debe estar
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor (tiempo y corriente)
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

//P46 DATASHEET ISL94202
//Define cuanta sobrecorriente en carga dispara la protección y cuanto tiempo debe estar
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor (tiempo y corriente)
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

//P48 DATASHEET ISL94202
//Define cuanta sobrecorriente en carga dispara la protección y cuanto tiempo debe estar
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor (tiempo y corriente)
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

//P49 DATASHEET ISL94202
//Deshabilita el equilibrio de las celdas si están por debajo de este umbral
bool setCBMin(uint16_t mV){
//  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMinADDR, code);
}

//P50 DATASHEET ISL94202
//Deshabilita el equilibrio de las celdas si están por encima de este umbral
bool setCBMax(uint16_t mV){
 // if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMaxADDR, code);
}

//P50 DATASHEET ISL94202
//Mínima diferencia entre celdas para equilibrarlas
bool setCBMinErr(uint16_t mV){
  if (mV<10 ||mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMinDifADDR, code);
}

//P51 DATASHEET ISL94202
//Máxima diferencia entre celdas para equilibrarlas
bool setCBMaxErr(uint16_t mV){
  if (mV>CELLHEXtomV(4095)) return false;
  uint16_t code = CELLmVtoHEX(mV);
  return writeEEPROM(CBMaxDifADDR, code);
}

//P53 DATASHEET ISL94202
//Tiempo que permanece apagado el equilibrado en cada ciclo
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor
bool setCBOnTime(uint16_t ms, uint8_t TU){ 
  if (ms>1023 || TU > 0b11) return false;
  uint16_t code = TU<<10 | ms;
  return writeEEPROM(CBOnTimADDR, code);
}

//P53 DATASHEET ISL94202
//Tiempo que permanece encendido el equilibrado en cada ciclo
//Se programa con las unidades (Definidas en ISL94202Config.h) y el valor
bool setCBOffTime(uint16_t ms, uint8_t TU){ 
 // if (ms>1023 || TU > 0b11) return false;
  uint16_t code = TU<<10 | ms;
  return writeEEPROM(CBOffTimADDR, code);
}

//P53 DATASHEET ISL94202
//Deshabilita el equilibrado en temperaturas bajas. Presenta umbral y recuperación
bool setCBUT(uint16_t thres, uint16_t recov){ 
//  if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CBUTThresADDR, code[0]) && writeEEPROM(CBUTRecovADDR, code[1]);
}

//P55 DATASHEET ISL94202
//Deshabilita el equilibrado en temperaturas altas. Presenta umbral y recuperación
bool setCBOT(uint16_t thres, uint16_t recov){ 
 // if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CBOTThresADDR, code[0]) && writeEEPROM(CBOTRecovADDR, code[1]);
}

//P57 DATASHEET ISL94202
//Define la sobretemperatura en la carga con su umbral y su recuperación
bool setCOT(uint16_t thres, uint16_t recov){ 
 // if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(COTThresADDR, code[0]) && writeEEPROM(COTRecovADDR, code[1]);
}

//P58 DATASHEET ISL94202
//Define la subtemperatura en la carga con su umbral y su recuperación
bool setCUT(uint16_t thres, uint16_t recov){ 
 // if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(CUTThresADDR, code[0]) && writeEEPROM(CUTRecovADDR, code[1]);
}

//P59 DATASHEET ISL94202
//Define la subtemperatura en la descarga con su umbral y su recuperación
bool setDCOT(uint16_t thres, uint16_t recov){ 
 // if (recov>thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(DCOTThresADDR, code[0]) && writeEEPROM(DCOTRecovADDR, code[1]);
}

//P61 DATASHEET ISL94202
//Define la subtemperatura en la descarga con su umbral y su recuperación
bool setDCUT(uint16_t thres, uint16_t recov){ 
//  if (recov<thres) return false;
  uint16_t code[2];
  code[0]=THmVtoHEX(thres);
  code[1]=THmVtoHEX(recov);
  return writeEEPROM(DCUTThresADDR, code[0]) && writeEEPROM(DCUTRecovADDR, code[1]);
}

//P67 DATASHEET ISL94202
//Define el número de celdas a leer
//Se codifican igual a las entradas que leen su tensión (ver P150 DATASHEET ISL94202)
bool setCellCount(uint16_t n){
if (!(n == 3 || n == 4 || n == 7 || n == 8)) return false;
  uint8_t code;
  switch(n){
    case 3: default: code = 0x83; break;  
    case 4: code = 0xC3; break;
    case 7: code = 0xEF; break;
    case 8: code = 0xFF; break;
  }
  return writeEEPROM(CellCountADDR, code, false);
}

//P67 DATASHEET ISL94202
//Contiene configuraciones en bits que habilitan/deshabilitan operaciones o controles.
bool setUp0Reg(bool PSD, bool XT2M, bool TGAIN, bool PCFETE, bool DOWD, bool OWPSD){
  uint8_t code = PSD << 7 | XT2M << 5| TGAIN << 4 | PCFETE << 2 | DOWD << 1 | OWPSD;
return writeEEPROM(SetUp0, code, false);
}

//P68 DATASHEET ISL94202
//Contiene configuraciones en bits que habilitan/deshabilitan operaciones o controles.
bool setUp1Reg(bool CBDD, bool CBDC, bool DFODUV, bool CFODOV, bool UVLOPD, bool CB_EOC){
  uint8_t code = CBDD << 7 | CBDC << 6| DFODUV << 5 | CFODOV << 4 | UVLOPD << 3 | CB_EOC;
return writeEEPROM(SetUp1, code, false);
}

//Una condición para escribir en la EEPROM es la ausencia de fallos
bool printFaults() {
    uint8_t stat0,stat1;
    readReg(0x80,stat0);
    readReg(0x81,stat1);

    if (stat0 == 0 && stat1 == 0)  return false;
    Serial.println("--- FALLOS DETECTADOS ---");
    // Registro 0x80 (STAT0) P71 DATASHEET ISL94202
    if (stat0 & (1 << 7)) Serial.println("[0x80.7] Subtemperatura en carga (CUTF)");
    if (stat0 & (1 << 6)) Serial.println("[0x80.6] Sobretemperatura en carga (COTF)");
    if (stat0 & (1 << 5)) Serial.println("[0x80.5] Subtemperarura en la descarga (DUTF)");
    if (stat0 & (1 << 4)) Serial.println("[0x80.4] Sobretemperarura en la descarga (DOTF)");
    if (stat0 & (1 << 3)) Serial.println("[0x80.3] Subtensión interna de bloqueo (UVLOF)");
    if (stat0 & (1 << 2)) Serial.println("[0x80.2] Subtensión interna (UVF)");
    if (stat0 & (1 << 1)) Serial.println("[0x80.1] Sobretensión interna de bloqueo (OVLOF)");
    if (stat0 & (1 << 0)) Serial.println("[0x80.0] Sobretensión interna (OVF)");

    // Registro 0x81 (STAT1) P73 DATASHEET ISL94202
    if (stat1 & (1 << 7)) Serial.println("[0x81.7] Voltaje de final de carga (VEOC)");
    //[81.6] RSV
    if (stat1 & (1 << 5)) Serial.println("[0x81.5] Circuito abierto (OWF)");
    if (stat1 & (1 << 4)) Serial.println("[0x81.4] Fallo de celdas (CELLF)");
    if (stat1 & (1 << 3)) Serial.println("[0x81.3] Cortocircuito en descarga (DSCF)");
    if (stat1 & (1 << 2)) Serial.println("[0x81.2] Sobrecorriente en descarga (DOCF)");
    if (stat1 & (1 << 1)) Serial.println("[0x81.1] Sobrecorriente en carga (COCF)");
    if (stat1 & (1 << 0)) Serial.println("[0x81.0] Sobretemperatura interna (IOTF)");
    Serial.println("-------------------------");
    return true;
}

bool checkEEPROMWriteReady() {
    uint8_t EEEN;
    bool check = true;
    readReg(0x89, EEEN); //P85 DATASHEET ISL94202
    if (!(EEEN & (1 << 0))) {
      Serial.print("Estas en modo: ");
        switch( getMode()){
          case 0: Serial.print("NORMAL. "); break;
          case 1: Serial.print("IDLE. "); break;
          case 2: Serial.print("DOZE. "); break;
          case 3: Serial.print("SLEEP. "); break;
          case 4: Serial.print("POWERDOWN. "); break;
          case 5: Serial.print("UNKNOWN. "); break;
        }
        Serial.println("Deberías estar en modo: NORMAL");
        Serial.println("[ERROR] Bit EEEN (0x89.0) está desactivado. Habilita la escritura en RAM antes de intentar el grabado.");
        check = false;
    }     

    //P75 DATASHEET ISL94202
    //Verificar presencia de carga o cargador en STAT0 (0x82)
    uint8_t stat2;
    readReg(0x82, stat2);
    if (stat2 & (1 << 0) || stat2 & (1 << 1)) { // Bit LD_PRSNT (82.0) y CHG_PRSNT (82.1)
        Serial.println("[ERROR] Cargador/Carga detectada (0x82). Desconecta el paquete antes de grabar.");
        check = false;
    }
    return check;
}

BMSMode getMode() {
  uint8_t stat3;
  if (!readReg(0x83, stat3)) return MODE_POWERDOWN; // sin ACK -> no responde -> Powerdown
  if (stat3 & (1 << 6)) return MODE_SLEEP;
  if (stat3 & (1 << 5)) return MODE_DOZE;
  if (stat3 & (1 << 4)) return MODE_IDLE;
  return MODE_NORMAL; // D[6:4]=000 y hubo ACK
}

bool waitForLowPowerState(uint16_t timeout_ms) {
  uint32_t start = millis();
  uint8_t stat2;
  while (millis() - start < timeout_ms) {
    if (readReg(0x82, stat2) && (stat2 & (1 << 6))) return true; // INT_SCAN=1 -> fuera de escaneo
    delayMicroseconds(200);
  }
  return false;
}

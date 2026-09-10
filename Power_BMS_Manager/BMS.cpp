#include "BMS.h"
#include <Wire.h>

//Realiza una lectura de dos bytes, adaptándose a las estructura del ISL94202
//La mayoría de registros se almacenan en 12bits, con dos bytes y 4 bits reservados.
uint16_t BMS::twoByteRead(uint8_t ADDR){
  Wire.beginTransmission(ISLADDR);
  Wire.write(ADDR);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)2);
  if (Wire.available()>=2){
    uint8_t low = Wire.read();
    return (uint16_t)(Wire.read() & (uint8_t)0x0F) << 8 | low; //Los últimos cuatro son reservados
  } else return 0;
}

//Escritura genérica de un byte
bool BMS::oneByteWrite(uint8_t reg, uint8_t data){
  Wire.beginTransmission(ISLADDR);
  Wire.write(reg);
  Wire.write(data);
  if (Wire.endTransmission() != 0) return false;

  return true;
}

//Lectura genérica de un byte
bool BMS::oneByteRead(uint8_t ADDR, uint8_t &data) {
  Wire.beginTransmission(ISLADDR);
  Wire.write(ADDR);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)ISLADDR, (uint8_t)1) != 1) return false;
  data = Wire.read();
  return true;
}

//P90 DATASHEET ISL94202
//Registros 0x90-0x9F: Almacenan las lecturas de voltaje de las celdas
bool BMS::updateCellsVoltages(){
 for (uint8_t i = 0; i < 8; i++) {
   if ((cellSelected >> i) & 0x01) { //Solo lee las que están conectadas. (codificadas según P67)
      uint16_t raw=twoByteRead(0x90+2*i);
      if (raw==0) return false;
      cellVoltage[i]=(raw*1.8f*8.0f)/(4095.0f*3.0f);
   }
  }
  return true;
}

//P90 DATASHEET ISL94202
//Registros 0x8E-0x8F: Almacenan las lecturas de intensidad medido por la resistencia shunt
void BMS::updatePackCurrent(){
  uint16_t raw=twoByteRead(0x8E);
  packCurrent=(raw*1.8f)/(4095.0f*currentGain*rSense);
}

//P92 DATASHEET ISL94202
//Registros 0xA6-0xA7: Almacenan las lecturas de tensión de la batería
bool BMS::updatePackVoltage(){
  uint16_t raw=twoByteRead(0xA6);
  if (raw==0) return false;
  packVoltage=(raw*1.8f*32.0f)/(4095.0f);
  return true;
}

//P71 DATASHEET ISL94202
//Registros 0x8E-0x8F: Almacenan las el valor de los registros de estado
bool BMS::updateStatus(){
  uint16_t raw=twoByteRead(0x80);
    status[0]= (uint8_t)(raw & 0x00FF);
    status[1]= (uint8_t)(raw >> 8);
  raw=twoByteRead(0x82);
    status[2]= (uint8_t)(raw & 0x00FF);
    status[3]= (uint8_t)(raw >> 8);
  return true;
}

//P91 DATASHEET ISL94202
//Registros 0xA0-0xA1: Almacenan las lecturas de temperatura medido por el sensor interno
//Registros 0xA2-0xA5: Almacenan las lecturas de temperatura medido por los termistores
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

// P96 DATASHEET ISL94202.
// Interpolación desde la Tabla 70. 
float BMS::xtVoltageToTemp(float voltage){
    const float V[] = {0.7396f, 0.6112f, 0.4537f, 0.2887f, 0.1495f};
    const float T[] = {-40.0f, 0.0f, 25.0f, 50.0f, 80.0f};

    voltage /= 2.0f; //Gain=2. P96 RENESAS ISL94202 DATASHEET. Figure 39.
    if (voltage > V[0] || voltage < V[4]) return 0; //Comprueba la ventana posible
    for (uint8_t i = 0; i < 4; i++) {
        if (voltage <= V[i] && voltage >= V[i + 1]) {
            return T[i]+(voltage - V[i])*((T[i + 1] - T[i])/(V[i + 1] - V[i]));  // Interpolación lineal
        }
    }
    return 0;
}

// P79-80 DATASHEET ISL94202.
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

// P67 DATASHEET ISL94202.
// Establece el número de celdas a medir
bool BMS::setCellCount(uint16_t n){
if (!(n == 3 || n == 4 || n == 6 || n == 7 || n == 8)) return false;
  uint8_t code;
  switch(n){
    case 3: default: code = 0x83; break;  
    case 4: code = 0xC3; break;
    case 6: code = 0xDF; break; 
    case 7: code = 0xEF; break;
    case 8: code = 0xFF; break;
  }
    Wire.beginTransmission(ISLADDR);
    Wire.write(0x49); // Registro de Cell Select
    Wire.write(code);
    if (Wire.endTransmission() == 0){
      cellSelected=code; //Actualiza el atributo de la clase
      return true;
    }
    return false;
}

// P78, P82 DATASHEET ISL94202.
// Permite el equilibrio forzado por MCU. Utiliza un proceso de varios registros
// 1. Se activa el equilibrio por MCU. Registro 0x87-D[5]
// 2. Se seleccionan las celdas a balancear. Registro 0x84
// 3. Se balancean las celdas seleccionadas. Registro 0x87-D[0]. (Solo si se ha realizado el paso 1)
bool BMS::balanceCells(uint8_t cells, int ms){
  uint8_t mode = getMode();
  bool check=true;
  if(mode!=0){
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
    Serial.println(" ");
    check = false; 
  }
  if(printStatus()) check=false;

  uint8_t cb = 1 << (cells - 1);

  // Activar equilibrio por MCU
  uint8_t control2 =0; //P82 DATASHEET
  if (!oneByteRead(0x87, control2)) {
    Serial.println("Fallo en la lectura del registro 0x87");
    Serial.println(" ");
    check = false;
  }

  control2 |= (1 << 5);
  if (!oneByteWrite(0x87, control2)) {
    Serial.println("Fallo en la escritura del registro 0x87");
    Serial.println(" ");
    check = false;
  }

  // Seleccionar las celdas a balancear
  if (!oneByteWrite(0x84, cb)) {
    Serial.println("Fallo en la escritura del registro 0x84");
    Serial.println(" ");
    check = false;
  } //P79 DATASHEET

  // CBAL_ON = 1 -> activar balanceo
  control2 |= (1 << 0);
  if (!oneByteWrite(0x87, control2)) {
    Serial.println("Fallo en la escritura del registro 0x87");
    Serial.println(" ");
    check = false;
  }

  // Mantener el balanceo durante ms
  delay(ms);

  // CBAL_ON = 0 -> detener balanceo
  control2 &= ~(1 << 0);
  if (!oneByteWrite(0x87, control2)) {
    Serial.println("Fallo en la escritura del registro 0x87");
    Serial.println(" ");
    check = false;
  } //P82 DATASHEET

  if (!oneByteWrite(0x84, 0x00)){
    Serial.println("Fallo en la escritura del registro 0x84");
    Serial.println(" ");
    check = false;
    }

  // Volver al control automático
  control2 &= ~(1 << 5);
  if (!oneByteWrite(0x87, control2)) {
    Serial.println("Fallo en la escritura del registro 0x87");
    Serial.println(" ");
    check = false;
  }
  return check;
}

//Devuelve modo de operación
BMSMode BMS::getMode() {
  uint8_t stat3;
  if (!oneByteRead(0x83, stat3)) return MODE_POWERDOWN; // sin ACK -> no responde -> Powerdown
  if (stat3 & (1 << 6)) return MODE_SLEEP;
  if (stat3 & (1 << 5)) return MODE_DOZE;
  if (stat3 & (1 << 4)) return MODE_IDLE;
  return MODE_NORMAL; // D[6:4]=000 y hubo ACK
}

bool BMS::printStatus() {
    uint8_t stat0,stat1;
    oneByteRead(0x80,stat0);
    oneByteRead(0x81,stat1);

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

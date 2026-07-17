#include<Arduino.h>
#include "TPSConfig.h"
#include "TPS26750.h"

// MSB: Most Significant Bit
// LSB: Least Significant Bit
//------------------------------------ PDO: Power Data Object----------------------------------------//
/* Información sobre el contrato de energía que realiza el USB power delivery
   En el tps se organizan en: 
       - 41-32: bits 29-20 del primero contrato PDO.
       - 31-0 : contrato PDO actual.
       - 31-30: PDO type: 00b - Fixed Voltage, 01b - Battery, 10b - Variable, 11b - Augmented PDO (APDO) 

   El formato PDO, según https://www.usb.org/document-library/usb-power-delivery : USB_PD:3.2 - P126
    Para Fixed Voltage
       - 29-22: Device Flags: Reservados. No tocar
       - 21-20: Peak Current
       - 19-10: Voltage: unidades de 50mV 
       - 9-0  : Maximum Current: unidades de 10mA (0-5A)
    Para Battery
       - 29-20: Maximum Voltage: unidades de 50mV
       - 19-10: Minimum Voltage: unidades de 50mV
       - 9-0  : Maximum Power: unidades de 250mW
    Para Variable
       - 29-20: Maximum Voltage: unidades de 50mV
       - 19-10: Minimum Voltage: unidades de 50mV
       - 9-0  : Maximum Current: unidades de 10mA (0-5A)
    Para APDO
       - 29-28: APDO type
       - 27,16,7: PPS Power Limited
       - 24-17: Maximum Voltage: unidades de 100mW
       - 15-8  : Minimum Voltage: unidades de 100mV
       - 6-0  : Maximum Current: unidades de 50mA (0-5A)
*/
  bool TPS26750::getContract(){ //P41 technical manual
    Wire.beginTransmission(TPSADDR);
    Wire.write(TPS_ACTIVE_PDO_CONTRACT);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)TPSADDR, (uint8_t)7); //El total del registro son 6 bytes
    if (Wire.available() >= 7) {
        uint8_t length = Wire.read();  // El primer byte devuelto por los chips de TI es siempre la longitud del bloque
        if (length != 6) return false; // Error: el registro 0x34 no tiene el tamaño esperado
        uint8_t b0 = Wire.read(); //PDO activo LSB
        uint8_t b1 = Wire.read(); //...
        uint8_t b2 = Wire.read(); //...
        uint8_t b3 = Wire.read(); //PDO activo MSB
        uint8_t b4 = Wire.read(); //Primero PDO y control
        uint8_t b5 = Wire.read(); //Reservado y control
        
        uint8_t pdo_type = (b3 >> 6) & 0x03;
        uint32_t pdo = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);

        switch(pdo_type){
          case 0: 
            voltage = ((pdo >> 10) & 0x3FF) * 50.0f / 1000.0f; 
            current = (pdo & 0x3FF) * 10.0f / 1000.0f; 
            break;
          case 1:
            maxvoltage = ((pdo >> 20) & 0x3FF) * 50.0f / 1000.0f;
            minvoltage = ((pdo >> 10) & 0x3FF) * 50.0f / 1000.0f;
            maxpower = (pdo & 0x3FF) * 250.0f / 1000.0f; 
            break;
          case 2: 
            maxvoltage = ((pdo >> 20) & 0x3FF) * 50.0f / 1000.0f;
            minvoltage = ((pdo >> 10) & 0x3FF) * 50.0f / 1000.0f;
            current = (pdo & 0x3FF) * 10.0f / 1000.0f;             
          break; 
          case 3: 
            maxvoltage = ((pdo >> 17) & 0xFF) * 100.0f / 1000.0f; 
            minvoltage = ((pdo >> 8) & 0xFF) * 100.0f / 1000.0f;
            current = (pdo & 0x7F) * 50.0f / 1000.0f;               
          break;
          default: return false; break; 
        }
        success = true;
    } else success = false;

    return true;
  }

//--------------------------Registros TPS Block Transfer--------------------------------------------//
/* TPS25750 Host Interface Technical Reference Manual P11
Se observa en la figura 1-3 que el primer byte leido es (byte count), por lo que la lectura del TPS 
se diferencia de la lectura general por I2C y permite lectura de nbytes. 
Siglas de la figura == S: Start, Sr: Repeated Start, Wr: Write, Rd: Read, A: Acknowledge, P: Stop
*/
  bool TPSnBytesRead(uint8_t DEVADDR, uint8_t REGADDR, uint8_t* data, uint8_t n) {
      Wire.beginTransmission(DEVADDR);
      Wire.write(REGADDR);
      Wire.endTransmission(false);
      Wire.requestFrom((uint8_t)DEVADDR, (uint8_t)(n+1));
      if (Wire.available()<1) return false;
        uint8_t actualLen = Wire.read(); // Leemos el byte de longitud devuelto por el chip
        if (actualLen == 0 || actualLen > n) return false; 
        for (uint8_t i = 0; i < actualLen; i++) {
            if (Wire.available()) data[i] = Wire.read();
            else return false;
        }
    return true;
    }

  bool TPS26750::TPSnBytesWrite(uint8_t DEVADDR, uint8_t REGADDR, const uint8_t* data, uint8_t n) {
    Wire.beginTransmission(DEVADDR);
    Wire.write(REGADDR);
    Wire.write(n);
    for (uint8_t i = 0; i < n; i++) {
        Wire.write(data[i]);
    }
    return (Wire.endTransmission() == 0);
  }
//---------------------------------------------------------------------------------------------------//
/* Función de carga de configuración del TPS. Necesita el archivo de configuración para funcionar
El funcionamiento se encuentra detallado en el host manual. Los pasos son los siguientes: 
  - Los cambios se realizan en modo PTCH. Una vez en modo PTCH, el TPS indica cuando esta listo
  - El host envía los datos en uno o varios comandos de i2c. Una vez terminado se indica vía PBMc.
*/
//----------------------------------Fuente-----------------------------------------------------------//
  /*Registros CMD1/DATA1 y bit ReadyforPatch P12-13.
  Registro MODE P17.
  Formato 4CC tabla 3-1 P43.
  PBMs - Start Patch Burst Download P48.
  PBMc - Patch Burst Download complete P49-51.
  PBMe - End Patch Burst Download P52.
  Flujo completo en P52-57.
  GO2P - Go to Patch Mode P58
  Resumen en P61.*/
//-----------------------------------Siglas---------------------------------------------------------//
/* 4CC - Four Character Code -> Codifican acciones
   PBM - Patch Burst Mode -> Mejora la velocidad al cargar los datos
   CMD - Command -> Se escriben códigos de cuatro carácteres para ejecutar una tarea
*/

  bool TPS26750::sendCommand4CC(const char* command) {
    //P52. Si tiene datos de entrada o salida se debe escribir/leer otros registros
      uint8_t cmdBytes[4];
      memcpy(cmdBytes, command, 4);
      if (!TPSnBytesWrite(TPSADDR, TPS_COMMAND_I2C1, cmdBytes, 4)) return false;

      return waitCmd1Complete(5000); 
  }

  bool TPS26750::waitCmd1Complete(uint32_t timeoutMs) {
    // TRM pág. 52: "repeatedly read the four byte content of CmdX register
    // until it reads 0x00 ... or '!CMD'"
    uint32_t start = millis();
    uint8_t cmd[4];
    while (true) {
        if (!TPSnBytesRead(TPSADDR, TPS_COMMAND_I2C1, cmd, 4)) return false;
        if (memcmp(cmd, "\0\0\0\0", 4) == 0) return true;   // completado con éxito
        if (memcmp(cmd, "!CMD", 4) == 0) return false;      // comando rechazado
        if (millis() - start > timeoutMs) return false;
        delay(2);
    } ;
}

bool TPS26750::waitReadyForPatch(uint32_t timeoutMs) {
    // TRM P52: 1.The device generates an I2C interrupt, INT_EVENT.ReadyForPatch, indicating that it’s ready for patch. The
    //host shall start the patch download process only after receiving this notification from the device.
    //(bit ReadyForPatch, INT_EVENT1 byte 11 / bit1 — ver Tabla 2-6/2-7, pág. 19)
    uint8_t events[11];
    uint32_t start = millis(); //Tiempo desde que comienza el programa (Similar getTick())
    while (millis() - start < timeoutMs) {
        if (!TPSnBytesRead(TPSADDR, TPS_INT_EVENT1, events, 11)) return false;
        if (events[10] & 0x02) { // ReadyForPatch
            uint8_t clear[11] = {0};
            clear[10] = 0x02;
            TPSnBytesWrite(TPSADDR, TPS_INT_CLEAR1, clear, 11);
            return true;
        }
        delay(5);
    }
    Serial.println("Tiempo de espera expirado. Error en la carga de configuración.");
    return false; // timeout esperando ReadyForPatch
}

bool TPS26750::waitForMode(const char* targetMode, uint32_t timeoutMs) {
    uint8_t mode[4];
    uint32_t start = millis();
    do {
        if (!TPSnBytesRead(TPSADDR, TPS_MODE, mode, 4)) return false;
        if (memcmp(mode, targetMode, 4) == 0) return true;
        delay(5);
    } while (millis() - start < timeoutMs);

    Serial.print("No encontró el MODE == ");
    Serial.write((const uint8_t*)targetMode, 4);
    Serial.print(", actual: ");
    Serial.write(mode, 4);
    Serial.println();
    return false;
}

bool TPS26750::loadConfig() {

    // Si MODE está en modo "APP" no hace falta parchear. Si esta en modo PTCH se continua
  if (!waitForMode("APP ", 50)) {   
        if (!waitForMode("PTCH", 2000)) return false; 
  } else return true; // ya esta en modo APP termina. 


  uint32_t patchSize = PATCH_SIZE;
  // El TPS debe mandar la señal "Ready for Patch". 
  if (!waitReadyForPatch(2000)) return false;

  // Se inicia la transacción PBM. P48. Tabla 3-9 "INPUT DATAX".
  // Los parámetros de PBM se deben enviar como input de un comando 4cc. 
  // Estos son: tamaño LE (4B) + dirección esclava temporal + timeout.
  uint8_t pbms_params[6];
  //Bytes 1-4: Low Region Binary bundle size in bytes (0-3)
  pbms_params[0] = (patchSize & 0xFF);
  pbms_params[1] = ((patchSize >> 8) & 0xFF);
  pbms_params[2] = ((patchSize >> 16) & 0xFF);
  pbms_params[3] = ((patchSize >> 24) & 0xFF);
  //Byte 5: i2c adress for downloading patch
  pbms_params[4] = PATCH_I2C_ADDR; 
  //Byte 6: Burst mode timeout
  pbms_params[5] = 0x32;           
  if (!TPSnBytesWrite(TPSADDR, TPS_DATA1, pbms_params, 6)) return false;
  if (!sendCommand4CC("PBMs")) return false;

  // Se puede leer el resultado del patch directamente del directorio DATA. Solo tiene 1 byte
  // Tabla 3-9 "OUTPUT DATAX".
  uint8_t pbmsStatus;
  if (!TPSnBytesRead(TPSADDR, TPS_DATA1, &pbmsStatus, 1)) return false;
  if (pbmsStatus != PBMS_SUCCESS) {
      sendCommand4CC("PBMe");
      return false;
  }

  // Se transmite el firmware en bloques desde Flash (PROGMEM).
  uint32_t bytesSent = 0;
  const uint8_t blockSize = 64;
  uint8_t chunkBuffer[blockSize];

  while (bytesSent < patchSize) {
      uint8_t currentChunkSize = min((uint32_t)blockSize, patchSize - bytesSent);
      memcpy_P(chunkBuffer, &tps_patch_data[bytesSent], currentChunkSize);

      Wire.beginTransmission(PATCH_I2C_ADDR);
      Wire.write(chunkBuffer, currentChunkSize);
      if (Wire.endTransmission() != 0) {
          sendCommand4CC("PBMe");
          return false;
      }
      bytesSent += currentChunkSize;
      delayMicroseconds(500);
  }

  // Se indica que se ha terminado de cargar los datos. P49-51. Tabla 3-10.
  if (!sendCommand4CC("PBMc")) return false;

  // Se comprueba el resultado de PBMc: DevicePatchCompleteStatus (Byte3) y
  // AppConfigPatchCompleteStatus (Byte4). P50. Tabla 3-10.
  uint8_t pbmcOut[4];
  if (TPSnBytesRead(TPSADDR, TPS_DATA1, pbmcOut, 4)) {
      uint8_t devicePatchStatus = pbmcOut[2]; // 0x00 = éxito
      uint8_t appConfigStatus   = pbmcOut[3]; // 0x00 = éxito
      if (devicePatchStatus != 0x00 || appConfigStatus != 0x00) {
          Serial.print("Error de Patch status: "); Serial.println(devicePatchStatus);
          Serial.print("Error de Comfig status: "); Serial.println(appConfigStatus);
          // Diagnóstico: 0x41 mismatch cabecera, 0x42 ROM incompatible,
          // 0x43 mismatch checksum código, 0x44/0x45 patch nulo/erróneo (Tabla 3-10)
          return false;
      }
  }
  // El TPS debe cambiar a modo "APP".
  return waitForMode("APP ", 200);
}
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
    Wire.requestFrom((uint8_t)TPSADDR, (uint8_t)6); //El total del registro son 6 bytes
    if (Wire.available() >= 6) {
        uint8_t length = Wire.read();  // El primer byte devuelto por los chips de TI es siempre la longitud del bloque
        if (length != 5) return false; // Error: el registro 0x34 no tiene el tamaño esperado
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

  bool TPS26750::sendCommand4CC(const char* command) {
      uint8_t cmdBytes[4];
      memcpy(cmdBytes, command, 4);

      //Escribimos el comando de 4 letras en el registro de comandos (0x08)
      if (!TPSnBytesWrite(TPSADDR, TPS_COMMAND_I2C1, cmdBytes, 4)) return false;

      //El comando se ha ejecutado cuando el registro vuelve a 0
      uint8_t checkCmd[4];
      uint32_t startTime = millis();
      while (millis() - startTime < 1000) { // Timeout de seguridad de 1 segundo
          delay(10);
          if (TPSnBytesRead(TPSADDR, TPS_COMMAND_I2C1, checkCmd, 4)) {
              if (checkCmd[0] == 0 && checkCmd[1] == 0 && checkCmd[2] == 0 && checkCmd[3] == 0) {
                  return true; // Comando procesado exitosamente por el chip
              }
          }
      }
      return false; // Error por Timeout
  }
//---------------------------------------------------------------------------------------------------//
/* Función de carga de configuración del TPS. Necesita el archivo de configuración para funcionar
taltal
*/
  bool TPS26750::loadConfig() {
      uint8_t mode[4];
      
      // 1. Leer el modo actual. Si ya está en modo "APP ", no hace falta parchear.
      if (!TPSnBytesRead(TPSADDR, TPS_MODE, mode, 4)) return false;
      if (memcmp(mode, "APP ", 4) == 0) return true; 
      
      // Si estás usando la Opción A, asegúrate de haber mapeado tps_patch_data y PATCH_SIZE
      uint32_t patchSize = PATCH_SIZE; 
      
      // 2. Preparar parámetros para iniciar la descarga del parche (PBMs)
      uint8_t pbms_params[6];
      pbms_params[0] = (patchSize & 0xFF);
      pbms_params[1] = ((patchSize >> 8) & 0xFF);
      pbms_params[2] = ((patchSize >> 16) & 0xFF);
      pbms_params[3] = ((patchSize >> 24) & 0xFF);
      pbms_params[4] = PATCH_I2C_ADDR; // Dirección virtual temporal (ej. 0x21)
      pbms_params[5] = 0x32;           // Timeout estándar
      
      // Escribir parámetros en DATA1 y ejecutar comando "PBMs"
      if (!TPSnBytesWrite(TPSADDR, TPS_DATA1, pbms_params, 6)) return false;
      if (!sendCommand4CC("PBMs")) return false; // Iniciar secuencia de parche

      delay(10); // Pequeña pausa para que el TPS procese la transición a modo descarga

      // 3. Transmisión del firmware en bloques desde la Flash (PROGMEM)
      uint32_t bytesSent = 0;
      const uint8_t blockSize = 64; 
      uint8_t chunkBuffer[blockSize];

      while (bytesSent < patchSize) {
          uint8_t currentChunkSize = min((uint32_t)blockSize, patchSize - bytesSent);
          
          // Copiar de Flash a RAM local en la Teensy
          memcpy_P(chunkBuffer, &tps_patch_data[bytesSent], currentChunkSize);
          
          // Enviar bloque a la dirección virtual de descarga
          Wire.beginTransmission(PATCH_I2C_ADDR);
          Wire.write(chunkBuffer, currentChunkSize);
          if (Wire.endTransmission() != 0) {
              sendCommand4CC("PBMe"); // Error: Forzar salida de modo descarga si falla
              return false;
          }
          bytesSent += currentChunkSize;
          delayMicroseconds(500); // Pequeño respiro para el bus
      }

      // 4. Finalizar y verificar la integridad del parche (PBMc)
      if (!sendCommand4CC("PBMc")) return false; 

      // Esperar a que el chip procese el firmware, verifique el CRC y se reinicie en modo APP
      delay(100); 
      
      // 5. Confirmación final de que el modo cambió con éxito a "APP "
      if (TPSnBytesRead(TPSADDR, TPS_MODE, mode, 4)) {
          return (memcmp(mode, "APP ", 4) == 0);
      }
      return false;
  }
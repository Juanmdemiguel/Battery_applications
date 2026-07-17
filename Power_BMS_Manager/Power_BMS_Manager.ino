#include "BMS.h"
#include <Wire.h>

BMS manager;

void setup() {
  Serial.begin(115200); 
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.println("Iniciando sistema BMS...");
  Wire.begin(); //Init as master
  Wire.setWireTimeout(1000, true);
  Wire.setClock(100000);
  bool i2c_ok = false;
  Wire.beginTransmission(ISLADDR);

   uint8_t err=Wire.endTransmission();
  if (err==0) {
      i2c_ok = true;
      Serial.println("BMS detectado correctamente.");
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
  manager.setCellCount(4) ? Serial.println("Número de celdas establecido en 4.") : Serial.println("Error I2C: Fallo en el cambio del número de celdas.") ;
}

void loop() {
  if (!manager.updatePackVoltage()) Serial.println("Error I2C: Fallo en lectura de voltaje del pack.");
  if (!manager.updateCellsVoltages()) Serial.println("Error I2C: Fallo en lectura de voltaje de las celdas.");
  if (!manager.updateTemp()) Serial.println("Error I2C: Fallo en lectura de temperatura.");
  manager.updatePackCurrent();

  // Imprimimos en el Monitor Serie
  Serial.println("--- DATOS BMS ---");
  
  Serial.print("Voltaje Pack: ");
  Serial.print(manager.getPackVoltage());
  Serial.println(" V");

  Serial.print("Corriente: ");
  Serial.print(manager.getPackCurrent());
  Serial.println(" A");

  for(int i = 0; i < 8; i++) {
    Serial.print("Celda "); Serial.print(i+1); Serial.print(": ");
    Serial.print(manager.getCellVoltage(i));
    Serial.println(" V");
  }

  Serial.println("-----------------");
  
  delay(5000); 
}


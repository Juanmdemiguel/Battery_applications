#include "BQ25756.h" 
#include "TPS26750.h"

BQ25756 charger;

void setup() {
  Serial.begin(115200); 
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  Serial.println("Iniciando cargador...");
  Wire.begin(); //Inicio como maestro
  Wire.setWireTimeout(1000, true);
  Wire.setClock(100000); //100kHz para TPS26750 + BQ25756
  bool i2c_ok = false;
  Wire.beginTransmission(BQADDR);

   uint8_t err=Wire.endTransmission();
  if (err==0) {
      i2c_ok = true;
      Serial.println("Cargador detectado correctamente.");
    } else {
      Serial.print("Código de error: "); Serial.println(err);
      Serial.println("Error I2C: Cargador no encontrado.");
      while (1){ // Bloquea el programa si no hay cargador con led de error
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
      } 
  }

//Se limita la corriente por seguridad. Fuera de fase de pruebas se puede comentar
  charger.setChargeCurrentLimit(2000); // Empezamos limitando a 2A para pruebas seguras
  charger.setInputCurrentLimit(5500);  // Límite de corriente de entrada a 5.5A

//Se habilita la carga
  charger.enableCharge();
  Serial.println("BQ25756 configurado con éxito. Iniciando bucle de monitorización...");
}

void loop() {
 charger.WD_control(); 

//Estado de la carga
  double vbat = charger.getBatteryVoltage(); // Devuelve el voltaje en mV
  double ibat = charger.getBatteryCurrent(); // Devuelve la corriente de carga en mA
      
    Serial.print("Charge status: ");
    switch (charger.getChargeStatus()) {
        case 0: Serial.println("No carga");                break;
        case 1: Serial.println("Carga de goteo");          break;
        case 2: Serial.println("Precarga");                break;
        case 3: Serial.println("Carga rápida (CC)");       break;
        case 4: Serial.println("Carga de reducción (CV)"); break;
        case 6: Serial.println("Charge temporizada");      break;
        case 7: Serial.println("Carga completa");          break;
        default: Serial.println("Descnocido");             break;
    }

  Serial.print("Voltaje Batería: ");
  Serial.print(vbat / 1000.0);
  Serial.print(" V | Corriente de Carga: ");
  Serial.print(ibat);
  Serial.println(" mA");

  delay(1000); // Lectura cada segundo
}
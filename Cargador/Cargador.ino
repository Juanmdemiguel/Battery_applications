#include "Cargador.h" 

Cargador charger;
unsigned long periodo = 0;

void setup() {
  Serial.begin(115200); 
  //Declaración de GPIOS
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_CHEM, INPUT_PULLUP); 
  pinMode(PIN_C0, INPUT_PULLUP); 
  pinMode(PIN_C1, INPUT_PULLUP); 
  pinMode(PIN_C2, INPUT_PULLUP); 
  pinMode(PIN_3A, INPUT_PULLUP); 
  pinMode(PIN_6A, INPUT_PULLUP); 
  pinMode(PIN_9A, INPUT_PULLUP); 
  pinMode(PIN_12A, INPUT_PULLUP); 

  pinMode(FB_A, OUTPUT); 
  pinMode(FB_B, OUTPUT); 
  pinMode(FB_C, OUTPUT); 

  delay(1000);
  Serial.println("Iniciando cargador...");
  Wire.begin(); //teensy: 18 SDA0, 19 SCL0
  Wire.setClock(100000); //100kHz para TPS26750 + BQ25756
  periodo = millis(); 

  if(charger.enableBQ() && charger.enableTPS()){
    charger.updateConfig();
    charger.printConfig();
  }
}

void loop() {
 if(millis()-periodo > 15000){
    charger.PDManagement();
    periodo = millis();
 }
}
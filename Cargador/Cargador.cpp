#include "Cargador.h"

bool Cargador::enableBQ(){
  Wire.beginTransmission(BQADDR); //Conexión I2C
  uint8_t err=Wire.endTransmission();
  if (err==0) {
      Serial.println("Cargador detectado correctamente.");
    } else {
      Serial.print("Código de error: "); Serial.println(err);
      Serial.println("Error I2C: Cargador no encontrado.");
      return false;
    }
  //Se limita la corriente por seguridad. Fuera de fase de pruebas se puede comentar
  if(!contCarga.setChargeCurrentLimit(2000)) return false; // Empezamos limitando a 2A para pruebas seguras
  if(!contCarga.setInputCurrentLimit(5500)) return false;  // Límite de corriente de entrada a 5.5A
  //Se habilita la carga
  if(!contCarga.enableCharge()) return false;
  //Se completa la configuración
  Serial.println("BQ25756 configurado con éxito. Iniciando bucle de monitorización...");
  ChargeEnable = true;
  return true;
}

bool Cargador::enableTPS(){
  Wire.beginTransmission(TPSADDR); //Conexión I2C
  uint8_t err=Wire.endTransmission();
  if (err==0) {
      Serial.println("Controlador USB detectado correctamente.");
      if(contUSB.loadConfig()) Serial.println("Configuración cargada correctamente");
      else {
        Serial.println("Fallo al cargar la configuración");
        return false;
      }
    } else {
      Serial.print("Código de error: "); Serial.println(err);
      Serial.println("Error I2C: Controlador USB no encontrado.");
      return false;
    }
  
  USBC = true; 
  return true;
}

void Cargador::printStatus(){
  //Estado de la carga
  if(ChargeEnable){
  contCarga.WD_control(); 
  double vbat = contCarga.getBatteryVoltage(); // Devuelve el voltaje en mV
  double ibat = contCarga.getBatteryCurrent(); // Devuelve la corriente de carga en mA
      
    Serial.print("Estado de la carga: ");
    switch (contCarga.getChargeStatus()) {
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
  }
}

bool Cargador::updateConfig(){
//Ojo la lógica es negada, si lee LOW significa que está activado
//Los pines INPUT van del 2 al 9
bool check = true;
for(uint8_t i=2;i<10;i++) digitalRead(i) == LOW ? Config[i-2]=true : Config[i-2]=false;

if(true){
  check &= updateChargeCurrent();
  check &= updateChargeVoltage();
} else{ //Pone el voltaje de carga al mínimo por seguridad
  digitalWrite(FB_C, false);
  digitalWrite(FB_B, false);
  digitalWrite(FB_A, false);
}
return check;
}

bool Cargador::updateChargeCurrent(){
  uint16_t mA;
  for(uint8_t i=4;i<9;i++){
    if (i==8) mA=15000;
    else if (Config[i]==true){
      mA=3*(i-3)*1000;
      break;
    }
  }
  Serial.print("Carga ajustada a "); 
  Serial.print(mA);
  Serial.println(" mA");

  return contCarga.setChargeCurrentLimit(mA); 
}

bool Cargador::updateChargeVoltage(){
  bool CDin[3]={};
  uint8_t  n_celdas = 0x00;
  for (uint8_t i = 1; i<4; i++) n_celdas = (n_celdas << 1) | Config[i];
  n_celdas += 2; //si sale 9 también codifica 8 celdas

  /*
    3S LiPo     -> 0 0 0
    4S LiPo     -> 0 0 1
    4S LiFePO4  -> 0 1 0
    6S LiFePO4  -> 0 1 1
    5S LiPo     -> 1 0 0
    6S LiPo     -> 1 1 0 
    7S LiFePO4  -> 1 1 0
    8S LiFePO4  -> 1 1 1
    7S LiPo     -> 1 0 1
*/
  if(Config[0]==false){//LiPo

  Serial.print("Batería LiPo de ");

    switch(n_celdas){
      case 2: //si no se selecciona nada se establecen 3 celdas
      case 3: CDin[0] = false; CDin[1] = false; CDin[2] = false; break; // 0 0 0 
      case 4: CDin[0] = false; CDin[1] = false; CDin[2] = true; break; // 0 0 1
      case 5: CDin[0] = true; CDin[1] = false; CDin[2] = false; break; // 1 0 0
      case 6: CDin[0] = true; CDin[1] = true; CDin[2] = false; break; // 1 1 0
      case 9:
      case 8: //Carga como 7s Lipo -> <100%soc
      case 7: CDin[0] = true; CDin[1] = false; CDin[2] = true; break; // 1 0 1
    }
  }else{//LFP

  Serial.print("Batería LFP de ");

    switch(n_celdas){
      case 2: //si no se selecciona nada se establecen 3 celdas
      case 3: CDin[0] = false; CDin[1] = false; CDin[2] = false; break;
      case 4: CDin[0] = false; CDin[1] = true; CDin[2] = false; break; // 0 1 0
      case 5: CDin[0] = false; CDin[1] = false; CDin[2] = true; break; //Carga como 4s Lipo -> <100%soc
      case 6: CDin[0] = false; CDin[1] = true; CDin[2] = true; break; // 0 1 1
      case 7: CDin[0] = true; CDin[1] = true; CDin[2] = false; break; // 1 1 0
      case 9:
      case 8: CDin[0] = true; CDin[1] = true; CDin[2] = true; break; // 1 1 1
    }
  }
  Serial.print(n_celdas);
  Serial.println(" celdas");

  digitalWrite(FB_C, CDin[0]);
  digitalWrite(FB_B, CDin[1]);
  digitalWrite(FB_A, CDin[2]);
  return n_celdas != 0x00;
}

bool Cargador::PDManagement(){
  bool check = true;
  check &= contUSB.getStatus();
  check &= contUSB.getContract();
  return check;
}

void Cargador::printConfig(){
  //Muestra los bits de los interruptores
  for(uint8_t i = 0; i<8; i++){
    Serial.print("Config[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(Config[i]);
  }
    
}
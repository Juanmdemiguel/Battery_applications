#include <Arduino.h>
#include <Wire.h>

//Helper de I2C, para centralizar las funciones y evitar múltiples instancias
class I2C {
  public:
      //La clase solo debe contener funciones estáticas, no debe ser capaz de instanciarse
      I2C() = delete; 

  static uint8_t byteRead(uint8_t DEVADDR, uint8_t REGADDR) {
      Wire.beginTransmission(DEVADDR);
      Wire.write(REGADDR);
      Wire.endTransmission(false);
      Wire.requestFrom((uint8_t)DEVADDR, (uint8_t)1);
      if (Wire.available()>=1){
          return Wire.read();
      } else return 0;
  }

  static uint16_t twoByteRead(uint8_t DEVADDR, uint8_t REGADDR) { //Lectura de bits en formato little endian
      Wire.beginTransmission(DEVADDR);
      Wire.write(REGADDR);
      Wire.endTransmission(false);
      Wire.requestFrom((uint8_t)DEVADDR, (uint8_t)2);
      if (Wire.available()>=2){
          uint8_t low = Wire.read();
          return (uint16_t)(Wire.read() & (uint8_t)0xFF) << 8 | low;
      } else return 0;
  }

  static bool byteWrite(uint8_t DEVADDR, uint8_t REGADDR, uint8_t value) {
      Wire.beginTransmission(DEVADDR);
      Wire.write(REGADDR);                          
      Wire.write(value);     
      return Wire.endTransmission() == 0;
  }

  static bool twoByteWrite(uint8_t DEVADDR, uint8_t REGADDR, uint16_t value) {
      Wire.beginTransmission(DEVADDR);
      Wire.write(REGADDR);                          
      Wire.write((uint8_t)(value & 0xFF)); //Primer byte. Incrementa el puntero con su escritura    
      Wire.write((uint8_t)((value >> 8) & 0xFF)); //Segundo byte
      return Wire.endTransmission() == 0;
  }
};

#include "BQ25756.h"

//-----------------------------------limite de corriente de carga----------------------------------//
uint16_t BQ25756::getChargeCurrentLimit() {
    // Solo contienen información 10:2 -> máscara para quitar 15:11 y desplazamiento
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_CHARGE_CURRENT_LIMIT) & 0x07FC; 
    uint16_t ichg_reg = rawValue >> 2;
    return ichg_reg * 50; //mA
}
bool BQ25756::setChargeCurrentLimit(uint16_t mA) {
    const uint16_t minCurrent = 400;
    if (mA < minCurrent || mA > NOMINAL_MAX_CHARGE_MA) return 0;
    config.chargeCurrentLimit = mA;

   //El paso a bits se realiza dividiendo entre 50. Ese valor se situa en bits 10:2
    uint16_t regValue = (mA / 50) << 2; 

    //Se mantienen los bits reservados y se modifican los que aportan información
    uint16_t reservedBits =  I2C::twoByteRead(BQADDR, BQ25756_CHARGE_CURRENT_LIMIT) & ~0x07FC;
    regValue |= reservedBits;
    return I2C::twoByteWrite(BQADDR, BQ25756_CHARGE_CURRENT_LIMIT, regValue);
}

uint16_t BQ25756::getTerminationCurrentLimit(){
   // Solo contienen información 9:2 -> máscara para quitar 15:10 y desplazamiento
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_TERMINATION_CURRENT_LIMIT) & 0x03FC; 
    uint16_t ichg_reg = rawValue >> 2;
    return ichg_reg * 50; //mA
}
//-----------------------------------limite de corriente de entrada----------------------------------//
uint16_t BQ25756::getInputCurrentLimit() {
 // Solo contienen información 10:2 -> máscara para quitar 15:11 y desplazamiento
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_INPUT_CURRENT_DPM_LIMIT) & 0x07FC; 
    uint16_t i_reg = rawValue >> 2;
    return i_reg * 50; //mA
}
bool BQ25756::setInputCurrentLimit(uint16_t mA){
    const uint16_t minCurrent = 400;
    if (mA < minCurrent || mA > NOMINAL_MAX_INPUT_MA) return 0;
    config.chargeCurrentLimit = mA;

    //El paso a bits se realiza dividiendo entre 50. Ese valor se situa en bits 10:2
    uint16_t regValue = (mA / 50) << 2; 

    //Se mantienen los bits reservados y se modifican los que aportan información
    uint16_t reservedBits =  I2C::twoByteRead(BQADDR, BQ25756_INPUT_CURRENT_DPM_LIMIT) & ~0x07FC;
    regValue |= reservedBits;
    return I2C::twoByteWrite(BQADDR, BQ25756_INPUT_CURRENT_DPM_LIMIT, regValue);
}

//-----------------------------------limite de voltaje de entrada----------------------------------//
uint16_t BQ25756::getInputVoltageLimit() {
 // Solo contienen información 13:2 -> máscara para quitar 15:13 y desplazamiento
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_INPUT_VOLTAGE_DPM_LIMIT) & 0x3FFC; 
    uint16_t v_reg = rawValue >> 2;
    return v_reg * 0.02; //V
}
bool BQ25756::setInputVoltageLimit(uint16_t V){
    const uint16_t minVoltage = 4.2;
    if (V < minVoltage || V > NOMINAL_MAX_OV_V) return 0;
    config.inputVoltageDPMLimit = V;

    //El paso a bits se realiza dividiendo entre 0.02. Ese valor se situa en bits 13:2
    uint16_t regValue = (uint8_t)(V / 0.02) << 2; 

    //Se mantienen los bits reservados y se modifican los que aportan información
    uint16_t reservedBits = I2C::twoByteRead(BQADDR, BQ25756_INPUT_VOLTAGE_DPM_LIMIT) & ~0x3FFC;
    regValue |= reservedBits;
    return I2C::twoByteWrite(BQADDR, BQ25756_INPUT_VOLTAGE_DPM_LIMIT, regValue);
}

//-----------------------------------Control del cargador-------------------------------------------//
  bool BQ25756::enableCharge(){
    uint8_t regValue = 0x01;
    uint8_t curReg = I2C::byteRead(BQADDR, BQ25756_CHARGER_CONTROL) & ~0x01;
    regValue |= curReg;
    config.chargeEnabled = true;
    return I2C::byteWrite(BQADDR, BQ25756_CHARGER_CONTROL, regValue);
  }

  bool BQ25756::disableCharge(){
    uint8_t regValue = 0x00;
    uint8_t curReg = I2C::byteRead(BQADDR, BQ25756_CHARGER_CONTROL) & ~0x01;
    regValue |= curReg;
    config.chargeEnabled = false;
    return I2C::byteWrite(BQADDR, BQ25756_CHARGER_CONTROL, regValue);
  }
  bool BQ25756::WD_control(){
    uint8_t regValue = I2C::byteRead(BQADDR, BQ25756_CHARGER_CONTROL) | (1 << 5); 
    return I2C::byteWrite(BQADDR, BQ25756_CHARGER_CONTROL, regValue);
  }
//-----------------------------------Control del power path-----------------------------------------//
//PFM-> Pulse Frequency Modulation -> Reduce la frecuencia de conmutación en corrientes bajas -> P46 Datasheet
bool BQ25756::PFM_control() {
  uint8_t regValue = I2C::byteRead(BQADDR, BQ25756_POWER_PATH_REVERSE_MODE_CONTROL) & ~0x1E;
  getTerminationCurrentLimit() < 2000 ? regValue &= ~(1 << 5) : regValue |= (1 << 5);
  return I2C::byteWrite(BQADDR, BQ25756_POWER_PATH_REVERSE_MODE_CONTROL, regValue);
}

uint8_t BQ25756::getChargeStatus(){
    uint8_t regVal = I2C::byteRead(BQADDR, BQ25756_CHARGER_STATUS_1);
    return regVal & 0x07;
}
//------------------------------------------Métricas de la carga-------------------------------------//
double BQ25756::getBatteryCurrent() {
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_IBAT_ADC);
    config.batCurrent = rawValue*0.002f;
    return config.batCurrent;
}

double BQ25756::getInputCurrent() {
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_IAC_ADC);
    config.inputCurrent = rawValue*0.0008f;
    return config.inputCurrent;
}

double BQ25756::getBatteryVoltage() {
    uint16_t rawValue = I2C::twoByteRead(BQADDR, BQ25756_VBAT_ADC);
    config.packVoltage = rawValue*0.002f;
    return config.packVoltage;
}
//---------------------------------------------------------------------------------------------------//
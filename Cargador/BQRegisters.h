#pragma once 
#include <Arduino.h>
//P39 DATASHEET BQ25756

// Dirección I2C del BQ25756
#define BQADDR 0x6B

//Parámetros definidos por hardware. Ver diseño electrónico en el TFG
#define SWITCHING_FREQUENCY_KHZ 300   // Frecuencia de conmutación del convertidor (300kHz)
#define NOMINAL_MAX_CHARGE_MA   15000  // Corriente de carga máxima deseada (15A)
#define NOMINAL_MAX_INPUT_MA    10500  // Corriente de entrada máxima desde la fuente (10.5A)
#define NOMINAL_MAX_OV_V 35 // Voltaje máximo deseado (35V)
#define NOMINAL_MAX_UV_V 3.75 // Voltaje mínimo deseado (3.75V)

//Registros incluidos en el diseño 
#define BQ25756_CHARGE_CURRENT_LIMIT                   0x02  // Charge Current Limit Register
#define BQ25756_INPUT_CURRENT_DPM_LIMIT                0x06  // Input Current DPM Limit Register
#define BQ25756_INPUT_VOLTAGE_DPM_LIMIT                0x08  // Input Voltage DPM Limit Register
#define BQ25756_TERMINATION_CURRENT_LIMIT              0x12  // Termination Current Limit Register
#define BQ25756_CHARGER_CONTROL                        0x17  // Charger Control Register
#define BQ25756_POWER_PATH_REVERSE_MODE_CONTROL        0x19  // Power Path & Reverse Mode Control
#define BQ25756_CHARGER_STATUS_1                       0x21  // Charger Status 1
#define BQ25756_IAC_ADC                                0x2D  // IAC ADC Register
#define BQ25756_IBAT_ADC                               0x2F  // IBAT ADC Register
#define BQ25756_VAC_ADC                                0x31  // VAC ADC Register
#define BQ25756_VBAT_ADC                               0x33  // VBAT ADC Register

//------------------------Definiciones de registros. Los incluidos en el diseño se encuentran comentados --------------------//

#define BQ25756_CHARGE_VOLTAGE_LIMIT                   0x00  // Charge Voltage Limit Register. No considerado en el código por su programación Hardware. Incluir para un ajuste más fino
//#define BQ25756_CHARGE_CURRENT_LIMIT                   0x02  // Charge Current Limit Register
//#define BQ25756_INPUT_CURRENT_DPM_LIMIT                0x06  // Input Current DPM Limit Register
//#define BQ25756_INPUT_VOLTAGE_DPM_LIMIT                0x08  // Input Voltage DPM Limit Register
#define BQ25756_REVERSE_MODE_INPUT_CURRENT_LIMIT       0x0A  // Reverse Mode Input Current Limit
#define BQ25756_REVERSE_MODE_INPUT_VOLTAGE_LIMIT       0x0C  // Reverse Mode Input Voltage Limit
#define BQ25756_PRECHARGE_CURRENT_LIMIT                0x10  // Precharge Current Limit Register
//#define BQ25756_TERMINATION_CURRENT_LIMIT              0x12  // Termination Current Limit Register
#define BQ25756_PRECHARGE_TERMINATION_CONTROL          0x14  // Precharge & Termination Control
#define BQ25756_TIMER_CONTROL                          0x15  // Timer Control Register
#define BQ25756_THREE_STAGE_CHARGE_CONTROL             0x16  // Three-Stage Charge Control
//#define BQ25756_CHARGER_CONTROL                        0x17  // Charger Control Register
#define BQ25756_PIN_CONTROL                            0x18  // Pin Control Register
//#define BQ25756_POWER_PATH_REVERSE_MODE_CONTROL        0x19  // Power Path & Reverse Mode Control

#define BQ25756_MPPT_CONTROL                           0x1A  // MPPT Control Register
#define BQ25756_TS_CHARGING_THRESHOLD_CONTROL          0x1B  // TS Charging Threshold Control
#define BQ25756_TS_CHARGING_REGION_BEHAVIOR_CONTROL    0x1C  // TS Charging Region Behavior Control
#define BQ25756_TS_REVERSE_MODE_THRESHOLD_CONTROL      0x1D  // TS Reverse Mode Threshold Control

#define BQ25756_REVERSE_UNDERVOLTAGE_CONTROL           0x1E  // Reverse Undervoltage Control
#define BQ25756_VAC_MAX_POWER_POINT_DETECTED           0x1F  // VAC Max Power Point Detected
//#define BQ25756_CHARGER_STATUS_1                       0x21  // Charger Status 1
#define BQ25756_CHARGER_STATUS_2                       0x22  // Charger Status 2
#define BQ25756_CHARGER_STATUS_3                       0x23  // Charger Status 3
#define BQ25756_FAULT_STATUS                           0x24  // Fault Status Register
#define BQ25756_CHARGER_FLAG_1                         0x25  // Charger Flag 1
#define BQ25756_CHARGER_FLAG_2                         0x26  // Charger Flag 2
#define BQ25756_FAULT_FLAG                             0x27  // Fault Flag Register
#define BQ25756_CHARGER_MASK_1                         0x28  // Charger Mask 1
#define BQ25756_CHARGER_MASK_2                         0x29  // Charger Mask 2
#define BQ25756_FAULT_MASK                             0x2A  // Fault Mask Register
#define BQ25756_ADC_CONTROL                            0x2B  // ADC Control Register
#define BQ25756_ADC_CHANNEL_CONTROL                    0x2C  // ADC Channel Control Register
//#define BQ25756_IAC_ADC                                0x2D  // IAC ADC Register
//#define BQ25756_IBAT_ADC                               0x2F  // IBAT ADC Register
#define BQ25756_VAC_ADC                                0x31  // VAC ADC Register
//#define BQ25756_VBAT_ADC                               0x33  // VBAT ADC Register
#define BQ25756_TS_ADC                                 0x37  // TS ADC Register
#define BQ25756_VFB_ADC                                0x39  // VFB ADC Register
#define BQ25756_GATE_DRIVER_STRENGTH_CONTROL           0x3B  // Gate Driver Strength Control
#define BQ25756_GATE_DRIVER_DEAD_TIME_CONTROL          0x3C  // Gate Driver Dead Time Control
#define BQ25756_PART_INFORMATION                       0x3D  // Part Information Register
#define BQ25756_REVERSE_MODE_BATTERY_DISCHARGE_CURRENT 0x62  // Reverse Mode Battery Discharge Current

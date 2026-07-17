#pragma once 

//Time parameters
#define microseconds 0b00
#define miliseconds 0b01
#define seconds 0b10
#define minutes 0b11

//Number of Cells
#define CELLCOUNT 3

//OV = Over Voltage, UV = Under Volatage
#define OVThreshold 4250 
#define OVRecovery 4149
#define OVLock 4500
#define UVThreshold 2699 
#define UVRecovery 3000
#define UVLock 1800
#define OVUVTimer 1
#define OVUVTimeUnits seconds

//OC = Over Current, SC = Short Circuit
#define EndOfChargeThreshold 4199
#define LowVoltageCharge 2300

#define DischargeOCAmps 32
#define DisChargeOCTime 160
#define DisChargeOCUnits miliseconds


#define ChargeOCAmps 8
#define ChargeOCTime 160
#define ChargeOCUnits miliseconds


#define DischargeSCAmps 128
#define DischargeSCTime 200
#define DischargeSCUnits microseconds


#define OVChargeDetectPulseWidth 1
#define UVChargeDetectPulseWidth 1


//CB = Cell Balancing 
#define CBMin 3100
#define CBMax 4030
#define CBMinErr 18.75
#define CBMaxErr 500
#define CBOnTime 2
#define CBOnTimeUnits seconds
#define CBOffTime 2
#define CBOffTimeUnits seconds

//OT = Over Temperature, UT = Under Temperature
#define CBOTThreshold 530
#define CBOTRecovery 590
#define CBUTThreshold 1344
#define CBUTRecovery 1190

#define ChargeOTThreshold 530
#define ChargeOTRecovery 590
#define ChargeUTThreshold 1344
#define ChargeUTRecovery 1190
#define DisChargeOTThreshold 530
#define DisChargeOTRecovery 590
#define DisChargeUTThreshold 1344
#define DisChargeUTRecovery 1190

//SetUp0 parameters
#define FailShutdown 0
#define TH2Mode 1
#define TempGain 0
#define PreChargeEnable 0
#define OpenWireDisable 0
#define OpenWireShutdown 0

//SetUp1 parameters
#define DischargeCB 0
#define ChargeCB 1
#define DischargeUV 0
#define ChargeOV 0
#define UVLockPowerDown 0
#define EndOfChargeCB 0
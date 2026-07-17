#pragma once 

//I2C adressed from datasheet
#define ISLADDR 0x28 //Internally wire.h does 0x28<<0 || 0x28<<1
//0x28 == 00101000. <<1 == 01010001 == 0x51. <<0 == 01010000 == 0x50 -> Only use if ADDR Pin is tied to Vss.

//OV = Over Voltage, UV = Under Voltage, EOC = End of Charge, DC = Discharge, C = Charge, OC = OverCurrent, 
//SC = ShorCircuit, CB = Cell Balance, LP = Low Power, DS = Deep Sleep, OW = Open Wire.

//Every one of these adressed contains 12bits, thus it needs two bytes to be written. The top 4 bits of each
//second byte are always 0. Registers that fall far from this category are explained separately.  
#define OVThresADDR         0x00 //Define el sobrevoltaje por celda [V]
#define OVRecovADDR         0x02 //Añade histéresis por celda. Define la recuperación de sobrevoltaje y debe ser menor que OV [V]
#define UVThresADDR         0x04 //Eq
#define UVRecovADDR         0x06 //Eq
#define OVLockADDR          0x08 //Si se supera 5 veces bloquea. Debe ser mayor que OV. [V]
#define UVLockADDR          0x0A //Eq
#define EOCThresADDR        0x0C //Define el final de carga del pack. 
#define LVCLADDR            0x0E // Activa el mosfet de precarga cuando las celdas no están casi cargadas
#define OVTimADDR           0x10 // Cuanto tiempo presenta OV para que alte falla
#define UVTimADDR           0x12 // Eq
#define OWTimADDR           0x14 // Ancho de pulso de circuito abierto
#define CBMinADDR           0x1C
#define CBMaxADDR           0x1E
#define CBMinDifADDR        0x20
#define CBMaxDifADDR        0x22
#define CBOnTimADDR         0x24
#define CBOffTimADDR        0x26
#define CBUTThresADDR       0x28
#define CBUTRecovADDR       0x2A
#define CBOTThresADDR       0x2C
#define CBOTRecovADDR       0x2E
#define COTThresADDR        0x30
#define COTRecovADDR        0x32
#define CUTThresADDR        0x34
#define CUTRecovADDR        0x36
#define DCOTThresADDR       0x38
#define DCOTRecovADDR       0x3A
#define DCUTThresADDR       0x3C
#define DCUTRecovADDR       0x3E
//Internal Over and Under Temperature are considered standard and constante for security
//Same can be said about Sleep Level Voltage Threshold, Watchdog and delay timer and Mode timers
#define CellCountADDR       0x49
#define SetUp0              0x4A
#define SetUp1              0x4B

//Every one of these adressed contains 15bits, thus it needs two bytes to be written. Registers contain both 3 bit set up and timer config
#define DCOCTIMADDR         0x16 // Corriente de descarga
#define COCTIMADDR          0x18 // Corriente de carga
#define DCSCTIMADDR         0x1A // Cortocircuito en la descarga
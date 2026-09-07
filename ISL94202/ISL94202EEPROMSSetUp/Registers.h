#pragma once 
//P32  DATASHEET ISL94202

//Dirección I2C. P141 DATASHEET ISL94202
#define ISLADDR 0x28 //Internamente, wire.h hace 0x28<<0 || 0x28<<1
//0x28 == 00101000. <<1 == 01010001 == 0x51. <<0 == 01010000 == 0x50 -> Only use if ADDR Pin is tied to Vss.

//OV = Over Voltage, UV = Under Voltage, EOC = End of Charge, DC = Discharge, C = Charge, OC = OverCurrent, 
//SC = ShorCircuit, CB = Cell Balance, LP = Low Power, DS = Deep Sleep, OW = Open Wire.

//Todos estos registros contienen 12bits, por lo que se deben escribir dos bytes. Los últimos 4 bits
//están reservados. Los registros que no entran en esta categoría se explican por separado.  
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

//Internal Over and Under Temperature se consideran estándar y constantes por seguridad
//Lo mismo se puede decir sobre el nivel de tensión de suspensión, el Watchdog, el temporizador de retardo y los temporizadores 
//de los modos.
#define CellCountADDR       0x49
#define SetUp0              0x4A
#define SetUp1              0x4B

//Registros que contienen 15bits. Necesitan escribirse con dos bytes. Estos registros contienen tanto configuración de 3 bits 
//como configuración del temporizador.
#define DCOCTIMADDR         0x16 // Corriente de descarga
#define COCTIMADDR          0x18 // Corriente de carga
#define DCSCTIMADDR         0x1A // Cortocircuito en la descarga
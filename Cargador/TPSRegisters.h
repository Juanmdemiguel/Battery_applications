#pragma once

#define TPSADDR 0x23 //#4. P32 datasheet
#define PATCH_I2C_ADDR   0x21 // Dirección virtual temporal para el parcheo
// Códigos de retorno de PBMs (Tabla 3-9, pág. 48)
#define PBMS_SUCCESS 0x00

//Registros incluidos en el diseño 
#define TPS_MODE                    0x03  // 4 ASCII chars: 'APP ', 'BOOT', 'PTCH'
#define TPS_COMMAND_I2C1            0x08  // Comando 4CC (CMD1)
#define TPS_DATA1                   0x09  // Datos asociados a CMD1 (hasta 64 bytes)
#define TPS_INT_EVENT1              0x14  // Eventos de interrupción (11 bytes)
#define TPS_INT_CLEAR1              0x18  // Escribir 1 limpia el evento en INT_EVENT1
#define TPS_ACTIVE_PDO_CONTRACT     0x34  // PDO del contrato activo

//------------------------Definiciones de registros. Los incluidos en el diseño se encuentran comentados --------------------//
// Todos los registros no listados aquí son reservados: no tocar.

//#define TPS_MODE                    0x03  // 4 ASCII chars: 'APP ', 'BOOT', 'PTCH'
#define TPS_CUSTOMER_USE            0x06  // 8 bytes libres para uso propio
//#define TPS_COMMAND_I2C1            0x08  // Comando 4CC (CMD1)
//#define TPS_DATA1                   0x09  // Datos asociados a CMD1 (hasta 64 bytes)
//#define TPS_INT_EVENT1              0x14  // Eventos de interrupción (11 bytes)
#define TPS_INT_MASK1               0x16  // Máscara de eventos
//#define TPS_INT_CLEAR1              0x18  // Escribir 1 limpia el evento en INT_EVENT1
#define TPS_STATUS                  0x1A  // Estado no relacionado con interrupciones
#define TPS_POWER_PATH_STATUS       0x26  // Estado de los switches de potencia
#define TPS_PORT_CONFIGURATION      0x28  // Config de hardware del puerto (requiere reconexión)
#define TPS_PORT_CONTROL            0x29  // Config de política (PR/DR swap, etc.)
#define TPS_BOOT_FLAGS              0x2D  // Estado del arranque, EEPROM, dead battery
#define TPS_RX_SOURCE_CAPS          0x30  // PDOs recibidos del source (solo lectura)
#define TPS_RX_SINK_CAPS            0x31  // PDOs recibidos del sink (solo lectura)
#define TPS_TX_SOURCE_CAPS          0x32  // PDOs que anuncias como source
#define TPS_TX_SINK_CAPS            0x33  // PDOs que anuncias como sink (tu caso)
//#define TPS_ACTIVE_PDO_CONTRACT     0x34  // PDO del contrato activo
#define TPS_ACTIVE_RDO_CONTRACT     0x35  // RDO del contrato activo
#define TPS_AUTONEGOTIATE_SINK      0x37  // Ventana de negociación automática como sink
#define TPS_POWER_STATUS            0x3F  // Estado de VBUS, source/sink, carga detectada
#define TPS_PD_STATUS               0x40  // Rol PD/TypeC actual, detalles de reset
#define TPS_PD3_CONFIGURATION       0x42  // Soporte de mensajes PD3.0 (PPS, battery, etc.)
#define TPS_RX_SOP_IDENTITY         0x48  // Identity VDOs recibidos (Discover Identity)
#define TPS_IO_CONFIG               0x5C  // Config de GPIOs (solo lectura en runtime)
#define TPS_TYPEC_STATE             0x69  // Estado de máquina Type-C y pines CC1/CC2
#define TPS_ADC_RESULTS             0x6A  // Lecturas de VBUS, GPIOx, ADCIN1/2 (debug)
#define TPS_SLEEP_CONTROL           0x70  // Config de modo sleep
#define TPS_GPIO_STATUS             0x72  // Dirección y nivel lógico de cada GPIO
#define TPS_TX_SRC_CAPS_EXT_DB      0x77  // Extended Source Caps (SCEDB)
#define TPS_TX_SOURCE_INFO          0x78  // Info de PDP presente/reportado como source
#define TPS_TX_PPS_STATUS_DB        0x7A  // PPS Status Data Block
#define TPS_TX_BATTERY_STATUS_DO    0x7B  // Battery Status Data Objects (BSDO)
#define TPS_TX_BATTERY_CAPS         0x7D  // Battery Capabilities (BCDB)
#define TPS_TX_SINK_CAPS_EXT_DB     0x7E  // Extended Sink Caps
#define TPS_LIQUID_DETECTION_CFG    0x98  // Config de detección de líquido/corrosión
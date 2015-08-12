#ifndef _Riego_H_
#define _Riego_H_

//#define RELEASE		//28194 + 814  => 27202 +808
//#define RELEASE_FINAL
//#define C_SEGURIDAD

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "miEEPROM.h"
//#include "controlZona.h"



//#include <Time.h>
//#include "Flash.h"

#include <SoftwareSerial.h>

//pins usados
/*
 	 0 RX a gsm
 	 1 TX a gsm
 	 2 caudalimetro
 	 3 temperatura
 	 4  D4 a lcd
 	 5  D5 a lcd
 	 6  D6 a lcd
 	 7  D7 a lcd
 	 8  RS a lcd
 	 9  E a  lcd     power on/off gsm  ---
 	 10 retroiluminacion lcd
 	 11 RX temp
 	 12 TX temp
 	 13
 	 14 GND
 	 15 AREF
 	 16 SDA
 	 17 SLC

 	 A0 - botones shield lcd
 	 A1 - amperimetro
 	 A2 - voltimetro

 */

#define MAX_BUFFER_SMS 161
#define MAX_BUFFER 151
#define ABRIR true
#define CERRAR false
#define PRINCIPAL 5
#define UNAHORA  30000 //3600000
#define VEINTESEGUNDOS  1000 //20000
#define UNSEGUNDO 1000
#define CINCOSEGUNDOS 5000
#define TIEMPOCIERRE 6000
#define TIEMPODISPLAY 15000
#define TIEMPOLUZ 10000
#define TIEMPOCURSOR 5000
#define UNMINUTO 10000 //60000
#define CINCOMINUTOS 2000 //300000
//definicion de pines
#define BOTONERA 0
#define CURRENT_SENSOR 1
#define VOLTAJE_BATERIA 2
#define RETROILUMINACION 10

#define RIEGO_VERSION_MAYOR "0" //15088 676 646
#define RIEGO_VERSION_MENOR "7.0" //20172 651 622
#define RIEGO_VERSION RIEGO_VERSION_MAYOR"."RIEGO_VERSION_MENOR

typedef union{  //28106  + 820  411   28360 + 824  438
	char SMS[MAX_BUFFER_SMS];
	char buffer[MAX_BUFFER];
} UBuffer;

typedef union{
	char aux[35];
} UBuffer2;



//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project Riego here

void tratarOpcion();	//*
void controlTiempo(void); 			//*


//metodos de pruebas a borrar
#ifndef RELEASE_FINAL
bool tratarRespuestaSerial(void); 	//*
void leerEEPROM(byte pos,boolean tipo);			//*

void configuracionSeguridad();
void mostrarConfiguracionSeguridad();

void riegoManual(byte zona);
void estadoProblemaEnZona(byte zona);


#endif

//#endif

//Do not add code below this line
#endif /* _Riego_H_ */

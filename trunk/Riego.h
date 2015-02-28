#ifndef _Riego_H_
#define _Riego_H_

//#define RELEASE		//26118 + 824 + 440
//#define RELEASE_FINAL //22172 + 810 + 454
//#define SIMPLE

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <inttypes.h>
#include <stdlib.h>

#include "controlZona.h"
#include "miEEPROM.h"


//#include <Time.h>
#include "Flash.h"



//add your includes for the project Riego here
#ifndef DEBUG_PROCESS
    #include <LiquidCrystal.h>
    #include <SoftwareSerial.h>
#endif // DEBUG_PROCESS

//pins usados
/*
 	 0 RX a gsm
 	 1 TX a gsm
 	 2	Realy 1
 	 3 	Realy 2
 	 4  D4 a lcd
 	 5  D5 a lcd
 	 6  D6 a lcd
 	 7  D7 a lcd
 	 8  RS a lcd
 	 9  E a  lcd     power on/off gsm  ---
 	 10 retroiluminacion lcd
 	 11 RX temp      Relay 3
 	 12 TX temp      Relay 4
 	 13 Relay 5
 	 14 GND
 	 15 AREF
 	 16 SDA
 	 17 SLC

 	 A0 - botones shield lcd
 	 A1 - caudalimetro



 */

#define MAX_BUFFER_SMS 161
#define MAX_BUFFER 151
#define ABRIR true
#define CERRAR false
#define PRINCIPAL 5
#define UNAHORA 3600000
#define UNSEGUNDO 1000

//definicion de pines
#define BOTONERA 0
#define CURRENT_SENSOR 1
#define VOLTAJE_BATERIA 2


#define RELAY_ON 1
#define RELAY_OFF 0
/*-----( Declare objects )-----*/
/*-----( Declare Variables )-----*/
#define Relay_1  2  // Arduino Digital I/O pin number
#define Relay_2  3
#define Relay_3  11
#define Relay_4  12
#define Relay_5  13
//#define Relay_6  9

#define RIEGO_VERSION_MAYOR "0" //15088 676 646
#define RIEGO_VERSION_MENOR "5.4" //20172 651 622
#define RIEGO_VERSION RIEGO_VERSION_MAYOR"."RIEGO_VERSION_MENOR

typedef union{  //28106  + 820  411   28360 + 824  438
	char SMS[MAX_BUFFER_SMS];
	char buffer[MAX_BUFFER];
} UBuffer;

typedef union{
	char aux[34];
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

int get_key(unsigned int input); 	//*
void tratarOpcion(byte x,byte y);	//*
void controlTiempo(void); 			//*
void getSMS(void);					//*
void setSMS(void);					//*
void getFechaHora(void);			//*
bool setFechaHora(byte opcion);		//*
void cambioNumero(byte tipo);		//*
void comandoGPRS(void);				//*
void tratarRespuestaGprs(void); 	//*
void getBateria(void);
void estadoProblemaEnZona(byte zona);
int lecturaPulsador(void);


//metodos de pruebas a borrar
#ifndef RELEASE_FINAL
bool tratarRespuestaSerial(void); 	//*
void pruebaRelay(void);				//*
void leerEEPROM(byte pos);			//*

#endif
int freeRam();						//*
//#endif

//Do not add code below this line
#endif /* _Riego_H_ */

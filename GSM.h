

#ifndef __GSM
#define __GSM
#include "Riego.h"
#include "Menu.h"
#include "controlZona.h"
#include "miEEPROM.h"
#include <SoftwareSerial.h>

#define GSM_LIB_VERSION_MAYOR "0"
#define GSM_LIB_VERSION_MENOR "6.0"

#define GSM_LIB_VERSION GSM_LIB_VERSION_MAYOR"."GSM_LIB_VERSION_MENOR

// pins definition
//#define GSM_ON              8 // connect GSM Module turn ON to pin 77
//#define GSM_RESET           9 // connect GSM Module RESET to pin 35

#define GSM_SERIAL_TX (uint8_t)12		//8
#define GSM_SERIAL_RX (uint8_t)11		//7

#define GSM_POWER_ON_OFF 9

#define MILLISPORDIA 86400000

/* R2    R1     R2 / (R1+R2)       R
	3280 9960  3280/(9960+3280) = 0.247734

   1/0.247734 = 4.0367

  Vmax = 5/R  V  = ((x*5)/1024)*(1/R)
	 20.1632

	15 = 762   3.72
	14 = 711   3.48
	13 = 660   3.22
	12 = 610   2.98
	11 = 559   2.73
	10 = 508   2.49
*/
#define DIVISORTENSION  0.0197435  //(5.0/1023)* 4.04

class GSM
{
public:
    // constructor
    GSM(Menu *menu,controlZona * zonas);
    ~GSM();
    void SIM900power(void);
    int available(void);
    size_t println(const String &s);
    size_t println(const char c[]);
    size_t println(char);
    size_t print(const char str[]);
    char * libVer(void);
    String readString(void);
    int read(void);
    char * enviaComando(const char str[]);
    char * enviaComando(const String &s);
    void establecerZona(byte alarma);
    void establecerHoraFin(byte alarma);
    void establecerHoraInicio(byte alarma);
    void iniciarRiegoZona(byte numeroAlarma);
    void pararRiegoZona(byte numeroAlarma);
    void inicializaAlarmas();
    void valvulaPrincipal(bool estado);
    bool isActivo(void);
    void getSMS(char *linea1,char *linea2,boolean pantallaEncendida);					//*
    void setSMS(char *linea2);					//*
    void getFechaHora(char*linea1,char*linea2);			//*
    bool setFechaHora(byte opcion,char*linea1,char*linea2);		//*
   // bool getProblemaEnZona(byte zona);
   // void setProblemaEnZona(byte zona,bool estado);
    void enviaSMSErrorPrincipal(void);
    long iniciaReloj(void);
    void enviaSMSErrorTodasLasZonas();
    void enviaSMSErrorZonas(char zona[6]);
    void enviaSMSError(byte tipo);
    float energiaBateria(void);
    void setTiempoValvula(unsigned long tiempo);
    unsigned long getTiempoValvula();
    void reiniciaValvula(void);
    void comandoGPRS();				//*
    void tratarRespuestaGprs(); 	//*

    size_t readBytesUntil(char terminator, char *buffer, size_t length);
    void limpiaBufferI(void);

private:
    SoftwareSerial* myPortSerial;
    char bufferI[MAX_BUFFER];
    unsigned long tiempoValvula;
    boolean cadena_errores[4];
    boolean principalAbierta;

    void limpiaSMS(void);
    char *procesaEnviaComando(void);
    Menu * myMenu;
    controlZona  * zona;

};
#endif


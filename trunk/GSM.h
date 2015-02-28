

#ifndef __GSM
#define __GSM

#include "Riego.h"
//#include "controlZona.h"
//#include "miEEPROM.h"

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
		2680 9960  2680/(9960+2680) = 0.212025*/
#define R 0.212025



class GSM
{
public:
    // constructor
    GSM();
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
    void establecerZona(controlZona *zonas,byte alarma);
    void establecerHoraFin(controlZona *zonas,byte alarma);
    void establecerHoraInicio(controlZona *zonas,byte alarma);
    void iniciarRiegoZona(byte numeroAlarma);
    void pararRiegoZona(byte numeroAlarma);
    void inicializaAlarmas(controlZona * zonas);
    void valvulaPrincipal(bool estado);
    bool isActivo(void);
   // bool getProblemaEnZona(byte zona);
   // void setProblemaEnZona(byte zona,bool estado);
    void enviaSMSErrorPrincipal(void);
    long iniciaReloj(void);
    void enviaSMSErrorTodasLasZonas();
    void enviaSMSErrorZonas(char zona[6]);
    void enviaSMSError(byte tipo);
    float energiaBateria(void);

    size_t readBytesUntil(char terminator, char *buffer, size_t length);

private:
    #ifndef DEBUG_PROCESS
        SoftwareSerial* myPortSerial;
    #endif
    char bufferI[MAX_BUFFER];
    char cadena_errores[4];
    void limpiaBufferI(void);
    void limpiaSMS(void);
    char *procesaEnviaComando(void);
};
#endif


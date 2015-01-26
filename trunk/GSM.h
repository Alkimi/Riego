

#ifndef __GSM
#define __GSM

#include "Riego.h"
#include "zonasDeRiego.h"
#include "miEEPROM.h"

#define GSM_LIB_VERSION_MAYOR "0"
#define GSM_LIB_VERSION_MENOR "5.3"

#define GSM_LIB_VERSION GSM_LIB_VERSION_MAYOR"."GSM_LIB_VERSION_MENOR

// pins definition
//#define GSM_ON              8 // connect GSM Module turn ON to pin 77
//#define GSM_RESET           9 // connect GSM Module RESET to pin 35

#define GSM_SERIAL_TX (uint8_t)12		//8
#define GSM_SERIAL_RX (uint8_t)11		//7

#define GSM_POWER_ON_OFF 9

#define MAX_BUFFER 150
#define MAX_POSICION_BUFFER MAX_BUFFER-1
#define abrir true
#define cerrar false


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
    size_t print(const char str[]);
    char * libVer(void);
    String readString(void);
    int read(void);
    char * enviaComando(const char str[]);
    char * enviaComando(const String &s);
    void establecerZona(t_zonaRiego *DatoZonaDeRiego);
    void establecerHoraFin(t_zonaRiego *DatoZonaDeRiego);
    void establecerHoraInicio(t_zonaRiego *DatoZonaDeRiego);
    void iniciarRiegoZona(byte numeroAlarma);
    void pararRiegoZona(byte numeroAlarma);
    void inicializaAlarmas(zonasDeRiego * zonas);
    void valvulaPrincipal(bool estado);
    bool isActivo(void);
    bool getProblemaEnZona(byte zona);
    void setProblemaEnZona(byte zona);

    size_t readBytesUntil(char terminator, char *buffer, size_t length);

private:
    #ifndef DEBUG_PROCESS
        SoftwareSerial* myPortSerial;
    #endif
    char bufferI[MAX_BUFFER];
    char bufferO[MAX_BUFFER];
    void limpiaBufferI(void);
    void limpiaBufferO(void);
    char *procesaEnviaComando(void);
};
#endif


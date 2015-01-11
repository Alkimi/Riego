/*GSM.cpp - library for the GSM Playground - GSM Shield for Arduino
 Released under the Creative Commons Attribution-Share Alike 3.0 License
 http://www.creativecommons.org/licenses/by-sa/3.0/
 www.hwkitchen.com
*/
#include "GSM.h"

extern "C" {
  #include <string.h>
}

/**********************************************************
  Constructor definition
***********************************************************/
GSM::GSM()
{
    #ifndef DEBUG_PROCESS
        //pinMode(GSM_POWER_ON_OFF,OUTPUT);
        myPortSerial = new SoftwareSerial(GSM_SERIAL_RX, GSM_SERIAL_TX);
        myPortSerial->begin(19200);
    #endif // DEBUG_PROCESS
	limpiaBufferI();
	limpiaBufferO();
}

char * GSM::libVer(void)
{
  return (GSM_LIB_VERSION);
}

void GSM::establecerZona(t_zonaRiego *zonaDeRiego){
	char *cadena;
	sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",zonaDeRiego->horaInicio,
			zonaDeRiego->minutoInicio,zonaDeRiego->numeroZona,zonaDeRiego->intervaloRiego);
#ifndef RELEASE
	Serial << F("establecer Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
}

void GSM::establecerHoraFin(t_zonaRiego *zonaDeRiego){
	char *cadena;
	int tiempo = (zonaDeRiego->horaInicio*60)+zonaDeRiego->minutoInicio+zonaDeRiego->duracion;
	byte hora = tiempo/60;
	if (hora>24) hora =hora-24;
	sprintf(bufferO,"AT+CALD=%d",zonaDeRiego->numeroZona);
#ifndef RELEASE
	Serial << F("borrando alarma Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
	sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",hora,tiempo%60,zonaDeRiego->numeroZona,0);
#ifndef RELEASE
	Serial << F("establecer Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
}

void GSM::establecerHoraInicio(t_zonaRiego *zonaDeRiego){
	char *cadena;
	sprintf(bufferO,"AT+CALD=%d",zonaDeRiego->numeroZona);
#ifndef RELEASE
	Serial << F("borrando alarma Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
	establecerZona(zonaDeRiego);
}

void GSM::iniciarRiegoZona(byte zona){
	char *cadena;
	sprintf(bufferO,"AT+GSPIO=0,%d,1,1",zona);
#ifndef RELEASE
	Serial << F("iniciar Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
}

void GSM::pararRiegoZona(byte zona){
	char *cadena;
	sprintf(bufferO,"AT+GSPIO=0,%d,1,0",zona);
#ifndef RELEASE
	Serial << F("parar Zona de Riego cadena: ")<< bufferO << endl;
#endif
	cadena=enviaComando(bufferO);
}

void GSM::SIM900power(void) // no funciona
{
	  pinMode(GSM_POWER_ON_OFF, OUTPUT);
	  digitalWrite(GSM_POWER_ON_OFF,LOW);
	  delay(1000);
	  digitalWrite(GSM_POWER_ON_OFF,HIGH);
	  delay(2000);
	  digitalWrite(GSM_POWER_ON_OFF,LOW);
	  delay(3000);
}

int GSM::available(void){
    #ifndef DEBUG_PROCESS
        return myPortSerial->available();
    #else
        return true;
    #endif
}


size_t GSM::print(const char str[])
{
    #ifndef DEBUG_PROCESS
        return myPortSerial->write(str);
    #else
        return 0;
    #endif

}

size_t GSM::println(const char c[]){
    #ifndef DEBUG_PROCESS
        return myPortSerial->println(c);
    #else
        return 0;
    #endif

}

size_t GSM::println(const String &s){
    #ifndef DEBUG_PROCESS
	return myPortSerial->println(s);
    #else
        return 0;
    #endif
}

String GSM::readString(void){
    #ifndef DEBUG_PROCESS
        return myPortSerial->readString();
    #else
        return "";
    #endif

}


int GSM::read(void){
    #ifndef DEBUG_PROCESS
        return myPortSerial->read();
    #else
        return 0;
    #endif

}

char * GSM::enviaComando(const char str[]){
	println(str);
#ifndef RELEASE
	Serial << F("Comando enviado: ") << str << endl;
	//Serial.print(F("comando enviado: "));

	//Serial.println(str);
#endif
	delay(1000);
	bool error=true;
	if (available()){
	  byte i=0;
	  bufferI[i]=read();
	  while (bufferI[i]>0){
		  i++;
		  bufferI[i]=read();
	  }
	  int contador=0;
	  for (byte j = 0; j<i;j++){
		  if (bufferI[j]=='E' && bufferI[j+1]=='R' && bufferI[j+2]=='R' && bufferI[j+3]=='O'){
			  break;
		  }
		  if (bufferI[j]=='O' && bufferI[j+1]=='K' && bufferI[j+2]==13){
			  error=false;
			  break;
		  }
		  if (bufferI[j]==13 && bufferI[j+1]==10){
			  j++;
			  continue;
		  }
		  bufferO[contador]=bufferI[j];
		  contador++;
	  }
	  bufferO[contador]=0x0;
	  limpiaBufferI();
	}
#ifndef RELEASE
	Serial << F("Varlor del error: ") << error << endl<< F("Comando respuesta filtrada enviada: ") << bufferO << endl;
#endif
	if (error) return NULL;
	return bufferO;
}

void GSM::limpiaBufferI(void){
	for (byte i=0;i<MAX_POSICION_BUFFER;i++){
		bufferI[i]=0;
	}
}

void GSM::limpiaBufferO(void){
	for (byte i=0;i<MAX_POSICION_BUFFER;i++){
		bufferO[i]=0;
	}
}

GSM::~GSM(){
    #ifndef DEBUG_PROCESS
        delete myPortSerial;
    #endif

}

///comando AT utiles
/*
 * AT+CCLK?   obtener hora
 * AT+CCLK="yy/MM/dd,hh:mm:ss+00"  establece la hora mas diferencia GTM
 * ATD*111#;     obtiene els saldo con el siguiente mensaje
 * CUSD: 0,"Yoigo Info:Tu saldo es de 19,90 euros (imp. ind. no inclusrTaenolo2 Mfim
 * AT+CBTE?   voltaje de la pila
 * AT+CPOWD=1 apagar el GSM
 * AT+CALA establece alarma
 * AT+CALA="23:38:50",3,0     respuesta +CALV: 2
 * AT+CALD=n  borra la alarma n
 * AT+SGPIO=0,1-6,1,low-high  establece el valor low o high en el pin 1-6
 * AT+CLIP=1  muestra identificador de llamada
 * CLIP: "numero",valor,"",,"",0  respuesta cuando llama alguien
 * AT+CMFG=1  sms en modo texto
 * AT+CMGS="+34numero"  envia sms
 * CMTI: "SM",1  mensaje sin leer
 * AT+CMGR=1 lee un sms
 *
 * CMTI
 */

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
	limpiaBufferI();
	limpiaBufferO();
    #ifndef DEBUG_PROCESS
        //pinMode(GSM_POWER_ON_OFF,OUTPUT);
        myPortSerial = new SoftwareSerial(GSM_SERIAL_RX, GSM_SERIAL_TX);
        myPortSerial->begin(4800);
        /*if (!isActivo()){
        	SIM900power();
        }*/
    #endif // DEBUG_PROCESS

}

bool GSM::isActivo(void){
	return (enviaComando(F("AT"))==NULL)?true:false;
}
/*
void GSM::enviaSMS(char *numero,boolean reventon,byte zona,byte litros){

}*/

char * GSM::libVer(void)
{
  return (GSM_LIB_VERSION);
}

void GSM::inicializaAlarmas(zonasDeRiego * zonas){
	enviaComando(F("AT+CALD=1;+CALD=2;+CALD=3;+CALD=4;+CALD=5"));
	for (int i=0;i<zonas->getNumeroZonasRiego();i++){
		sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",zonas->getZonaDeRiego(i)->horaInicio,
		zonas->getZonaDeRiego(i)->minutoInicio,zonas->getZonaDeRiego(i)->numeroZona,zonas->getZonaDeRiego(i)->intervaloRiego);
		enviaComando(bufferO);
	}
}

void GSM::valvulaPrincipal(bool estado){
	if (estado){
		enviaComando(F("AT+SGIPI=0,5,1,1;+SGIPO=0,6,1,0"));
	}else{
		enviaComando(F("AT+SGIPI=0,5,1,0;+SGIPO=0,6,1,1"));

	}
	delay(7000);
	enviaComando(F("AT+SGIPI=0,5,1,0;+SGIPO=0,6,1,0"));
}

void GSM::establecerZona(t_zonaRiego *DatoZonaDeRiego){
	char *cadena;
	sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",DatoZonaDeRiego->horaInicio,
			DatoZonaDeRiego->minutoInicio,DatoZonaDeRiego->numeroZona,DatoZonaDeRiego->intervaloRiego);
	cadena=enviaComando(bufferO);
}

void GSM::establecerHoraFin(t_zonaRiego *DatoZonaDeRiego){
	char *cadena;
	int tiempo = (DatoZonaDeRiego->horaInicio*60)+DatoZonaDeRiego->minutoInicio+DatoZonaDeRiego->duracion;
	byte hora = tiempo/60;
	if (hora>24) hora =hora-24;
	sprintf(bufferO,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",DatoZonaDeRiego->numeroZona,hora,tiempo%60,DatoZonaDeRiego->numeroZona,0);
	cadena=enviaComando(bufferO);
}

void GSM::establecerHoraInicio(t_zonaRiego *DatoZonaDeRiego){
	char *cadena;
	sprintf(bufferO,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",DatoZonaDeRiego->numeroZona,DatoZonaDeRiego->horaInicio,
			DatoZonaDeRiego->minutoInicio,DatoZonaDeRiego->numeroZona,DatoZonaDeRiego->intervaloRiego);
	cadena=enviaComando(bufferO);
}

bool GSM::getProblemaEnZona(byte zona){
	//leemos el varlo de la epron de la zona
	byte valor= EEPROM.read(16+zona);
	return (valor!=0x0)?true:false;
}

void GSM::setProblemaEnZona(byte zona){
	//invertimos el valor de eprom
	byte valor= EEPROM.read(16+zona);
	(valor!=0x0)?valor=0x0:valor=0x1;
	EEPROM.write(16+zona,valor);
}

void GSM::iniciarRiegoZona(byte numeroAlarma){
	char *cadena;
	valvulaPrincipal(abrir);
	sprintf(bufferO,"AT+SGPIO=0,%d,1,1",numeroAlarma);
	cadena=enviaComando(bufferO);
}

void GSM::pararRiegoZona(byte numeroAlarma){
	char *cadena;
	valvulaPrincipal(cerrar);
	sprintf(bufferO,"AT+SGPIO=0,%d,1,0",numeroAlarma);
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
#ifndef RELEASE_FINAL
	Serial << F("Comando enviado: ") << str << endl;
#endif
	return procesaEnviaComando();
}

char * GSM::enviaComando(const String &s){
	println(s);
#ifndef RELEASE_FINAL
	Serial << F("Comando enviado: ") << s << endl;
#endif
	return procesaEnviaComando();
}

char * GSM::procesaEnviaComando(void){
	limpiaBufferI();
	long tiempo;
	tiempo=millis();
	int contador=0;
	int lectura;
	do{
		if (available()>0){
			lectura=available();
			for(int i=0;i<lectura;i++){
				if (contador < MAX_BUFFER){
					bufferI[contador]=read();
					contador++;
				}
			}
		}
	}while(millis()-tiempo<1000);
	if ((bufferI[2]=='E' && bufferI[3]=='R' && bufferI[4]=='R' && bufferI[5]=='O' && bufferI[6]=='R') || (contador ==0)) return NULL;

	contador=0;
    for (int i = 2; i<MAX_BUFFER;i++){
	  if (bufferI[i]=='O' && bufferI[i+1]=='K' && bufferI[i+2]==13){
		  i+=3;
		  continue;
	  }
	  if (bufferI[i]==13 && bufferI[i+1]==10){
		  i++;
		  continue;
	  }
	  if (bufferI[i]==0x0) break;
	  bufferO[contador]=bufferI[i];
	  contador++;
    }
    bufferO[contador]=0x0;

#ifndef RELEASE_FINAL
	Serial <<F("Comando despues de procesar: ") << bufferO<<endl;
#endif
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
 * AT+CMGDA="DEL ALL"  borra todos los SMS
 * AT+SCLASS0=1  guarda los mensajes push en sim
 *
 * CMTI
 */

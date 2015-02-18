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
        if (!isActivo()){
        	SIM900power();
        }
    #endif // DEBUG_PROCESS
}

long GSM::iniciaReloj(void){
	char *cadena;
	long horas;
	long minutos;
	long segundos;
	cadena=enviaComando(F("AT+CCLK?"));
	cadena+=17;
	cadena[2]=0x0;
	horas=atol(cadena)*3600;
	cadena+=3;
	cadena[2]=0x0;
	minutos=horas+(atol(cadena)*60);
	cadena+=3;
	cadena[2]=0x0;
	segundos=(minutos+atol(cadena))*1000;
	return MILLISPORDIA-segundos;
}

bool GSM::isActivo(void){
	return true;
	return (enviaComando(F("AT"))==NULL)?false:true;
}
/*
void GSM::enviaSMS(char *numero,boolean reventon,byte zona,byte litros){

}*/

void GSM::enviaSMSError(byte tipo){
	    byte valor=tipo;
	    char vector[9];
	    if (valor>30){
	    	valor-=30;
	    	tipo=3;
	    }
	    char * aux;
		char temp;
		limpiaSMS();
		strcat_P(SMS,PSTR("Se ha producido un rebenton, se cierra"));
		switch (tipo){
		  case 1: //rebenton general
			  strcat_P(SMS,PSTR(" la valvula principal. Hay que rearmar"));
			break;
		  case 2:
			  strcpy_P(SMS,PSTR("Todas las zonas necesitan ser rearmadas"));
		    break;
		  case 3:  // rebenton en zonas
			  strcat_P(SMS,PSTR("n las zonas "));
			int j=0;
			//se añaden las zonas
			for (int i=0;i<4;i++){
				vector[j]=cadena_errores[i];
				vector[j++]=' ';
				j++;
			}
			strcat(SMS,vector);
			strcat_P(SMS,PSTR(". Hay que rearmar"));
			break;
		}
		//obtener fecha y hora
		strcat_P(SMS,PSTR(" manualmente. Dia: "));
		aux=enviaComando(F("AT+CCLK?"));
		aux=aux+8;
		temp=aux[6];
		aux[6]=aux[0];
		aux[0]=temp;
		temp=aux[7];
		aux[7]=aux[1];
		aux[1]=temp;
		aux[8]=0x0;
		strcat(SMS,aux);
		strcat_P(SMS,PSTR(" Hora: "));
		aux=aux+9;
		aux[8]=0x0;
		strcat(SMS,aux);
		strcat_P(SMS,PSTR(" Pila: "));
		aux=enviaComando(F("AT+CBTE?"));
		aux=aux+7;
		float pila=atof(aux)/1000.00;
		dtostrf(pila, 4, 2, vector);
		strcat(SMS,vector);
		//obtenemos la eneriga de a bateria
		strcat_P(SMS,PSTR(" Bateria: "));
		dtostrf(energiaBateria(),5,2,vector);
		strcat(SMS,vector);
		//obtener numero sms
		EEPROM.lecturaEeprom16(0, bufferO);
		//componer sms
		sprintf(bufferI,"AT+CMGS=\"%s\"",bufferO);

		Serial << F("comando: ")<<bufferI<<endl<<F("cadena: ")<<SMS<<endl;
		/*//enviar sms
		enviaComando(bufferI);
		//envia mensaje
		println(SMS);
		//envia final mensaje
		println((char)26);*/

}

void GSM::enviaSMSErrorTodasLasZonas(){
	enviaSMSError(2);
}

float GSM::energiaBateria(void){
    /*Vmax = 5/R  V  = ((x*5)/1024)*R
	 23.58

	15 = 3.1803
	14 = 2.9683
	13 = 2.7563
	12 = 2.5443
	11 = 2.3322
	10 = 2.1202
	*/
	return (((analogRead(analogPinVoltage)*5.0)/1024)/R);

}

void GSM::enviaSMSErrorZonas(char zonas[4]){
	for (int i=0;i<4;i++){
		cadena_errores[i]=zonas[i];
	}
	enviaSMSError(3);
}

char * GSM::libVer(void)
{
  return (GSM_LIB_VERSION);
}

void GSM::inicializaAlarmas(controlZona * zonas){
	enviaComando(F("AT+CALD=1;+CALD=2;+CALD=3;+CALD=4;+CALD=5"));
	for (int i=0;i<zonas->getNumeroZonasRiego();i++){
		if (zonas->isZonaActiva(i)){
			sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",
					zonas->getHoraZona(i),zonas->getMinutoZona(i),i,zonas->getIntervaloZona(i));
			enviaComando(bufferO);
		}
	}
}

void GSM::valvulaPrincipal(bool estado){
	if (estado){
		enviaComando(F("AT+SGPIO=0,5,1,1;+SGPIO=0,6,1,0"));
	}else{
		enviaComando(F("AT+SGPIO=0,5,1,0;+SGPIO=0,6,1,1"));

	}
	delay(7000);
	enviaComando(F("AT+SGPIO=0,5,1,0;+SGPIO=0,6,1,0"));
}

void GSM::establecerZona(controlZona *zonas, byte alarma){
	sprintf(bufferO,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",zonas->getHoraZona(alarma),zonas->getMinutoZona(alarma),zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));
	enviaComando(bufferO);
}

void GSM::establecerHoraFin(controlZona *zonas,byte alarma){
	int tiempo = (zonas->getHoraZona(alarma)*60)+zonas->getMinutoZona(alarma)+zonas->getDuracionZona(alarma);
	byte hora = tiempo/60;
	if (hora>24) hora =hora-24;
	sprintf(bufferO,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",zonas->getNumeroZona(alarma),hora,tiempo%60,zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));
	enviaComando(bufferO);
}

void GSM::establecerHoraInicio(controlZona *zonas, byte alarma){
	sprintf(bufferO,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",
			zonas->getNumeroZona(alarma),zonas->getHoraZona(alarma),
			zonas->getMinutoZona(alarma),zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));

	enviaComando(bufferO);
}
/*
bool GSM::getProblemaEnZona(byte zona){
	//leemos el varlo de la epron de la zona
	byte valor= EEPROM.read(16+zona);
	return (valor!=0x0)?true:false;
}*/

void GSM::setProblemaEnZona(byte zona,bool estado){
	//invertimos el valor de eprom
	/*byte valor= EEPROM.read(16+zona);
	(valor!=0x0)?valor=0x0:valor=0x1;*/
	EEPROM.write(16+zona,estado);
}

void GSM::enviaSMSErrorPrincipal(void){
	enviaSMSError(1);
}

void GSM::iniciarRiegoZona(byte numeroAlarma){
	char *cadena;
	valvulaPrincipal(ABRIR);
	sprintf(bufferO,"AT+SGPIO=0,%d,1,1",numeroAlarma);
	cadena=enviaComando(bufferO);
}

void GSM::pararRiegoZona(byte numeroAlarma){
	char *cadena;
	valvulaPrincipal(CERRAR);
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
        return myPortSerial->available();
}


size_t GSM::print(const char str[])
{
        return myPortSerial->write(str);
}

size_t GSM::println(const char c[]){
        return myPortSerial->println(c);
}

size_t GSM::println(const String &s){
	return myPortSerial->println(s);
}

size_t GSM::println(char s){
	return myPortSerial->println(s);
}


String GSM::readString(void){
        return myPortSerial->readString();
}


int GSM::read(void){
        return myPortSerial->read();
}

char * GSM::enviaComando(const char str[]){
	println(str);
	delay(100);
#ifndef RELEASE_FINAL
	Serial << F("Comando enviado: ") << str << endl;
#endif
	return procesaEnviaComando();
}

char * GSM::enviaComando(const String &s){
	println(s);
	delay(100);
#ifndef RELEASE_FINAL
	Serial << F("Comando enviado: ") << s << endl;
#endif
	return procesaEnviaComando();
}

char * GSM::procesaEnviaComando(void){
	limpiaBufferI();
	limpiaBufferO();
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
	contador=0;
    for (int i = 0; i<MAX_BUFFER;i++){
      if ((bufferI[i]=='E' && bufferI[i+1]=='R' && bufferI[i+2]=='R' && bufferI[i+3]=='O' && bufferI[i+4]=='R')) return NULL;

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
	for (byte i=0;i<MAX_BUFFER;i++){
		bufferI[i]=0x0;
	}
}

void GSM::limpiaBufferO(void){
	for (byte i=0;i<MAX_BUFFER;i++){
		bufferO[i]=0x0;
	}
}

void GSM::limpiaSMS(void){
	for (byte i=0;i<MAX_BUFFER_SMS;i++){
		SMS[i]=0x0;
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

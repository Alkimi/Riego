/*GSM.cpp - library for the GSM Playground - GSM Shield for Arduino
 Released under the Creative Commons Attribution-Share Alike 3.0 License
 http://www.creativecommons.org/licenses/by-sa/3.0/
 www.hwkitchen.com
*/
#include "GSM.h"
extern "C" {
  #include <string.h>
}

extern UBuffer buffer;
extern UBuffer2 buffer2;

/**********************************************************
  Constructor definition
***********************************************************/
GSM::GSM()
{
	principalAbierta=false;
	limpiaBufferI();
	myPortSerial = new SoftwareSerial(GSM_SERIAL_RX, GSM_SERIAL_TX);
	myPortSerial->begin(4800);
	if (!isActivo()){
		SIM900power();
	}
}

void GSM::setTiempoValvula(unsigned long tiempo){
	tiempoValvula=tiempo;
}

unsigned long GSM::getTiempoValvula(){
	return tiempoValvula;
}

long GSM::iniciaReloj(void){
	char *cadena;
	long horas;
	long minutos;
	long segundos;
	EEPROM.leeCadenaEEPROM(417,buffer2.aux); // AT+CCLK?
	cadena=enviaComando(buffer2.aux);
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
	EEPROM.leeCadenaEEPROM(405,buffer2.aux); //AT
	return (enviaComando(buffer2.aux)==NULL)?false:true;
}
/*
void GSM::enviaSMS(char *numero,boolean reventon,byte zona,byte litros){

}*/

void GSM::enviaSMSError(byte tipo){
	    //byte valor=tipo; //TODO
	   // byte i=0;
	   /* if (valor>30){
	    	valor-=30;
	    	tipo=3;
	    }*/
	    char * aux;
		char temp;
		limpiaSMS();
		int posicion=0;
		if (tipo<4){
		   posicion=EEPROM.leeCadenaEEPROM(753,buffer.SMS); //Se ha producido un rebenton, se cierra
		}else{
		   posicion=EEPROM.leeCadenaEEPROM(933,buffer.SMS); //La bateria principal esta
		}
		switch (tipo){
		  case 1: //rebenton general
			  aux=&buffer.SMS[posicion];
			  posicion=posicion+EEPROM.leeCadenaEEPROM(792,aux); // la valvula principal
			  aux=&buffer.SMS[posicion];
			  posicion=posicion+EEPROM.leeCadenaEEPROM(867,aux); //. Hay que rearmar
			break;
		  case 2:
			  posicion=EEPROM.leeCadenaEEPROM(814,buffer.SMS); //Todas las zonas necesitan ser rearmadas
			break;
		  case 3:  // rebenton en zonas
			  aux=&buffer.SMS[posicion];
			  posicion=posicion+EEPROM.leeCadenaEEPROM(854,aux); //n las zonas
			//se añaden las zonas
			  /*byte i=0;
			  while (i!=4){*/
			for (byte i=0;i<4;i++,posicion++){
				if (cadena_errores[i++]==false){
					buffer.SMS[posicion++]=i+49;
				    buffer.SMS[posicion++]=' ';
				}
			  }
			  aux=&buffer.SMS[posicion];
			  posicion=posicion+EEPROM.leeCadenaEEPROM(867,aux); //. Hay que rearmar
			  break;
		  case 4:
				  aux=&buffer.SMS[posicion];
  				  posicion=posicion+EEPROM.leeCadenaEEPROM(959,aux);//baja, si llega al
				  aux=&buffer.SMS[posicion];
				  posicion=posicion+EEPROM.leeCadenaEEPROM(982,aux); //  nivel critico se apagara el sistama.
			  break;
		  case 5:
				  aux=&buffer.SMS[posicion];
				  posicion=posicion+EEPROM.leeCadenaEEPROM(978,aux);// en
				  aux=&buffer.SMS[posicion];
				  posicion=posicion+EEPROM.leeCadenaEEPROM(982,aux); //  nivel critico se apagara el sistama.
			  break;
		  case 6:
			  aux=&buffer.SMS[posicion];
			  posicion=posicion+EEPROM.leeCadenaEEPROM(704,aux);// recargada, se reanuda el funcionamiento normal.
			  break;
		}

		if (tipo<4){
			aux=&buffer.SMS[posicion];
			posicion=posicion+EEPROM.leeCadenaEEPROM(885,aux); // manualmente.
		}
		//obtener fecha y hora
		aux=&buffer.SMS[posicion];
		posicion=posicion+EEPROM.leeCadenaEEPROM(899,aux); // Dia:
		EEPROM.leeCadenaEEPROM(417,buffer2.aux); // AT+CCLK?
		aux=enviaComando(buffer2.aux);
		aux=aux+8;
		temp=aux[6];
		aux[6]=aux[0];
		aux[0]=temp;
		temp=aux[7];
		aux[7]=aux[1];
		aux[1]=temp;
		aux[8]=0x0;
		byte i=0;
		while (aux[i]!=0x0){
		//for (byte i=0;i<8;i++,posicion++){
			buffer.SMS[posicion++]=aux[i++];
		}
		aux=&buffer.SMS[posicion];
		posicion=posicion+EEPROM.leeCadenaEEPROM(906,aux); // hora:
		aux=buffer2.aux+17;
		aux[8]=0x0;
		i=0;
		while(aux[i]!=0x0){
		//for (byte i=0;i<8;i++,posicion++){
			buffer.SMS[posicion++]=aux[i++];
		}
		aux=&buffer.SMS[posicion];
		posicion=posicion+EEPROM.leeCadenaEEPROM(914,aux); // pila:
		EEPROM.leeCadenaEEPROM(408,buffer2.aux); // AT+CBTE?
		aux=enviaComando(buffer2.aux);
		aux=aux+7;
		float pila=atof(aux)/1000.00;
		dtostrf(pila, 4, 2, buffer2.aux);
		i=0;
		while(buffer2.aux[i]!=0x0){
		//for (byte i=0;i<4;i++,posicion++){
			buffer.SMS[posicion++]=buffer2.aux[i++];
		}
		aux=&buffer.SMS[posicion];
		posicion=posicion+EEPROM.leeCadenaEEPROM(922,aux); // bateria
		dtostrf(energiaBateria(),5,2,buffer2.aux);
		i=0;
		while(buffer2.aux[i]!=0x0){
		//for (byte i=0;i<5;i++,posicion++){
			buffer.SMS[posicion++]=buffer2.aux[i++];
		}
		aux=&buffer.SMS[posicion];
		posicion=posicion+EEPROM.leeCadenaEEPROM(1020,aux); // V
		//obtener numero sms
		EEPROM.lecturaEeprom16(0, buffer2.aux);
		//componer sms
		sprintf(bufferI,"AT+CMGS=\"%s\"",buffer2.aux);
#ifndef RELEASE
		 Serial.print(F("comando")); Serial.println(bufferI);
		 Serial.print(F("cadena: ")); Serial.println(buffer.SMS);
#else
		//enviar sms
		enviaComando(bufferI);
		//envia mensaje
		println(buffer.SMS);
		//envia final mensaje
		println((char)26);
#endif
}

void GSM::enviaSMSErrorTodasLasZonas(){
	enviaSMSError(2);
}

float GSM::energiaBateria(void){
	int suma=0;
	for (byte i=0;i<10;i++){
		suma = suma + analogRead(VOLTAJE_BATERIA);
	}
	suma = suma /10;
	return suma * DIVISORTENSION;
}

void GSM::enviaSMSErrorZonas(char zonas[4]){
	for (byte i=0;i<4;i++){
		cadena_errores[i]=zonas[i];
	}
	enviaSMSError(3);
}

char * GSM::libVer(void)
{
  return (GSM_LIB_VERSION);
}

void GSM::inicializaAlarmas(controlZona * zonas){
	EEPROM.leeCadenaEEPROM(331,buffer.buffer); //AT+CALD=1;+CALD=2;+CALD=3;+CALD=4;+CALD=5
	enviaComando(buffer.buffer);
	for (byte i=0;i<zonas->getNumeroZonasRiego();i++){
		if (zonas->isZonaActiva(i)==true){
			sprintf(buffer2.aux,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",
					zonas->getHoraZona(i),zonas->getMinutoZona(i),i,zonas->getIntervaloZona(i));
			enviaComando(buffer2.aux);
		}
	}
}

void GSM::valvulaPrincipal(bool estado){
	EEPROM.leeCadenaEEPROM(373,buffer2.aux); // "AT+SGPIO=0,5,1,X;+SGPIO=0,6,1,X"
	if (estado){
		if (!principalAbierta){ //AT+SGPIO=0,5,1,1;+SGPIO=0,6,1,0
			principalAbierta=true;
			buffer2.aux[15]='1';
			buffer2.aux[31]='0';
			//enviaComando(F("AT+SGPIO=0,5,1,1;+SGPIO=0,6,1,0"));
		}
	}else{
		if (principalAbierta){ //AT+SGPIO=0,5,1,0;+SGPIO=0,6,1,1
			principalAbierta=false;
			buffer2.aux[15]='0';
			buffer2.aux[31]='1';
			//enviaComando(F("AT+SGPIO=0,5,1,0;+SGPIO=0,6,1,1"));
		}
	}
	enviaComando(buffer2.aux);
	setTiempoValvula(millis());
}

void GSM::reiniciaValvula(){
	EEPROM.leeCadenaEEPROM(373,buffer2.aux); // "AT+SGPIO=0,5,1,X;+SGPIO=0,6,1,X"
	buffer2.aux[15]='0';
	buffer2.aux[31]='0';
	enviaComando(buffer2.aux); // "AT+SGPIO=0,5,1,0;+SGPIO=0,6,1,0"
}

void GSM::establecerZona(controlZona *zonas, byte alarma){
	sprintf(buffer2.aux,"AT+CALA=\"%.2i:%.2i:00\",%d,%d",zonas->getHoraZona(alarma),zonas->getMinutoZona(alarma),zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));
	enviaComando(buffer2.aux);
}

void GSM::establecerHoraFin(controlZona *zonas,byte alarma){
	int tiempo = (zonas->getHoraZona(alarma)*60)+zonas->getMinutoZona(alarma)+zonas->getDuracionZona(alarma);
	byte hora = tiempo/60;
	if (hora>24) hora =hora-24;
	sprintf(buffer2.aux,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",zonas->getNumeroZona(alarma),hora,tiempo%60,zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));
	enviaComando(buffer2.aux);
}

void GSM::establecerHoraInicio(controlZona *zonas, byte alarma){
	sprintf(buffer2.aux,"AT+CALD=%d;+CALA=\"%.2i:%.2i:00\",%d,%d",
			zonas->getNumeroZona(alarma),zonas->getHoraZona(alarma),
			zonas->getMinutoZona(alarma),zonas->getNumeroZona(alarma),zonas->getIntervaloZona(alarma));

	enviaComando(buffer2.aux);
}

void GSM::enviaSMSErrorPrincipal(void){
	enviaSMSError(1);
}

void GSM::iniciarRiegoZona(byte numeroAlarma){
	sprintf(buffer2.aux,"AT+SGPIO=0,%d,1,1",numeroAlarma);
	enviaComando(buffer2.aux);
}

void GSM::pararRiegoZona(byte numeroAlarma){
	sprintf(buffer2.aux,"AT+SGPIO=0,%d,1,0",numeroAlarma);
	enviaComando(buffer2.aux);
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
	return procesaEnviaComando();
}

char * GSM::enviaComando(const String &s){
	println(s);
	delay(100);
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
			byte i=0;
			while(i!=lectura){
				if (contador < MAX_BUFFER){
					bufferI[contador]=read();
					contador++;
				}
				i++;
			}
		}
	}while(millis()-tiempo<1000);
	contador=0;
    for (byte i = 0; i<MAX_BUFFER;i++){
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
	  buffer2.aux[contador]=bufferI[i];
	  contador++;
    }
    buffer2.aux[contador]=0x0;

#ifndef RELEASE_FINAL
    Serial.print(F("comando despues de procesar: ")); Serial.println(buffer2.aux);
#endif
	return buffer2.aux;
}

void GSM::limpiaBufferI(void){
	for (byte i=0;i<MAX_BUFFER;i++){
		bufferI[i]=0x0;
	}
}

void GSM::limpiaSMS(void){
	for (byte i=0;i<MAX_BUFFER_SMS;i++){
		buffer.SMS[i]=0x0;
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

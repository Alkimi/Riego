/*
 * controlZona.cpp
 *
 *  Created on: 15/2/2015
 *      Author: Alkimi
 */

#include "controlZona.h"
//#include "miEEPROM.h"

extern UBuffer2 buffer2;

controlZona::controlZona(Menu * menu,Botonera * botones){
	char *aux = buffer2.aux;
	char *aux2 = &buffer2.aux[16];
	byte posicion=0;
	myMenu=menu;
	botonera = botones;
	EEPROM.lecturaEeprom16(2,aux);
	EEPROM.lecturaEeprom16(3,aux2);

	for (byte i=0;i<NUMERO_ZONAS;i++){
		control[i].numeroZona =i;
		control[i].activa= aux[posicion++]==0x0?false:true;
		if (control[i].activa==true){
			zonasActivas++;
		}
		control[i].litrosPorRiego=aux[posicion++];
		control[i].horaInicio=aux[posicion++];
		control[i].minutoInicio=aux[posicion++];
		control[i].intervaloRiego=aux[posicion++];
		control[i].duracion=aux[posicion++];
		control[i].litrosTotales=0;
		control[i].tiempo=0;
		control[i].regando=false;
		control[i].reventon=false;
		control[i].manual=false;
	}
	control[0].activa=true;
	zonasRegando=0;
	zonasReventon=0;
	maxLitrosRiego=0;
	totalLitros=0;
	zonasManual=0;
	totalZonas=NUMERO_ZONAS;
	//activa = 0-1
	//litros = 100
	//hora = 20
	//mintuos = 0
	//intervalo = 0
	// tiempo = 60
}

controlZona::~controlZona() {
}

bool controlZona::isRegando(void){
	for (byte i=1;i<NUMERO_ZONAS;i++){
		if (control[i].regando){
			control[LLAVE_PRINCIPAL].regando=true;
			return true;
		}
	}
	control[LLAVE_PRINCIPAL].regando=false;
	return false;
}

bool controlZona::isManualZona(byte zona){
	return control[zona].manual;
}

bool controlZona::isRegandoZona(byte zona){
	return control[zona].regando;
}

bool controlZona::isReventonZona(byte zona){
	//leemos el varlo de la epron de la zona
	control[zona].reventon = (EEPROM.read(16+zona)!=0x0)?true:false;
	return control[zona].reventon;
}

bool controlZona::isReventon(void){
	for (byte i=1;i<NUMERO_ZONAS;i++){
		if (isReventonZona(i)){
			return true;
		}
	}
    return false;
}

void controlZona::setFinRiegoZona(byte zona){
	control[zona].litrosTotales=0;
	control[zona].regando=false;
	cuentaZonasRegando();
	isRegando();

}

bool controlZona::setIncrementaLitros(byte litros){
	bool reventon= false;
	if (zonasRegando>0){
		totalLitros+=litros;
		byte incremento = litros / zonasRegando;
		for (byte i=1;i<NUMERO_ZONAS;i++){
			if (control[i].activa && control[i].regando &&  !control[i].reventon){
				control[i].litrosTotales+=incremento;
				if (control[i].litrosTotales>control[i].litrosPorRiego){
					control[i].reventon=true;
					reventon= true;
					setRebentonZona(i,reventon);
				}
			}
		}
	}
	cuentaZonasReventon();
	return reventon;

}
void controlZona::setRebentonZona(byte zona,bool estado){
	//invertimos el valor de eprom
	//byte valor= EEPROM.read(16+zona);
	//(valor!=0x0)?valor=0x0:valor=0x1;
//	EEPROM.write(16+zona,estado);
}

void controlZona::setLitrosPorRiegoEnZona(byte zona, byte litros){
	control[zona].litrosPorRiego=litros;
	control[zona].activa=true;
	cuentaZonasActivas();

}

void controlZona::setManualZona(byte zona, bool valor,unsigned long tiempo){
	control[zona].manual=valor;
	control[zona].tiempo=tiempo;
	if ((valor == true) && (control[0].manual== false)){
			control[0].manual=true;
	}
	cuentaZonasManual();
	if ((valor == false) && (zonasManual==1)) {
		control[0].manual=false;
		zonasManual=0;
	}

}

void controlZona::setRegandoZona(byte zona, bool valor){
	control[zona].regando=valor;
	cuentaZonasRegando();
}

void controlZona::setReiniciaZona(byte zona){
	control[zona].regando = false;
	control[zona].reventon = false;
	setRebentonZona(zona,false);
	totalLitros-= control[zona].litrosTotales;
	if (totalLitros<0){
		totalLitros=0;
	}
	control[zona].litrosTotales=0;
	cuentaZonasRegando();
	cuentaZonasReventon();
}

void controlZona::cuentaZonasRegando(void){
	zonasRegando=0;
	maxLitrosRiego =0;
	for (byte i=1;i<NUMERO_ZONAS;i++){
		if (control[i].regando){
			zonasRegando++;
			maxLitrosRiego+=control[i].litrosPorRiego;
		}
	}
}

void controlZona::cuentaZonasActivas(void){
	zonasActivas=0;
	for (byte i=1;i<NUMERO_ZONAS;i++){
		if (control[i].activa){
			zonasActivas++;
		}
	}
}

byte controlZona::getHoraZona(byte zona) {
	return control[zona].horaInicio;
}

byte controlZona::getMinutoZona(byte zona) {
	return control[zona].minutoInicio;
}

byte controlZona::getDuracionZona(byte zona) {
	return control[zona].duracion;
}

byte controlZona::getIntervaloZona(byte zona) {
	return control[zona].intervaloRiego;
}

byte controlZona::getNumeroZona(byte zona) {
	return control[zona].numeroZona;
}

byte controlZona::getLitrosPorRiegoZona(byte zona){
	return control[zona].litrosPorRiego;
}

void controlZona::cuentaZonasReventon(void){
	zonasReventon=0;
	for (byte i=1;i<NUMERO_ZONAS;i++){

		if (control[i].reventon){
			zonasReventon++;
		}
	}
}

void controlZona::cuentaZonasManual(void){
	zonasManual=0;
	for (byte i=0;i<NUMERO_ZONAS;i++){
		if (control[i].manual!=0){
			zonasManual++;
		}
	}
}

bool controlZona::isTodasZonasRegando(void){
	return (zonasActivas==zonasRegando)?true:false;
}

bool controlZona::isTodasZonasReventon(void){
	return (zonasActivas==zonasReventon)?true:false;
}

bool controlZona::isMaxLitrosRiego(void){
	return (totalLitros> maxLitrosRiego)?true:false;
}

unsigned long controlZona::getTiempoZona(byte zona){
	return control[zona].tiempo;
}

byte controlZona::getNumeroZonasActivas(void){
	return zonasActivas;
}

int controlZona::getTotalLitros(void){
	return totalLitros;
}

int controlZona::getMaxLitrosRiego(void){
	return maxLitrosRiego;
}

byte controlZona::getNumeroZonasRiego(void){
	return totalZonas;
}

bool controlZona::isManual(void){
		return (zonasManual>0)?true:false;
}

bool controlZona::isZonaActiva(byte zona){
	return control[zona].activa;
}


void controlZona::mostrarConfigurarInformacionZonas(boolean tipo){
	boolean salida=false;
	byte zona =1;
	int key;
	unsigned long tiempoEspera=millis();
	byte maxZona=getNumeroZonasRiego()-1;
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(527,linea1); //Zonas activas
	EEPROM.leeCadenaEEPROM(559,linea2); //"               "
	linea1[14]=getNumeroZonasActivas()+48;
	myMenu->posicionActual(linea1,linea2);
	do {
		key=botonera->lecturaPulsador();
		if (key != -1) {
			tiempoEspera=millis();
			if (key == 0) {  // Se ha pulsado la tecla derecha
				//zona+1
				zona++;
				if (zona>maxZona){
					zona=1;
				}
			}
			if (key == 3) {  // Se ha pulsado la tecla izquierda
				//zona -1
				zona--;
				if (zona<1){
					zona=maxZona;
				}
			}
			if (key == 4) {  // Se ha pulsado la tecla de seleccion
				//salida
				salida=true;
			}
			if (!salida){
				//mostramos la informacion
				EEPROM.leeCadenaEEPROM(543,linea1); //Zona x act. NO
				linea1[5]=getNumeroZona(zona)+48;
				if (isZonaActiva(zona)==true){
						linea1[12]='S';
						linea1[13]='I';
						sprintf(linea2,"%.2i:%.2i %.3d %.3d",getHoraZona(zona),getMinutoZona(zona),
								getLitrosPorRiegoZona(zona),getDuracionZona(zona));
				}else{
					EEPROM.leeCadenaEEPROM(559,linea2); //"               "
				}
				if (tipo==false){
					botonera->cambioValor(linea1,linea2,12);
					if (isZonaActiva(zona)==true){
							linea1[12]='S';
							linea1[13]='I';
							actualizaDatosZona(zona);
					}else{
						EEPROM.leeCadenaEEPROM(559,linea2); //"               "
					}
				}
				myMenu->posicionActual(linea1,linea2);
				//Zona x activa: SI/NO
				//hora: xx:yy litros MAX: xxx Duraccion: xxx  Regando? SI/NO Rebenton? SI/NO Manual? SI/NO
			}
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
			}
		}
	}while (!salida);
}

void controlZona::actualizaDatosZona(byte zona){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	linea1="Zona activa ";
	linea1[13]=zona+48;
	linea2[14]=0x0;
	linea2="Hora: 00:00";
	botonera->cambioNumeroLimite(linea1,linea2,1);
	botonera->cambioNumeroLimite(linea1,linea2,2);

}




#ifndef RELEASE_FINAL
void controlZona::imprimirZonas(void){
	Serial.println();
	Serial.print(F("Zonas de riego activas totales: "));
	Serial.println(zonasActivas);
	for (byte i = 0;i<NUMERO_ZONAS;i++){
		Serial.print(F("Zona: "));	Serial.print(i);
		Serial.print(F(" esActiva: "));
		if (control[i].activa==true){
			Serial.println(F("SI"));
		}else{
			Serial.println(F("NO"));
					}
		Serial.println();
		if (control[i].activa){
			Serial.print(F("\tHora: "));Serial.print(control[i].horaInicio);
			Serial.print(F(":"));Serial.println(control[i].minutoInicio);
			Serial.print(F("\tDuracion: "));Serial.print(control[i].duracion);Serial.println(F(" minutos"));
			Serial.print(F("\tlitros maximos por riego: "));Serial.print(control[i].litrosPorRiego);
			Serial.print(F(" litros actuales: "));	Serial.println(control[i].litrosTotales);
			Serial.print(F("\tRegando?  ")); Serial.print(control[i].regando);
			Serial.print(F(" Reventon? "));	Serial.print(control[i].reventon);
			Serial.print(F(" Manual? ")); Serial.println(control[i].manual);
		}
	}
}
#endif

/*
 * zonasDeRiego.cpp
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#include "zonasDeRiego.h"
#include "miEEPROM.h"

zonasDeRiego::zonasDeRiego() {
	char aux[33];
	char * aux2= &aux[16];
	EEPROM.lecturaEeprom16(4,aux);
	EEPROM.lecturaEeprom16(5,aux2);
	numeroZonasRiego=(aux[0]-48);
	zonaDeRiego = new t_zonaRiego[numeroZonasRiego];
	byte posicion=1;
	for (int i =0;i<numeroZonasRiego;i++){
		zonaDeRiego[i].numeroZona=(aux[posicion++]-48);
		zonaDeRiego[i].litrosPorRiego=aux[posicion++];
		zonaDeRiego[i].horaInicio=(aux[posicion++]-48);
		zonaDeRiego[i].minutoInicio=(aux[posicion++]-48);
		zonaDeRiego[i].intervaloRiego=(aux[posicion++]-48);
		zonaDeRiego[i].duracion=(aux[posicion++]-48);
		zonaDeRiego[i].primeraVez=false;
	}
//E:4 2 4 d C N 0 5 2 d C R 0 5
//    2  4  d  C  N  5  <  2  d  1  T  0  <
	/*x  nz l  h  m  i  d  nz l  h  m  i  d  nz l  h
	  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

	  m  i  d  nz l  h  m  i  d  nz l  h  m  i  d*/
}

zonasDeRiego::~zonasDeRiego() {
	delete zonaDeRiego;
}

bool zonasDeRiego::getEstadoPrimeraVez(byte numeroAlarma){
	for (byte contador =0;contador<numeroZonasRiego;contador++){
		if (zonaDeRiego[contador].numeroZona==numeroAlarma){
			return zonaDeRiego[contador].primeraVez;
		}
	}
	return true;
}

void zonasDeRiego::setEstadoPrimeraVez(byte numeroAlarma){
	for (byte contador =0;contador<numeroZonasRiego;contador++){
			if (zonaDeRiego[contador].numeroZona==numeroAlarma){
				zonaDeRiego[contador].primeraVez = !zonaDeRiego[contador].primeraVez;
			}
		}

}


int zonasDeRiego::getNumeroZonasRiego(){
	return numeroZonasRiego;
}

t_zonaRiego * zonasDeRiego::getZonaDeRiego(byte posicion){
	return &zonaDeRiego[posicion];

}

t_zonaRiego * zonasDeRiego::getZonaDeRiegoAlarma(byte numeroAlarma){
	for (byte contador =0;contador<numeroZonasRiego;contador++){
			if (zonaDeRiego[contador].numeroZona==numeroAlarma){
				return &zonaDeRiego[contador];
			}
		}
	return &zonaDeRiego[numeroAlarma];

}

byte zonasDeRiego::litrosTotalesZonaRiego(byte numeroZona){
	return zonaDeRiego[numeroZona].litrosPorRiego;
}

#ifndef RELEASE_FINAL
void zonasDeRiego::imprimirZonas(void){
	Serial<<endl <<F("Zonas de riego totales: ") << numeroZonasRiego << endl;
	for (int i = 0;i<numeroZonasRiego;i++){
		Serial <<F("Zona ")<<i<<F(" en memoria numero: ")<<zonaDeRiego[i].numeroZona<<endl;
		Serial <<F("\tHora: ")<<zonaDeRiego[i].horaInicio<<F(" minutos: ")<<zonaDeRiego[i].minutoInicio<<endl;
		Serial <<F("\tDuracion: ")<<zonaDeRiego[i].duracion<<F(" minutos")<<F(" litros por riego: ") << zonaDeRiego[i].litrosPorRiego<<endl;
		Serial <<F("\tPrimera vez: ")<<zonaDeRiego[i].primeraVez <<endl<<endl;
	}
}
#endif

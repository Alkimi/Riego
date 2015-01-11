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
	numeroZonasRiego=aux[0];
	zonaDeRiego = new t_zonaRiego[numeroZonasRiego];
	byte posicion=1;
	for (int i =0;i<numeroZonasRiego;i++){
		zonaDeRiego[i].numeroZona=aux[posicion++];
		zonaDeRiego[i].litrosPorRiego=aux[posicion++];
		zonaDeRiego[i].horaInicio=aux[posicion++];
		zonaDeRiego[i].minutoInicio=aux[posicion++];
		zonaDeRiego[i].intervaloRiego=aux[posicion++];
		zonaDeRiego[i].duracion=aux[posicion++];
		zonaDeRiego[i].primeraVez=false;
	}
	/*x  nz l  h  m  i  d  nz l  h  m  i  d  nz l  h
	  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

	  m  i  d  nz l  h  m  i  d  nz l  h  m  i  d*/
}

zonasDeRiego::~zonasDeRiego() {
	delete zonaDeRiego;
}

bool zonasDeRiego::getEstadoPrimeraVez(byte posicion){
	return zonaDeRiego[posicion].primeraVez;
}

void zonasDeRiego::setEstadoPrimeraVez(byte posicion){
	zonaDeRiego[posicion].primeraVez = !zonaDeRiego[posicion].primeraVez;
}


int zonasDeRiego::getNumeroZonasRiego(){
	return numeroZonasRiego;
}

t_zonaRiego * zonasDeRiego::getZonaDeRiego(byte posicion){
	return &zonaDeRiego[posicion];
}

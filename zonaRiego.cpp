/*
 * zonaRiego.cpp
 *
 *  Created on: 4/1/2015
 *      Author: Alkimi
 */

#include "zonaRiego.h"

zonaRiego::zonaRiego(){

}

zonaRiego::zonaRiego(byte numeroZona, int litrosPorRiego,
		unsigned long intervaloRiego,unsigned long horaInicio) {
	this->numeroZona = numeroZona;
	this->litrosPorRiego = litrosPorRiego;
	this->intervaloRiego = intervaloRiego;
	this->horaInicio = horaInicio;
}


zonaRiego::~zonaRiego() {
	// TODO Auto-generated destructor stub
}

unsigned long zonaRiego::getIntervaloRiego() const {
	return intervaloRiego;
}


int zonaRiego::getLitrosPorRiego() const {
	return litrosPorRiego;
}


byte zonaRiego::getNumeroZona() const {
	return numeroZona;
}

unsigned long zonaRiego::getHoraInicio() const {
	return horaInicio;
}


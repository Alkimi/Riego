/*
 * zonaRiego.h
 *
 *  Created on: 4/1/2015
 *      Author: Alkimi
 */

#ifndef ZONARIEGO_H_
#define ZONARIEGO_H_
#include "Riego.h"

class zonaRiego {
public:
	zonaRiego();
	zonaRiego(byte numeroZona,int litrosPorRiego, unsigned long intervaloRiego,unsigned long horaInicio);
	virtual ~zonaRiego();
	unsigned long getIntervaloRiego() const;
	unsigned long getHoraInicio() const;
	int getLitrosPorRiego() const;
	byte getNumeroZona() const;

private:
	byte numeroZona;
	int litrosPorRiego;
	unsigned long intervaloRiego;
	unsigned long horaInicio;
};

#endif /* ZONARIEGO_H_ */

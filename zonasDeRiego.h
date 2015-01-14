/*
 * zonasDeRiego.h
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#ifndef ZONASDERIEGO_H_
#define ZONASDERIEGO_H_
#include "Riego.h"

class zonasDeRiego {
public:
	zonasDeRiego();
	virtual ~zonasDeRiego();
	int getNumeroZonasRiego();
	t_zonaRiego * getZonaDeRiego(byte posicion);
	bool getEstadoPrimeraVez(byte posicion);
	void setEstadoPrimeraVez(byte posicion);
#ifndef RELEASE_FINAL
	void imprimirZonas(void);
#endif
private:
	t_zonaRiego *zonaDeRiego;
	int numeroZonasRiego;
};

#endif /* ZONASDERIEGO_H_ */

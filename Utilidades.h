/*
 * Utilidades.h
 *
 *  Created on: 18/4/2015
 *      Author: Alkimi
 */

#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#define DEBUG_UTIL

class Utilidades {
public:
	Utilidades();
	virtual ~Utilidades();
	int freeRam();						//*

};

extern Utilidades util;

#endif /* UTILIDADES_H_ */

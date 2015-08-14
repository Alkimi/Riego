/*
 * Utilidades.cpp
 *
 *  Created on: 18/4/2015
 *      Author: Alkimi
 */

#include "Utilidades.h"
#ifdef DEBUG_UTIL
#endif

Utilidades::Utilidades() {

}

Utilidades::~Utilidades() {
}

int Utilidades::freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}





Utilidades util;

/*
 * Botonera.h
 *
 *  Created on: 11/8/2015
 *      Author: Alkimi
 */

#ifndef BOTONERA_H_
#define BOTONERA_H_
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Menu.h"    //28050 820

#define CINCOSEGUNDOS 5000
#define BOTONERA 0
#define NUM_KEYS 5

#define NO_KEY -1
#define KEY_DERECHA 0
#define KEY_ARRIBA 1
#define KEY_ABAJO 2
#define KEY_IZQUIERDA 3
#define KEY_SELECT 4

#define ADC_KEY_DERECHA 50
#define ADC_KEY_ARRIBA 200
#define ADC_KEY_ABAJO 400
#define ADC_KEY_IZQUIERDA 600
#define ADC_KEY_SELECT 800

class Botonera {
public:
	Botonera(Menu * menu);
	bool cambioValor(char *linea1,char *linea2,byte opcion);
	int lecturaPulsador(void);
	virtual ~Botonera();

private:
	Menu * myMenu;
	void cambioValorOpcion(char *linea1,char *linea2,byte opcion,byte actual,byte posicion);
	byte desplazamiento(byte opcion, byte posicion, boolean derecha);
	int get_key(unsigned int input);
	byte getLimite(byte opcion);
	byte getContadorInicio(char *linea1,char *linea2,byte opcion);
};

#endif /* BOTONERA_H_ */

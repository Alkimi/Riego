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
#include "Menu.h"    //28100 840

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

	int lecturaPulsador(void);

	//void cambioNumeroSN(char *linea1,char *linea2,byte opcion);
	//void cambioNumero(char *linea1, char* linea2,byte opcion);
	void cambioNumeroLimite(char *linea1,char *linea2,byte opcion);

	void cambioValor(char *linea1,char *linea2,byte opcion);

	virtual ~Botonera();


private:
	int keyNumero;
	Menu * myMenu;
	int get_key(unsigned int input);
	byte getLimite(byte opcion);
	void cambioValorOpcion(char *linea1,char *linea2,byte opcion,byte actual,byte posicion);
	byte desplazamiento(byte opcion, byte posicion, boolean derecha);
};

#endif /* BOTONERA_H_ */

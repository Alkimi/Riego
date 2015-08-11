/*
 * Botonera.cpp
 *
 *  Created on: 11/8/2015
 *      Author: Alkimi
 */

#include "Botonera.h"

Botonera::Botonera(Menu * menu) {
	// TODO Auto-generated constructor stub
	myMenu = menu;
}

// Convertimos el valor leido en analogico en un numero de boton pulsado
int Botonera::get_key(unsigned int input) {
	if (input < ADC_KEY_ARRIBA){
		return KEY_ARRIBA;
	}else{
		if (input < ADC_KEY_ABAJO){
			return KEY_ABAJO;
		}else{
			if (input < ADC_KEY_DERECHA){
			   return KEY_DERECHA;
			}else{
				if (input < ADC_KEY_IZQUIERDA){
					return KEY_IZQUIERDA;
				}else{
					if (input < ADC_KEY_SELECT){
						return KEY_SELECT;
					}else{
						return NO_KEY;
					}
				}
			}
		}
	}
}

int Botonera::lecturaPulsador(void){
	keyNumero = get_key(analogRead(BOTONERA));    // Obtenemos el boton pulsado
	if (keyNumero != NO_KEY) {  // if keypress is detected
		delay(100);  // Espera para evitar los rebotes de las pulsaciones
		keyNumero = get_key(analogRead(BOTONERA)); // Obtenemos el boton pulsado
		if (keyNumero != NO_KEY) {
			return keyNumero;
		}
	}
	return keyNumero;
}

/*
//correcto
void Botonera::cambioNumero(char *linea1,char* linea2,byte opcion) {
	bool salida = false;
	boolean fechaHora=false;
	unsigned long tiempoEspera=millis();
	char caracter;
	byte xmax, xmin;
	byte xNumero;
	byte columna;
	int keyNumero;

	char * linea;

	xmax = 14;
	xmin = 7;
	linea = linea2;
	columna = 0;

	switch (opcion) {
	case 0:
		xmin = 7;
		xmax = 10;
		columna = 1;
		break;
	case 1:
		xmin = 3;
		xmax = 11;
		columna = 1;
		break;
	case 2:
		fechaHora=true;
		myMenu->borraLinea2();
		linea = linea1;
		break;
	case 3:
		fechaHora=true;
		myMenu->borraLinea1();
		myMenu->borraLinea2();
		myMenu->linea1(linea);
		break;
	}

	myMenu->SetCursor(xmin, columna);
	caracter = linea[xmin];
	xNumero = xmin;
	do {
		keyNumero=lecturaPulsador();
		if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
			tiempoEspera=millis();
			myMenu->SetCursor(xNumero, columna);
			if (keyNumero == 0) {  // Se ha pulsado la tecla derecha
				xNumero++;
				if ((fechaHora==true) && ((xNumero == 9) || (xNumero == 12)))
					xNumero++;
				if (xNumero > xmax)
					xNumero = xmin;
				caracter = linea[xNumero];
			}
			if (keyNumero == 1) {   // Se ha pulsado la tecla arriba
					caracter--;
					if (caracter < '0')
						caracter = '9';
					linea[xNumero] = caracter;
					myMenu->write(caracter);

			}
			if (keyNumero == 2) {  // Se ha pulsado la tecla abajo
					caracter++;
					if (caracter > '9')
						caracter = '0';
					linea[xNumero] = caracter;
					myMenu->write(caracter);
			}

			if (keyNumero == 3) {  // Se ha pulsado la tecla izquierda
				xNumero--;
				if ((fechaHora==true) && ((xNumero == 9) || (xNumero == 12)))
					xNumero--;
				if (xNumero < xmin)
					xNumero = xmax;
				caracter = linea[xNumero];
			}
			if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
				salida = true;
			}
			myMenu->SetCursor(xNumero, columna);
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
			}
		}
	} while (!salida);
}*/

/*

void Botonera::cambioNumeroSN(char *linea1,char *linea2,byte opcion){
	bool salida = false;
	unsigned long tiempoEspera=millis();
		char caracter;
		int keyNumero;
		byte myX;
		myMenu->borraLinea2();
		if (opcion==1){
			myX=11;
		}
		if (opcion==2){
			myX=12;
		}
		myMenu->SetCursor(myX, 0);
		caracter = linea1[myX];
		do {
			keyNumero=lecturaPulsador();
			if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
				tiempoEspera=millis();
				myMenu->SetCursor(myX, 0);
				if ((keyNumero == 1) || (keyNumero == 2)) {   // Se ha pulsado la tecla arriba o abajo
					if (caracter=='S'){
						myMenu->print("NO");
						linea1[myX] = 'N';
						linea1[myX+1] = 'O';
						caracter='N';
					}else{
						myMenu->print("SI");
						linea1[myX] = 'S';
						linea1[myX+1] = 'I';
						caracter='S';
					}
				}
				if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
					salida = true;
				}
			}else{
				if (millis()-tiempoEspera> CINCOSEGUNDOS){
					salida=true;
				}
			}
		} while (!salida);
}*/


void Botonera::cambioNumeroLimite(char *linea1,char *linea2,byte opcion){
	bool salida = false;
	unsigned long tiempoEspera=millis();
	byte contador = 0;
	byte limite =0;
	char * cadena;
		int keyNumero;
		byte myX;
		if (opcion==1){
			limite=23;
			myX=6;
		}
		if (opcion==2){
			limite=59;
			myX=9;
		}
		if (opcion==3){
			limite=255;
		}
		myMenu->SetCursor(myX, 0);
		cadena = &linea1[myX-1];
		do {
			keyNumero=lecturaPulsador();
			if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
				tiempoEspera=millis();
				myMenu->SetCursor(myX, 1);
				if (keyNumero == 1) {
					contador++;
					if (contador> limite) {
						contador=0;
					}
				}
				if (keyNumero == 2){
					contador--;
					if (contador<0) {
						contador=limite;
					}
				}
				sprintf(cadena,"%.2i",contador);

				if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
					salida = true;
				}
			}else{
				if (millis()-tiempoEspera> CINCOSEGUNDOS){
					salida=true;
				}
			}
		} while (!salida);
}

void Botonera::cambioValorOpcion(char *linea1,char *linea2,byte opcion,byte actual,byte posicion){
	//tipo si-no
	if (opcion<20){
		//byte myX=posicion;
		myMenu->borraLinea2();
		/*if (opcion==11){
			myX=11;
		}
		if (opcion==12){
			myX=12;
		}*/
		myMenu->SetCursor(posicion, 0);
		if (actual==0){
			myMenu->print("NO");
			linea1[posicion] = 'N';
			linea1[posicion+1] = 'O';
		}else{
			myMenu->print("SI");
			linea1[posicion] = 'S';
			linea1[posicion+1] = 'I';
		}
	}

	//tipo numerico 0-9
	if (opcion>20 && opcion<30){

		char caracter;
		byte columna =0;
		char * linea;

		linea = linea2;

		switch (opcion) {
			case 20:
			case 21:
				columna = 1;
				break;
			case 22:
				myMenu->borraLinea2();
				linea = linea1;
				break;
			case 23:
				myMenu->borraLinea1();
				myMenu->borraLinea2();
				myMenu->linea1(linea);
				break;
		}
		myMenu->SetCursor(posicion, columna);
		caracter = actual+48;
		linea[posicion] = caracter;
		myMenu->write(caracter);
	}

	//
}


byte Botonera::getLimite(byte opcion){
	//tipo si-no
	if (opcion<20){
		return 1;
	}

	//tipo numerico 0-9
	if (opcion>20 && opcion<30){
		return 9;
	}
	//
	if (opcion>30) {
		return 10;
	}
}

byte Botonera::desplazamiento(byte opcion, byte posicion, boolean derecha){
	byte posicionDestino=posicion;

	if (opcion>20 && opcion <30){
		boolean fechaHora=true;

		byte xmax = 14;
		byte xmin = 7;
		switch (opcion) {
			case 20:
				xmax = 10;
				fechaHora=false;
				break;
			case 21:
				xmin = 3;
				xmax = 11;
				fechaHora=false;
				break;
		}
		if (derecha==true){
			posicionDestino++;
			if ((fechaHora==true) && ((posicionDestino == 9) || (posicionDestino == 12)))
				posicionDestino++;
			if (posicionDestino > xmax)
				posicionDestino = xmin;
		}else{
			posicionDestino--;
			if ((fechaHora==true) && ((posicionDestino == 9) || (posicionDestino == 12)))
				posicionDestino--;
			if (posicionDestino < xmin)
				posicion = xmax;
		}
	}
	return posicionDestino;
}

void Botonera::cambioValor(char *linea1,char *linea2,byte opcion){
	bool salida = false;
	unsigned long tiempoEspera=millis();
	byte contador=0;
	byte posicion=0;
	byte limite = getLimite(opcion);
	do {
		keyNumero=lecturaPulsador();
		if (keyNumero != NO_KEY) {
			tiempoEspera=millis();
			///myMenu->SetCursor(myX, 1);
			if (keyNumero == KEY_DERECHA) {  // Se ha pulsado la tecla derecha
				posicion=desplazamiento(opcion,posicion,true);
			}
			if (keyNumero == KEY_ARRIBA) {
				contador++;
				if (contador> limite) {
					contador=0;
				}
				cambioValorOpcion(linea1,linea2,opcion,limite,posicion);
			}
			if (keyNumero == KEY_ABAJO){
				contador--;
				if (contador<0) {
					contador=limite;
				}
				cambioValorOpcion(linea1,linea2,opcion,limite,posicion);
			}
			//sprintf(cadena,"%.2i",contador);
			if (keyNumero == KEY_IZQUIERDA) {  // Se ha pulsado la tecla izquierda
				posicion=desplazamiento(opcion,posicion,false);
			}

			if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
				salida = true;
			}
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
			}
		}
	} while (!salida);
}


Botonera::~Botonera() {
	// TODO Auto-generated destructor stub
}


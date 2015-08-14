/*
 * Botonera.cpp
 *
 *  Created on: 11/8/2015
 *      Author: Alkimi
 */

#include "Botonera.h"
#define RELEASE

Botonera::Botonera(Menu * menu) {
	myMenu = menu;
}


bool Botonera::cambioValor(char *linea1,char *linea2,byte opcion){
#ifndef RELEASE
	Serial.println(F("dentro de Botonera::cambioValor "));
#endif
	bool salida = false;
	bool tiempo= true;
	unsigned long tiempoEspera=millis();
	int keyNumero;
	int contador=getContadorInicio(linea1,linea2,opcion);
	byte posicion=desplazamiento(opcion,0,true,true);
	byte limite = getLimite(opcion);
#ifndef RELEASE
	Serial.print(F("limite: "));Serial.print(limite);Serial.print(F(" posicion: "));Serial.print(posicion);Serial.print(F(" opcion: "));Serial.println(opcion);
#endif
	do {
		keyNumero=lecturaPulsador();
		if (keyNumero != NO_KEY) {
			tiempoEspera=millis();
			///myMenu->SetCursor(myX, 1);
			if (keyNumero == KEY_DERECHA) {  // Se ha pulsado la tecla derecha
				posicion=desplazamiento(opcion,posicion,true,false);
			}
			if (keyNumero == KEY_ARRIBA) {
				contador++;
				if (contador> limite) {
					contador=0;
				}
				cambioValorOpcion(linea1,linea2,opcion,contador,posicion);
			}
			if (keyNumero == KEY_ABAJO){
				contador--;
				if (contador<0) {
					contador=limite;
				}
				cambioValorOpcion(linea1,linea2,opcion,contador,posicion);
			}
			//sprintf(cadena,"%.2i",contador);
			if (keyNumero == KEY_IZQUIERDA) {  // Se ha pulsado la tecla izquierda
				posicion=desplazamiento(opcion,posicion,false,false);
			}

			if (keyNumero == KEY_SELECT) { // Se ha pulsado la tecla de seleccion
				salida = true;
			}
#ifndef RELEASE
	Serial.print(F("limite: "));Serial.print(limite);Serial.print(F(" posicion: "));Serial.print(posicion);Serial.print(F(" opcion: "));Serial.println(opcion);
#endif
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
				tiempo=false;
			}
		}
	} while (!salida);
#ifndef RELEASE
	Serial.println(F("fuera de Botonera::cambioValor "));
#endif
	return tiempo;
}

int Botonera::lecturaPulsador(void){
	int keyNumero;
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
		char * linea = linea2;

		if (opcion==20 || opcion ==21){
			columna = 1;
		}else{
			if (opcion==22){
				myMenu->borraLinea2();
				linea = linea1;
			}else{ //23
				myMenu->borraLinea1();
				myMenu->borraLinea2();
				myMenu->linea1(linea);
			}
		}
		myMenu->SetCursor(posicion, columna);
		caracter = actual+48;
		linea[posicion] = caracter;
		myMenu->write(caracter);
	}

	// tipo hora 23
	if (opcion==31){
		myMenu->SetCursor(posicion,1);
		linea2[6]= (actual / 10)+48;
		myMenu->write(linea2[6]);
		linea2[7]= (actual % 10)+48;
		myMenu->write(linea2[7]);
	}

	// tipo minuto 59
	if (opcion==32){
		myMenu->SetCursor(posicion,1);
		linea2[9]= (actual / 10)+48;
		myMenu->write(linea2[9]);
		linea2[10]= (actual % 10)+48;
		myMenu->write(linea2[10]);
	}

	// tipo 255
	if (opcion==33){
		myMenu->SetCursor(posicion,1);
		linea2[9]= (actual / 100)+48;
		myMenu->write(linea2[9]);
		byte aux = actual % 100;
		linea2[10]= (aux / 10)+48;
		myMenu->write(linea2[10]);
		linea2[11]= (aux % 10)+48;
		myMenu->write(linea2[11]);
	}

	if (opcion==34){
		char caracter;
		myMenu->SetCursor(posicion, 1);
		caracter = actual+49; // para que empieze en 1 en lugar de 0
		linea2[posicion] = caracter;
		myMenu->write(caracter);
	}
}

byte Botonera::desplazamiento(byte opcion, byte posicion, boolean derecha, boolean inicial){
	byte posicionDestino=posicion;

	if (opcion<20){
		return opcion;
	}

	if (opcion>20 && opcion <40){
		boolean fechaHora=true;

		byte xmax = 14;
		byte xmin = 7;

		if (opcion==20){
			xmax = 10;
			fechaHora=false;
		}else{
			if (opcion==21){
				xmin = 3;
				xmax = 11;
				fechaHora=false;
			}else{
				if (opcion==31){
					return 6;
				}else{
					if ((opcion==32) || (opcion ==33)){
						return 9;
					}else{
						if (opcion==34){
							return 14;
						}
					}
				}
			}
		}
		if (inicial==true){
			return xmin;
		}else{
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
	}
	return posicionDestino;
}

byte Botonera::getContadorInicio(char *linea1,char *linea2,byte opcion){
	//tipo si-no
	if (opcion<20){
		if (linea1[opcion]=='N'){
			return 0;
		}else{
			return 1;
		}
	}

	//tipo numerico 0-9
	if ((opcion==20)|| (opcion==23)){
		return linea2[7]-48;
	}

	if (opcion==21){
		return linea2[3]-48;
	}

	if (opcion==22){
		return linea1[7]-48;
	}

	if (opcion==31){
		return (((linea2[6]-48)*10)+(linea2[7]-48));
	}

	if (opcion==32){
		return (((linea2[9]-48)*10)+(linea2[10]-48));
	}

	if (opcion==33){
		return ((((linea2[9]-48)*100)+(linea2[10]-48)*10)+(linea2[11]-48));
	}

	if (opcion==34){
		return linea2[14]-49;
	}

	return 0;
}


// Convertimos el valor leido en analogico en un numero de boton pulsado
int Botonera::get_key(unsigned int input) {
	if (input < ADC_KEY_DERECHA ){
		return KEY_DERECHA ;
	}else{
		if (input < ADC_KEY_ARRIBA ){
			return KEY_ARRIBA ;
		}else{
			if (input < ADC_KEY_ABAJO ){
			   return KEY_ABAJO ;
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
	if (opcion==31) {
		return 23;
	}

	if (opcion==32){
		return 59;
	}

	if (opcion==33){
		return 255;
	}

	if (opcion==34){
		return 4;
	}
	return 0;
}

Botonera::~Botonera() {
	// TODO Auto-generated destructor stub
}


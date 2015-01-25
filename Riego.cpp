#include <Arduino.h>
#include "Riego.h"
#include "GSM.H"
#include "Menu.h"
#include "zonasDeRiego.h"
#include "miEEPROM.h"

const byte numeroDeMenus = 5;
const byte numeroMaximoDeSubmenus = 4;

//FLASH_STRING_ARRAY(tituloMenu,PSTR("1 Configuración"),PSTR("2 Información"), PSTR("3  Menu 3"),PSTR("4  Menu 4"),PSTR("5  Menu 5"));
const char tituloMenu[numeroDeMenus][17] = { "1 Configuracion", "2 Infomacion",
		"3  Menu 3       ", "4  Menu 4       ", "5  Menu 5       " };

byte numeroDeSubmenus[numeroDeMenus] = { 4, 2, 2, 1, 4 };

/*FLASH_STRING_ARRAY(tituloSubmenu,PSTR("1.1 xxxxxxxxxx"),PSTR("1.2 Destino SMS"),PSTR("1.3 Fecha"),PSTR("1.4 Hora"),
 PSTR("2.1 Fecha/Hora"),PSTR("2.2 Destino SMS"),PSTR(""),PSTR(""),
 PSTR("3.1 Submenu 1"),PSTR("3.2 Submenu 2"),PSTR(""),PSTR(""),
 PSTR("4.1 Submenu 1"),PSTR(""),PSTR(""),PSTR(""),
 PSTR("5.1 Submenu 1"),PSTR("5.2 Submenu 2"),PSTR("5.3 Submenu 3"),PSTR("5.4 Submenu 4"));
 */
const char tituloSubmenu[numeroDeMenus * numeroMaximoDeSubmenus][17] = {
		"1.1 xxxxxxxxxx", "1.2 Destino SMS", "1.3 Fecha", "1.4 Hora",
		"2.1 Fecha/Hora", "2.2 Destino SMS", "", "", "3.1 Submenu 1",
		"3.2 Submenu 2", "", "", "4.1 Submenu 1", "", "", "", "5.1 Submenu 1",
		"5.2 Submenu 2", "5.3 Submenu 3", "5.4 Submenu 4" };

unsigned int adc_key_val[5] = { 50, 200, 400, 600, 800 };
char NUM_KEYS = 5;
int adc_key_in;
byte key = 255;
byte oldkey = 255;
boolean luzEncendida = true;
boolean cursorActivo = false;
unsigned long tiempo;
unsigned long tiempo2;
byte x = 0;
byte y = 0;
String tratarRespuesta;
char* cadena;
char aux[33];
Menu myMenu("", numeroDeMenus);
GSM gprs;
zonasDeRiego zonas;

char linea1[16];
char linea2[16];

// Convertimos el valor leido en analogico en un numero de boton pulsado
byte get_key(unsigned int input) {
	byte k;

	for (k = 0; k < NUM_KEYS; k++) {
		if (input < adc_key_val[k]) {
			return k;
		}
	}

	if (k >= NUM_KEYS)
		k = 255;  // Error en la lectura
	return k;
}

void controlTiempo(void) {
	if (millis() - tiempo > 15000) { // a los 15 segundos apagamos el display
		myMenu.noDisplay();
	}
	// Si han pasado mas de 10 segundos apagamos la luz
	if (millis() - tiempo > 10000) {
		pinMode(10, OUTPUT);
		digitalWrite(10, LOW);
		luzEncendida = false;
	}
	// Si han pasado mas de 5 segundos apagamos el cursor
	if (millis() - tiempo > 5000) {
		myMenu.noBlink();
		cursorActivo = false;
	}
}

void getFechaHora(void) {
	strcpy_P(linea1, PSTR("Fecha: "));
	strcpy_P(linea2, PSTR("Hora : "));
	cadena = gprs.enviaComando(F("AT+CCLK?"));
	linea1[7] = cadena[14];
	linea1[8] = cadena[15];
	linea1[9] = cadena[13];
	linea1[10] = cadena[11];
	linea1[11] = cadena[12];
	linea1[12] = cadena[10];
	linea1[13] = cadena[8];
	linea1[14] = cadena[9];
	cadena += 17;
	cadena[8] = 0;
	strcat(linea2, cadena);
	/*
	 +CCLK: "15/01/11,16:56:39+02"
	 0123456789012345678901234567890
	 0         1         2         3*/
}

bool setFechaHora(byte opcion) {
	bool salida = true;
	sprintf(aux, "AT+CCLK=\"%c%c/%c%c/%c%c,%c%c:%c%c:%c%c+02\"", linea1[13],
			linea1[14], linea1[10], linea1[11], linea1[7], linea1[8], linea2[7],
			linea2[8], linea2[10], linea2[11], linea2[13], linea2[14]);
	cadena = gprs.enviaComando(aux);
	if (cadena == NULL) {
		myMenu.noBlink();
		if (opcion == 2) {
			myMenu.linea2(F("Fecha erronea"));
		} else {
			myMenu.linea2(F("Hora  erronea"));
		}
		salida = false;
		delay(3000);
		myMenu.borraLinea2();
		myMenu.blink();
	}
	return salida;

}

void tratarOpcion(byte x, byte y) {
	byte opcion = (x * numeroMaximoDeSubmenus) + y;
#ifndef RELEASE
	Serial << endl << F("Entrando en tratar opcion") << endl
			<< F("Memoria libre: ") << freeRam() << endl << F("X: ") << x
			<< F(" Y: ") << y << F(" opcion: ") << opcion << endl;
#endif
	switch (opcion) {
	case 1:
		getSMS();
		cambioNumero(opcion);
		setSMS();
		break;
	case 2:
	case 3:
	case 4:
		myMenu.noBlink();
		getFechaHora();
		myMenu.posicionActual(linea1, linea2);
		if ((opcion == 2) || (opcion == 3)) {
			myMenu.blink();
			do {
				cambioNumero(opcion);
			} while (setFechaHora(opcion) == false);
		} else {
			delay(3000);
		}
		myMenu.blink();
		break;
	case 5:
		myMenu.noBlink();
		getSMS();
		delay(3000);
		myMenu.blink();
		break;
	}
#ifndef RELEASE
	Serial << F("saliendo de tratar opcion") << endl << F("Memoria libre: ")
			<< freeRam() << endl;
#endif
}

void cambioNumero(byte opcion) {
	bool salida = false;
	char caracter;
	byte oldkeyNumero = 255;
	byte xmax, xmin;
	byte xNumero;
	byte columna;
	int adc_key_inNumero;
	byte keyNumero = 255;
	char * linea;

	xmax = 14;
	xmin = 7;
	linea = linea2;
	columna = 0;

	switch (opcion) {
	case 1:
		xmin = 3;
		xmax = 11;
		columna = 1;
		break;
	case 2:
		myMenu.borraLinea2();
		linea = linea1;
		break;
	case 3:
		myMenu.borraLinea1();
		myMenu.borraLinea2();
		myMenu.linea1(linea);
		break;
	}
	myMenu.SetCursor(xmin, columna);
	caracter = linea[xmin];
	xNumero = xmin;
	delay(100);
	do {
		adc_key_inNumero = analogRead(0);    // Leemos el valor de la pulsacion
		keyNumero = get_key(adc_key_inNumero);    // Obtenemos el boton pulsado
		if (keyNumero != oldkeyNumero) {  // if keypress is detected
			delay(50);  // Espera para evitar los rebotes de las pulsaciones
			adc_key_inNumero = analogRead(0); // Leemos el valor de la pulsacion
			keyNumero = get_key(adc_key_inNumero); // Obtenemos el boton pulsado
			if (keyNumero != oldkeyNumero) {
				oldkeyNumero = keyNumero;
				if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
					myMenu.SetCursor(xNumero, columna);
#ifndef RELEASE
					Serial << F("entrada Tecla pulsada: ") << keyNumero << endl
							<< F(" x= ") << xNumero << F(" caracter: ")
							<< caracter << F(" linea: ") << linea << endl;
#endif
					if (!luzEncendida) {
						pinMode(10, INPUT);
						luzEncendida = true;
						cursorActivo = true;
						myMenu.blink(); // Mostramos el cursor parpadeando
						myMenu.display(); //encendemos la pantalla
					}

					if (keyNumero == 0) {  // Se ha pulsado la tecla derecha
						xNumero++;
						if ((opcion > 1) && ((xNumero == 9) || (xNumero == 12)))
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
						myMenu.write(caracter);
					}
					if (keyNumero == 2) {  // Se ha pulsado la tecla abajo
						caracter++;
						if (caracter > '9')
							caracter = '0';
						linea[xNumero] = caracter;
						myMenu.write(caracter);
					}

					if (keyNumero == 3) {  // Se ha pulsado la tecla izquierda
						xNumero--;
						if ((opcion > 1) && ((xNumero == 9) || (xNumero == 12)))
							xNumero--;
						if (xNumero < xmin)
							xNumero = xmax;
						caracter = linea[xNumero];
					}
					if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
						salida = true;
					}
					myMenu.SetCursor(xNumero, columna);
#ifndef RELEASE
					Serial << F("SALIDA Tecla pulsada: ") << keyNumero << endl
							<< F(" x= ") << xNumero << F(" caracter: ")
							<< caracter << F(" linea: ") << linea << endl;
#endif
				}
			}
		}
	} while (!salida);
}

void setSMS(void) {
	EEPROM.escrituraEeprom16(0, linea2);
	delay(3000);
}

void getSMS(void) {
	strcpy_P(linea1, PSTR("Destino SMS:"));
	EEPROM.lecturaEeprom16(0, linea2);
	myMenu.posicionActual(linea1, linea2);
}

void setup() {
	Serial.begin(4800);
#ifndef RELEASE_FINAL
	Serial << F("Memoria libre: ") << freeRam() << endl
			<< F("------------------------") << endl;
	Serial << F("Riego Total V ") << RIEGO_VERSION << endl << F("Menu V ")
			<< myMenu.libVer() << endl << F("GPRS V ") << gprs.libVer() << endl;
#endif
	myMenu.inicia(gprs.libVer());
	myMenu.posicionActual(tituloMenu[x],
			tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
#ifndef RELEASE_FINAL
	zonas.imprimirZonas();
#endif
	gprs.inicializaAlarmas(&zonas);
	tiempo = millis();

}

void loop() {
	comandoGPRS();
	controlTiempo();
	adc_key_in = analogRead(0);    // Leemos el valor de la pulsacion
	key = get_key(adc_key_in);    // Obtenemos el boton pulsado

	if (key != oldkey) {  // if keypress is detected

		delay(50);  // Espera para evitar los rebotes de las pulsaciones
		adc_key_in = analogRead(0);    // Leemos el valor de la pulsacion
		key = get_key(adc_key_in);    // Obtenemos el boton pulsado
		if (key != oldkey) {
			tiempo = millis();
			if (!luzEncendida) { // Al pulsar cualquier tecla encendemos la pantalla
				pinMode(10, INPUT);
				luzEncendida = true;
			} else { // si la pantalla esta encendida seguimos funcionando normalmente
				oldkey = key;
				if (key >= 0 && key < NUM_KEYS) { // Si se ha pulsado cualquier tecla
					myMenu.blink(); // Mostramos el cursor parpadeando
					myMenu.display(); //encendemos la pantalla
					cursorActivo = true;
				}
				if (key == 0) {  // Se ha pulsado la tecla derecha
					x++;
					if (x > numeroDeMenus - 1)
						x = 0;
					y = 0;
				}
				if (key == 1) {   // Se ha pulsado la tecla arriba
					y--;
					if (y > NUM_KEYS)
						y = numeroDeSubmenus[x] - 1;
				}
				if (key == 2) {  // Se ha pulsado la tecla abajo
					y++;
					if (y > numeroDeSubmenus[x] - 1)
						y = 0;
				}

				if (key == 3) {  // Se ha pulsado la tecla izquierda
					x--;
					if (x > NUM_KEYS)
						x = numeroDeMenus - 1;
					y = 0;
				}
				if (key == 4) {  // Se ha pulsado la tecla de seleccion
					tratarOpcion(x, y);
					tiempo = millis();
					pinMode(10, INPUT);
					luzEncendida = true;
					myMenu.display(); //encendemos la pantalla
					cursorActivo = true;
					myMenu.blink(); // Mostramos el cursor parpadeando
				}
				myMenu.posicionActual(tituloMenu[x],
						tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
			}
		}
	}
	delay(50);
}

void tratarRespuestaGprs() {
	//alama
#ifndef RELEASE
	Serial << F("dentro de tratarRespuesta GPRS") << endl << F("Cadena recivida: ") << tratarRespuesta << endl;

	/*for (int i = 0;i<tratarRespuesta.length();i++){
		Serial << F("ASCII:  ") << (byte)tratarRespuesta.charAt(i) << F(" caracter: ") << tratarRespuesta.charAt(i)<< F(" Indice: ")<< i << endl;


	}*/
#endif
	tratarRespuesta=tratarRespuesta.substring(2,tratarRespuesta.length()-2);
	if (tratarRespuesta.startsWith(PSTR("+CALV:"))) {
#ifndef RELEASE
		Serial << F("dentro de CALV") << endl;
#endif

		byte alarma = ((byte) tratarRespuesta.charAt(7)) - 48;
#ifndef RELEASE_FINAL
		zonas.imprimirZonas();
#endif
		if (zonas.getEstadoPrimeraVez(alarma) == false) //salta la alarma se establece la duracion
		{
			zonas.setEstadoPrimeraVez(alarma);  //cambiamos es estado
			gprs.iniciarRiegoZona(alarma);
			gprs.establecerHoraFin(zonas.getZonaDeRiego(alarma));
		} else    // salta la alarma porque ha terminado el tiempo de riego
		{
			zonas.setEstadoPrimeraVez(alarma);    //cambiamos es estado
			gprs.pararRiegoZona(alarma);
			gprs.establecerHoraInicio(zonas.getZonaDeRiego(alarma));
		}
#ifndef RELEASE_FINAL
		zonas.imprimirZonas();
#endif
#ifndef RELEASE
		Serial << F("fuera de CALV") << endl;
#endif
	}
#ifndef RELEASE
	Serial << F("fuera de tratarRespuesta GPRS") << endl;
#endif

}

void comandoGPRS(void) {
	if (gprs.available()) // if date is comming from softwareserial port ==> data is comming from gprs shield
	{
		tratarRespuesta = gprs.readString();
#ifndef RELEASE
		Serial << F("leyendo del gprs") << endl;
#endif
		tratarRespuestaGprs();
#ifndef RELEASE
		Serial << tratarRespuesta << endl;
#endif
		//Serial.println(tratarRespuesta.toCharArray());
	}
#ifndef RELEASE_FINAL
	if (Serial.available()) // if data is available on hardwareserial port ==> data is comming from PC or notebook
	{
		tratarRespuesta = Serial.readString();
		if (tratarRespuestaSerial()) {
#ifndef RELEASE
			Serial << F("escribiendo en gprs") << endl << tratarRespuesta
					<< endl;
#endif

			gprs.enviaComando(tratarRespuesta);       // write it to the GPRS shield
		}
	}
#endif
}

#ifndef RELEASE_FINAL
/////////////////////////////////////////////funciones debub
bool tratarRespuestaSerial() {
	bool salidaRespuesta = true;
#ifndef RELEASE
	Serial << F("dentro de tratarRespuesta Serial") << endl;
#endif
	if (tratarRespuesta.startsWith("ER:")){
		for (int i = 0;i<1024;i++){
			EEPROM.write(i,'\x0');
		}
	}
	if (tratarRespuesta.startsWith("E:")) {
#ifndef RELEASE
		Serial << F("dentro de Escritura:") << endl;
#endif
		byte pos = (tratarRespuesta.charAt(2)-48);
#ifndef RELEASE
		Serial << F("valor de pos= ") << pos << endl;
#endif
		pos = pos * 16;
		for (int i = 4; i < tratarRespuesta.length(); i++) {
			EEPROM.write(pos, (byte) tratarRespuesta.charAt(i));
			Serial <<F("posicion: ")<<pos<<F(" valor: ")<<(byte) tratarRespuesta.charAt(i)<< F(" caracter: ") <<tratarRespuesta.charAt(i) <<endl;
			pos++;
			i++;
		}
#ifndef RELEASE
		Serial << F("fuera de Escritura:") << endl;
#endif
		salidaRespuesta = false;
	}
	if (tratarRespuesta.startsWith("L:")) {
#ifndef RELEASE
		Serial << F("dentro de Lectura:") << endl;
#endif
		leerEEPROM();
#ifndef RELEASE
		Serial << F("fuera de lectura:") << endl;
#endif
		salidaRespuesta = false;
	}
#ifndef RELEASE
	Serial << F("fuera de tratarRespuesta Serial") << endl;
#endif
	return salidaRespuesta;

}

int freeRam() {
#ifndef DEBUG_PROCESS
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#else
	return 0;
#endif
}

void leerEEPROM(void) {
#ifndef RELEASE
	byte valor;
	Serial << endl
			<< F(
					"0                   1                   2                   3")
			<< endl;
	Serial
			<< F(
					"0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1")
			<< endl;
	Serial
			<< F(
					"---------------------------------------------------------------")
			<< endl;
	for (int i = 0; i < 320; i++) {
		valor = EEPROM.read(i);
		if (i > 0 && i % 32 == 0) {
			Serial << endl;
		}
		Serial << valor << F(" ");

	}
	Serial << endl;
#endif
}
#endif


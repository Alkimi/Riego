#include <Arduino.h>
#include "Riego.h"
#include "GSM.H"
#include "Menu.h"
#include "miEEPROM.h"
#include "controlZona.h"

const byte numeroMenusMaximo = 5;
byte numeroMenusActivos = 4;
const byte numeroMaximoDeSubmenus = 5;

//FLASH_STRING_ARRAY(tituloMenu,PSTR("1 Configuración"),PSTR("2 Información"), PSTR("3  Menu 3"),PSTR("4  Menu 4"),PSTR("5  Menu 5"));
const char tituloMenu[numeroMenusMaximo][17] = { "1 Configuracion", "2 Infomacion",
		"3 Riego manual","4 Reinicia Zona", "Reinica Manual"};

byte numeroDeSubmenus[numeroMenusMaximo] = { 4, 2, 4, 4, 1 };

/*FLASH_STRING_ARRAY(tituloSubmenu,PSTR("1.1 xxxxxxxxxx"),PSTR("1.2 Destino SMS"),PSTR("1.3 Fecha"),PSTR("1.4 Hora"),
 PSTR("2.1 Fecha/Hora"),PSTR("2.2 Destino SMS"),PSTR(""),PSTR(""),
 PSTR("3.1 Submenu 1"),PSTR("3.2 Submenu 2"),PSTR(""),PSTR(""),
 PSTR("4.1 Submenu 1"),PSTR(""),PSTR(""),PSTR(""),
 PSTR("5.1 Submenu 1"),PSTR("5.2 Submenu 2"),PSTR("5.3 Submenu 3"),PSTR("5.4 Submenu 4"));
 */
const char tituloSubmenu[numeroMenusMaximo * numeroMaximoDeSubmenus][17] = {
		"1.1 xxxxxxxxxx", "1.2 Destino SMS", "1.3 Fecha", "1.4 Hora","",
		"2.1 Fecha/Hora", "2.2 Destino SMS", "", "","",
		"3.1 Agua Casa","3.2 Zona 1","3.3 Zona 2","3.4 Zona 3","3.5 Zona 4",
		"4.1 Zona 1","4.2 Zona 2", "4.3 Zona 3", "4.4 Zona 4", "",
		"Pricipal", "", "", "",""};

unsigned int adc_key_val[5] = { 50, 200, 400, 600, 800 };
char NUM_KEYS = 5;
int key = -1;
boolean luzEncendida = true;
boolean cursorActivo = false;
boolean pantallaEncendida = true;
unsigned long tiempo;
unsigned long tiempo2;
int x = 0;
int y = 0;
String tratarRespuesta;
char* cadena;
char aux[33];
Menu myMenu("", numeroMenusMaximo);
GSM gprs;
controlZona ControlZonas;
volatile long numeroPulsos=0;
unsigned long totalPulsos=0;
unsigned long anteriorTotalPulsos=0;
boolean funcionamiento=true;
long reinicio;
int totalLitrosParicales=0;;

char linea1[16];
char linea2[16];

// Convertimos el valor leido en analogico en un numero de boton pulsado
int get_key(unsigned int input) {
	for (byte k = 0; k < NUM_KEYS; k++) {
		if (input < adc_key_val[k]) {
			return k;
		}
	}
	return -1;
}

void aumentaPulso(void){
	numeroPulsos++;
}

void controlTiempo(void) {
	if (millis()>reinicio){
		//reinicia el sistema
	}

	if (funcionamiento && (millis() - tiempo2) > 5000 ){// cada 20 segundos comprueba el agua consumida
		tiempo2=millis();
		noInterrupts();
		totalPulsos+=numeroPulsos;
		numeroPulsos=0;
		interrupts();

		if (totalPulsos>anteriorTotalPulsos){
			Serial <<F("Total pulsos: ")<<totalPulsos<<F(" total pulsos anterior: ")<<anteriorTotalPulsos<<endl;
			anteriorTotalPulsos=totalPulsos;
			Serial <<F("actualizando anterior")<<endl<<F("Regando: ")<< ControlZonas.isRegando()<<endl;

			if (ControlZonas.isRegando()){
				//convertimos pulsos en litros parciales
			   	//totalLitrosParicales+=(totalPulsos/175);
				totalLitrosParicales=totalPulsos;
			   	totalPulsos=0;
			   	anteriorTotalPulsos=0;

			   	//obtenemos el total de litros de todas las zonas
			   	Serial << F("Total litros: ")<<ControlZonas.getTotalLitros()<< endl<<F(" maximo litros riego: ")<<ControlZonas.getMaxLitrosRiego()<<endl;
				if (ControlZonas.isMaxLitrosRiego()){
					if (ControlZonas.isReventon()){
						if (ControlZonas.isTodasZonasReventon()){
							// cerrar principal
							gprs.valvulaPrincipal(CERRAR);
							gprs.enviaSMSErrorTodasLasZonas();
							gprs.setProblemaEnZona(PRINCIPAL,true);
							funcionamiento=false;
							x=0;y=0;
							//paso a modo manual
							numeroMenusActivos++;
						}else{
							//cerramos valvulas afectadas
							//enviamos sms de zonas cerradas
							gprs.valvulaPrincipal(CERRAR);
						}
					}
				}
				ControlZonas.setIncrementaLitros(totalLitrosParicales);
				totalLitrosParicales=0;
			}else{ // detectado rebenton sin estar regando
				// cerrar principal
				gprs.valvulaPrincipal(CERRAR);
				// enviar sms
				gprs.enviaSMSErrorPrincipal();
				gprs.setProblemaEnZona(PRINCIPAL,true);
				funcionamiento=false;
				x=0;y=0;
				//paso a modo manual
				numeroMenusActivos++;
			}
		}
	}
	if (millis() - tiempo > 15000) { // a los 15 segundos apagamos el display
		pantallaEncendida=false;
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

void estadoProblemaEnZona(byte zona){
	zona=zona-7;
	if (zona==PRINCIPAL){
		strcpy_P(linea1, PSTR("Estado principal:"));
	}else{
		strcpy_P(linea1, PSTR("Estado zona:    "));
		linea1[13]=zona+48;
	}
	myMenu.linea1(linea1);
	if (!ControlZonas.isRegandoZona(zona)){
	//if (!gprs.getProblemaEnZona(zona)){
		strcpy_P(linea2, PSTR("ZONA correcta "));
		myMenu.linea2(linea2);
	}else{
		myMenu.blink();
		strcpy_P(linea2, PSTR("   REINICIAR   "));
		myMenu.linea2(linea2);
		//esperamos a tecla select
		bool salida = false;
		do {
			if (lecturaPulsador() == 4) { // Se ha pulsado la tecla de seleccion
				salida = true;
			}
		} while (!salida);
		//guardamos valor en eprom
		if (zona==PRINCIPAL){
			gprs.valvulaPrincipal(ABRIR);
			noInterrupts();
			numeroPulsos=0;
			numeroMenusActivos--;
			totalPulsos=0;
			x=0;y=0;
			anteriorTotalPulsos=0;
			interrupts();
		}else{
			//gprs.valvulaZona(zona);
		}
		gprs.setProblemaEnZona(zona,false);
		funcionamiento=true;
		strcpy_P(linea2, PSTR("ZONA correcta "));
		myMenu.linea2(linea2);
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
			myMenu.linea2("Fecha erronea");
		} else {
			myMenu.linea2("Hora  erronea");
		}
		delay(1000);
		salida = false;
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
	myMenu.noBlink();
	switch (opcion) {
	case 1:
		getSMS();
		myMenu.blink();
		cambioNumero(opcion);
		setSMS();
		break;
	case 2:
	case 3:
	case 4:
		getFechaHora();
		myMenu.posicionActual(linea1, linea2);
		if ((opcion == 2) || (opcion == 3)) {
			do {
				myMenu.blink();
				cambioNumero(opcion);
			} while (setFechaHora(opcion) == false);
		}

		break;
	case 5:
		getSMS();
		break;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		estadoProblemaEnZona(opcion);
		break;
	}
	delay(3000);
	myMenu.blink();
#ifndef RELEASE
	Serial << F("saliendo de tratar opcion") << endl << F("Memoria libre: ")
			<< freeRam() << endl;
#endif
}

int lecturaPulsador(void){
	int adc_key_inNumero;
	int keyNumero = -1;

	adc_key_inNumero = analogRead(0);    // Leemos el valor de la pulsacion
	keyNumero = get_key(adc_key_inNumero);    // Obtenemos el boton pulsado
	if (keyNumero != -1) {  // if keypress is detected
		delay(100);  // Espera para evitar los rebotes de las pulsaciones
		adc_key_inNumero = analogRead(0); // Leemos el valor de la pulsacion
		keyNumero = get_key(adc_key_inNumero); // Obtenemos el boton pulsado
		if (keyNumero != -1) {
			if (!luzEncendida) {
				pinMode(10, INPUT);
				luzEncendida = true;
				cursorActivo = true;
				pantallaEncendida=true;
				myMenu.blink(); // Mostramos el cursor parpadeando
				myMenu.display(); //encendemos la pantalla
			}
			return keyNumero;
		}
	}
	return -1;
}

void cambioNumero(byte opcion) {
	bool salida = false;
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
	do {
		keyNumero=lecturaPulsador();
		if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
			myMenu.SetCursor(xNumero, columna);
#ifndef RELEASE
			Serial << F("entrada Tecla pulsada: ") << keyNumero << endl
					<< F(" x= ") << xNumero << F(" caracter: ")
					<< caracter << F(" linea: ") << linea << endl;
#endif

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
	} while (!salida);
}

void setSMS(void) {
	EEPROM.escrituraEeprom16(0, linea2);
}

void getSMS(void) {
	myMenu.noBlink();
	EEPROM.lecturaEeprom16(0, linea2);
	if (pantallaEncendida){
		strcpy_P(linea1, PSTR("Destino SMS:"));
		myMenu.posicionActual(linea1, linea2);
	}
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
	ControlZonas.imprimirZonas();
#endif
	gprs.inicializaAlarmas(&ControlZonas);

	tiempo = millis();
	attachInterrupt(0, aumentaPulso, FALLING); // interrupcion 0 habilitada en el pin 2 sobre el metodo aumentaPulso en el flanco de bajada
	reinicio = gprs.iniciaReloj();
	Serial << F("millis hasta fin del dia: ")<<reinicio<<endl;
}

void loop() {
	comandoGPRS();
	controlTiempo();
	key=lecturaPulsador();
	if (key != -1) {
		Serial <<F("valor x: ")<<x<<F(" valor y: ")<<y<<endl;
		tiempo = millis();
		if (key == 0) {  // Se ha pulsado la tecla derecha
			x++;
			if (x > numeroMenusActivos - 1)
				x = 0;
			y = 0;
		}
		if (key == 1) {   // Se ha pulsado la tecla arriba
			y--;
			if (y < 0 )
				y = numeroDeSubmenus[x] - 1;
		}
		if (key == 2) {  // Se ha pulsado la tecla abajo
			y++;
			if (y > numeroDeSubmenus[x] - 1)
				y = 0;
		}

		if (key == 3) {  // Se ha pulsado la tecla izquierda
			x--;
			if (x < 0)
				x = numeroMenusActivos - 1;
			y = 0;
		}
		if (key == 4) {  // Se ha pulsado la tecla de seleccion
			tratarOpcion(x, y);
			tiempo = millis();
			pinMode(10, INPUT);
			luzEncendida = true;
			pantallaEncendida=true;
			myMenu.display(); //encendemos la pantalla
			cursorActivo = true;
			myMenu.blink(); // Mostramos el cursor parpadeando
		}
		myMenu.posicionActual(tituloMenu[x],
				tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
	}
}

void tratarRespuestaGprs() {
	//alama
#ifndef RELEASE
	Serial << F("dentro de tratarRespuesta GPRS") << endl;

	/*for (int i = 0;i<tratarRespuesta.length();i++){
		Serial << F("ASCII:  ") << (byte)tratarRespuesta.charAt(i) << F(" caracter: ") << tratarRespuesta.charAt(i)<< F(" Indice: ")<< i << endl;


	}*/
#endif
	tratarRespuesta=tratarRespuesta.substring(2,tratarRespuesta.length()-2);
	if ((tratarRespuesta.charAt(1)=='C' && tratarRespuesta.charAt(2)=='A' && tratarRespuesta.charAt(3)=='L' && tratarRespuesta.charAt(4)=='V')){
#ifndef RELEASE
		Serial << F("dentro de CALV") << endl;
#endif

		byte alarma = ((byte) tratarRespuesta.charAt(7)) - 48;
#ifndef RELEASE_FINAL
		ControlZonas.imprimirZonas();
#endif
		if (!ControlZonas.isReventonZona(alarma)){
			Serial <<F("No hay problema en zona: ")<<alarma<<endl;
			if (!ControlZonas.isRegandoZona(alarma)){//salta la alarma se establece la duracion
				Serial <<F("inicio riego")<<endl;
				ControlZonas.setRegandoZona(alarma,true);
				gprs.iniciarRiegoZona(alarma);
				gprs.establecerHoraFin(&ControlZonas,alarma);
			} else    // salta la alarma porque ha terminado el tiempo de riego
			{
				Serial <<F("fin riego")<<endl;
				ControlZonas.setRegandoZona(alarma,false);
				gprs.pararRiegoZona(alarma);
				gprs.establecerHoraInicio(&ControlZonas,alarma);
			}
		}else{
			Serial <<F("hay problema en zona: ")<<alarma<<F(" no hace nada")<<endl;
		}
#ifndef RELEASE_FINAL
		ControlZonas.imprimirZonas();
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
#ifndef SIMPLE
	if (gprs.available()) // if date is comming from softwareserial port ==> data is comming from gprs shield
	{
		tratarRespuesta = gprs.readString();
		Serial << F("leyendo del gprs: ") <<  tratarRespuesta<< endl;
		tratarRespuestaGprs();
	}
#ifndef RELEASE_FINAL
	if (Serial.available()) // if data is available on hardwareserial port ==> data is comming from PC or notebook
	{
		tratarRespuesta = Serial.readString();
		if (tratarRespuestaSerial()) {
			Serial << F("escribiendo en gprs: ")<< tratarRespuesta << endl;
			gprs.enviaComando(tratarRespuesta);       // write it to the GPRS shield
		}
	}
#endif

#else
	if (gprs.available()){ // if date is comming from softwareserial port ==> data is comming from gprs shield
		Serial << F("leyendo del gprs") << endl;
		Serial << gprs.readString()<<endl;
	}
	if (Serial.available()) {// if data is available on hardwareserial port ==> data is comming from PC or notebook
		Serial << F("escribiendo en gprs") << endl;
		gprs.println(Serial.readString());
	}
#endif
}

#ifndef RELEASE_FINAL
/////////////////////////////////////////////funciones debug
bool tratarRespuestaSerial() {
	bool salidaRespuesta = true;
	//borrado de eeprom
	if (tratarRespuesta.startsWith("ER:")){
		for (int i = 0;i<1024;i++){
			EEPROM.write(i,'\x0');
		}
	}
	//escritura en eeprom
	if (tratarRespuesta.startsWith("E:")) {
		byte pos = (tratarRespuesta.charAt(2)-48);
		pos = pos * 16;
		for (int i = 4; i < tratarRespuesta.length(); i++) {
			EEPROM.write(pos, (byte) tratarRespuesta.charAt(i));
			Serial <<F("posicion: ")<<pos<<F(" valor: ")<<(byte) tratarRespuesta.charAt(i)<< F(" caracter: ") <<tratarRespuesta.charAt(i) <<endl;
			pos++;
			i++;
		}
		salidaRespuesta = false;
	}
	//lectura de eeprom
	if (tratarRespuesta.startsWith("L:")) {
		byte pos = (tratarRespuesta.charAt(2)-48);
		leerEEPROM(pos);
		salidaRespuesta = false;
	}

	//imprimir zonas
	if (tratarRespuesta.startsWith("I:")) {
		ControlZonas.imprimirZonas();
		salidaRespuesta = false;
	}

	//hora
	if (tratarRespuesta.startsWith("H:")) {
		gprs.enviaComando(F("AT+CCLK?"));
		salidaRespuesta = false;
	}
	//alarma
	if (tratarRespuesta.startsWith("A:")) {
		gprs.enviaComando(F("AT+CALA?"));
		salidaRespuesta = false;
	}


	return salidaRespuesta;




}

int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void leerEEPROM(byte pos) {
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
	int limite;
	if (pos==0)
		limite =320;
	else
		limite = pos*16;
	for (int i = 0; i < limite; i++) {
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


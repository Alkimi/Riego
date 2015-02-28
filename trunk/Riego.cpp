#include <Arduino.h>
#include "Riego.h"
#include "GSM.H"
#include "Menu.h"

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
unsigned long tiempo3;
unsigned long segundo;
int x = 0;
int y = 0;
char* cadena;
Menu myMenu("", numeroMenusMaximo);
GSM gprs;
controlZona ControlZonas;
volatile long numeroPulsos=0;
unsigned long totalPulsos=0;
unsigned long anteriorTotalPulsos=0;
boolean funcionamiento=true;
unsigned long reinicio;
float consumo;
int totalLitrosParicales=0;
int posicionMemoria=160;

UBuffer buffer;
UBuffer2 buffer2;
char *linea1 = buffer2.aux;
char *linea2 = &buffer2.aux[17];

void pins_init()
{
	pinMode(BOTONERA, INPUT);
	pinMode(CURRENT_SENSOR, INPUT);
}

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
	//numeroPulsos++;
}

void controlTiempo(void) {
	if (millis()>reinicio){
		//reinicia el sistema
	}

	/*if (millis() - segundo > UNSEGUNDO){
		 consumo = (0.049 * analogRead(CURRENT_SENSOR)) -25; //28952 + 822
		 //consumo = map(analogRead(CURRENT_SENSOR), 0, 1023, -20000, 20000); //29212 + 822
		 if (EEPROM.read(159)==0x0){
		 	 EEPROM.writeFloat(posicionMemoria++,consumo);
		 	 if (posicionMemoria>400){
		 		 EEPROM.write(159,0x1);
		 	 }
		 }
	}*/

	if (millis() - tiempo3 > 60000) {
		tiempo3=millis();
		if (ControlZonas.isManual()){
			if(ControlZonas.isManualZona(0)){
				gprs.pararRiegoZona(5);
				ControlZonas.setManualZona(0,false,0);
			}
			for (int i=1;i<5;i++){
				if (ControlZonas.isManualZona(i)){
					if (millis() - ControlZonas.getTiempoZona(i) > UNAHORA){
						gprs.pararRiegoZona(i);
						ControlZonas.setManualZona(i,false,0);
					}
				}
			}
			if (ControlZonas.isManual()){
				gprs.valvulaPrincipal(CERRAR);
			}
		}
	}

	if (funcionamiento && (millis() - tiempo2) > 5000 ){// cada 20 segundos comprueba el agua consumida
		tiempo2=millis();
		noInterrupts();
		totalPulsos+=numeroPulsos;
		numeroPulsos=0;
		interrupts();

		if (totalPulsos>anteriorTotalPulsos){
#ifndef RELEASE
			Serial <<F("Total pulsos: ")<<totalPulsos<<F(" total pulsos anterior: ")<<anteriorTotalPulsos<<endl;
#endif
			anteriorTotalPulsos=totalPulsos;
#ifndef RELEASE
			Serial <<F("actualizando anterior")<<endl<<F("Regando: ")<< ControlZonas.isRegando()<<endl;
#endif
			if (ControlZonas.isRegando()){
				//convertimos pulsos en litros parciales
			   	//totalLitrosParicales+=(totalPulsos/175);
				totalLitrosParicales=totalPulsos;
			   	totalPulsos=0;
			   	anteriorTotalPulsos=0;

			   	//obtenemos el total de litros de todas las zonas
#ifndef RELEASE
			   	Serial << F("Total litros: ")<<ControlZonas.getTotalLitros()<< endl<<F(" maximo litros riego: ")<<ControlZonas.getMaxLitrosRiego()<<endl;
#endif
				if (ControlZonas.isMaxLitrosRiego()){
					if (ControlZonas.isReventon()){
						if (ControlZonas.isTodasZonasReventon()){
							// cerrar principal
							gprs.valvulaPrincipal(CERRAR);
							gprs.enviaSMSErrorTodasLasZonas();
							//gprs.setProblemaEnZona(PRINCIPAL,true);
							ControlZonas.setRebentonZona(PRINCIPAL,true);
							funcionamiento=false;
							x=0;y=0;
							//paso a modo manual
							numeroMenusActivos++;
						}else{
							//cerramos valvulas afectadas
							gprs.pararRiegoZona(1);
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
				ControlZonas.setRebentonZona(PRINCIPAL,true);
				//gprs.setProblemaEnZona(PRINCIPAL,true);
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
	if (zona==PRINCIPAL){
		strcpy_P(linea1, PSTR("Estado principal:"));
	}else{
		strcpy_P(linea1, PSTR("Estado zona:    "));
		linea1[13]=zona+48;
	}
	myMenu.linea1(linea1);
	if (!ControlZonas.isReventonZona(zona)){
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
		ControlZonas.setReiniciaZona(zona);
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
			gprs.iniciarRiegoZona(zona);
		}

		ControlZonas.setReiniciaZona(zona);
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
	sprintf(buffer2.aux, "AT+CCLK=\"%c%c/%c%c/%c%c,%c%c:%c%c:%c%c+02\"", linea1[13],
			linea1[14], linea1[10], linea1[11], linea1[7], linea1[8], linea2[7],
			linea2[8], linea2[10], linea2[11], linea2[13], linea2[14]);
	cadena = gprs.enviaComando(buffer2.aux);
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
	case 5:
		getFechaHora();
		myMenu.posicionActual(linea1, linea2);
		if ((opcion == 2) || (opcion == 3)) {
			do {
				myMenu.blink();
				cambioNumero(opcion);
			} while (setFechaHora(opcion) == false);
		}

		break;
	case 6:
		getSMS();
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		strcpy_P(linea1, PSTR(" Riego Manual "));

		if (ControlZonas.isManualZona(opcion-10)){ //si ya esta regando lo paramos
			ControlZonas.setManualZona(opcion-10,false,0);
			gprs.pararRiegoZona(opcion-10);
			strcpy_P(linea2, PSTR(" A P A G A D O "));
		}else{ // si no esta regando emepzamos a regar

			ControlZonas.setManualZona(opcion-10,true,millis());
			gprs.iniciarRiegoZona(opcion-15);
			strcpy_P(linea2, PSTR("E N CEN D I D O"));
		}
		myMenu.posicionActual(linea1, linea2);
		break;
	case 15:
	case 16:
	case 17:
	case 18:
		estadoProblemaEnZona(opcion-15);
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

	adc_key_inNumero = analogRead(BOTONERA);    // Leemos el valor de la pulsacion
	keyNumero = get_key(adc_key_inNumero);    // Obtenemos el boton pulsado
	if (keyNumero != -1) {  // if keypress is detected
		delay(100);  // Espera para evitar los rebotes de las pulsaciones
		adc_key_inNumero = analogRead(BOTONERA); // Leemos el valor de la pulsacion
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
		linea2[0]='+';
		linea2[1]='3';
		linea2[2]='4';
		myMenu.posicionActual(linea1, linea2);
	}
}

void setup() {
	Serial.begin(4800);

//#ifndef RELEASE_FINAL
	Serial << F("Memoria libre: ") << freeRam() << endl << F("------------------------") << endl;
	Serial << F("Riego Total V ") << RIEGO_VERSION << endl << F("Menu V ")
			<< myMenu.libVer() << endl << F("GPRS V ") << gprs.libVer() << endl;
//#endif
	myMenu.inicia(gprs.libVer());
	myMenu.posicionActual(tituloMenu[x],
			tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
#ifndef RELEASE_FINAL
	ControlZonas.imprimirZonas();
#endif
	gprs.inicializaAlarmas(&ControlZonas);

	tiempo = millis();
	tiempo3 = millis();
	segundo = millis();
	attachInterrupt(0, aumentaPulso, FALLING); // interrupcion 0 habilitada en el pin 2 sobre el metodo aumentaPulso en el flanco de bajada
	reinicio = gprs.iniciaReloj();
#ifndef RELEASE
	Serial << F("millis hasta fin del dia: ")<<reinicio<<endl;
#endif
	pins_init();

}

void loop() {
	comandoGPRS();
	controlTiempo();
	key=lecturaPulsador();
	if (key != -1) {
#ifndef RELEASE
		Serial <<F("valor x: ")<<x<<F(" valor y: ")<<y<<endl;
#endif
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
#endif
	if (buffer.buffer[3]=='C' && buffer.buffer[4]=='A' && buffer.buffer[5]=='L' && buffer.buffer[6]=='V'){
#ifndef RELEASE
		Serial << F("dentro de CALV") << endl;
#endif

		byte alarma = buffer.buffer[9] - 48;
#ifndef RELEASE_FINAL
		ControlZonas.imprimirZonas();
#endif
		if (!ControlZonas.isReventonZona(alarma)){
#ifndef RELEASE
			Serial <<F("No hay problema en zona: ")<<alarma<<endl;
#endif
			if (!ControlZonas.isRegandoZona(alarma)){//salta la alarma se establece la duracion
#ifndef RELEASE
				Serial <<F("inicio riego")<<endl;
#endif
				ControlZonas.setRegandoZona(alarma,true);
				gprs.iniciarRiegoZona(alarma);
				gprs.establecerHoraFin(&ControlZonas,alarma);
			} else    // salta la alarma porque ha terminado el tiempo de riego
			{
#ifndef RELEASE
				Serial <<F("fin riego")<<endl;
#endif
				ControlZonas.setRegandoZona(alarma,false);
				gprs.pararRiegoZona(alarma);
				gprs.establecerHoraInicio(&ControlZonas,alarma);
			}
		}
#ifndef RELEASE
		else{
			Serial <<F("hay problema en zona: ")<<alarma<<F(" no hace nada")<<endl;
		}
#endif
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
	//	buffer.buffer = gprs.readString();
#ifndef RELEASE
		Serial << F("leyendo del gprs: ") << buffer.buffer<< endl;
#endif
		tratarRespuestaGprs();
	}
#ifndef RELEASE_FINAL
	if (Serial.available()) // if data is available on hardwareserial port ==> data is comming from PC or notebook
	{
		//buffer.buffer = Serial.readString();
		if (tratarRespuestaSerial()) {
#ifndef RELEASE
			Serial << F("escribiendo en gprs: ")<<buffer.buffer << endl;
#endif
			gprs.enviaComando(buffer.buffer);       // write it to the GPRS shield
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


//#ifndef RELEASE_FINAL
int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#ifndef RELEASE_FINAL
/////////////////////////////////////////////funciones debug
bool tratarRespuestaSerial() {
	bool salidaRespuesta = true;
	Serial <<F("Evaluando entrada recibida:")<<endl;
	//borrado de eeprom
	if (buffer.buffer[0]=='E' && buffer.buffer[1]=='R' &&  buffer.buffer[2]==':'){
		Serial <<F("borrado: ")<<endl;
		for (int i = 0;i<1024;i++){
			EEPROM.write(i,'\x0');
		}
		salidaRespuesta = false;
	}
	//escritura en eeprom
	if (buffer.buffer[0]=='E' && buffer.buffer[1]==':'){
		Serial <<F("escritura: ")<<endl;
		byte pos = buffer.buffer[2]-48;
		pos = pos * 16;
		byte i=4;
		do{
			EEPROM.write(pos, buffer.buffer[i]);
			Serial <<F("posicion: ")<<pos<<F(" valor: ")<<(byte) buffer.buffer[i]<< F(" caracter: ") <<buffer.buffer[i] <<endl;
			pos++;
			i++;
		}while (buffer.buffer[i]!='0x0');
		salidaRespuesta = false;
	}
	//lectura de eeprom bloque  L:000 o L:
	if (buffer.buffer[0]=='L' && buffer.buffer[1]==':'){
		Serial <<F("lectura: ")<<endl;
		if (buffer.buffer[4]!='0x0'){
			int posicion=0;
			int mul=100;
			for (byte i=2;i<5;i++){
				posicion=posicion+((buffer.buffer[i]-48)*mul);
				mul=mul/10;
			}
			leerEEPROM(posicion);
		}else{
			leerEEPROM(0);
		}
		salidaRespuesta = false;
	}

	//imprimir zonas
	if (buffer.buffer[0]=='I' && buffer.buffer[1]==':'){
		Serial <<F("imprimir zonas: ")<<endl;
		ControlZonas.imprimirZonas();
		salidaRespuesta = false;
	}

	//hora
	if (buffer.buffer[0]=='H' && buffer.buffer[1]==':'){
		Serial <<F("hora: ")<<endl;
		gprs.enviaComando(F("AT+CCLK?"));
		salidaRespuesta = false;
	}
	//alarma
	if (buffer.buffer[0]=='A' && buffer.buffer[1]==':'){
		Serial <<F("alarma: ")<<endl;
		gprs.enviaComando(F("AT+CALA?"));
		salidaRespuesta = false;
	}

	//lecturas consumo
	if (buffer.buffer[0]=='C' && buffer.buffer[1]==':'){
		Serial <<F("consumo: ")<<endl;
		for (int i = 160;i<400;i++){
			Serial <<F("Consumo: ")<<EEPROM.readFloat(i)<<F(" mA")<<endl;
			i=i+4;
		}
		salidaRespuesta = false;
	}

	//escritura posicion valor  EP:0000 000
	if (buffer.buffer[0]=='E' && buffer.buffer[1]=='P' &&  buffer.buffer[2]==':'){
		int posicion=0;
		int valor=0;
		int mul=1000;
		for (byte i=3;i<7;i++){
			Serial <<F("posicion: ")<<i<<F(" valor: ")<<(byte)buffer.buffer[i]<< F(" caracter: ") <<buffer.buffer[i];
			Serial <<F(" mul: ")<<mul;
			posicion=posicion+((buffer.buffer[i]-48)*mul);
			mul=mul/10;
			Serial <<F(" posicion= ")<<posicion<<endl;
		}
		mul=100;
		for (byte i=8;i<11;i++){
			Serial <<F("posicion: ")<<i<<F(" valor: ")<<(byte)buffer.buffer[i]<< F(" caracter: ") <<buffer.buffer[i];
			Serial <<F(" mul: ")<<mul;
			valor=valor+((buffer.buffer[i]-48)*mul);
			mul=mul/10;
			Serial <<F(" posicion= ")<<posicion<<endl;
		}
		Serial <<F("escritura en posicion: ")<<posicion<<F(" con valor: ")<<valor<<endl;
		EEPROM.write(posicion,valor);
		salidaRespuesta = false;
	}

	Serial <<F("fin Evaluando entrada recibida:")<<endl;
	return salidaRespuesta;




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
			Serial <<F("\t")<<i-1<<F("\t")<<i/16<< endl;
		}
		Serial << valor << F(" ");

	}
	Serial <<F("\t")<<limite-1<<F("\t")<<limite/16<< endl;
#endif
}
#endif


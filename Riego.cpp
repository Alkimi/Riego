#include "Riego.h"
#include "GSM.H"
#include "Menu.h"
#ifndef RELEASE_FINAL
#include "Utilidades.h"
#endif

const byte numeroMenusMaximo = 5;
byte numeroMenusActivos = 4;
const byte numeroMaximoDeSubmenus = 5;

const char tituloMenu[numeroMenusMaximo][17] = { "1 Configuracion", "2 Infomacion",
		"3 Riego manual","4 Reinicia Zona", "Reinica Manual"};

byte numeroDeSubmenus[numeroMenusMaximo] = { 5, 4, 5, 4, 1 };

const char tituloSubmenu[numeroMenusMaximo * numeroMaximoDeSubmenus][17] = {
		"1.1 SEGURIDAD", "1.2 Destino SMS", "1.3 Fecha", "1.4 Hora","1.5 ZONAS",
		"2.1 SEGURIDAD", "2.2 Destino SMS", "2.3 Fecha/Hora", "2.4 ZONAS","",
		"3.1 Agua Casa","3.2 Zona 1","3.3 Zona 2","3.4 Zona 3","3.5 Zona 4",
		"4.1 Zona 1","4.2 Zona 2", "4.3 Zona 3", "4.4 Zona 4", "",
		"Pricipal", "", "", "",""};

unsigned int adc_key_val[5] = { 50, 200, 400, 600, 800 };
char NUM_KEYS = 5;
int key = -1;
boolean luzEncendida = true;
boolean cursorActivo = false;
boolean pantallaEncendida = true;
unsigned long tiempoDisplay;
unsigned long tiempoCursor;
unsigned long tiempoLuz;
unsigned long veinteSegundos;
unsigned long tiempo3;
unsigned long segundo;
unsigned long cincomintuos;
int x = 0;
int y = 0;
Menu myMenu("", numeroMenusMaximo);
GSM gprs;
controlZona ControlZonas;
volatile long numeroPulsos=0;
unsigned long totalPulsos=0;
unsigned long anteriorTotalPulsos=0;
unsigned long reinicio;
unsigned long totalPulsosParciales=0;
int totalLitrosParicales=0;
boolean valvulaPrincipal=false;
boolean cierrePorBateriaBaja=true; // cuando entramos no sabemos el estado del sistema asi que lo bloqueamos

UBuffer buffer;
UBuffer2 buffer2;

boolean SEGURIDAD=true;

void aumentaPulso(void){
	numeroPulsos++;
}

void pins_init()
{
	//analogReference(DEFAULT);
/*	pinMode(BOTONERA, INPUT);
	pinMode(CURRENT_SENSOR, INPUT);*/
	//pinMode(GSM_POWER_ON_OFF,OUTPUT);
	pinMode(VOLTAJE_BATERIA,INPUT);
	pinMode(RETROILUMINACION, OUTPUT);
	digitalWrite(RETROILUMINACION, HIGH);

	//interrupciones
	attachInterrupt(0, aumentaPulso, FALLING); // interrupcion 0 habilitada en el pin 2 sobre el metodo aumentaPulso en el flanco de bajada
}

// Convertimos el valor leido en analogico en un numero de boton pulsado
int get_key(unsigned int input) {
	for (byte contador = 0; contador < NUM_KEYS; contador++) {
		if (input < adc_key_val[contador]) {
			return contador;
		}
	}
	return -1;
}

void controlTiempo(void) {
	/*if (millis()>reinicio){
		//reinicia el sistema
	}*/

	if ((millis()- cincomintuos) > CINCOMINUTOS){ //TODO ajustar los voltajes
		float suma=0.00;
		cincomintuos=millis();
		suma = gprs.energiaBateria();
		//Serial.print(F("Bateria: "));Serial.print(suma);Serial.println(F(" V"));

		if (cierrePorBateriaBaja==true){
			if(suma>12.00){ // se ha recuperado la carga
				EEPROM.write(16,0x0); // bateria critica
				EEPROM.write(17,0x0); //envio sms bateria baja
				//enviar SMS
				gprs.enviaSMSError(6);
				cierrePorBateriaBaja=false;
			}
		}else{
			if (suma>12.00){
				if (EEPROM.read(17)==0x1){  //bateria en estado normal
					EEPROM.write(17,0x0); // marcamos como la bateria esta recuperada y mandamos SMS
					//enviar SMS
					gprs.enviaSMSError(6);
				}
			}else{
			  if ((suma<12.00) && (suma>5.00)){  				//bateria baja. enviamos sms aviso
				if (EEPROM.read(17)==0x0){
					//envio SMS
					gprs.enviaSMSError(4);
					EEPROM.write(17,0x1);
				}
			  }else{
				if (suma<5.00){				//bateria critica, cierre de todas las zonas
					for (byte i =0;i< NUMERO_ZONAS;i++){
						gprs.pararRiegoZona(i);
					}
					gprs.valvulaPrincipal(CERRAR);
					valvulaPrincipal=true;
					// se guarda en eepron el estado critico
					cierrePorBateriaBaja=true;
					EEPROM.write(16,0x1);
					//envio de sms
					gprs.enviaSMSError(5);
				}
			  }
			}
		}
	}

	if ((cierrePorBateriaBaja==false) && (millis() - veinteSegundos) > VEINTESEGUNDOS ){// cada 20 segundos comprueba el agua consumida
		    veinteSegundos=millis();
			noInterrupts();
			totalPulsos+=numeroPulsos;
			totalPulsosParciales=numeroPulsos;
			numeroPulsos=0;
			interrupts();

			if (totalPulsos>anteriorTotalPulsos){
	#ifndef RELEASE
				Serial.print(F("Total pulsos: "));Serial.print(totalPulsos);
				Serial.print(F(" total pulsos anterior: "));Serial.println(anteriorTotalPulsos);
	#endif
				anteriorTotalPulsos=totalPulsos;
	#ifndef RELEASE
				Serial.println(F("actualizando anterior"));
				Serial.print(F("Regando: "));Serial.println(ControlZonas.isRegando());
			   	Serial.print(F("total litros parciales: "));Serial.println(totalLitrosParicales);
	#endif
				if (ControlZonas.isRegando()){
					//convertimos pulsos en litros parciales
				   	totalLitrosParicales+=(totalPulsosParciales/125);
				   	totalPulsosParciales=0;
				   	anteriorTotalPulsos=0;

				   	//obtenemos el total de litros de todas las zonas
	#ifndef RELEASE
				   	Serial.print(F("Total litros: "));Serial.print(ControlZonas.getTotalLitros());
				   	Serial.print(F(" maximo litros riego: "));Serial.println(ControlZonas.getMaxLitrosRiego());
	#endif
					if (ControlZonas.isMaxLitrosRiego()){
						if (ControlZonas.isReventon()){
							if (ControlZonas.isTodasZonasReventon()){
								// cerrar principal
								valvulaPrincipal=true;
								gprs.valvulaPrincipal(CERRAR);
								gprs.enviaSMSErrorTodasLasZonas();
								//gprs.setProblemaEnZona(PRINCIPAL,true);
								ControlZonas.setRebentonZona(PRINCIPAL,true);
								x=0;y=0;
								//paso a modo manual
								numeroMenusActivos++;
							}else{
								//cerramos valvulas afectadas
								gprs.pararRiegoZona(1);
								//enviamos sms de zonas cerradas
								valvulaPrincipal=true;
								gprs.valvulaPrincipal(CERRAR);
							}
						}
					}
					ControlZonas.setIncrementaLitros(totalLitrosParicales);
					totalLitrosParicales=0;
				}else{
					if (ControlZonas.isManual()==false){
						// detectado rebenton sin estar regando
						// cerrar principal
						valvulaPrincipal=true;
						gprs.valvulaPrincipal(CERRAR);
						// enviar sms
						gprs.enviaSMSErrorPrincipal();
						ControlZonas.setRebentonZona(PRINCIPAL,true);
						//gprs.setProblemaEnZona(PRINCIPAL,true);
						x=0;y=0;
						//paso a modo manual
						numeroMenusActivos++;
					}
				}
			}
		}

	//correcto
	if (valvulaPrincipal){
		if (millis() - gprs.getTiempoValvula() > TIEMPOCIERRE){
			valvulaPrincipal= false;
			gprs.reiniciaValvula();
		}
	}

	//control riego manual
	if ((cierrePorBateriaBaja==false) && (millis() - tiempo3) > UNMINUTO) {
		tiempo3=millis();
		if (ControlZonas.isManual()){
			//paramos todas las zonas que lleven mas de una hora regando
			for (int i=1;i<5;i++){
				if (ControlZonas.isManualZona(i)){
					if (millis() - ControlZonas.getTiempoZona(i) > UNAHORA){
						gprs.pararRiegoZona(i);
						ControlZonas.setManualZona(i,false,0);
					}
				}
			}
			if (!ControlZonas.isManual()){
			//	Si todas las zonas paradas, paramos principal
				valvulaPrincipal=true;
				gprs.valvulaPrincipal(CERRAR);
				ControlZonas.setManualZona(0,false,0);
			}
		}
	}

	//si ha pasado mas de 15 segundos apagamos el display
	if (millis() - tiempoDisplay > TIEMPODISPLAY) {
		tiempoDisplay=millis();
		pantallaEncendida=false;
		myMenu.noDisplay();
	}

	// Si han pasado mas de 10 segundos apagamos la luz
	if (millis() - tiempoLuz > TIEMPOLUZ) {
		tiempoLuz=millis();
		digitalWrite(RETROILUMINACION, LOW);
		luzEncendida = false;
	}

	// Si han pasado mas de 5 segundos apagamos el cursor
	if (millis() - tiempoCursor > TIEMPOCURSOR) {
		tiempoCursor=millis();
		myMenu.noBlink();
		cursorActivo = false;
	}
}

void estadoProblemaEnZona(byte zona){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];

	if (zona==PRINCIPAL){
		EEPROM.leeCadenaEEPROM(426,linea1); //estado principal
	}else{
		EEPROM.leeCadenaEEPROM(444,linea1); //estado zona
		linea1[13]=zona+48;
	}
	myMenu.linea1(linea1);
	if (!ControlZonas.isReventonZona(zona)){
		EEPROM.leeCadenaEEPROM(477,linea2); //zona correcta
		myMenu.linea2(linea2);
	}else{
		myMenu.blink();
		EEPROM.leeCadenaEEPROM(461,linea2); //   REINICIAR
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
			valvulaPrincipal=true;
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
		EEPROM.leeCadenaEEPROM(477,linea2); //zona correcta
		myMenu.linea2(linea2);
	}
}

//correcto
void getFechaHora(char *linea1,char *linea2) {
	char l1[17];
	char l2[17];
	EEPROM.leeCadenaEEPROM(492,l1); //fecha:
	EEPROM.leeCadenaEEPROM(906,l2); // hora:
	EEPROM.leeCadenaEEPROM(417,buffer2.aux); // AT+CCLK?
	gprs.enviaComando(buffer2.aux);
	l1[7] = buffer2.aux[14];
	l1[8] = buffer2.aux[15];
	l1[9] = buffer2.aux[13];
	l1[10] = buffer2.aux[11];
	l1[11] = buffer2.aux[12];
	l1[12] = buffer2.aux[10];
	l1[13] = buffer2.aux[8];
	l1[14] = buffer2.aux[9];
	l1[15] =0x0;
	l1[16] =0x0;
	for (byte i=17,j=7;i<25;i++,j++){
		l2[j]=buffer2.aux[i];
	}
	l2[15] =0x0;
	l2[16] =0x0;
	for (byte i=0;i<17;i++){
		linea1[i]=l1[i];
		linea2[i]=l2[i];
	}
	myMenu.posicionActual(linea1, linea2);
	/*
	 +CCLK: "15/01/11,16:56:39+02"
	 0123456789012345678901234567890
	 0         1         2         3*/
}

//correcto
bool setFechaHora(byte opcion,char *linea1,char * linea2) {
	bool salida = true;
	char * cadena;
	sprintf(buffer2.aux, "AT+CCLK=\"%c%c/%c%c/%c%c,%c%c:%c%c:%c%c+02\"", linea1[13],
			linea1[14], linea1[10], linea1[11], linea1[7], linea1[8], linea2[7],
			linea2[8], linea2[10], linea2[11], linea2[13], linea2[14]);
	cadena = gprs.enviaComando(buffer2.aux);
	if (cadena == NULL) {
		myMenu.noBlink();
		if (opcion == 2) {
			EEPROM.leeCadenaEEPROM(500,linea2); //Fecha erronea
			myMenu.linea2(linea2);
		} else {
			EEPROM.leeCadenaEEPROM(514,linea2); //Hora erronea
			myMenu.linea2(linea2);
		}
		delay(1000);
		salida = false;
	}
	return salida;

}

void mostrarConfiguracionSeguridad(){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(667,linea1); //SEGURIDAD: NO

	if (EEPROM.read(18)!=0x0){
		linea1[11]='S';
		linea1[12]='I';
		EEPROM.leeCadenaEEPROM(692,linea2); //valor: 0000
		for (byte i=19,j=7;i<23;i++,j++){
			linea2[j]=EEPROM.read(i);
		}
		linea2[11]=0x0;
	}
	linea1[13]=0x0;
	myMenu.posicionActual(linea1,linea2);
}

void mostrarInformacionZonas(){
	boolean salida=false;
	byte zona =1;
	unsigned long tiempoEspera=millis();
	byte maxZona=ControlZonas.getNumeroZonasRiego()-1;
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(527,linea1); //Zonas activas
	EEPROM.leeCadenaEEPROM(559,linea2); //"               "
	linea1[14]=ControlZonas.getNumeroZonasActivas()+48;
	myMenu.posicionActual(linea1,linea2);
	do {
		key=lecturaPulsador();
		if (key != -1) {
			tiempoEspera=millis();
			if (key == 0) {  // Se ha pulsado la tecla derecha
				//zona+1
				zona++;
				if (zona>maxZona){
					zona=1;
				}
			}
			if (key == 3) {  // Se ha pulsado la tecla izquierda
				//zona -1
				zona--;
				if (zona<1){
					zona=maxZona;
				}
			}
			if (key == 4) {  // Se ha pulsado la tecla de seleccion
				//salida
				salida=true;
			}
			if (!salida){
				//mostramos la informacion
				EEPROM.leeCadenaEEPROM(543,linea1); //Zona x act. NO
				linea1[5]=ControlZonas.getNumeroZona(zona)+48;
				if (ControlZonas.isZonaActiva(zona)==true){
					linea1[12]='S';
					linea1[13]='I';
					sprintf(linea2,"%.2i:%.2i %.3d %.3d",ControlZonas.getHoraZona(zona),ControlZonas.getMinutoZona(zona),
							ControlZonas.getLitrosPorRiegoZona(zona),ControlZonas.getDuracionZona(zona));
				}else{
					EEPROM.leeCadenaEEPROM(559,linea2); //"               "
				}
				myMenu.posicionActual(linea1,linea2);
				//Zona x activa: SI/NO
				//hora: xx:yy litros MAX: xxx Duraccion: xxx  Regando? SI/NO Rebenton? SI/NO Manual? SI/NO
			}
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
			}
		}
	}while (!salida);
}

void configuracionSeguridad(){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(667,linea1); //SEGURIDAD: NO
	EEPROM.leeCadenaEEPROM(692,linea2); //valor: 0000
	myMenu.posicionActual(linea1,linea2);
	cambioNumeroSN(linea1,linea2);
	if (linea1[11]=='S'){
		myMenu.posicionActual(linea1,linea2);
		cambioNumero(0,linea1,linea2);
		//guardamos en eeprom
		EEPROM.write(18,1);
		for (byte i=7,j=19;i<11;i++,j++){
			EEPROM.write(j,linea2[i]);
		}
	}else{
		//guardmos en eeprom
		EEPROM.write(18,0);
	}
}

void tratarOpcion(byte x, byte y) {
	byte opcion = (x * numeroMaximoDeSubmenus) + y;
	myMenu.noBlink();
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	Serial.print("opcion: ");Serial.println(opcion); //TODO  a quitar
	switch (opcion) {
	case 0:	// configuracion seguridad;
		configuracionSeguridad();
		break;
	case 1: //configuracion SMS destino
		getSMS(linea1,linea2);
		myMenu.blink();
		cambioNumero(opcion,linea1,linea2);
		setSMS(linea2);
		break;
	case 4: //configuracion zonas
		break;

	case 2: //configuracion Fecha
	case 3: //configuracion hora
	case 7: //mostrar fecha y hora
		getFechaHora(linea1,linea2);
		if ((opcion == 2) || (opcion == 3)) {
			do {
				myMenu.blink();
				cambioNumero(opcion,linea1,linea2);
			} while (setFechaHora(opcion,linea1,linea2) == false);
		}
		break;
	case 6: //mostrar sms destino
		getSMS(linea1,linea2);
		break;
	case 5: //mostrar confirguracion seguridad
		mostrarConfiguracionSeguridad();
		break;
	case 8: // mostar confirguracion zonas
		mostrarInformacionZonas();
		break;
	case 10: // reigo manual
	case 11:
	case 12:
	case 13:
	case 14:
			riegoManual(opcion-9);
		break;
	case 15:  //rearmado problema en zona
	case 16:
	case 17:
	case 18:
		estadoProblemaEnZona(opcion-15);
		break;
	}
	delay(3000);
	myMenu.blink();
}


//correcto
void riegoManual(byte zona){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(639,linea1);//  Riego Manual

	if (cierrePorBateriaBaja==true){
		EEPROM.leeCadenaEEPROM(624,linea2);// BATERIA BAJA
		myMenu.posicionActual(linea1, linea2);
	}else{
		EEPROM.leeCadenaEEPROM(607,linea2);//    INICIANDO
		myMenu.posicionActual(linea1, linea2);

		if (ControlZonas.isManualZona(zona)==true){ //si ya esta regando lo paramos
			ControlZonas.setManualZona(zona,false,0);
			gprs.pararRiegoZona(zona);
			EEPROM.leeCadenaEEPROM(591,linea2);// A P A G A D O

			if (!ControlZonas.isManual()){
				//Si todas las zonas paradas, paramos principal
				valvulaPrincipal=true;
				gprs.valvulaPrincipal(CERRAR);
				ControlZonas.setManualZona(0,false,0);
			}
		}else{ // si no esta regando emepzamos a regar

			ControlZonas.setManualZona(zona,true,millis());
			gprs.iniciarRiegoZona(zona);
			EEPROM.leeCadenaEEPROM(574,linea2);//E N CEN D I D O

			if (ControlZonas.isManual()){
			//Si alguna zona abierta, abrimos principal
				valvulaPrincipal=true;
				gprs.valvulaPrincipal(ABRIR);
			}
		}
		myMenu.posicionActual(linea1, linea2);
	}
}

//correcto
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
			return keyNumero;
		}
	}
	return -1;
}

void cambioNumeroSN(char *linea1,char *linea2){
	bool salida = false;
	unsigned long tiempoEspera=millis();
		char caracter;
		int keyNumero;
		myMenu.borraLinea2();

		myMenu.SetCursor(11, 0);
		caracter = linea1[11];
		do {
			keyNumero=lecturaPulsador();
			if (keyNumero >= 0 && keyNumero < NUM_KEYS) {
				tiempoEspera=millis();
				myMenu.SetCursor(11, 0);
				if ((keyNumero == 1) || (keyNumero == 2)) {   // Se ha pulsado la tecla arriba o abajo
					if (caracter=='S'){
						myMenu.print("NO");
						linea1[11] = 'N';
						linea1[12] = 'O';
						caracter='N';
					}else{
						myMenu.print("SI");
						linea1[11] = 'S';
						linea1[12] = 'I';
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
}

//correcto
void cambioNumero(byte opcion,char *linea1,char* linea2) {
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
		myMenu.borraLinea2();
		linea = linea1;
		break;
	case 3:
		fechaHora=true;
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
			tiempoEspera=millis();
			myMenu.SetCursor(xNumero, columna);
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
				if ((fechaHora==true) && ((xNumero == 9) || (xNumero == 12)))
					xNumero--;
				if (xNumero < xmin)
					xNumero = xmax;
				caracter = linea[xNumero];
			}
			if (keyNumero == 4) { // Se ha pulsado la tecla de seleccion
				salida = true;
			}
			myMenu.SetCursor(xNumero, columna);
		}else{
			if (millis()-tiempoEspera> CINCOSEGUNDOS){
				salida=true;
			}
		}
	} while (!salida);
}

//correcto
void setSMS(char *linea2) {
	EEPROM.escrituraEeprom16(0, linea2);
}

//correcto
void getSMS(char* linea1,char*linea2) {
	myMenu.noBlink();
	EEPROM.lecturaEeprom16(0, linea2);
	if (pantallaEncendida){
		EEPROM.leeCadenaEEPROM(653,linea1);//Destino SMS:
		linea2[0]='+';
		linea2[1]='3';
		linea2[2]='4';
		myMenu.posicionActual(linea1, linea2);
	}
}

void setup() {
	pins_init();
	Serial.begin(4800);
	SEGURIDAD=true;
	cierrePorBateriaBaja= EEPROM.read(16)==0x0?false:true; // no sabemos el estado inicial de la bateria, asi que la leemos

#ifndef RELEASE
	Serial.print(F("Memoria libre: "));	Serial.println(util.freeRam());Serial.println(F("----------------------"));
	Serial.println();
	Serial.print(F("Riego Total V "));Serial.println(RIEGO_VERSION);
	Serial.print(F("Menu V "));	Serial.println(myMenu.libVer());
	Serial.print(F("GPRS V "));	Serial.println(gprs.libVer());
#endif
	myMenu.inicia(gprs.libVer());
#ifdef C_SEGURIDAD
	if (EEPROM.read(18)==0x1){  // contrasenia activa +200
		char *linea1 = buffer2.aux;
		char *linea2 = &buffer2.aux[17];
		EEPROM.leeCadenaEEPROM(681,linea1); //Contraseña
		EEPROM.leeCadenaEEPROM(692,linea2); //valor: 0000
		myMenu.posicionActual(linea1,linea2);
		cambioNumero(0,linea1,linea2);
		for (byte i=0,j=7,k=19;i<4;i++,j++,k++){
			if (linea2[j]!=EEPROM.read(k)){
				SEGURIDAD=false;
				myMenu.noDisplay();
				myMenu.noBlink();
				digitalWrite(RETROILUMINACION, LOW);
				break;
			}
		}
	}
#endif
	if (SEGURIDAD==true){
		myMenu.posicionActual(tituloMenu[x],
				tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
		gprs.inicializaAlarmas(&ControlZonas);

		tiempoLuz=millis();
		tiempoCursor=millis();
		tiempoDisplay=millis();
		tiempo3 = millis();
		segundo = millis();

		reinicio = gprs.iniciaReloj();
	#ifndef RELEASE
		Serial.print(F("millis hasta fin del dia. "));Serial.println(reinicio);
	#endif
	}
#ifndef RELEASE
	Serial.print(F("Memoria libre: "));	Serial.println(util.freeRam());Serial.println(F("----------------------"));
#endif
}

void loop() {
  if (SEGURIDAD==true){
	comandoGPRS();
	controlTiempo();
	key=lecturaPulsador();
	if (key != -1) {
		tiempoCursor=millis();
		tiempoDisplay=millis();
		tiempoLuz=millis();
		digitalWrite(RETROILUMINACION, HIGH);
		luzEncendida = true;
		pantallaEncendida=true;
		myMenu.display(); //encendemos la pantalla
		cursorActivo = true;
		myMenu.blink(); // Mostramos el cursor parpadeando

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
		}
		myMenu.posicionActual(tituloMenu[x],
				tituloSubmenu[(x * numeroMaximoDeSubmenus) + y]);
	}
  }
}

void tratarRespuestaGprs() {
	gprs.limpiaBufferI();
	long tiempoL = millis();
	int lectura=0;
	int contador=0;

	do{
			if (gprs.available()>0){
				lectura=gprs.available();
				for(int i=0;i<lectura;i++){
					if (contador < MAX_BUFFER){
						buffer.buffer[contador]=gprs.read();
						contador++;
					}
				}
			}
	}while(millis()-tiempoL<1000);

	//alama
#ifndef RELEASE
	Serial.println(F("dentro de tratarRespuesta GPRS"));
#endif
	if (buffer.buffer[3]=='C' && buffer.buffer[4]=='A' && buffer.buffer[5]=='L' && buffer.buffer[6]=='V'){
#ifndef RELEASE
		Serial.println(F("dentro de CALV"));
#endif

		byte alarma = buffer.buffer[9] - 48;
		if (!ControlZonas.isReventonZona(alarma)){
#ifndef RELEASE
			Serial.print(F("No hay problema en zona: "));Serial.println(alarma);
#endif
			if (!ControlZonas.isRegandoZona(alarma)){//salta la alarma se establece la duracion
#ifndef RELEASE
				Serial.println(F("inicio riego"));
#endif
				ControlZonas.setRegandoZona(alarma,true);
				gprs.iniciarRiegoZona(alarma);
				gprs.establecerHoraFin(&ControlZonas,alarma);
			} else    // salta la alarma porque ha terminado el tiempo de riego
			{
#ifndef RELEASE
				Serial.println(F("fin riego"));
#endif
				ControlZonas.setRegandoZona(alarma,false);
				gprs.pararRiegoZona(alarma);
				gprs.establecerHoraInicio(&ControlZonas,alarma);
			}
		}
#ifndef RELEASE
		else{
			Serial.print(F("hay problema en zona: "));Serial.print(alarma);	Serial.println(F(" no hace nada"));
		}
		Serial.println(F("fuera de CALV"));
#endif
	}
#ifndef RELEASE
	Serial.println(F("fuera de tratarRespuesta GPRS"));
#endif

}

void comandoGPRS(void) {
	if (gprs.available()) // if date is comming from softwareserial port ==> data is comming from gprs shield
	{
		tratarRespuestaGprs();
	}
#ifndef RELEASE_FINAL
	if (Serial.available()) // if data is available on hardwareserial port ==> data is comming from PC or notebook
	{
		if (tratarRespuestaSerial()==true) {
			gprs.enviaComando(buffer.buffer);       // write it to the GPRS shield
		}
	}
#endif
}

#ifndef RELEASE_FINAL
/////////////////////////////////////////////funciones debug

bool tratarRespuestaSerial() {
	bool salidaRespuesta = true;

	Serial.readString().toCharArray(buffer.buffer,150,0);

	//SMS: tipo
	if (buffer.buffer[0]=='S' && buffer.buffer[1]=='M' &&  buffer.buffer[2]=='S' && buffer.buffer[3]==':'){
		gprs.enviaSMSError(buffer.buffer[4]-48);
		salidaRespuesta=false;
	}

	//imprimir zonas
	if (buffer.buffer[0]=='I' && buffer.buffer[1]==':'){
		ControlZonas.imprimirZonas();
		salidaRespuesta = false;
	}

	//hora
	if (buffer.buffer[0]=='H' && buffer.buffer[1]==':'){
		EEPROM.leeCadenaEEPROM(417,buffer2.aux); // AT+CCLK?
		gprs.enviaComando(buffer2.aux);
		salidaRespuesta = false;
	}
	//alarma
	if (buffer.buffer[0]=='A' && buffer.buffer[1]==':'){
		gprs.enviaComando(F("AT+CALA?"));
		salidaRespuesta = false;
	}
	/*
	//inicio calculo consumo agua
	if (buffer.buffer[0]=='I' && buffer.buffer[1]=='C'  && buffer.buffer[2]==':'){
	#ifndef RELEASE
		Serial.print(F("Inicio consumo agua: "));Serial.println();
	#endif
		medida=millis();
		noInterrupts();
		totalPulsos=0;
		numeroPulsos=0;
		interrupts();
		salidaRespuesta = false;
	}*/

	//escritura en eeprom E:00 xxxxxxxxxxxxxxxxxxxxxxxxxx
	salidaRespuesta=EEPROM.escrituraEEPROM(buffer.buffer);

	//escritura posicion valor  EP:0000 000
	 salidaRespuesta=EEPROM.escrituraPosicionValor(buffer.buffer);

	//borrado de eeprom
	if (buffer.buffer[0]=='E' && buffer.buffer[1]=='R' &&  buffer.buffer[2]==':'){
		salidaRespuesta = EEPROM.borradoEEPROM();
	}

	//reinicio valores zonas
	if (buffer.buffer[0]=='I' && buffer.buffer[1]=='Z' &&  buffer.buffer[2]==':'){
		salidaRespuesta = EEPROM.iniciaZonas();
	}

	//lectura de eeprom bloque  LI:000 o LI:0 o LC:000 o LC:0
	salidaRespuesta=EEPROM.lecturaContenidoEEPROM(buffer.buffer);

	Serial.print(F("fin Evaluando entrada recibida, valor salida respuesta: "));Serial.println(salidaRespuesta);
	return salidaRespuesta;
}
#endif



#include "Riego.h"
#include "GSM.H"
#include "Menu.h"
#include "Botonera.h"

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
int key;
Menu myMenu("", numeroMenusMaximo);
Botonera botonera(&myMenu);
controlZona ControlZonas(&myMenu,&botonera);
GSM gprs(&myMenu,&ControlZonas);

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

void configuracionSeguridad(){
	char *linea1 = buffer2.aux;
	char *linea2 = &buffer2.aux[17];
	EEPROM.leeCadenaEEPROM(667,linea1); //SEGURIDAD: NO
	EEPROM.leeCadenaEEPROM(692,linea2); //valor: 0000
	myMenu.posicionActual(linea1,linea2);
	botonera.cambioValor(linea1,linea2,11);
	if (linea1[11]=='S'){
		myMenu.posicionActual(linea1,linea2);
		botonera.cambioValor(linea1,linea2,20);
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
			if (botonera.lecturaPulsador() == 4) { // Se ha pulsado la tecla de seleccion
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




void controlTiempo(void) {
	/*if (millis()>reinicio){
		//reinicia el sistema
	}*/

/*	if ((millis()- cincomintuos) > CINCOMINUTOS){ //TODO ajustar los voltajes
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
	}*/

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
		gprs.getSMS(linea1,linea2,pantallaEncendida);
		myMenu.blink();
		botonera.cambioValor(linea1,linea2,opcion+20);
		gprs.setSMS(linea2);
		break;
	case 4: //configuracion zonas
		ControlZonas.mostrarConfigurarInformacionZonas(false);
		break;

	case 2: //configuracion Fecha
	case 3: //configuracion hora
	case 7: //mostrar fecha y hora
		gprs.getFechaHora(linea1,linea2);
		if ((opcion == 2) || (opcion == 3)) {
			do {
				myMenu.blink();
				botonera.cambioValor(linea1,linea2, opcion+20);
			} while (gprs.setFechaHora(opcion,linea1,linea2) == false);
		}
		break;
	case 6: //mostrar sms destino
		gprs.getSMS(linea1,linea2,pantallaEncendida);
		break;
	case 5: //mostrar confirguracion seguridad
		mostrarConfiguracionSeguridad();
		break;
	case 8: // mostar confirguracion zonas
		ControlZonas.mostrarConfigurarInformacionZonas(true);
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
		gprs.inicializaAlarmas();

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
	  gprs.comandoGPRS();
	controlTiempo();
	key=botonera.lecturaPulsador();
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



/*
 * controlZona.cpp
 *
 *  Created on: 15/2/2015
 *      Author: Alkimi
 */

#include "controlZona.h"
//#include "miEEPROM.h"

controlZona::controlZona(){
	char aux[33];
	char * aux2= &aux[16];
	byte posicion=0;
	EEPROM.lecturaEeprom16(4,aux);
	EEPROM.lecturaEeprom16(5,aux2);

	for (int i=0;i<NUMERO_ZONAS;i++){
		control[i].numeroZona =i;
		control[i].activa=false;
		control[i].litrosPorRiego=aux[posicion++];
		control[i].horaInicio=aux[posicion++];
		control[i].minutoInicio=aux[posicion++];
		control[i].intervaloRiego=aux[posicion++];
		control[i].duracion=aux[posicion++];
		control[i].litrosTotales=0;
		control[i].tiempo=0;
		control[i].regando=false;
		control[i].reventon=false;
		control[i].manual=false;
	}
	control[0].activa=true;
	zonasRegando=0;
	zonasActivas=0;
	zonasReventon=0;
	maxLitrosRiego=0;
	totalLitros=0;
	totalZonas=NUMERO_ZONAS;

	//E:4 2 4 d C N 0 5 2 d C R 0 5
	//	E:4 2 4 d @ G 0 2 2 d @ H 0 2
		/*
		A	17			0		0			@		16			_		47
		B	18			9		9			I		25			`		48
		C	19			:		10			N		30			a		49
		D	20			;		11			S		35			c		51
		E	21			<		12			X		40			h		56
		F	22			=		13			[		43			k		59
		G	23			>		14			]		44
		H	24			?		15			^		46
		*/


	    /*x  nz l  h  m  i  d  nz l  h  m  i  d  nz l  h
		  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

		  m  i  d  nz l  h  m  i  d  nz l  h  m  i  d*/

}

controlZona::~controlZona() {
	// TODO Auto-generated destructor stub
}

bool controlZona::isRegando(void){
	for (int i=1;i<NUMERO_ZONAS;i++){
		if (control[i].regando){
			control[LLAVE_PRINCIPAL].regando=true;
			return true;
		}
	}
	control[LLAVE_PRINCIPAL].regando=false;
	return false;
}

bool controlZona::isManualZona(byte zona){
	return control[zona].manual;
}

bool controlZona::isRegandoZona(byte zona){
	return control[zona].regando;
}

bool controlZona::isReventonZona(byte zona){
	//leemos el varlo de la epron de la zona
	control[zona].reventon = (EEPROM.read(16+zona)!=0x0)?true:false;
	return control[zona].reventon;
}

bool controlZona::isReventon(void){
	for (byte i=1;i<NUMERO_ZONAS;i++){
		if (isReventonZona(i)){
			return true;
		}
	}
    return false;
}

void controlZona::setFinRiegoZona(byte zona){
	control[zona].litrosTotales=0;
	control[zona].regando=false;
	cuentaZonasRegando();
	isRegando();

}

bool controlZona::setIncrementaLitros(byte litros){
	bool reventon= false;
	if (zonasRegando>0){
		totalLitros+=litros;
		byte incremento = litros / zonasRegando;
		for (int i=1;i<NUMERO_ZONAS;i++){
			if (control[i].activa && control[i].regando &&  !control[i].reventon){
				control[i].litrosTotales+=incremento;
				if (control[i].litrosTotales>control[i].litrosPorRiego){
					control[i].reventon=true;
					reventon= true;
					setRebentonZona(i,reventon);
				}
			}
		}
	}
	cuentaZonasReventon();
	return reventon;

}
void controlZona::setRebentonZona(byte zona,bool estado){
	//invertimos el valor de eprom
	//byte valor= EEPROM.read(16+zona);
	//(valor!=0x0)?valor=0x0:valor=0x1;
	EEPROM.write(16+zona,estado);
}

void controlZona::setLitrosPorRiegoEnZona(byte zona, byte litros){
	control[zona].litrosPorRiego=litros;
	control[zona].activa=true;
	cuentaZonasActivas();

}

void controlZona::setManualZona(byte zona, bool valor,unsigned long tiempo){
	control[zona].manual=valor;
	control[zona].tiempo=tiempo;
	cuentaZonasManual();
}

void controlZona::setRegandoZona(byte zona, bool valor){
	control[zona].regando=valor;
	cuentaZonasRegando();
}

void controlZona::setReiniciaZona(byte zona){
	control[zona].regando = false;
	control[zona].reventon = false;
	setRebentonZona(zona,false);
	totalLitros-= control[zona].litrosTotales;
	if (totalLitros<0){
		totalLitros=0;
	}
	control[zona].litrosTotales=0;
	cuentaZonasRegando();
	cuentaZonasReventon();
}

void controlZona::cuentaZonasRegando(void){
	zonasRegando=0;
	maxLitrosRiego =0;
	for (int i=1;i<NUMERO_ZONAS;i++){
		if (control[i].regando){
			zonasRegando++;
			maxLitrosRiego+=control[i].litrosPorRiego;
		}
	}
}

void controlZona::cuentaZonasActivas(void){
	zonasActivas=0;
	for (int i=1;i<NUMERO_ZONAS;i++){
		if (control[i].activa){
			zonasActivas++;
		}
	}
}

byte controlZona::getHoraZona(byte zona) {
	return control[zona].horaInicio;
}

byte controlZona::getMinutoZona(byte zona) {
	return control[zona].minutoInicio;
}

byte controlZona::getDuracionZona(byte zona) {
	return control[zona].duracion;
}

byte controlZona::getIntervaloZona(byte zona) {
	return control[zona].intervaloRiego;
}

byte controlZona::getNumeroZona(byte zona) {
	return control[zona].numeroZona;
}

void controlZona::cuentaZonasReventon(void){
	zonasReventon=0;
	for (int i=1;i<NUMERO_ZONAS;i++){
		if (control[i].reventon){
			zonasReventon++;
		}
	}
}

void controlZona::cuentaZonasManual(void){
	zonasManual=0;
	for (int i=1;i<NUMERO_ZONAS;i++){
		if (control[i].manual){
			zonasManual++;
		}
	}
}

bool controlZona::isTodasZonasRegando(void){
	return (zonasActivas==zonasRegando)?true:false;
}

bool controlZona::isTodasZonasReventon(void){
	return (zonasActivas==zonasReventon)?true:false;
}

bool controlZona::isMaxLitrosRiego(void){
	return (totalLitros> maxLitrosRiego)?true:false;
}

unsigned long controlZona::getTiempoZona(byte zona){
	return control[zona].tiempo;
}

int controlZona::getTotalLitros(void){
	return totalLitros;
}

int controlZona::getMaxLitrosRiego(void){
	return maxLitrosRiego;
}

byte controlZona::getNumeroZonasRiego(void){
	return totalZonas;
}

bool controlZona::isManual(void){
		return (zonasManual>0)?true:false;
}

bool controlZona::isZonaActiva(byte zona){
	return control[zona].activa;
}


#ifndef RELEASE_FINAL
void controlZona::imprimirZonas(void){
	Serial<<endl <<F("Zonas de riego activas totales: ") << zonasActivas << endl;
	for (int i = 0;i<NUMERO_ZONAS;i++){
		Serial <<F("Zona: ")<<i<<F(" esActiva: ");
		if (control[i].activa){
			Serial <<F("SI");
		}else{
			Serial <<F("NO");
		}
		Serial <<endl;
		if (control[i].activa){
			Serial <<F("\tHora: ")<<control[i].horaInicio<<F(" minutos: ")<<control[i].minutoInicio<<endl;
			Serial <<F("\tDuracion: ")<<control[i].duracion<<F(" minutos")<<endl;
			Serial <<F("\tlitros maximos por riego: ") << control[i].litrosPorRiego<<F(" litros actuales: ")<<control[i].litrosTotales<<endl;
			Serial <<F("\tRegando?  ")<<control[i].regando <<F(" Reventon? ")<<control[i].reventon <<endl;
		}
	}
}
#endif

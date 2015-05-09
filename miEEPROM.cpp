/*
 * miEEPROM.cpp
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#include "Arduino.h"
#include "miEEPROM.h"
#include <avr/eeprom.h>

uint8_t miEEPROM::read(unsigned int address)
{
        return eeprom_read_byte((unsigned char *) address);
}

void miEEPROM::write(unsigned int address, uint8_t value)
{
    	eeprom_write_byte((unsigned char *) address, value);

}

char *miEEPROM::lecturaEeprom16(byte posicion, char *direccion){
	if (posicion>63) posicion=63;
	int pos=posicion<<4; //multiplicamos por 16
	byte i=0;
	for ( i=0;i<16;i++){
		direccion[i]=read(pos+i);
	}
	direccion[i]=0x0;
	return direccion;
}

void miEEPROM::escrituraEeprom16(byte posicion,const char str[]){
	if (posicion>63) posicion=63;
	int pos=posicion<<4;  //multiplicamos por 16;
	for (byte i=0;i<16;i++){
		write((pos++),str[i]);
	}
}

float miEEPROM::readFloat(unsigned int addr){
 dataFloat data;
 for(byte i = 0; i < 4; i++) {
  data.b[i] = EEPROM.read(addr+i);
 }
 return data.f;
}


void miEEPROM::writeFloat(unsigned int addr, float x){
	 dataFloat data;
 data.f = x;
 for(byte i = 0; i < 4; i++) {
    write(addr+i, data.b[i]);
 }
}

boolean miEEPROM::borradoEEPROM(){  //borrado de eeprom
  	for (int j = 0;j<1024;j++){
  		write(j,'\x0');
  	}
  return true;
}

boolean miEEPROM::iniciaZonas(){
	for (byte i=0,j=32;i<5;i++){
		if (i%2==0){
			write(j++,1); // activa
		}else{
			write(j++,0); // activa
		}
		write(j++,100+i); //litros
		write(j++,20); //hora
		write(j++,0); // minutos
		write(j++,0); // frecuencia
		write(j++,60+i); // duraccion
	}
	return true;
}


boolean miEEPROM::contenidoEEPROM(byte pos, boolean tipo) {
	byte valor;
	Serial.println();
	Serial.print(F("0                   1                   2                   3"));Serial.println();
	Serial.print(F("0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1"));Serial.println();
	Serial.print(F("---------------------------------------------------------------"));Serial.println();
	Serial.println();
	int limite;
	if (pos==0)
		limite =320;
	else{
		if (pos>64){
			pos=64;
		}
		limite = pos<<4; //multiplicamos * 16
    }
	for (int j = 0; j < limite; j++) {
		valor = EEPROM.read(j);
		if (j > 0 && j % 32 == 0) {
			Serial.print(F("\t"));Serial.print(j-1);Serial.print(F("\t"));Serial.println(j/16);
		}
		if (tipo==true){
			Serial.print((char)valor);  // mostramos el caracter
		}else{
			Serial.print(valor); // mostramos el numero
		}
		Serial.print(F(" "));
	}
	Serial.print(F("\t"));Serial.print(limite-1);Serial.print(F("\t"));Serial.println(limite/16);
	return true;
}

//escritura posicion valor  EP:0000 000
boolean miEEPROM::escrituraPosicionValor(const char c[]){
	boolean salida=false;
	if (c[0]=='E' && c[1]=='P' &&  c[2]==':'){
		int posicion=0;
		byte valor=0;
		byte mul=1;
		byte i=0;
		for (i=3;i<7;i++){
			if (c[i]==0 || c[i]==255 ||c[i]==32){
				break;
			}
			posicion=(posicion*mul)+(c[i]-48);
			mul=10;
		}
		mul=1;
		i++;
		for (;i<i+4;i++){
			if (c[i]==0 || c[i]==255 || c[i]==32){
				break;
			}
			valor=(valor*mul)+(c[i]-48);
			mul=10;
		}
		write(posicion,valor);
		salida=true;
	}
	return salida;
}

//lectura de eeprom bloque  LI:000 o LI:0 o LC:000 o LC:0
boolean miEEPROM::lecturaContenidoEEPROM(const char c[]){
	boolean salida=false;
if (c[0]=='L' && (c[1]=='I' || c[1] =='C') && c[2]==':'){
		if (c[3]!=0 || c[3]!=255){
			int posicion=0;
			int mul=1;
			for (byte i=3;i<6;i++){
				if (c[i]==0 || c[i]==255){
					break;
				}
				posicion=(posicion*mul)+(c[i]-48);
				mul=10;
			}
			if (c[1]=='I'){
				contenidoEEPROM(posicion,false);
			}else{
				contenidoEEPROM(posicion,true);
			}
		}else{
			if (c[1]=='I'){
				contenidoEEPROM(0,false);
			}else{
				contenidoEEPROM(0,true);
			}
		}
		salida = true;
	}
	return salida;
}


//escritura en eeprom E:00 xxxxxxxxxxxxxxxxxxxxxxxxxx
boolean miEEPROM::escrituraEEPROM(const char c[]){
   boolean salida= false;
	if (c[0]=='E' && c[1]==':'){
		int posicion=0;
		byte mul=1;
		byte i=0;
		for ( i=2;i<4;i++){
			if (c[i]==0 || c[i]==255 || c[i]==32){
				break;
			}
			posicion=(posicion*mul)+(c[i]-48);
			mul=10;
		}
		if (posicion>63 ){
			posicion =63;
		}

		posicion = posicion <<4; //multiplicacion por 16
		i++;
		byte salida1=0;
		do{
			write(posicion, c[i]);
			posicion++;
			i++;
			if (c[i]==0 ||c[i]==255){
				salida1=1;
			}
		}while (salida1==0);
		salida=true;
	}
}

int miEEPROM::leeCadenaEEPROM(int posicion, char c[]){
 char caracter;
 int contador=-1;
 posicion--;
 c--;
  boolean salida=false;
 do{
	 posicion++;
     c++;
	 contador++;

	 caracter = (char)EEPROM.read(posicion);
	 if (caracter=='#'){
		 salida=true;
	 }else{
		c[0]=caracter;
	 }

 }while (!salida);
 c[0]='\x0';
 return contador;
}

miEEPROM EEPROM;



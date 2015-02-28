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
	byte i;
	if (posicion>63) posicion=63;
	int pos=posicion*16;
	for (i=0;i<16;i++){
		direccion[i]=read(pos+i);
	}
	direccion[i]=0x0;
	return direccion;
}

void miEEPROM::escrituraEeprom16(byte posicion,const char str[]){
	byte i;
	if (posicion>63) posicion=63;
	int pos=posicion*16;
	for (i=0;i<16;i++){
		write((pos+i),str[i]);
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
  EEPROM.write(addr+i, data.b[i]);
 }
}

miEEPROM EEPROM;



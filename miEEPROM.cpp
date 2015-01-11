/*
 * miEEPROM.cpp
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#include "Arduino.h"
#include "miEEPROM.h"
#include <avr/eeprom.h>

uint8_t miEEPROM::read(int address)
{
    #ifndef DEBUG_PROCESS
        return eeprom_read_byte((unsigned char *) address);
    #else
        return 1;
    #endif
}

void miEEPROM::write(int address, uint8_t value)
{
    #ifndef DEBUG_PROCESS
    	eeprom_write_byte((unsigned char *) address, value);
    #else
        Serial << F("eeprom write(")<<address<<","<<value<<endl;
    #endif // DEBUG_PROCESS

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

miEEPROM EEPROM;



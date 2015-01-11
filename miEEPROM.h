/*
 * miEEPROM.h
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#ifndef MIEEPROM_H_
#define MIEEPROM_H_

class miEEPROM {
public:
    uint8_t read(int);
    void write(int, uint8_t);
    char *lecturaEeprom16(byte posicion, char * direccion);
	void escrituraEeprom16(byte posicion,const char str[]);
};

extern miEEPROM EEPROM;

#endif /* MIEEPROM_H_ */

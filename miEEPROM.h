/*
 * miEEPROM.h
 *
 *  Created on: 6/1/2015
 *      Author: Alkimi
 */

#ifndef MIEEPROM_H_
#define MIEEPROM_H_

union dataFloat{
  byte b[4];
  float f;
 };

class miEEPROM {
public:
    uint8_t read(unsigned int);
    void write(unsigned int, uint8_t);
    char *lecturaEeprom16(byte posicion, char * direccion);
	void escrituraEeprom16(byte posicion,const char str[]);
	float readFloat(unsigned int addr);
	void writeFloat(unsigned int addr, float x);
	boolean borradoEEPROM();
	boolean escrituraPosicionValor(const char c[]);
	boolean lecturaContenidoEEPROM(const char c[]);
	boolean escrituraEEPROM(const char c[]);
	int leeCadenaEEPROM(int posicion,  char c[]);

	boolean iniciaZonas();

private:
	  boolean contenidoEEPROM(byte pos, boolean tipo);

};

extern miEEPROM EEPROM;

#endif /* MIEEPROM_H_ */

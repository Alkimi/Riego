/*
 * controlZona.h
 *
 *  Created on: 15/2/2015
 *      Author: Alkimi
 */

#ifndef CONTROLZONA_H_
#define CONTROLZONA_H_
#include "Riego.h"


#define NUMERO_ZONAS 5  // 0 la principal 4 de riego
#define LLAVE_PRINCIPAL 0

typedef struct
  {
	byte intervaloRiego;
	byte horaInicio;
	byte minutoInicio;
	byte duracion;
	byte numeroZona;
	byte litrosPorRiego;
	byte litrosTotales;
	bool activa;
	bool regando;
	bool reventon;
  } t_controlZonas;


class controlZona {
public:
	controlZona();
	virtual ~controlZona();

	bool isRegando(void);
	bool isRegandoZona(byte zona);

	bool isReventon(void);
	bool isReventonZona(byte zona);

	bool isTodasZonasRegando(void);
	bool isTodasZonasReventon(void);

	bool isZonaActiva(byte zona);

	bool isMaxLitrosRiego(void);

	bool setIncrementaLitros(byte litros);
	void setFinRiegoZona(byte zona);
	void setRegandoZona(byte zona, bool valor);
	void setLitrosPorRiegoEnZona(byte zona,byte litros);
	void setReiniciaZona(byte zona);

	int getTotalLitros(void);
	int getMaxLitrosRiego(void);
	byte getNumeroZonasRiego(void);
	byte getHoraZona(byte zona);
	byte getMinutoZona(byte zona);
	byte getDuracionZona(byte zona);
	byte getIntervaloZona(byte zona);
	byte getNumeroZona(byte zona);

#ifndef RELEASE_FINAL
	void imprimirZonas(void);
#endif

private:
	//byte zonasActivasRegando(void);
	void cuentaZonasActivas(void);
	void cuentaZonasRegando(void);
	void cuentaZonasReventon(void);
	t_controlZonas control[NUMERO_ZONAS];
	byte zonasRegando;
	byte totalZonas;
	byte zonasActivas;
	byte zonasReventon;
	int maxLitrosRiego;
	int totalLitros;
};

#endif /* CONTROLZONA_H_ */

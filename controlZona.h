/*
 * controlZona.h
 *
 *  Created on: 15/2/2015
 *      Author: Alkimi
 */

#ifndef CONTROLZONA_H_
#define CONTROLZONA_H_
//#include "Riego.h"
#include "Menu.h"
#include "Botonera.h"


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
	unsigned long tiempo;
	bool activa;
	bool regando;
	bool reventon;
	bool manual;
  } t_controlZonas;


class controlZona {
public:
	controlZona(Menu * menu, Botonera * botones);
	virtual ~controlZona();

	bool isRegando(void);
	bool isRegandoZona(byte zona);

	bool isReventon(void);
	bool isReventonZona(byte zona);

	bool isManualZona(byte zona);
	bool isManual(void);

	bool isTodasZonasRegando(void);
	bool isTodasZonasReventon(void);

	bool isZonaActiva(byte zona);

	bool isMaxLitrosRiego(void);

	bool setIncrementaLitros(byte litros);
	void setFinRiegoZona(byte zona);
	void setRegandoZona(byte zona, bool valor);
	void setRebentonZona(byte zona,bool estado);
	void setManualZona(byte zona,bool valor,unsigned long tiempo);
	void setReiniciaZona(byte zona);

	void setHoraZona(byte zona, byte hora);
	void setMinutoZona(byte zona, byte minutos);
	void setDuracionZona(byte zona, byte duracion);
	void setIntervaloZona(byte zona, byte intervalo);
	void setLitrosPorRiegoZona(byte zona, byte litros);
	void setZonaActiva(byte zona,bool activa);

	int getTotalLitros(void);
	int getMaxLitrosRiego(void);
	byte getNumeroZonasRiego(void);
	byte getHoraZona(byte zona);
	byte getMinutoZona(byte zona);
	byte getDuracionZona(byte zona);
	byte getIntervaloZona(byte zona);
	byte getNumeroZona(byte zona);
	byte getLitrosPorRiegoZona(byte zona);

	byte getNumeroZonasActivas();
	unsigned long getTiempoZona(byte zona);

	void mostrarConfigurarInformacionZonas(boolean tipo);

#ifndef RELEASE_FINAL
	void imprimirZonas(void);
#endif

private:
	void cuentaZonasActivas(void);
	void cuentaZonasRegando(void);
	void cuentaZonasReventon(void);
	void cuentaZonasManual(void);
	t_controlZonas control[NUMERO_ZONAS];
	byte zonasRegando;
	byte totalZonas;
	byte zonasActivas;
	byte zonasReventon;
	byte zonasManual;
	int maxLitrosRiego;
	int totalLitros;
	Menu * myMenu;
	Botonera * botonera;

};

#endif /* CONTROLZONA_H_ */

#ifndef Menu_h
#define Menu_h
#include "Riego.h"

#define MENU_LIB_VERSION_MAYOR "0"
#define MENU_LIB_VERSION_MENOR "11.2"

#define MENU_LIB_VERSION MENU_LIB_VERSION_MAYOR"."MENU_LIB_VERSION_MENOR

class Menu {
public:
  Menu() ;
  Menu(String options, unsigned int numOptions) ;
  Menu(String options, unsigned int numOptions, unsigned int currentOption) ;

  void menuDown() ;
  void menuUp() ;
  String getOptionText(unsigned int option) ;
  void show() ;
  unsigned int getCurrentOption() ;
  void setOptions(String options, unsigned int numOptions) ;
  void setOptions(String options, unsigned int numOptions, unsigned int currentOption) ;
  size_t print(const char str[]);
  void noDisplay(void);
  void display(void);
  void noBlink(void);
  void blink(void);
  void botonSelect(byte x,byte y);
  void SetCursor(byte x,byte y);
  size_t write(byte c);
  void posicionActual(const char *x, const char *y);
  void posicionActual(const __FlashStringHelper *x,const __FlashStringHelper *y);
  void inicia(const char GSM[]);
  char * libVer(void);
  void linea1(const char str[]);
  void linea1(const __FlashStringHelper *str);
  void linea2(const char str[]);
  void linea2(const __FlashStringHelper *str);
  void borraLinea1(void);
  void borraLinea2(void);
private:
  unsigned int _currentOption ;
  String _options;
  unsigned int _numOptions ;
  void printOptions() ;
  #ifndef DEBUG_PROCESS
          LiquidCrystal *lcd;
  #endif

};

#endif

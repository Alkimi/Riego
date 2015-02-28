#include "Menu.h"

Menu::Menu()
{
    setOptions("", 0, 0) ;
}

Menu::Menu(String options, unsigned int numOptions)
{
    setOptions(options, numOptions, 0) ;
}

Menu::Menu(String options, unsigned int numOptions, unsigned int currentOption)
{
    setOptions(options, numOptions, currentOption) ;
}

void Menu::show()
{
    printOptions();
}

void Menu::menuDown()
{
    _currentOption++ ;
    if(_currentOption == _numOptions) _currentOption = 0 ;
    printOptions() ;
}

void Menu::menuUp()
{
    if(_currentOption == 0) _currentOption = _numOptions ;
    _currentOption-- ;

    printOptions() ;
}

void Menu::printOptions()
{
#ifndef DEBUG_PROCESS
    lcd->noBlink();
    lcd->setCursor(0,0) ;
    lcd->print(getOptionText(_currentOption)) ;
    lcd->setCursor(0,1) ;
    lcd->print(getOptionText(_currentOption==_numOptions-1?0:_currentOption+1)) ;
    lcd->setCursor(0,0) ;
    lcd->blink();
#endif
}

String Menu::getOptionText(unsigned int option)
{
    String val, options;
    unsigned int j = 0;
    options = _options ;
    for(unsigned int i=0; i <= option; i++)
    {
        j = options.indexOf("|") ;
        j = (j==-1? options.length(): j) ;
        val = options.substring(0, j ) ;
        options = options.substring(j + 1, options.length()) ;
    }
    return val ;
}

unsigned int Menu::getCurrentOption()
{
    return _currentOption ;
}

void Menu::setOptions(String options, unsigned int numOptions)
{
    setOptions(options, numOptions, 0) ;
}

void Menu::setOptions(String options, unsigned int numOptions, unsigned int currentOption)
{
    _options = options ;
    _numOptions = numOptions ;
    _currentOption = currentOption ;
}

void Menu::inicia(const char GSM[])
{
    lcd = new LiquidCrystal(8, 9, 4, 5, 6 , 7);
    lcd->begin(16, 2);
    lcd->clear();
    linea1(F("Riego Total"));
    linea2(F("Menu V "));
    lcd->print(MENU_LIB_VERSION);
#ifdef RELEASE_FINAL
    delay(2000);
#endif
    linea2(F("GSM  V "));
    lcd->print(GSM);
#ifdef RELEASE_FINAL
    delay(4000);
#endif
    linea1(F("Muevase con las "));
    linea2(F("teclas direccion"));
#ifdef RELEASE_FINAL
    delay(4000);
#endif
#ifndef DEBUG_PROCESS
    lcd->clear();
#endif // DEBUG_PROCESS

}

size_t Menu::print(const char str[])
{
    return lcd->write(str);
}

void Menu::noDisplay(void)
{
    lcd->noDisplay();
}

void Menu::display(void)
{
    lcd->display();
}

void Menu::blink(void)
{
    lcd->blink();
}

void Menu::noBlink(void)
{
    lcd->noBlink();
}

void Menu::botonSelect(byte x,byte y)
{
    lcd->clear();
    linea1(F("selec menu: "));
    lcd->write((x+1)+48);
    linea2(F("submenu: "));
    lcd->write((y+1)+48);
    delay(5000);
}

void Menu::SetCursor(byte x,byte y)
{
    lcd->setCursor(x,y);
}

size_t Menu::write(byte c)
{
    return lcd->write(c);
}

void Menu::linea1(const char str[])
{
    lcd->setCursor(0,0);
    lcd->print(str);
}

void Menu::linea1(const __FlashStringHelper *str)
{
    lcd->setCursor(0,0);
    lcd->print(str);
}

void Menu::linea2(const char str[])
{
    lcd->setCursor(0,1);
    lcd->print(str);
}

void Menu::linea2(const __FlashStringHelper *str)
{
    lcd->setCursor(0,1);
    lcd->print(str);
}

void Menu::borraLinea1(void)
{
    lcd->setCursor(0,0);
    for(int i=0; i<15; i++)lcd->write(' ');
}

void Menu::borraLinea2(void)
{
    lcd->setCursor(0,1);
    for(int i=0; i<15; i++)lcd->write(' ');
}

void Menu::posicionActual(const char *x, const char *y)
{
    lcd->clear();
    linea1(x);
    linea2(y);
}

void Menu::posicionActual(const __FlashStringHelper *x,const __FlashStringHelper *y)
{
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print(x);
    lcd->setCursor(0,1);
    lcd->print(y);
}

char* Menu::libVer(void)
{
    return (MENU_LIB_VERSION);
}

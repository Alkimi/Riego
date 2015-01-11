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
#ifndef DEBUG_PROCESS
    lcd = new LiquidCrystal(8, 9, 4, 5, 6 , 7);
    lcd->begin(16, 2);
    lcd->clear();
#else
    Serial << F("new LiquidCrystal(8, 9, 4, 5, 6 , 7)") << endl << F("begin(16,2)") << F("clear()")<< endl;
#endif // DEBUG_PROCESS
    linea1(F("Riego Total"));
    linea2(F("Menu V "));
#ifndef DEBUG_PROCESS
    lcd->print(MENU_LIB_VERSION);
#else
    Serial << MENU_LIB_VERSION << endl;
#endif // DEBUG_PROCESS
#ifdef RELEASE
    delay(2000);
#endif
    linea2(F("GSM  V "));
#ifndef DEBUG_PROCESS
    lcd->print(GSM);
#else
    Serial << GSM << endl;
#endif // DEBUG_PROCESS
#ifdef RELEASE
    delay(4000);
#endif
    linea1(F("Muevase con las "));
    linea2(F("teclas direccion"));
#ifdef RELEASE
    delay(4000);
#endif
#ifndef DEBUG_PROCESS
    lcd->clear();
#endif // DEBUG_PROCESS

}

size_t Menu::print(const char str[])
{
#ifndef DEBUG_PROCESS
    return lcd->write(str);
#else
    Serial << str;
#endif
}

void Menu::noDisplay(void)
{
#ifndef DEBUG_PROCESS
    lcd->noDisplay();
#else
    Serial << F("noDisplay()") << endl;
#endif // DEBUG_PROCESS

}

void Menu::display(void)
{
#ifndef DEBUG_PROCESS
    lcd->display();
#else
    Serial << F("display()") << endl;
#endif // DEBUG_PROCESS

}

void Menu::blink(void)
{
#ifndef DEBUG_PROCESS
    lcd->blink();
#else
    Serial << F("blink()") << endl;
#endif // DEBUG_PROCESS

}

void Menu::noBlink(void)
{
#ifndef DEBUG_PROCESS
    lcd->noBlink();
#else
    Serial << F("noBlink()") << endl;
#endif // DEBUG_PROCESS

}

void Menu::botonSelect(byte x,byte y)
{
#ifndef DEBUG_PROCESS
    lcd->clear();
    linea1(F("selec menu: "));
    lcd->write((x+1)+48);
    linea2(F("submenu: "));
    lcd->write((y+1)+48);
    delay(5000);
#else
    Serial << F("clear()") << endl;
    linea1("selec menu: ");
    Serial << (x+1)+48 << endl;
    linea2("submenu: ");
    Serial << (y+1)+48 << endl;
    delay(5000);
#endif // DEBUG_PROCESS

}

void Menu::SetCursor(byte x,byte y)
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(x,y);
#else
    Serial << F("setCursor(")<< x <<F(",")<<y<<endl;
#endif // DEBUG_PROCESS


}

size_t Menu::write(byte c)
{
#ifndef DEBUG_PROCESS
    return lcd->write(c);
#else
    Serial << c;
#endif // DEBUG_PROCESS

}

void Menu::linea1(const char str[])
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,0);
    lcd->print(str);
#else
    Serial << F("setCursor(0,0)")<<endl << str << endl;
#endif // DEBUG_PROCESS

}

void Menu::linea1(const __FlashStringHelper *str)
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,0);
    lcd->print(str);
#else
    Serial << F("setCursor(0,0)")<<endl << str << endl;
#endif // DEBUG_PROCESS

}

void Menu::linea2(const char str[])
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,1);
    lcd->print(str);
#else
    Serial << F("setCursor(0,1)")<<endl << str << endl;
#endif // DEBUG_PROCESS
}

void Menu::linea2(const __FlashStringHelper *str)
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,1);
    lcd->print(str);
#else
    Serial << F("setCursor(0,1)")<<endl << str << endl;
#endif // DEBUG_PROCESS
}

void Menu::borraLinea1(void)
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,0);
    for(int i=0; i<15; i++)lcd->write(' ');
#else
    Serial << F("setCursor(0,0)")<<endl<<endl;
#endif // DEBUG_PROCESS
}

void Menu::borraLinea2(void)
{
#ifndef DEBUG_PROCESS
    lcd->setCursor(0,1);
    for(int i=0; i<15; i++)lcd->write(' ');
#else
    Serial << F("setCursor(0,0)")<<endl << endl;
#endif // DEBUG_PROCESS
}

void Menu::posicionActual(const char *x, const char *y)
{
#ifndef DEBUG_PROCESS
    lcd->clear();
#else
    Serial << F("clear()")<<endl;
#endif // DEBUG_PROCESS
    linea1(x);
    linea2(y);
}

void Menu::posicionActual(const __FlashStringHelper *x,const __FlashStringHelper *y)
{
#ifndef DEBUG_PROCESS
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print(x);
    lcd->setCursor(0,1);
    lcd->print(y);
#else
    Serial << F("clear()")<<endl << F("setCursor(0,1)")<<  x <<endl<< F("setCursor(0,1)")<< y <<endl;
#endif // DEBUG_PROCESS

}

char* Menu::libVer(void)
{
    return (MENU_LIB_VERSION);
}

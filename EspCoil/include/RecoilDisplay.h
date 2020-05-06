#ifndef RECOIL_DISPLAY_H
#define RECOIL_DISPLAY_H
#include <string.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"

class RecoilDisplay
{
public:
    RecoilDisplay();
    void init();
    void writeDisplay(String text, int fontSize, int alignment, int lineNumber, bool erase, bool drawScreen);
private:

};

#endif
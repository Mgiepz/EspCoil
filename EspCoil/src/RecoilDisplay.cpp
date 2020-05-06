#include "RecoilDisplay.h"
#include "Icons.h"

// Initialize the OLED display using Arduino Wire:
static SSD1306Wire display(0x3c, 5, 4);
static OLEDDisplayUi ui ( &display );

RecoilDisplay::RecoilDisplay(){

}

void RecoilDisplay::init(){
    // Initialising the UI will init the display too.
    //ui.setTargetFPS(10);
    Serial.println("setup Display ");
    display.init();
    display.setFont(ArialMT_Plain_10);
    display.setContrast(255);
    display.clear();
    display.drawXbm(0, 0, recoil_splash_width, recoil_splash_height, recoil_splash_bits);

    display.display();
}
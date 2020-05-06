#include <Arduino.h>
#include <Wire.h>
#include "RecoilBlaster.h"
#include "RecoilDisplay.h"

RecoilDisplay display = RecoilDisplay();
RecoilBlaster blaster = RecoilBlaster::Instance();

void setup()
{
    Serial.begin(9600);
    Serial.print("setup() running on core ");
    Serial.println(xPortGetCoreID());
    display.init();
    blaster.init();
    blaster.connect();
}

void handleSerial() {
 while (Serial.available() > 0) {
   char incomingCharacter = Serial.read();
   switch (incomingCharacter) {
     case 'r':
      blaster.startReload();
      delay(3000);
      blaster.finishReload();
      break;
     case '-':
      break;
    }
 }
}

void loop()
{
    handleSerial();
    delay(1000); // Delay a second between loops.

} // End of loop


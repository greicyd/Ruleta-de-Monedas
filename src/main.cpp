#include <Arduino.h>
#include "RFIDReader.h"

RFIDReader rfidReader;

void setup() {
  rfidReader.setup();
}

void loop() {
  rfidReader.readCard();
  // Otras operaciones en el loop principal.
}

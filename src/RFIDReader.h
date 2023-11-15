#ifndef RFID_READER_H
#define RFID_READER_H

#include <SPI.h>
#include <MFRC522.h>
//gh
#define RST_PIN     22  // Configura estos pines según tu conexión
#define SS_PIN      21  // Configura estos pines según tu conexión
//Hola
class RFIDReader {
public:
  RFIDReader();
  void setup();
  void readCard();
private:
  MFRC522 mfrc522;
};

#endif

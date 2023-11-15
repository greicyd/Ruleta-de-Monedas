#include "RFIDReader.h"

RFIDReader::RFIDReader() : mfrc522(SS_PIN, RST_PIN) {}

void RFIDReader::setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Reader Initialized");
}
#123
void RFIDReader::readCard() {
  
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  Serial.print("Card UID: ");
  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println(cardUID);
  
  mfrc522.PICC_HaltA();
  delay(1000);  // Puedes ajustar este tiempo según tus necesidades


}

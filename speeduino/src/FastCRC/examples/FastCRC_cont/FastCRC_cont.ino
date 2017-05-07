/*
  FastCRC-Example

  (c) Frank Boesing 2014
  
  This example shows how to use the update functions.
*/

#include <FastCRC.h>

FastCRC16 CRC16;

uint8_t buf[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

void setup() {
  uint16_t crc;
  
  delay(1500);
  Serial.begin(115200);

  Serial.println("CRC Example");
  Serial.println();

  Serial.print("CCITT-CRC of \"");

  for (unsigned int i = 0; i < sizeof(buf); i++) {
    Serial.print((char) buf[i]);
  }

  Serial.print("\" is: 0x");
  

  //Calculate first half of buffer: 
  crc = CRC16.ccitt(&buf[0], 4);
  
  //Calculate seconde half of buffer:
  crc = CRC16.ccitt_upd(&buf[4],5);
  
  Serial.println(crc, HEX );

}


void loop() {
}



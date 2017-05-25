/*
  FastCRC-Example

  (c) Frank Boesing 2014
*/

#include <FastCRC.h>

FastCRC32 CRC32;

uint8_t buf[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

void setup() {

  delay(1500);
  Serial.begin(115200);

  Serial.println("CRC Example");
  Serial.println();

  Serial.print("CRC32 of \"");

  for (unsigned int i = 0; i < sizeof(buf); i++) {
    Serial.print((char) buf[i]);
  }

  Serial.print("\" is: 0x");
  Serial.println( CRC32.crc32(buf, sizeof(buf)), HEX );

}


void loop() {
}



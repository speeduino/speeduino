/*
  FastCRC-Example

  (c) Frank Boesing 2014
*/

#include <FastCRC.h>

FastCRC16 CRC16;

uint8_t buf[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

void setup() {

  delay(1500);
  Serial.begin(115200);

  Serial.println("CRC Example");
  Serial.println();

  Serial.print("CCITT-CRC of \"");

  for (unsigned int i = 0; i < sizeof(buf); i++) {
    Serial.print((char) buf[i]);
  }

  Serial.print("\" is: 0x");
  Serial.println( CRC16.ccitt(buf, sizeof(buf)), HEX );

}


void loop() {
}



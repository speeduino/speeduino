//FastCRC
//Validate computed CRCs
//
//(c) Frank Boesing 2014

#include <util/crc16.h>
#include <FastCRC.h>

FastCRC7 CRC7;
FastCRC8 CRC8;
FastCRC16 CRC16;
FastCRC32 CRC32;


uint8_t buf[9] = {'1','2','3','4','5','6','7','8','9'};


void printVals(const char * name, uint32_t check, uint32_t val){
	Serial.print(name);
	if (check == val)
		Serial.print(" is ok");
	else
		Serial.print(" is NOT ok");
	Serial.println();
}

void setup() {
uint32_t crc;

  delay(1500);
  Serial.begin(115200);

  Serial.println("CRC Validation");
  
  crc = CRC7.crc7(buf, sizeof(buf));
  printVals("CRC7", 0x75, crc);
  
  crc = CRC8.smbus(buf, sizeof(buf));
  printVals("SMBUS", 0xf4, crc);

  crc = CRC8.maxim(buf, sizeof(buf));
  printVals("Maxim", 0xa1, crc);

  crc = CRC16.ccitt(buf, sizeof(buf));
  printVals("CCITT", 0x29b1, crc);

  crc = CRC16.mcrf4xx(buf, sizeof(buf));
  printVals("MCRF4XX", 0x6f91, crc);

  crc = CRC16.modbus(buf, sizeof(buf));
  printVals("MODBUS", 0x4b37, crc);

  crc = CRC16.kermit(buf, sizeof(buf));
  printVals("KERMIT", 0x2189, crc);

  crc = CRC16.xmodem(buf, sizeof(buf));
  printVals("XMODEM", 0x31c3, crc);

  crc = CRC16.x25(buf, sizeof(buf));
  printVals("X.25", 0x906e, crc);

  crc = CRC32.crc32(buf, sizeof(buf));
  printVals("CRC32", 0xcbf43926, crc);

  crc = CRC32.cksum(buf, sizeof(buf));
  printVals("CKSUM", 0x765e7680, crc);
}


void loop() {
}

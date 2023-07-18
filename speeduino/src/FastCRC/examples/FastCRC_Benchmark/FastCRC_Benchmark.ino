/*
  FastCRC
   Benchmark

   (c) Frank Boesing 2014-2016

   Edit FastCRC.h for smaller Tables (#define CRC_BIGTABLES 1) - not needed for Teensy 3.x
*/

#include <util/crc16.h>
#include <FastCRC.h>


//#define LOFLASH // <- Uncomment this for devices with small flashmemory

//Determince the max. possible size for the data:
#if defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define BUFSIZE (56 * 1024) // 56KB for Teensy 3.x
#elif defined(__MK20DX128__)
#define BUFSIZE (12 * 1024) // 12KB for Teensy 3.0
#elif defined(__MKL26Z64__)
#define BUFSIZE (4 * 1024)  // 4KB for Teensy LC
#else
#define BUFSIZE (1 * 1024)  // 1KB for Teensy 2.0 / others...(use max. possible!)
#endif

uint8_t buf[BUFSIZE];

FastCRC8 CRC8;
FastCRC16 CRC16;
FastCRC32 CRC32;


// Supporting functions for Software CRC

inline uint16_t softcrc(uint16_t seed, uint8_t *data, uint16_t datalen) {
  for (uint16_t i = 0; i < datalen; i++) {
    seed = _crc16_update(seed,  data[i]);
  }
  return seed;
}

inline uint16_t softcrcIbutton(uint16_t seed, uint8_t *data, uint16_t datalen) {
  for (uint16_t i = 0; i < datalen; i++) {
    seed = _crc_ibutton_update(seed,  data[i]);
  }
  return seed;
}

inline uint16_t softcrcCCIT(uint16_t seed, uint8_t *data, uint16_t datalen) {
  for (uint16_t i = 0; i < datalen; i++) {
    seed = _crc_ccitt_update(seed,  data[i]);
  }
  return seed;
}

inline uint16_t softcrcXMODEM(uint16_t seed, uint8_t *data, uint16_t datalen) {
  for (uint16_t i = 0; i < datalen; i++) {
    seed = _crc_xmodem_update(seed,  data[i]);
  }
  return seed;
}


void printVals(const char * name, const uint32_t crc, const uint32_t time) {
  Serial.print(name);
  Serial.print("\tValue:0x");
  Serial.print(crc, HEX);
  Serial.print(", Time: ");
  Serial.print(time);
  Serial.print(" us (");
  Serial.print((8.*BUFSIZE) / time);
  Serial.println(" mbs)");

}

void setup() {
  int time;
  uint32_t crc;

  delay(1500);
  Serial.begin(115200);

  Serial.println("CRC Benchmark");
  Serial.print("F_CPU: ");
  Serial.print((int) (F_CPU / 1E6));
  Serial.print(" MHz, length: ");
  Serial.print(BUFSIZE);
  Serial.println(" Bytes.");
  Serial.println();

  //Fill array with testdata
  for (uint16_t i = 0; i < BUFSIZE; i++) {
    buf[i] = (i + 1) & 0xff;
  }

  /* 8 BIT */
  time = micros();
  crc = CRC8.maxim(buf, BUFSIZE);
  time = micros() - time;
  printVals("Maxim (iButton) FastCRC:", crc, time);

  time = micros();
  crc = softcrcIbutton(0, buf, BUFSIZE);
  time = micros() - time;
  printVals("Maxim (iButton) builtin:", crc, time);

#if !defined(LOFLASH)
  time = micros();
  crc = CRC16.modbus(buf, BUFSIZE);
  time = micros() - time;
  printVals("MODBUS FastCRC:", crc, time);

  time = micros();
  crc = softcrc(0xffff, buf, BUFSIZE);
  time = micros() - time;
  printVals("MODBUS builtin: ", crc, time);

  time = micros();
  crc = CRC16.xmodem(buf, BUFSIZE);
  time = micros() - time;
  printVals("XMODEM FastCRC:", crc, time);

  time = micros();
  crc = softcrcXMODEM(0, buf, BUFSIZE);
  time = micros() - time;
  printVals("XMODEM builtin: ", crc, time);
#endif

  /* 16 BIT */
  time = micros();
  crc = CRC16.mcrf4xx(buf, BUFSIZE);
  time = micros() - time;
  printVals("MCRF4XX FastCRC:", crc, time);


  time = micros();
  crc = softcrcCCIT(0xffff, buf, BUFSIZE);
  time = micros() - time;
  printVals("MCRF4XX builtin:", crc, time);

#if !defined(LOFLASH)
  time = micros();
  crc = CRC16.kermit(buf, BUFSIZE);
  time = micros() - time;
  printVals("KERMIT FastCRC:", crc, time);
#endif


  /* 32 BIT */
#if !defined(LOFLASH)
  time = micros();
  crc = CRC32.crc32(buf, BUFSIZE);
  time = micros() - time;
  printVals("Ethernet FastCRC:", crc, time);
#endif

}

void loop() {
}



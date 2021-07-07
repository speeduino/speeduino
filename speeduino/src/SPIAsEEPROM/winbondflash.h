/*
	Winbond spi flash memory chip operating library for Arduino
	by WarMonkey (luoshumymail@gmail.com)
	for more information, please visit bbs.kechuang.org
	latest version available on http://code.google.com/p/winbondflash
*/

#ifndef _WINBONDFLASH_H__
#define _WINBONDFLASH_H__

#include <inttypes.h>
#include <SPI.h>
#include <Arduino.h>
#include <errno.h>

//W25Q64 = 256_bytes_per_page * 16_pages_per_sector * 16_sectors_per_block * 128_blocks_per_chip
//= 256b*16*16*128 = 8Mbyte = 64MBits

//W25Q16 = 256_bytes_per_page * 16_pages_per_sector * 16_sectors_per_block * 32_blocks_per_chip
//= 256b*16*16*32 = 2Mbyte = 16MBits

#define _W25Q80  winbondFlashClass::W25Q80
#define _W25Q16  winbondFlashClass::W25Q16
#define _W25Q32  winbondFlashClass::W25Q32
#define _W25Q64  winbondFlashClass::W25Q64
#define _W25Q128 winbondFlashClass::W25Q128

class winbondFlashClass {
public:  
  enum partNumber {
    custom = -1,
    autoDetect = 0,
    W25Q80 = 1,
    W25Q16 = 2,
    W25Q32 = 4,
    W25Q64 = 8,
    W25Q128 = 16
  };

  bool begin(partNumber _partno = autoDetect);
  void end();

  long bytes();
  uint16_t pages();
  uint16_t sectors();
  uint16_t blocks();

  uint16_t read(uint32_t addr,uint8_t *buf,uint16_t n=256);

  void setWriteEnable(bool cmd = true);
  inline void WE(bool cmd = true) {setWriteEnable(cmd);}
  
  //WE() every time before write or erase
  void writePage(uint32_t addr_start,uint8_t *buf, uint16_t n);//addr is 8bit-aligned, 0x00ffff00
  //write a page, sizeof(buf) is 256 bytes
  void eraseSector(uint32_t addr);//addr is 12bit-aligned, 0x00fff000
  //erase a sector ( 4096bytes ), return false if error
  void erase32kBlock(uint32_t addr);//addr is 15bit-aligned, 0x00ff8000
  //erase a 32k block ( 32768b )
  void erase64kBlock(uint32_t addr);//addr is 16bit-aligned, 0x00ff0000
  //erase a 64k block ( 65536b )
  void eraseAll();
  //chip erase, return true if successfully started, busy()==false -> erase complete

  void eraseSuspend();
  void eraseResume();

  bool busy();
  
  uint8_t  readManufacturer();
  uint16_t readPartID();
  uint64_t readUniqueID();
  uint16_t readSR();

private:
  partNumber partno;
  bool checkPartNo(partNumber _partno);

protected:
  virtual void select() = 0;
  virtual uint8_t transfer(uint8_t x) = 0;
  virtual void deselect() = 0;
  
};

class winbondFlashSPI: public winbondFlashClass {
private:
  uint8_t nss;
  SPIClass *spi_port;
  inline void select() {
    digitalWrite(nss,LOW);
  }

  inline uint8_t transfer(uint8_t x) {
    byte y = spi_port->transfer(x);
    return y;
  }

  inline void deselect() {
    digitalWrite(nss,HIGH);
  }

public:
  bool begin(partNumber _partno = autoDetect,SPIClass &_spi = SPI,uint8_t _nss = SS);
  void end();
};

#endif


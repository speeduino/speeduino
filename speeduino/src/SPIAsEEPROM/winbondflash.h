/*
  Winbond spi flash memory chip operating library for Arduino
  by WarMonkey (luoshumymail@gmail.com)
  for more information, please visit bbs.kechuang.org
  latest version available on http://code.google.com/p/winbondflash
*/

#ifndef _WINBONDFLASH_H__
#define _WINBONDFLASH_H__

#define _WINBONDFLASH_16BIT

#include <inttypes.h>
#include <SPI.h>
#include <Arduino.h>
#include <errno.h>

// W25Q64 = 256_bytes_per_page * 16_pages_per_sector * 16_sectors_per_block * 128_blocks_per_chip
//= 256b*16*16*128 = 8Mbyte = 64MBits

// W25Q16 = 256_bytes_per_page * 16_pages_per_sector * 16_sectors_per_block * 32_blocks_per_chip
//= 256b*16*16*32 = 2Mbyte = 16MBits

#define _W25Q80 winbondFlashClass::W25Q80
#define _W25Q16 winbondFlashClass::W25Q16
#define _W25Q32 winbondFlashClass::W25Q32
#define _W25Q64 winbondFlashClass::W25Q64
#define _W25Q128 winbondFlashClass::W25Q128

class winbondFlashClass
{
public:
  enum partNumber
  {
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

  uint16_t read(uint32_t addr_start, uint8_t *buf, uint16_t n = 256);

  void setWriteEnable(bool cmd = true);
  // WE() every time before write or erase
  inline void WE(bool cmd = true) { setWriteEnable(cmd); }
  // write a page, sizeof(buf) is 256 bytes
  void writePage(uint32_t addr_start, uint8_t *buf, uint16_t n); // addr is 8bit-aligned, 0x00ffff00
  // erase a sector ( 4096bytes ), return false if error
  void eraseSector(uint32_t addr); // addr is 12bit-aligned, 0x00fff000
  // erase a 32k block ( 32768b )
  void erase32kBlock(uint32_t addr); // addr is 15bit-aligned, 0x00ff8000
  // erase a 64k block ( 65536b )
  void erase64kBlock(uint32_t addr); // addr is 16bit-aligned, 0x00ff0000
  // chip erase, return true if successfully started, busy()==false -> erase complete
  void eraseAll();

  void eraseSuspend();
  void eraseResume();

  bool busy();

  uint8_t readManufacturer();
  uint16_t readPartID();
  uint64_t readUniqueID();
  uint16_t readSR();

private:
  partNumber partno;
  bool checkPartNo(partNumber _partno);

protected:
  virtual void select() = 0;
  virtual uint8_t transfer(uint8_t x) = 0;
  virtual uint16_t transfer16(uint16_t x) = 0;
  virtual void tranfer(uint8_t *tx_buffer, uint16_t len) = 0;
  virtual void tranfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len) = 0;
  virtual void deselect() = 0;
};

class winbondFlashSPI : public winbondFlashClass
{
private:
  #if defined(ARDUINO_ARCH_STM32) || defined(__IMXRT1062__)
    volatile uint32_t nssMask, *nssPort;
  #else
    volatile uint8_t nssMask, *nssPort;
  #endif
  uint8_t nss;
  SPIClass *spi_port;

  inline void select()
  {
    //digitalWrite(nss, LOW);
    *nssPort &= ~(nssMask); 
  }

  inline void tranfer(uint8_t *tx_buffer, uint16_t len)
  {
    #ifndef ARDUINO_ARCH_STM32
    uint16_t i = 0;
    uint16_t data = 0;
    #ifdef _WINBONDFLASH_16BIT
    while ((len - i) > 2)
    {
      data = (*tx_buffer++ << 8) | *tx_buffer++;
      transfer16(data);
      i += 2;
    }
    #endif
    while (i < len)
    {
      transfer(*tx_buffer++);
      i++;
    }
    #else
    spi_port->transfer(tx_buffer, len);
    #endif
  }

  inline void tranfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t len)
  {
    #ifndef ARDUINO_ARCH_STM32
    uint16_t i = 0;
    #ifdef _WINBONDFLASH_16BIT
    uint16_t data = 0;
    while ((len - i) > 2)
    {
      data = (*tx_buffer++ << 8) | *tx_buffer++;
      data = transfer16(data);
      *rx_buffer++ = (data >> 8) & 0xFF;
      *rx_buffer++ = data & 0xFF;
      i += 2;
    }
    #endif
    while (i < len)
    {
      *rx_buffer++ = transfer(*tx_buffer++);
      i++;
    }
    #else
    spi_port->transfer(tx_buffer, rx_buffer, len);
    #endif
  }

  inline uint8_t transfer(uint8_t x)
  {
    return spi_port->transfer(x);
  }

  inline uint16_t transfer16(uint16_t x)
  {
#ifndef _WINBONDFLASH_16BIT
    uint16_t dataIn;
    dataIn = spi_port->transfer(x >> 8) << 8;
    dataIn |= spi_port->transfer(x & 0xFF);
    return dataIn;
#else
    return spi_port->transfer16(x);
#endif
  }

  inline void deselect()
  {
    //digitalWrite(nss, HIGH);
    *nssPort |= (nssMask);
  }

public:
  bool begin(partNumber _partno = autoDetect, SPIClass &_spi = SPI, uint8_t _nss = SS);
  void end();
};

#endif


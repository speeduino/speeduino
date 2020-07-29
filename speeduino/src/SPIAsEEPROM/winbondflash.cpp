/*
	Winbond spi flash memory chip operating library for Arduino
	by WarMonkey (luoshumymail@gmail.com)
	for more information, please visit bbs.kechuang.org
	latest version available on http://code.google.com/p/winbondflash
*/

#include "winbondflash.h"

//COMMANDS
#define W_EN 	0x06	//write enable
#define W_DE	0x04	//write disable
#define R_SR1	0x05	//read status reg 1
#define R_SR2	0x35	//read status reg 2
#define W_SR	0x01	//write status reg
#define PAGE_PGM	0x02	//page program
#define QPAGE_PGM	0x32	//quad input page program
#define BLK_E_64K	0xD8	//block erase 64KB
#define BLK_E_32K	0x52	//block erase 32KB
#define SECTOR_E	0x20	//sector erase 4KB
#define CHIP_ERASE	0xc7	//chip erase
#define CHIP_ERASE2	0x60	//=CHIP_ERASE
#define E_SUSPEND	0x75	//erase suspend
#define E_RESUME	0x7a	//erase resume
#define PDWN		0xb9	//power down
#define HIGH_PERF_M	0xa3	//high performance mode
#define CONT_R_RST	0xff	//continuous read mode reset
#define RELEASE		0xab	//release power down or HPM/Dev ID (deprecated)
#define R_MANUF_ID	0x90	//read Manufacturer and Dev ID (deprecated)
#define R_UNIQUE_ID	0x4b	//read unique ID (suggested)
#define R_JEDEC_ID	0x9f	//read JEDEC ID = Manuf+ID (suggested)
#define READ		0x03
#define FAST_READ	0x0b

#define SR1_BUSY_MASK	0x01
#define SR1_WEN_MASK	0x02

#define WINBOND_MANUF	0xef

#define DEFAULT_TIMEOUT 200

typedef struct {
    winbondFlashClass::partNumber pn;
    uint16_t id;
    uint32_t bytes;
    uint32_t pages;
    uint16_t sectors;
    uint16_t blocks;
}pnListType;
  
static const pnListType pnList[] PROGMEM = {
    { winbondFlashClass::W25Q80, 0x4014,1048576, 4096, 256, 16  },
    { winbondFlashClass::W25Q16, 0x4015,2097152, 8192, 512, 32  },
    { winbondFlashClass::W25Q32, 0x4016,4194304, 16384,1024,64  },
    { winbondFlashClass::W25Q64, 0x4017,8388608, 32768,2048,128 },
    { winbondFlashClass::W25Q128,0x4018,16777216,65536,4096,256 }
};
  
  
uint16_t winbondFlashClass::readSR()
{
  uint8_t r1,r2;
  select();
  transfer(R_SR1);
  r1 = transfer(0xff);
  deselect();
  deselect();//some delay
  select();
  transfer(R_SR2);
  r2 = transfer(0xff);
  deselect();
  return (((uint16_t)r2)<<8)|r1;
}

uint8_t winbondFlashClass::readManufacturer()
{
  uint8_t c;
  select();
  transfer(R_JEDEC_ID);
  c = transfer(0x00);
  transfer(0x00);
  transfer(0x00);
  deselect();
  return c;
}

uint64_t winbondFlashClass::readUniqueID()
{
  uint64_t uid;
  uint8_t *arr;
  arr = (uint8_t*)&uid;
  select();
  transfer(R_UNIQUE_ID);
  transfer(0x00);
  transfer(0x00);
  transfer(0x00);
  transfer(0x00);
  //for little endian machine only
  for(int i=7;i>=0;i--)
  {
    arr[i] = transfer(0x00);
  }
  deselect();
  return uid;
}

uint16_t winbondFlashClass::readPartID()
{
  uint8_t a,b;
  select();
  transfer(R_JEDEC_ID);
  transfer(0x00);
  a = transfer(0x00);
  b = transfer(0x00);
  deselect();
  return (a<<8)|b;
}

bool winbondFlashClass::checkPartNo(partNumber _partno)
{
  uint8_t manuf;
  uint16_t id;
  
  select();
  transfer(R_JEDEC_ID);
  manuf = transfer(0x00);
  id = transfer(0x00) << 8;
  id |= transfer(0x00);
  deselect();

  if(manuf != WINBOND_MANUF){
    return false;
  }

  if(_partno == custom)
    return true;

  if(_partno == autoDetect)
  {
    for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
    {
      if(id == pgm_read_word(&(pnList[i].id)))
      {
        _partno = (partNumber)pgm_read_byte(&(pnList[i].pn));
        return true;
      }
    }
    if(_partno == autoDetect)
    {
      return false;
    }
  }

  //test chip id and partNo
  for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
  {
    if(_partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
    {
      if(id == pgm_read_word(&(pnList[i].id)))//id equal
        return true;
      else
        return false;
    }
  }
  return false;//partNo not found
}

bool winbondFlashClass::busy()
{
  uint8_t r1;
  select();
  transfer(R_SR1);
  r1 = transfer(0xff);
  deselect();
  if(r1 & SR1_BUSY_MASK)
    return true;
  return false;
}

void winbondFlashClass::setWriteEnable(bool cmd)
{
  select();
  transfer( cmd ? W_EN : W_DE );
  deselect();
}

long winbondFlashClass::bytes()
{
  for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
  {
    if(partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
    {
      return pgm_read_dword(&(pnList[i].bytes));
    }
  }
  return 0;
}

uint16_t winbondFlashClass::pages()
{
  for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
  {
    if(partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
    {
      return pgm_read_word(&(pnList[i].pages));
    }
  }
  return 0;
}

uint16_t winbondFlashClass::sectors()
{
  for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
  {
    if(partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
    {
      return pgm_read_word(&(pnList[i].sectors));
    }
  }
  return 0;
}

uint16_t winbondFlashClass::blocks()
{
  for(uint32_t i=0;i<sizeof(pnList)/sizeof(pnList[0]);i++)
  {
    if(partno == (partNumber)pgm_read_byte(&(pnList[i].pn)))
    {
      return pgm_read_word(&(pnList[i].blocks));
    }
  }
  return 0;
}

bool winbondFlashClass::begin(partNumber _partno)
{
  select();
  transfer(RELEASE);
  deselect();
  delayMicroseconds(15);//>3us
 
  if(!checkPartNo(_partno)) return false;
  else return true;
}

void winbondFlashClass::end()
{
  select();
  transfer(PDWN);
  deselect();
  delayMicroseconds(15);//>3us
}

uint16_t winbondFlashClass::read (uint32_t addr,uint8_t *buf,uint16_t n)
{
  if(busy())
    return 0;
  
  select();
  transfer(READ);
  transfer(addr>>16);
  transfer(addr>>8);
  transfer(addr);
  for(uint16_t i=0;i<n;i++)
  {
    buf[i] = transfer(0x00);
  }
  deselect();
  
  return n;
}

void winbondFlashClass::writePage(uint32_t addr_start,uint8_t *buf, uint16_t n)
{
  select();
  transfer(PAGE_PGM);
  transfer(addr_start>>16);
  transfer(addr_start>>8);
  transfer(addr_start);
  
  
  for(uint16_t i=0; i < n; i++)
  {
      transfer(buf[i]);
  }

  //uint8_t i=0;	
  //do {
  //  transfer(buf[i]);
  //  i++;
  //}while(i!=0);
  
  deselect();
}

void winbondFlashClass::eraseSector(uint32_t addr_start)
{
  select();
  transfer(SECTOR_E);
  transfer(addr_start>>16);
  transfer(addr_start>>8);
  transfer(addr_start);
  deselect();
}

void winbondFlashClass::erase32kBlock(uint32_t addr_start)
{
  select();
  transfer(BLK_E_32K);
  transfer(addr_start>>16);
  transfer(addr_start>>8);
  transfer(addr_start);
  deselect();
}

void winbondFlashClass::erase64kBlock(uint32_t addr_start)
{
  select();
  transfer(BLK_E_64K);
  transfer(addr_start>>16);
  transfer(addr_start>>8);
  transfer(addr_start);
  deselect();
}

void winbondFlashClass::eraseAll()
{
  select();
  transfer(CHIP_ERASE);
  deselect();
}

void winbondFlashClass::eraseSuspend()
{
  select();
  transfer(E_SUSPEND);
  deselect();
}

void winbondFlashClass::eraseResume()
{
  select();
  transfer(E_RESUME);
  deselect();
}

bool winbondFlashSPI::begin(partNumber _partno,SPIClass &_spi,uint8_t _nss)
{
  nss = _nss;
  spi_port = &_spi;
  deselect();
  //Serial.println("SPI OK");

  return winbondFlashClass::begin(_partno);
}

void winbondFlashSPI::end()
{
  winbondFlashClass::end();
  // spi_port->end();
}




#pragma once
#include <Arduino.h>
#include "table.h"

/**
 * @brief Page count, as defined in the INI file
 * 
 * @return uint8_t The page count
 */
uint8_t getPageCount(); 

/**
 * @brief Page size as defined in the .ini file
 * 
 * @param pageNum The page number 
 * @return uint16_t The page size in bytes
 */
uint16_t getPageSize(byte pageNum);


// These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
#define veMapPage    2
#define veSetPage    1 //Note that this and the veMapPage were swapped in Feb 2019 as the 'algorithm' field must be declared in the ini before it's used in the fuel table
#define ignMapPage   3
#define ignSetPage   4//Config Page 2
#define afrMapPage   5
#define afrSetPage   6//Config Page 3
#define boostvvtPage 7
#define seqFuelPage  8
#define canbusPage   9//Config Page 9
#define warmupPage   10 //Config Page 10
#define fuelMap2Page 11
#define wmiMapPage   12
#define progOutsPage 13
#define ignMap2Page  14

/**
 * @brief Gets a single value from a page, with data aligned as per the ini file
 * 
 * @param pageNum The page number to retrieve data from
 * @param offset The address in the page that should be returned. This is as per the page definition in the ini
 * @return byte The requested value
 */
byte getPageValue(byte pageNum, uint16_t offset);

/**
 * @brief Sets a single value from a page, with data aligned as per the ini file
 * 
 * @param pageNum The page number to retrieve data from
 * @param offset The address in the page that should be returned. This is as per the page definition in the ini
 * @param value The new value
 */
void setPageValue(byte pageNum, uint16_t offset, byte value);

/*
 * @brief Calculates and returns the CRC32 value of a given page of memory
 * 
 * @param pageNum The page number to retrieve data from
 * @return uint32_t The CRC
 */
uint32_t calculateCRC32(byte pageNum);

// ============================== Page Iteration ==========================

// Type of entity the offset mapped to
enum struct entity_type : uint8_t { 
    Raw,    // A block of memory
    Table,  // A 3D table
    None,   // No entity, but a valid offset
    End     // The offset was past any known entity for the page
};

struct page_entity_t {
    // The entity that the offset mapped to
    union {
        table3D *pTable;
        void *pData;
    };
    uint8_t page;   // The page the entity belongs to
    uint16_t start; // The start position of the entity, in bytes, from the start of the page
    uint16_t size;  // Size of the entity in bytes
    entity_type type;
};

// Support iteration over a pages entities.
// Check for entity.type==entity_type::End
page_entity_t page_begin(byte pageNum);
page_entity_t advance(const page_entity_t &it);
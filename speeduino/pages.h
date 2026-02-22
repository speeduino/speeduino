#pragma once
#include <Arduino.h>
#include "table3d.h"
#include "config_pages.h"

/**
 * Page size in bytes
 */
uint16_t getPageSize(uint8_t pageNum /**< [in] The page number */ );

// These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
constexpr uint8_t veMapPage     = 2;
constexpr uint8_t veSetPage     = 1; //Note that this and the veMapPage were swapped in Feb 2019 as the 'algorithm' field must be declared in the ini before it's used in the fuel table
constexpr uint8_t ignMapPage    = 3;
constexpr uint8_t ignSetPage    = 4;
constexpr uint8_t afrMapPage    = 5;
constexpr uint8_t afrSetPage    = 6;
constexpr uint8_t boostvvtPage  = 7;
constexpr uint8_t seqFuelPage   = 8;
constexpr uint8_t canbusPage    = 9;
constexpr uint8_t warmupPage    = 10;
constexpr uint8_t fuelMap2Page  = 11;
constexpr uint8_t wmiMapPage    = 12;
constexpr uint8_t progOutsPage  = 13;
constexpr uint8_t ignMap2Page   = 14;
constexpr uint8_t boostvvtPage2 = 15;
constexpr uint8_t MIN_PAGE_NUM  = veSetPage;
constexpr uint8_t MAX_PAGE_NUM  = (boostvvtPage2+1U);

// ============================== Per-byte page access ==========================

/** @brief Gets a single value from a page, with data aligned as per the ini file */
byte getPageValue(  uint8_t pageNum,       /**< [in] The page number to retrieve data from. */
                    uint16_t offset     /**< [in] The address in the page that should be returned. This is as per the page definition in the ini. */
                    );

/** 
 * @brief Sets a single value from a page, with data aligned as per the ini file 
 * 
 * @returns true if value set, false otherwise
 */
bool setPageValue(  uint8_t pageNum,       /**< [in] The page number to update. */
                    uint16_t offset,    /**< [in] The offset within the page.  */
                    byte value          /**< [in] The new value */
                    );


// ============================== Page Iteration ==========================

// A logical TS page is actually multiple in memory entities. Allow iteration
// over those entities.

// Type of entity
enum class EntityType : uint8_t { 
    Raw,        // A block of memory
    Table,      // A 3D table
    NoEntity,   // No entity, but a valid offset
    End         // The offset was past any known entity for the page
};

/** @brief The *unique* location of an entity within all pages */
struct entity_page_location_t {
    uint8_t page;   // The index of the page the entity belongs to
    uint8_t index;  // The sub-index of the item within the page

    constexpr entity_page_location_t(uint8_t pageNum, uint8_t pageSubIndex)
    : page(pageNum)
    , index(pageSubIndex)
    {        
    }

    friend bool operator==(const entity_page_location_t &lhs, const entity_page_location_t &rhs)
    {
        return lhs.page==rhs.page
            && lhs.index==rhs.index;
    }

    friend bool operator!=(const entity_page_location_t &lhs, const entity_page_location_t &rhs)
    {
        return !(lhs==rhs);
    }    
};

/** @brief  Position and size of an entity within a page */
struct entity_page_address_t {
    uint16_t start; // The start position of the entity, in bytes, from the start of the page
    uint16_t size;  // Size of the entity in bytes

    entity_page_address_t(uint16_t base, uint16_t length)
    : start(base)
    , size(length)
    {        
    }

    /**
     * @brief Check if the offset is within the entity address range
     * 
     * @param offset Address offset from the start of the page
     * @return if the offset is within the entity address range, false otherwise
     */
    bool isOffsetInEntity(uint16_t offset) const
    {
        return offset >= start && offset < start+size;
    }

    entity_page_address_t next(uint16_t nextBlockSize) const
    {
        return entity_page_address_t(start+size, nextBlockSize);
    }
};

// A entity on a logical page.
struct page_iterator_t {
    union 
    {
        table3d_t *pTable;      // If the entity is a table, this points to the table
        config_page_t *pRaw;    // If the entity is a raw block, this points to it
    };
    EntityType type;
    table_type_t table_key = table_type_None;
    entity_page_location_t location;
    entity_page_address_t address;

    page_iterator_t(EntityType theType, const entity_page_location_t &entityLocation, const entity_page_address_t &entityAddress)
    : type(theType)
    , location(entityLocation)    
    , address(entityAddress)
    {
    }

    void setNoEntity(void)
    {
        pRaw = nullptr;
        type = EntityType::NoEntity;
        table_key = table_type_None;
    }

    void setTable(table3d_t *table, table_type_t key)
    {
        pTable = table;
        type = EntityType::Table;
        table_key = key;
    }

    void setRaw(config_page_t *pBuffer)
    {
        pRaw = pBuffer;
        type = EntityType::Raw;
        table_key = table_type_None;
    }
};

// ============================== Per-byte entity access ==========================

/** @brief Gets a single value from an entity, with data aligned as per the ini file */
byte getEntityValue(const page_iterator_t &entity,  /**< [in] The entity to update */ 
                    uint16_t offset                /**< [in] The offset within the entity */
                    );

/**
 * @brief Sets a single value from a page, with data aligned as per the ini file
 * @returns true if value set, false otherwise
 */
bool setEntityValue(page_iterator_t &entity,  /**< [in] The entity to update */ 
                    uint16_t offset,          /**< [in] The offset within the entity */
                    byte value                /**< [in] The new value */
                    );

/**
 * Initiates iteration over a pages entities.
 * Test `entity.type==End` to determine the end of the page.
 */
page_iterator_t page_begin(uint8_t pageNum /**< [in] The page number to iterate over. */);

/**
 * Moves the iterator to the next sub-entity on the page
 */
page_iterator_t advance(const page_iterator_t &it /**< [in] The current iterator */);

/**
 * Convert page iterator to table value iterator.
 */
table_value_iterator rows_begin(const page_iterator_t &it);

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_begin(const page_iterator_t &it);

/**
 * Convert page iterator to table x axis iterator.
 */
table_axis_iterator x_rbegin(const page_iterator_t &it);

/**
 * Convert page iterator to table y axis iterator.
 */
table_axis_iterator y_begin(const page_iterator_t &it);

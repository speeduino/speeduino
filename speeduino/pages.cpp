#include "pages.h"

entity_address map_page_offset_to_memory(uint8_t pageNumber, uint16_t offset)
{
  switch (pageNumber)
  {
    case veMapPage:
      return { { &fuelTable, offset }, .type = Table };
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      return { { &ignitionTable, offset }, .type = Table };
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      return { { &afrTable, offset }, .type = Table };
      break;

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
      if (offset < 80) //New value is on the Y (TPS) axis of the boost table
      {
        return { { &boostTable, offset }, .type = Table };
      }
      else if (offset < 160)
      {
        return { { &vvtTable, offset-80 }, .type = Table };
      }
      else  if (offset < 240)
      {
        return { { &stagingTable, offset-160 }, .type = Table };
      }
      break;

    case seqFuelPage:
      if (offset < 48) 
      {
        return { { &trim1Table, offset }, .type = Table };
      }
      //Trim table 2
      else if (offset < 96) 
      { 
        return { { &trim2Table, offset-48 }, .type = Table };
      }
      //Trim table 3
      else if (offset < 144)
      {
        return { { &trim3Table, offset-96 }, .type = Table };
      }
      //Trim table 4
      else if (offset < 192)
      {
        return { { &trim4Table, offset-144 }, .type = Table };
      }
      //Trim table 5
      else if (offset < 240)
      {
        return { { &trim5Table, offset-192 }, .type = Table };
      }
      //Trim table 6
      else if (offset < 288)
      {
        return { { &trim6Table, offset-240 }, .type = Table };
      }
      //Trim table 7
      else if (offset < 336)
      {
        return { { &trim7Table, offset-288 }, .type = Table };
      }
      //Trim table 8
      else if (offset<384)
      {
        return { { &trim8Table, offset-336 }, .type = Table };
      }
      break;

    case fuelMap2Page:
      return { { &fuelTable2, offset }, .type = Table };
      break;

    case wmiMapPage:
      if (offset < 80) //New value is on the Y (MAP) axis of the wmi table
      {
        return { { &wmiTable, offset }, .type = Table };
      }
      //End of wmi table
      else if (offset>159 && offset<240)
      {
        return { { &dwellTable, offset-160 }, .type = Table };
      }
      break;
    
    case ignMap2Page: //Ignition settings page (Page 2)
      return { { &ignitionTable2, offset }, .type = Table };
      break;

    case veSetPage: 
      return { { (table3D*)&configPage2, offset }, .type = Raw };
      break;

    case ignSetPage: 
      return { { (table3D*)&configPage4, offset }, .type = Raw };
      break;

    case afrSetPage: 
    return { { (table3D*)&configPage6, offset }, .type = Raw };
      break;

    case canbusPage:  
      return { { (table3D*)&configPage9, offset }, .type = Raw };
      break;

    case warmupPage: 
      return { { (table3D*)&configPage10, offset }, .type = Raw };
      break;

    case progOutsPage: 
      return { { (table3D*)&configPage13, offset }, .type = Raw };
      break;      

    default:
      break;
  }
  
  return { { nullptr, 0 }, .type = page_subtype_t::None };
}
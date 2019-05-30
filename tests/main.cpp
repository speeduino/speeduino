#include <iostream>
#include <Arduino.h>
#include <EEPROM.h>
#include "init.h"
#include "variables.h"
#include "storage.h"

struct config2 myPage2  = *(config2*)(&EEPROM.memory[EEPROM_CONFIG2_START]);
struct config4 myPage4  = *(config4*)(&EEPROM.memory[EEPROM_CONFIG4_START]);
struct config6 myPage6  = *(config6*)(&EEPROM.memory[EEPROM_CONFIG6_START]);
struct config9 myPage9  = *(config9*)(&EEPROM.memory[EEPROM_CONFIG9_START]);
struct config10 myPage10 = *(config10*)(&EEPROM.memory[EEPROM_CONFIG10_START]);

void init_memory()
{
    int i = EEPROMClass::mem_size;
    while(i--) EEPROM.write(i,i);
}

int main()
{
    init_memory();
    std::cout << "Hello World!" << std::endl;
    initialiseAll();
    std::cout << "Done!" << std::endl;
    return 0;
}
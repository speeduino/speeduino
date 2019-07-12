#pragma once

#include <fstream>

class EEPROMClass
{
public:
    static const int mem_size = 4*1024;

private:
    char memory[mem_size];
    std::fstream file;

public:
    EEPROMClass()
    {
        file.open("myeeprom.bin", std::ios::in | std::ios::binary);
	if(file.is_open())
        {
            file.seekg(0, std::ios::beg);
            file.read(memory, mem_size);
            file.close();
        }
        else
        {
            for(int i = 0; i < EEPROMClass::mem_size; i++) memory[i] = 0xff;
        }
    }
    ~EEPROMClass()
    {
        file.open("myeeprom.bin", std::ios::out | std::ios::binary);
        if(file.is_open())
        {
            file.seekp(0, std::ios::beg);
            file.write(memory, mem_size);
            file.close();
            std::cout << "closing myeeprom.bin" << std::endl;
        }
    }
    int read(int address)
    {
        int value = 255;
        if(address >= 0 && address < mem_size) value = (memory[address] & 0xff);
        else std::cout << "read address out of range:" << address << std::endl;
        return value;
    }
    void write(int address, int value)
    {
        if(address >= 0 && address < mem_size) memory[address] = (value & 0xff);
        else std::cout << "write address out of range:" << address << std::endl;
    }
    void update(int address, int value)
    {
        if(read(address) != value) write(address, value);
    }
};
extern EEPROMClass EEPROM;

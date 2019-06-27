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
    }
    ~EEPROMClass()
    {
        file.open("myeeprom.bin", std::ios::out | std::ios::binary);
        if(file.is_open())
        {
            file.seekp(0, std::ios::beg);
            file.write(memory, mem_size);
            file.close();
        }
    }
    int read(int address)
    {
        if(address >= 0 && address < mem_size) return memory[address];
        return 0;
    }
    void write(int address, int value)
    {
        if(address >= 0 && address < mem_size) memory[address] = value;
    }
    void update(int address, int value)
    {
        if(read(address) != value) write(address, value);
    }
};
extern EEPROMClass EEPROM;

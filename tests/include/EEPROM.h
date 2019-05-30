#pragma once

class EEPROMClass
{
public:
    static const int mem_size = 4069;
    char memory[mem_size];

    int read(int i)
    {
        if(i >= 0 && i < mem_size) return memory[i];
        return 0;
    }
    void write(int i, int j)
    {
        if(i >= 0 && i < mem_size) memory[i] = j;
    }
    void update(int i, int j)
    {
        if(read(i) != j) write(i, j);
    }
};
extern EEPROMClass EEPROM;

#include "heap_alloc.h"

const uint16_t _HEAP_SIZE = 2500; // Table alloc takes 2144 bytes on mega2560
static uint8_t _heap[_HEAP_SIZE];
static uint16_t _heap_pointer = 0;

void* heap_alloc(uint16_t size)
{
    uint8_t* value = nullptr;
    if (size < (_HEAP_SIZE - _heap_pointer))
    {
        value = &_heap[_heap_pointer];
        _heap_pointer += size;
    }
    return value;
}

uint16_t heap_freespace()
{
    return _HEAP_SIZE - _heap_pointer;
}
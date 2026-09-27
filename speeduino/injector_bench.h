#pragma once
#include <stdint.h>
uint8_t injectorBenchTelemetry();
bool injectorBenchOwnsOutputs();
bool injectorBenchOwnsPin(uint8_t pin);
void injectorBenchTick();
void injectorBenchKeepAlive();
void injectorBenchStop();
// In-place framed command handler; returns response length including result code.
uint16_t injectorBenchCommand(uint8_t *payload, uint16_t length);

struct FuelSchedule;
bool injectorBenchHandleTimer(FuelSchedule &schedule);

#pragma once

/**
 * @file 
 * @brief Unit conversion functions and constants.
 */

#include <stdint.h>
#include "maths.h"

/**< All temperature measurements are stored offset by 40 degrees.
This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215 */
constexpr int16_t CALIBRATION_TEMPERATURE_OFFSET = 40; 

/* TO DO: store *all* temps in storage units: *only* convert to display units during I/O. 
This will avoid a lot of conversions (speed, simplicity) */

/**
 * @brief Convert an internal temperature value (-40 to 215°C) to a storage value (0-255).
 * 
 * @param temperature in °C, from -40 to 215. 
 * @return uint8_t 
 */
static constexpr inline uint8_t temperatureAddOffset(int16_t temperature) 
{
  return temperature + CALIBRATION_TEMPERATURE_OFFSET;
}

/**
 * @brief Convert a storage value (0-255) to an internal temperature value (-40 to 215°C).
 * 
 * Reverse operation of temperatureAddOffset.
 * 
 * @param temperature 
 * @return int8_t 
 */
static constexpr inline int16_t temperatureRemoveOffset(uint8_t temperature) 
{
  return (int16_t)temperature - CALIBRATION_TEMPERATURE_OFFSET;
}

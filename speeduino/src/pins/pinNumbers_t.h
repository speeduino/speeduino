#pragma once

#include <stdint.h>
#include "../../board_definition.h"
#include "../../config_pages.h"

/** @brief An array of pin numbers */
template <uint8_t N>
struct pin_array_t
{
  uint8_t pins[N] = {0};

  /**
   * @brief Populate by copying from a source array stored in flash
   * 
   * Handles array length mismatches. 
   */
  void __attribute__((optimize("Os"))) copy_P(const uint8_t *pSrc, uint8_t length)
  {
    uint8_t elementsToCopy = min(N, length);
    (void)memcpy_P(pins, pSrc, elementsToCopy*sizeof(pins[0]));
    // Fill remainder with zero
    memset(pins+elementsToCopy, 0, (N-elementsToCopy)*sizeof(pins[0]));
  }

  /** @brief Does the array contain \p pin? */
  bool isPinUsed(uint8_t pin) const
  {
    uint8_t index = 0;
    while (index<N && pin!=pins[index])
    {
      ++index;
    }
    return index<N;
  }
};

/** @brief Injector control pin numbers */
struct injector_pins_t : pin_array_t<INJ_CHANNELS>
{
  /**
   * @brief Populate by copying from a source array stored in flash
   * 
   * Limits the source array length to the number of injectors defined in
   * the tune.
   */
  void copy_P(const uint8_t *pSrc, uint8_t length, const config2 &page2);
};

/** @brief Coil control pin numbers */
struct coil_pins_t : pin_array_t<IGN_CHANNELS>
{
};

struct decoder_pins_t
{
  uint8_t primary = 0; ///< Crank angle sensor pin
  uint8_t secondary = 0; ///< Cam angle sensor pin
  uint8_t tertiary = 0; ///< 2nd cam angle sensor pin (often VVT)
};

/** @brief Input sensor pins */
struct sensor_pins_t
{
  uint8_t TPS = 0;///< TPS input pin
  uint8_t MAP = 0; ///< MAP sensor pin
  uint8_t EMAP = 0; ///< EMAP sensor pin
  uint8_t IAT = 0; ///< IAT sensor pin
  uint8_t CLT = 0; ///< CLS sensor pin
  uint8_t O2 = 0; ///< O2 Sensor pin
  uint8_t O2_2 = 0; ///< Second O2 pin
  uint8_t Bat = 0; ///< Battery voltage pin
  uint8_t CTPS; ///< Input for triggering closed throttle state
  uint8_t flex; ///< Pin with the flex sensor attached
  uint8_t fuelPressure;
  uint8_t oilPressure;
  uint8_t baro; //Pin that an external barometric pressure sensor is attached to (If used)

  bool isPinUsed(uint8_t pin) const;
};

/** @brief Store the pin assignments, as defined by the board */
struct pinNumbers_t
{
  injector_pins_t injectorPins;
  coil_pins_t coilPins;
  decoder_pins_t triggerPins;
  sensor_pins_t sensors;
 uint8_t pinTachOut = 0; //Tacho output
 uint8_t pinFuelPump = 0; //Fuel pump on/off
 uint8_t pinIdle1 = 0; //Single wire idle control
 uint8_t pinIdle2 = 0; //2 wire idle control
 uint8_t pinIdleUp = 0; //Input for triggering Idle Up
 uint8_t pinIdleUpOutput = 0; //Output that follows (normal or inverted) the idle up pin
 uint8_t pinFuel2Input = 0; //Input for switching to the 2nd fuel table
 uint8_t pinSpark2Input = 0; //Input for switching to the 2nd ignition table
 uint8_t pinBoost = 0;
 uint8_t pinVVT_1 = 0;		// vvt output 1
 uint8_t pinVVT_2 = 0;		// vvt output 2
 uint8_t pinFan = 0;       // Cooling fan output
 uint8_t pinStepperDir = 0; //Direction pin for the stepper motor driver
 uint8_t pinStepperStep = 0; //Step pin for the stepper motor driver
 uint8_t pinStepperEnable = 0; //Turning the DRV8825 driver on/off
 uint8_t pinLaunch = 0;
 uint8_t pinIgnBypass = 0; //The pin used for an ignition bypass (Optional)
 uint8_t pinVSS = 0; 
 uint8_t pinResetControl = 0; // Output pin used control resetting the Arduino
 uint8_t pinWMIEmpty = 0; // Water tank empty sensor
 uint8_t pinWMIIndicator = 0; // No water indicator bulb
 uint8_t pinWMIEnabled = 0; // ON-OFF output to relay/pump/solenoid
 uint8_t pinSDEnable = 0; //Input for manually enabling SD logging
 uint8_t pinAirConComp = 0;    // Air conditioning compressor output
 uint8_t pinAirConFan = 0;    // Stand-alone air conditioning fan output
 uint8_t pinAirConRequest = 0; // Air conditioning request input
#if defined(MC33810_SUPPORT)
  uint8_t pinMC33810_1_CS = 0;
  uint8_t pinMC33810_2_CS = 0;
  uint8_t mc33810InjBits[8];
  uint8_t mc33810IgnBits[8];
#endif
};
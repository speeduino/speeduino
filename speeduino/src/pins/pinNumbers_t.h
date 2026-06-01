#pragma once

#include <stdint.h>
#include "../../board_definition.h"
#include "../../config_pages.h"
#include "../stdlib/array.h"

/** @brief An array of pin numbers */
template <uint8_t N, typename base_type = array<uint8_t, N>>
struct pin_array_t : public base_type
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr pin_array_t()
  {
    base_type::fill(NOT_A_PIN);
  }
  // LCOV_EXCL_STOP

  template <uint8_t M>
  explicit constexpr pin_array_t(const uint8_t (&initPins)[M])
  : pin_array_t()
  {
    copy(this->_elements, initPins);
  }

  /** @brief Does the array contain \p pin? */
  bool __attribute__((optimize("Os"))) isPinUsed(uint8_t pin) const
  {
    uint8_t index = NOT_A_PIN;
    while (index<N && pin!=this->operator[](index))
    {
      ++index;
    }
    return index<N;
  }
};

/** @brief Injector control pin numbers */
using injector_pins_t = pin_array_t<INJ_CHANNELS>;

/** @brief Coil control pin numbers */
using coil_pins_t = pin_array_t<IGN_CHANNELS>;

/** @brief Store the pin assignments, as defined by the board */
struct pinNumbers_t
{
  injector_pins_t injectorPins;
  coil_pins_t coilPins;
  uint8_t pinTrigger = NOT_A_PIN; //The CAS pin
  uint8_t pinTrigger2 = NOT_A_PIN; //The Cam Sensor pin known as secondary input
  uint8_t pinTrigger3 = NOT_A_PIN;	//the 2nd cam sensor pin known as tertiary input
  uint8_t pinTPS = NOT_A_PIN;//TPS input pin
  uint8_t pinMAP = NOT_A_PIN; //MAP sensor pin
  uint8_t pinEMAP = NOT_A_PIN; //EMAP sensor pin
  uint8_t pinMAP2 = NOT_A_PIN; //2nd MAP sensor (Currently unused)
  uint8_t pinIAT = NOT_A_PIN; //IAT sensor pin
  uint8_t pinCLT = NOT_A_PIN; //CLS sensor pin
  uint8_t pinO2 = NOT_A_PIN; //O2 Sensor pin
  uint8_t pinO2_2 = NOT_A_PIN; //second O2 pin
  uint8_t pinBat = NOT_A_PIN; //Battery voltage pin
  uint8_t pinDisplayReset = NOT_A_PIN; // OLED reset pin
  uint8_t pinTachOut = NOT_A_PIN; //Tacho output
  uint8_t pinFuelPump = NOT_A_PIN; //Fuel pump on/off
  uint8_t pinIdle1 = NOT_A_PIN; //Single wire idle control
  uint8_t pinIdle2 = NOT_A_PIN; //2 wire idle control (Not currently used)
  uint8_t pinIdleUp = NOT_A_PIN; //Input for triggering Idle Up
  uint8_t pinIdleUpOutput = NOT_A_PIN; //Output that follows (normal or inverted) the idle up pin
  uint8_t pinCTPS = NOT_A_PIN; //Input for triggering closed throttle state
  uint8_t pinFuel2Input = NOT_A_PIN; //Input for switching to the 2nd fuel table
  uint8_t pinSpark2Input = NOT_A_PIN; //Input for switching to the 2nd ignition table
  uint8_t pinSpareTemp1 = NOT_A_PIN; // Future use only
  uint8_t pinSpareTemp2 = NOT_A_PIN; // Future use only
  uint8_t pinSpareOut1 = NOT_A_PIN; //Generic output
  uint8_t pinSpareOut2 = NOT_A_PIN; //Generic output
  uint8_t pinSpareOut3 = NOT_A_PIN; //Generic output
  uint8_t pinSpareOut4 = NOT_A_PIN; //Generic output
  uint8_t pinSpareOut5 = NOT_A_PIN; //Generic output
  uint8_t pinSpareOut6 = NOT_A_PIN; //Generic output
  uint8_t pinSpareHOut1 = NOT_A_PIN; //spare high current output
  uint8_t pinSpareHOut2 = NOT_A_PIN; // spare high current output
  uint8_t pinSpareLOut1 = NOT_A_PIN; // spare low current output
  uint8_t pinSpareLOut2 = NOT_A_PIN; // spare low current output
  uint8_t pinSpareLOut3 = NOT_A_PIN;
  uint8_t pinSpareLOut4 = NOT_A_PIN;
  uint8_t pinSpareLOut5 = NOT_A_PIN;
  uint8_t pinBoost = NOT_A_PIN;
  uint8_t pinVVT_1 = NOT_A_PIN;		// vvt output 1
  uint8_t pinVVT_2 = NOT_A_PIN;		// vvt output 2
  uint8_t pinFan = NOT_A_PIN;       // Cooling fan output
  uint8_t pinStepperDir = NOT_A_PIN; //Direction pin for the stepper motor driver
  uint8_t pinStepperStep = NOT_A_PIN; //Step pin for the stepper motor driver
  uint8_t pinStepperEnable = NOT_A_PIN; //Turning the DRV8825 driver on/off
  uint8_t pinLaunch = NOT_A_PIN;
  uint8_t pinIgnBypass = NOT_A_PIN; //The pin used for an ignition bypass (Optional)
  uint8_t pinFlex = NOT_A_PIN; //Pin with the flex sensor attached
  uint8_t pinVSS = NOT_A_PIN; 
  uint8_t pinBaro = NOT_A_PIN; //Pin that an external barometric pressure sensor is attached to (If used)
  uint8_t pinResetControl = NOT_A_PIN; // Output pin used control resetting the Arduino
  uint8_t pinFuelPressure = NOT_A_PIN;
  uint8_t pinOilPressure = NOT_A_PIN;
  uint8_t pinWMIEmpty = NOT_A_PIN; // Water tank empty sensor
  uint8_t pinWMIIndicator = NOT_A_PIN; // No water indicator bulb
  uint8_t pinWMIEnabled = NOT_A_PIN; // ON-OFF output to relay/pump/solenoid
#if defined(MC33810_SUPPORT)
  uint8_t pinMC33810_1_CS = NOT_A_PIN;
  uint8_t pinMC33810_2_CS = NOT_A_PIN;
  uint8_t mc33810InjBits[8];
  uint8_t mc33810IgnBits[8];
#endif
  uint8_t pinSDEnable = NOT_A_PIN; //Input for manually enabling SD logging
#ifdef USE_SPI_EEPROM
  uint8_t pinSPIFlash_CS = NOT_A_PIN;
#endif
  uint8_t pinAirConComp = NOT_A_PIN;    // Air conditioning compressor output
  uint8_t pinAirConFan = NOT_A_PIN;    // Stand-alone air conditioning fan output
  uint8_t pinAirConRequest = NOT_A_PIN; // Air conditioning request input
};
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
    uint8_t index = 0;
    while (index<N && pin!=this->operator[](index))
    {
      ++index;
    }
    return index<N;
  }
};

/** @brief Injector control pin numbers */
struct injector_pins_t : pin_array_t<INJ_CHANNELS>
{
  using pin_array_t<INJ_CHANNELS>::pin_array_t; // Inherit constructors

  /**
   * @brief Limits the source array length to the number of injectors defined in
   * the tune.
   */
  void clampToNumInjectors(const config2 &page2);
};

/** @brief Coil control pin numbers */
struct coil_pins_t : pin_array_t<IGN_CHANNELS>
{
  using pin_array_t<IGN_CHANNELS>::pin_array_t; // Inherit constructors
};

struct decoder_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr decoder_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t primary = NOT_A_PIN; ///< Crank angle sensor pin
  uint8_t secondary = NOT_A_PIN; ///< Cam angle sensor pin
  uint8_t tertiary = NOT_A_PIN; ///< 2nd cam angle sensor pin (often VVT)
};

/** @brief Input sensor pins */
struct sensor_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr sensor_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t TPS = NOT_A_PIN;///< TPS input pin
  uint8_t MAP = NOT_A_PIN; ///< MAP sensor pin
  uint8_t EMAP = NOT_A_PIN; ///< EMAP sensor pin
  uint8_t IAT = NOT_A_PIN; ///< IAT sensor pin
  uint8_t CLT = NOT_A_PIN; ///< CLS sensor pin
  uint8_t O2 = NOT_A_PIN; ///< O2 Sensor pin
  uint8_t O2_2 = NOT_A_PIN; ///< Second O2 pin
  uint8_t Bat = NOT_A_PIN; ///< Battery voltage pin
  uint8_t CTPS = NOT_A_PIN; ///< Input for triggering closed throttle state
  uint8_t flex = NOT_A_PIN; ///< Pin with the flex sensor attached
  uint8_t fuelPressure = NOT_A_PIN;
  uint8_t oilPressure = NOT_A_PIN;
  uint8_t baro = NOT_A_PIN; //Pin that an external barometric pressure sensor is attached to (If used)

  bool isPinUsed(uint8_t pin) const;
};

/** @brief Idle pins */
struct idle_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr idle_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t idle1 = NOT_A_PIN; ///< Single wire idle control
  uint8_t idle2 = NOT_A_PIN; ///< 2 wire idle control
  uint8_t idleUp = NOT_A_PIN; ///< Input for triggering Idle Up
  uint8_t idleUpOutput = NOT_A_PIN; ///< Output that follows (normal or inverted) the idle up pin
  uint8_t stepperDir = NOT_A_PIN; //Direction pin for the stepper motor driver
  uint8_t stepperStep = NOT_A_PIN; //Step pin for the stepper motor driver
  uint8_t stepperEnable = NOT_A_PIN; //Turning the DRV8825 driver on/off
};

/** @brief Water Methanol Injection (WMI) pins */
struct wmi_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr wmi_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t empty = NOT_A_PIN; ///< Water tank empty sensor
  uint8_t indicator = NOT_A_PIN; ///< No water indicator bulb
  uint8_t enabled = NOT_A_PIN; ///< ON-OFF output to relay/pump/solenoid
};

/** @brief Air conditioning pins */
struct aircon_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr aircon_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t compressor = NOT_A_PIN;    ///< Air conditioning compressor output
  uint8_t fan = NOT_A_PIN;    ///< Stand-alone air conditioning fan output
  uint8_t request = NOT_A_PIN; ///< Air conditioning request input
};

/** @brief Pin configuration for an MC33810 board */
struct mc33810_pins_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr mc33810_pins_t() = default;
  // LCOV_EXCL_STOP

  uint8_t CS_1 = NOT_A_PIN; ///< Chip select pin for the first MC33810 IC (Channels 1-4)
  uint8_t CS_2 = NOT_A_PIN; ///< Chip select pin for the second MC33810 IC (Channels 5-8)
  pin_array_t<8> injBits;
  pin_array_t<8> ignBits;
};

/** @brief Store the pin assignments, as defined by the tune and board */
struct pinNumbers_t
{
  // LCOV_EXCL_START
  // Default constructor test coverage isn't registered since it's only used in constexpr context.
  constexpr pinNumbers_t() = default;
  // LCOV_EXCL_STOP

  injector_pins_t injectorPins;
  coil_pins_t coilPins;
  decoder_pins_t triggerPins;
  sensor_pins_t sensors;
  idle_pins_t idle;
  wmi_pins_t wmi;
  aircon_pins_t airCon;

  uint8_t pinTachOut = NOT_A_PIN; ///< Tacho output
  uint8_t pinFuelPump = NOT_A_PIN; ///< Fuel pump on/off
  uint8_t pinFan = NOT_A_PIN;       ///< Cooling fan output
  uint8_t pinResetControl = NOT_A_PIN; ///< Output pin used control resetting the Arduino
  uint8_t pinFuel2Input = NOT_A_PIN; ///< Input for switching to the 2nd fuel table
  uint8_t pinSpark2Input = NOT_A_PIN; ///< Input for switching to the 2nd ignition table
  uint8_t pinBoost = NOT_A_PIN; ///< Boost control output
  uint8_t pinVVT_1 = NOT_A_PIN;		///< vvt output 1
  uint8_t pinVVT_2 = NOT_A_PIN;		///< vvt output 2
  uint8_t pinLaunch = NOT_A_PIN; ///< Launch control
  uint8_t pinIgnBypass = NOT_A_PIN; ///< The pin used for an ignition bypass (Optional)
  uint8_t pinVSS = NOT_A_PIN; ///< Speed sensor
#ifdef SD_LOGGING
  uint8_t pinSDEnable = NOT_A_PIN; //Input for manually enabling SD logging
#endif
#if defined(MC33810_SUPPORT)
  mc33810_pins_t mc33810;
#endif
};

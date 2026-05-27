#pragma once

#include "pinNumbers_t.h"

/** @brief A builder for creating pinNumbers_t instances */
struct pinNumber_builder_t
{
  pinNumbers_t pins;

  constexpr pinNumber_builder_t() = default;
  explicit constexpr pinNumber_builder_t(const pinNumbers_t &initialPins)
  : pins(initialPins)
  {
  }

  template <uint8_t M>
  constexpr pinNumber_builder_t& withInjectorPins(const uint8_t (&initPins)[M])
  {
    pins.injectorPins = injector_pins_t(initPins);
    return *this;
  }
  template <uint8_t M>
  constexpr pinNumber_builder_t& withCoilPins(const uint8_t (&initPins)[M])
  {
    pins.coilPins = coil_pins_t(initPins);
    return *this;
  }

  constexpr pinNumber_builder_t& withPrimaryDecoder(uint8_t pin)
  {
    pins.triggerPins.primary = pin;
    return *this;
  }
  constexpr pinNumber_builder_t& withSecondaryDecoder(uint8_t pin)
  {
    pins.triggerPins.secondary = pin;
    return *this;
  }
  constexpr pinNumber_builder_t& withTertiaryDecoder(uint8_t pin)
  {
    pins.triggerPins.tertiary = pin;
    return *this;
  }
  constexpr pinNumber_builder_t& withDecoderPins(uint8_t primary, uint8_t secondary)
  {
    pins.triggerPins.primary = primary;
    pins.triggerPins.secondary = secondary;
    return *this;
  }
  constexpr pinNumber_builder_t& withDecoderPins(uint8_t primary, uint8_t secondary, uint8_t tertiary)
  {
    pins.triggerPins.primary = primary;
    pins.triggerPins.secondary = secondary;
    pins.triggerPins.tertiary = tertiary;
    return *this;
  }

    constexpr pinNumber_builder_t& withTPS(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::TPS);
    }
    constexpr pinNumber_builder_t& withMAP(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::MAP);
    }
    constexpr pinNumber_builder_t& withEMAP(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::EMAP);
    }
    constexpr pinNumber_builder_t& withIAT(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::IAT);
    }
    constexpr pinNumber_builder_t& withCLT(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::CLT);
    }
    constexpr pinNumber_builder_t& withO2(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::O2);
    }
    constexpr pinNumber_builder_t& withO2_2(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::O2_2);
    }
    constexpr pinNumber_builder_t& withBat(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::Bat);
    }
    constexpr pinNumber_builder_t& withCTPS(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::CTPS);
    }
    constexpr pinNumber_builder_t& withFlex(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::flex);
    }
    constexpr pinNumber_builder_t& withFuelPressure(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::fuelPressure);
    }
    constexpr pinNumber_builder_t& withOilPressure(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::oilPressure);
    }
    constexpr pinNumber_builder_t& withBaro(uint8_t pin)
    {
        return withSensorPin(pin, &sensor_pins_t::baro);
    }

    constexpr pinNumber_builder_t& withIdle1(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::idle1);
    }
    constexpr pinNumber_builder_t& withIdle2(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::idle2);
    }
    constexpr pinNumber_builder_t& withIdleUp(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::idleUp);
    }
    constexpr pinNumber_builder_t& withIdleUpOutput(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::idleUpOutput);
    }
    constexpr pinNumber_builder_t& withStepperDir(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::stepperDir);
    }
    constexpr pinNumber_builder_t& withStepperStep(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::stepperStep);
    }
    constexpr pinNumber_builder_t& withStepperEnable(uint8_t pin)
    {
        return withIdlePin(pin, &idle_pins_t::stepperEnable);
    }

    constexpr pinNumber_builder_t& withWmiEmpty(uint8_t pin)
    {
        return withWmiPin(pin, &wmi_pins_t::empty);
    }
    constexpr pinNumber_builder_t& withWmiIndicator(uint8_t pin)
    {
        return withWmiPin(pin, &wmi_pins_t::indicator);
    }
    constexpr pinNumber_builder_t& withWmiEnabled(uint8_t pin)
    {
        return withWmiPin(pin, &wmi_pins_t::enabled);
    }

    constexpr pinNumber_builder_t& withTach(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinTachOut);
    }
    constexpr pinNumber_builder_t& withFuelPump(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinFuelPump);
    }
    constexpr pinNumber_builder_t& withFan(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinFan);
    }
    constexpr pinNumber_builder_t& withResetControl(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinResetControl);
    }
    constexpr pinNumber_builder_t& withFuel2Input(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinFuel2Input);
    }
    constexpr pinNumber_builder_t& withSpark2Input(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinSpark2Input);
    }
    constexpr pinNumber_builder_t& withBoost(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinBoost);
    }
    constexpr pinNumber_builder_t& withVVT1(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinVVT_1);
    }
    constexpr pinNumber_builder_t& withVVT2(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinVVT_2);
    }
    constexpr pinNumber_builder_t& withLaunch(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinLaunch);
    }
    constexpr pinNumber_builder_t& withIgnBypass(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinIgnBypass);
    }
    constexpr pinNumber_builder_t& withVSS(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinVSS);
    }
#ifdef SD_LOGGING
    constexpr pinNumber_builder_t& withSDEnable(uint8_t pin)
    {
        return withPin(pin, &pinNumbers_t::pinSDEnable);
    }
#endif
#if defined(MC33810_SUPPORT)
    constexpr pinNumber_builder_t& withMC33810(uint8_t pinCS1, uint8_t pinCS2,
                                            const uint8_t (&injBits)[_countof(mc33810_pins_t::injBits)],
                                            const uint8_t (&ignBits)[_countof(mc33810_pins_t::ignBits)])
    {
        pins.mc33810.CS_1 = pinCS1;
        pins.mc33810.CS_2 = pinCS2;
        copy(pins.mc33810.injBits._elements, injBits);
        copy(pins.mc33810.ignBits._elements, ignBits);
        return *this;
        
    }
#endif

  constexpr pinNumbers_t build() const
  {
    return pins;
  }

  constexpr pinNumber_builder_t& withPin(uint8_t pin, uint8_t pinNumbers_t::* pMember)
  {
    pins.*pMember = pin;
    return *this;
  }
  constexpr pinNumber_builder_t& withSensorPin(uint8_t pin, uint8_t sensor_pins_t::* pMember)
  {
    pins.sensors.*pMember = pin;
    return *this;
  }
  constexpr pinNumber_builder_t& withIdlePin(uint8_t pin, uint8_t idle_pins_t::* pMember)
  {
    pins.idle.*pMember = pin;
    return *this;
  }  
  constexpr pinNumber_builder_t& withWmiPin(uint8_t pin, uint8_t wmi_pins_t::* pMember)
  {
    pins.wmi.*pMember = pin;
    return *this;
  }  
};
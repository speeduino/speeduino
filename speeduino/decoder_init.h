#pragma once

#include <stdint.h>

#define DECODER_MISSING_TOOTH     0
#define DECODER_BASIC_DISTRIBUTOR 1
#define DECODER_DUAL_WHEEL        2
#define DECODER_GM7X              3
#define DECODER_4G63              4
#define DECODER_24X               5
#define DECODER_JEEP2000          6
#define DECODER_AUDI135           7
#define DECODER_HONDA_D17         8
#define DECODER_MIATA_9905        9
#define DECODER_MAZDA_AU          10
#define DECODER_NON360            11
#define DECODER_NISSAN_360        12
#define DECODER_SUBARU_67         13
#define DECODER_DAIHATSU_PLUS1    14
#define DECODER_HARLEY            15
#define DECODER_36_2_2_2          16
#define DECODER_36_2_1            17
#define DECODER_420A              18
#define DECODER_WEBER             19
#define DECODER_ST170             20
#define DECODER_DRZ400            21
#define DECODER_NGC               22
#define DECODER_VMAX              23
#define DECODER_RENIX             24
#define DECODER_ROVERMEMS         25
#define DECODER_SUZUKI_K6A        26
#define DECODER_HONDA_J32         27
#define DECODER_FORD_TFI          28

/** @brief This constant represents no trigger edge */
static constexpr uint8_t TRIGGER_EDGE_NONE = 99;

/** @brief This structure represents a trigger interrupt */
struct interrupt_t
{
  /** @brief The callback function to be called on interrupt */
  void (*callback)(void);
  /** @brief The edge type for the interrupt. E.g. RISING, FALLING, CHANGE */
  uint8_t edge;

  /** @brief Attach the interrupt to a pin */
  void attach(uint8_t pin) const;

  /** @brief Detach the interrupt from a pin */
  void detach(uint8_t pin) const;
};

/** @brief This structure represents a decoder configuration */
struct decoder_t
{
  /** @brief The primary interrupt configuration - usually the crank trigger */
  interrupt_t primary;
  /** @brief The secondary interrupt configuration - usually the cam trigger */
  interrupt_t secondary;
  /** @brief The tertiary interrupt configuration - for decoders that use a 3rd input. E.g. VVT */
  interrupt_t tertiary;

  /** @brief The function to get the RPM */
  uint16_t (*getRPM)(void);
  /** @brief The function to get the crank angle */
  int (*getCrankAngle)(void);
  /** @brief The function to set the end teeth for ignition calculations */
  void (*setEndTeeth)(void);  
};
const decoder_t& getDecoder(void);

void initialiseDecoder(uint8_t decoderType);

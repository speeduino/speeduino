/**
 * @file 
 * @brief A builder for decoder_t structures
 */
#pragma once

#include "decoder_t.h"

/** @brief A builder for decoder_t - will make sure all required fields are set */
struct decoder_builder_t {
  decoder_t _decoder;

  decoder_builder_t(void);
  explicit decoder_builder_t(const decoder_t &decoder);

  decoder_builder_t& setPrimaryTrigger(interrupt_t trigger);
  decoder_builder_t& setPrimaryTrigger(interrupt_t::callback_t handler, uint8_t edge);

  decoder_builder_t& setSecondaryTrigger(interrupt_t trigger);
  decoder_builder_t& setSecondaryTrigger(interrupt_t::callback_t handler, uint8_t edge);

  decoder_builder_t& setTertiaryTrigger(interrupt_t trigger);
  decoder_builder_t& setTertiaryTrigger(interrupt_t::callback_t handler, uint8_t edge);

  decoder_builder_t& setGetRPM(decoder_t::getRPM_t getRPM);
  decoder_builder_t& setGetCrankAngle(decoder_t::getCrankAngle_t getCrankAngle);
  decoder_builder_t& setSetEndTeeth(decoder_t::setEndTeeth_t setEndTeeth);
  decoder_builder_t& setReset(decoder_t::reset_t reset);
  decoder_builder_t& setIsEngineRunning(decoder_t::engine_running_t isRunning);
  decoder_builder_t& setGetStatus(decoder_t::status_fun_t getStatus);
  decoder_builder_t& setGetFeatures(decoder_t::feature_fun_t getFeatures);

  decoder_t build(void) const
  {
    return _decoder;
  }
};

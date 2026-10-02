#pragma once

#include "decoder_state.h"

namespace decoders {

namespace detail {

/** Tooth timestamps copied in one critical section, so a 32-bit read cannot tear and the pair cannot split. */
struct tooth_speed_sample_t {
  uint32_t lastToothTime;
  uint32_t prevToothTime;
  uint32_t lastToothOneTime;
  uint32_t prevToothOneTime;
  uint32_t startRevolutions;
  uint16_t toothAngle;
  SyncStatus syncStatus;
};

tooth_speed_sample_t atomicToothSpeedSample(const statuses &current, const state_t &decoderState);

uint32_t publishedRevolutionTime(const statuses &current);

bool toothOneInterval(const tooth_speed_sample_t &sample, uint32_t &intervalUs);
bool lastToothInterval(const tooth_speed_sample_t &sample, uint32_t &intervalUs);

bool revolutionTimeFromLastTooth(const tooth_speed_sample_t &sample, uint32_t &revolutionTime);

// As nearly all the decoders use a common method of determining revolution time (The time the last full revolution took) A common function is simpler.
// If the revolution time cannot be calculated, the current value is returned unchanged.
uint32_t stdGetRevolutionTime(const statuses &current, const state_t &decoderState, bool isCamTeeth);

/**
This is a special case of revolution time measure that is based on the time between the last 2 teeth rather than the time of the last full revolution.
This gives much more volatile reading, but is quite useful during cranking, particularly on low resolution patterns.
It can only be used on patterns where the teeth are evenly spaced.
It takes an argument of the full (COMPLETE) number of teeth per revolution.
For a missing tooth wheel, this is the number if the tooth had NOT been missing (Eg 36-1 = 36)
If the revolution time cannot be calculated, the current value is returned unchanged.
*/
uint32_t crankingGetRevolutionTime(const statuses &current, const state_t &decoderState, const config4 &page4, byte totalTeeth, bool isCamTeeth);

}
}
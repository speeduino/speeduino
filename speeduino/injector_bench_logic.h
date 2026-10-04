#pragma once
#include <stdint.h>

namespace injector_bench {
enum class State : uint8_t { Idle, Waiting, On, Complete, Aborted };
struct Settings { uint16_t channel, onUs; uint32_t offUs; uint16_t count; };
inline bool valid(const Settings &s) {
  return s.channel <= 9 && s.onUs >= 100 && s.onUs <= 20000
      && s.offUs >= 100 && (uint32_t(s.onUs) + s.offUs) <= 500000U && s.count != 0
      && (uint64_t(s.onUs) + s.offUs) * s.count <= 120000000ULL;
}
struct Sequence {
  State state = State::Idle;
  uint16_t completed = 0;
  Settings settings = {0, 1000, 99000, 100};
  void start(const Settings &s) { settings=s; completed=0; state=State::Waiting; }
  bool running() const { return state==State::Waiting || state==State::On; }
  // Called once per hardware timer event. Return duration until the next edge.
  uint32_t edge() {
    if (state==State::Waiting) { state=State::On; return settings.onUs; }
    if (state==State::On) {
      ++completed;
      if (completed==settings.count) { state=State::Complete; return 0; }
      state=State::Waiting;
      return settings.offUs;
    }
    return 0;
  }
  void abort() { if (running()) state=State::Aborted; }
};
inline uint16_t read16(const uint8_t *p) { return uint16_t(p[0]) | (uint16_t(p[1])<<8); }
inline void write16(uint8_t *p,uint16_t v) { p[0]=uint8_t(v); p[1]=uint8_t(v>>8); }
inline bool range(uint16_t offset,uint16_t count,uint16_t size=8) { return offset<=size && count<=size-offset; }
}

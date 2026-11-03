#pragma once
#include "storage.h"
#include "sensors.h"
#include "pages.h"

/// @cond

// Exposed for unit testing
namespace storage {

namespace details {

enum class SensorCalibrationTableElement : uint8_t {
  Crc = PAGE_IDX_CALIBRATION_CRC,
  Values = PAGE_IDX_CALIBRATION_VALUES,
  Bins = PAGE_IDX_CALIBRATION_BINS,
};

constexpr uint16_t getCalibrationElementSize(SensorCalibrationTable sensor, SensorCalibrationTableElement element)
{
  if (element==SensorCalibrationTableElement::Crc) {
    return sizeof(uint32_t);
  }
  if (sensor==SensorCalibrationTable::CoolantSensor) {
    if (element==SensorCalibrationTableElement::Values) {
      return sizeof(decltype(cltCalibrationTable)::values);
    }
    if (element==SensorCalibrationTableElement::Bins) {
      return sizeof(decltype(cltCalibrationTable)::axis);
    }
  }
  if (sensor==SensorCalibrationTable::IntakeAirTempSensor) {
    if (element==SensorCalibrationTableElement::Values) {
      return sizeof(decltype(iatCalibrationTable)::values);
    }
    if (element==SensorCalibrationTableElement::Bins) {
      return sizeof(decltype(iatCalibrationTable)::axis);
    }
  }
  if (sensor==SensorCalibrationTable::O2Sensor) {
    if (element==SensorCalibrationTableElement::Values) {
      return sizeof(decltype(o2CalibrationTable)::values);
    }
    if (element==SensorCalibrationTableElement::Bins) {
      return sizeof(decltype(o2CalibrationTable)::axis);
    }
  }

  return 0; // Fail safe
}
}

}
/// @endcond

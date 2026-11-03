#include "src/pins/outputPin.h"
#include "statuses.h"

namespace ignBypassController {

namespace details {

    struct state_t
    {
        outputPin_t ignBypassPin;
        EngineRotationStatus rotationStatus = EngineRotationStatus::Stopped;
    };

} // namespace details

} // namespace ignBypassController
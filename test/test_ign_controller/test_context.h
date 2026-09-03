#pragma once
#include "scheduler_ignition_controller.h"
#include "src/pins/pinNumbers_t.h"
#include "../fake_decoder_status.h"
#include "crankMaths.h"
#include "decoder_builder.h"
#include "src/pins/pinMapping.h"

struct test_context_t
{
    config2 page2 = {};
    config4 page4 = {};
    config10 page10 = {};
    config13 page13 = {};
    statuses current = {};
    pinNumbers_t pins = {};

    test_context_t() {
        // Set basic engine config defaults
        page2.nCylinders = 4U;
        page2.strokes = FOUR_STROKE;
        page4.sparkMode = IGN_MODE_WASTED;
        fakeDecoderStatus.syncStatus = SyncStatus::Full;
        current.maxIgnOutputs = 4U;
        current.dwell = 3000U; // 3ms dwell
        current.advance = 15U;  // 15 degrees advance
        current.decoder = decoder_builder_t().setGetStatus(getFakeDecoderStatus).build();
        setAngleConverterRevolutionTime(UDIV_ROUND_CLOSEST(60UL*1000000UL, 4000, uint32_t));
    }

    test_context_t(uint8_t boardId)
    {
        page2.pinMapping = boardId;
        pins = getPinMapping(boardId);
    }

    void initialise(void)
    {
        ::initialiseIgnitionSchedules(current, page2, page4, page10, page13, pins);
    }

    void calculateIgnitionAngles(void)
    {
        ::calculateIgnitionAngles(page2, page4, page13, current);        
    }
};
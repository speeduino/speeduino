#include "decoder_builder.h"

#pragma GCC optimize ("Os")

static void nullTriggerHandler (void){return;} //initialisation function for triggerhandlers, does exactly nothing
static uint16_t nullGetRPM(void){return 0;} //initialisation function for getRpm, returns safe value of 0
static int nullGetCrankAngle(void){return 0;} //initialisation function for getCrankAngle, returns safe value of 0

decoder_builder_t::decoder_builder_t(void)
{
    (void)setPrimaryTrigger(&nullTriggerHandler, TRIGGER_EDGE_NONE);
    (void)setSecondaryTrigger(&nullTriggerHandler, TRIGGER_EDGE_NONE);
    (void)setTertiaryTrigger(&nullTriggerHandler, TRIGGER_EDGE_NONE);
    (void)setGetRPM(&nullGetRPM);
    (void)setGetCrankAngle(&nullGetCrankAngle);
    (void)setSetEndTeeth(&nullTriggerHandler);
    (void)setReset(&nullTriggerHandler);
}

decoder_builder_t& decoder_builder_t::setPrimaryTrigger(interrupt_t trigger)
{
    if (trigger.callback == nullptr || trigger.edge == TRIGGER_EDGE_NONE)
    {
        trigger = { &nullTriggerHandler, TRIGGER_EDGE_NONE };
    } 
    decoder.primary = trigger;
    return *this;
}
decoder_builder_t& decoder_builder_t::setPrimaryTrigger(interrupt_t::callback_t handler, uint8_t edge)
{
    return setPrimaryTrigger( interrupt_t{ handler, edge } );
}

decoder_builder_t& decoder_builder_t::setSecondaryTrigger(interrupt_t trigger)
{
    if (trigger.callback == nullptr || trigger.edge == TRIGGER_EDGE_NONE)
    {
        trigger = { &nullTriggerHandler, TRIGGER_EDGE_NONE };
    }
    decoder.secondary = trigger;
    return *this;
}
decoder_builder_t& decoder_builder_t::setSecondaryTrigger(interrupt_t::callback_t handler, uint8_t edge)
{
    return setSecondaryTrigger( interrupt_t{ handler, edge } );
}

decoder_builder_t& decoder_builder_t::setTertiaryTrigger(interrupt_t trigger)
{
    if (trigger.callback == nullptr || trigger.edge == TRIGGER_EDGE_NONE)
    {
        trigger = { &nullTriggerHandler, TRIGGER_EDGE_NONE };
    }
    decoder.tertiary = trigger;
    return *this;
}
decoder_builder_t& decoder_builder_t::setTertiaryTrigger(interrupt_t::callback_t handler, uint8_t edge)
{
    return setTertiaryTrigger( interrupt_t{ handler, edge } );
}

decoder_builder_t& decoder_builder_t::setGetRPM(decoder_t::getRPM_t getRPM)
{
    decoder.getRPM = getRPM==nullptr ? &nullGetRPM : getRPM;
    return *this;
}

decoder_builder_t& decoder_builder_t::setGetCrankAngle(decoder_t::getCrankAngle_t getCrankAngle)
{
    decoder.getCrankAngle = getCrankAngle==nullptr ? &nullGetCrankAngle : getCrankAngle;
    return *this;
}

decoder_builder_t& decoder_builder_t::setSetEndTeeth(decoder_t::setEndTeeth_t setEndTeeth)
{
    decoder.setEndTeeth = setEndTeeth==nullptr ? &nullTriggerHandler : setEndTeeth;
    return *this;
}

decoder_builder_t& decoder_builder_t::setReset(decoder_t::reset_t reset)
{
    decoder.reset = reset==nullptr ? &nullTriggerHandler : reset;
    return *this;
}

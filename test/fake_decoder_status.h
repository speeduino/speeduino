#pragma once

#include "decoder_t.h"

extern decoder_status_t fakeDecoderStatus;
static inline decoder_status_t getFakeDecoderStatus(void) noexcept
{
    return fakeDecoderStatus;
}
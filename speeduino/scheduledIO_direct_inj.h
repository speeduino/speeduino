#pragma once

void initInjDirectIO(const uint8_t (&pins)[INJ_CHANNELS]);

void openInjector_DIRECT(uint8_t channel);
void closeInjector_DIRECT(uint8_t channel);

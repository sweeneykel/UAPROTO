#pragma once
#include "project_types.h"

namespace rotary_encoder_control {
// Dedicated software I2C on SDA GPIO15 / SCL GPIO16. false = absent, not fatal.
bool begin();
// One event per call; rotation steps are retained until delivered.
// Positive hardware count -> Next; negative -> Previous.
// One completed click -> SamplePressed. No application action is performed here.
EncoderEvent pollEvent();
void setIndicator(IndicatorColor color); // Placeholder for a later increment.
}
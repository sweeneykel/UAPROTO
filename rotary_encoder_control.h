#pragma once
#include "project_types.h"

namespace rotary_encoder_control {
void begin();
EncoderEvent pollEvent();
void setIndicator(IndicatorColor color);
}

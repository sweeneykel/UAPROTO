#pragma once
#include "project_types.h"

namespace OLED_display {
void begin();
void showResult(uint8_t squareIndex, const SquareResult &result);
}

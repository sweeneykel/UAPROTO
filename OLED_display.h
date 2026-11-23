#pragma once
#include "project_types.h"

namespace OLED_display {
// Call after shared Wire/PMU initialization. Failure leaves the camera usable.
bool begin();
// Zero-based square index. Displays one result; no camera/network dependencies.
void showResult(uint8_t squareIndex, const SquareResult &result);
}
#include "OLED_display.h"

namespace OLED_display {
void begin()
{
    // TODO: Adapt existing U8g2 SSD1306 128x64 setup and screen orientation.
    // Shared I2C: SDA GPIO7, SCL GPIO6; initialize bus/PMU once centrally.
    // Retain existing OLED demonstration source unchanged for reference.
}
void showResult(uint8_t squareIndex, const SquareResult &result)
{
    (void)squareIndex;
    (void)result;
    // TODO: Show one selected result, identified as square 1..4.
    // Matched: lookup string. NoMatch: that square's error code (TBD).
    // TODO: Define initial NotSampled display.
}
}

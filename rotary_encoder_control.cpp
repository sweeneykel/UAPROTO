#include "rotary_encoder_control.h"

namespace rotary_encoder_control {
void begin()
{
    // TODO: Initialize SparkFun Qwiic Twist on the shared I2C bus.
}
EncoderEvent pollEvent()
{
    // TODO: Translate rotation/button input into events.
    // Coordinator owns selected index, clamping, sampling, and LED timing.
    return EncoderEvent::None;
}
void setIndicator(IndicatorColor color)
{
    (void)color;
    // TODO: Set Twist RGB LED: green ready, yellow sampling, red mismatch.
}
}

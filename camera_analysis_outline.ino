#include "camera_data_collection.h"
#include "camera_data_processing.h"
#include "OLED_display.h"
#include "rotary_encoder_control.h"

// Outline only: no hardware, network, sampling, lookup, or timing is active.
void setup()
{
    // TODO: Initialize shared Wire/I2C and PMU once.
    // TODO: Initialize camera, OLED, and Qwiic Twist modules.
    // TODO: Set indicator green when startup succeeds and ready.
}

void loop()
{
    // TODO: Poll encoder without blocking.
    // Rotation: clamp selected square to 0..3; display its held result.
    // Press: request one frame, collect all four RGB averages, process,
    //        hold results until next press, and show yellow for 10 seconds.
    // After yellow: red if any square is unmatched; otherwise green.
    // A new press starts a new sample, including when the indicator is red.
    // TODO: Keep browser preview and sampling-region overlay responsive.
}

#include "camera_data_collection.h"

namespace camera_data_collection {
void begin()
{
    // TODO: Adapt existing camera initialization; coordinate shared PMU/I2C.
}
void updatePreview()
{
    // TODO: Browser live preview, fixed 4x1 overlay, and reported RGB values.
    // Grid only; no visual color transformation.
}
bool captureSample(Sample &sample)
{
    (void)sample;
    // TODO: Define fixed region dimensions/gaps and convert frame data to RGB.
    return false; // Unimplemented.
}
}

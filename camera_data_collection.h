#pragma once
#include "project_types.h"

namespace camera_data_collection {
void begin();
void updatePreview();
// TODO: Capture one frame and average RGB in four fixed square regions.
// 4x1 along the image's longer dimension, presented left to right.
// Small gaps mimic a pH strip. Exclude gaps and overlay from averaging.
// Return false on capture failure; do not substitute a valid black sample.
bool captureSample(Sample &sample);
}

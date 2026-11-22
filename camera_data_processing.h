#pragma once
#include "project_types.h"

namespace camera_data_processing {
// Independent of camera, OLED, encoder, and hardware.
AnalysisResults processSample(const Sample &sample);
}

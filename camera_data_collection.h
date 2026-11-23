#pragma once
#include "project_types.h"

namespace camera_data_collection {
// One frame, four averages. On failure the caller's sample is unchanged.
bool captureSample(Sample &sample);
}
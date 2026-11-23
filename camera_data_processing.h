#pragma once
#include "project_types.h"

namespace camera_data_processing {
struct LookupEntry {
    uint8_t square; // 0 = all squares, otherwise 1..4 (left to right).
    RgbValue minimum; // Inclusive channel limits, 0..255.
    RgbValue maximum;
    const char *text; // Alphanumeric result, e.g. "A1".
};

// Independent of camera, display, networking, and encoder.
// First matching entry wins. Empty table => NoMatch for each square.
AnalysisResults processSample(const Sample &sample);
AnalysisResults processSample(const Sample &sample, const LookupEntry *entries, size_t count);
bool hasMismatch(const AnalysisResults &results);
}
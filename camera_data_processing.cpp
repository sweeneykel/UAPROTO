#include "camera_data_processing.h"
#include "camera_lookup_table.h"

namespace camera_data_processing {
namespace {
bool inRange(const RgbValue &rgb, const LookupEntry &entry)
{
    return rgb.r >= entry.minimum.r && rgb.r <= entry.maximum.r &&
           rgb.g >= entry.minimum.g && rgb.g <= entry.maximum.g &&
           rgb.b >= entry.minimum.b && rgb.b <= entry.maximum.b;
}
}

AnalysisResults processSample(const Sample &sample)
{
    return processSample(sample, LOOKUP_TABLE.data(), LOOKUP_TABLE.size());
}

AnalysisResults processSample(const Sample &sample, const LookupEntry *entries, size_t count)
{
    AnalysisResults results;
    for (uint8_t i = 0; i < SQUARE_COUNT; ++i) {
        auto &result = results.squares[i];
        result.status = ResultStatus::NoMatch;
        // Descriptive placeholder; the final per-square error code remains TBD.
        result.text = "NO MATCH - code TBD";
        for (size_t j = 0; entries && j < count; ++j) {
            const auto &entry = entries[j];
            if (entry.square != 0 && entry.square != i + 1) continue;
            // Missing/empty output strings are not valid lookup results.
            if (!entry.text || !entry.text[0]) continue;
            if (inRange(sample.squares[i], entry)) {
                result.status = ResultStatus::Matched;
                result.text = entry.text;
                break;
            }
        }
    }
    return results;
}

bool hasMismatch(const AnalysisResults &results)
{
    for (const auto &result : results.squares) {
        if (result.status == ResultStatus::NoMatch) return true;
    }
    return false;
}
}
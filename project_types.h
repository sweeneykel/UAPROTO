#pragma once
#include <Arduino.h>

constexpr uint8_t SQUARE_COUNT = 4;
constexpr uint32_t ANALYSIS_INDICATOR_MS = 10000;

struct RgbValue {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct Sample {
    uint16_t width = 0, height = 0;
    // Fixed left-to-right order. All four averages come from one frame.
    RgbValue squares[SQUARE_COUNT];
};

enum class ResultStatus : uint8_t { NotSampled, Matched, NoMatch };

struct SquareResult {
    ResultStatus status = ResultStatus::NotSampled;
    String text; // Alphanumeric lookup result or per-square error code (TBD).
};

struct AnalysisResults {
    SquareResult squares[SQUARE_COUNT];
};

enum class IndicatorColor : uint8_t { Green, Yellow, Red };
enum class EncoderEvent : uint8_t { None, Previous, Next, SamplePressed };

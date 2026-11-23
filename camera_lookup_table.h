#pragma once
#include <array>
#include "camera_data_processing.h"

namespace camera_data_processing {
// User-provided prototype ranges, scoped to squares 1..4 left to right.
// Format: { square (0=all), {min R,G,B}, {max R,G,B}, "result" }
// All channel limits are inclusive. First matching entry wins.
inline const std::array<LookupEntry, 4> LOOKUP_TABLE = {{
    {1, {152, 156, 154}, {172, 176, 174}, "Leukocytes - Negative"},
    {2, {165, 167, 167}, {185, 187, 187}, "pH - 7.0"},
    {3, {143, 144, 142}, {163, 164, 162}, "Protein - Negative"},
    {4, {93, 92, 89}, {113, 112, 109}, "Nitrite - Negative"},
}};
}
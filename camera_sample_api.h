#pragma once
#include <Arduino.h>
#include "esp_http_server.h"

// Browser coordinator: keeps camera collection and lookup processing independent.
namespace camera_sample_api {
esp_err_t begin(httpd_handle_t server);
// Move the OLED selection by signed steps. Clamps to squares 1..4, never wraps.
// Does not capture or modify readings. Safe alongside browser sampling.
void selectRelative(int8_t steps);
}
#include "camera_sample_api.h"
#include "camera_data_collection.h"
#include "camera_data_processing.h"
#include "OLED_display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace camera_sample_api {
namespace {
Sample lastSample;
AnalysisResults lastResults;
bool hasSample = false;
uint32_t sequence = 0;
uint8_t selectedSquare = 0;
SemaphoreHandle_t resultMutex = nullptr;

// Lookup strings may contain quotes, backslashes, or control characters.
String jsonString(const String &value)
{
    String json = "\"";
    for (size_t i = 0; i < value.length(); ++i) {
        const unsigned char c = value[i];
        if (c == '"' || c == '\\') {
            json += '\\'; json += static_cast<char>(c);
        } else if (c < 0x20) {
            char escaped[7];
            snprintf(escaped, sizeof(escaped), "\\u%04x", static_cast<unsigned int>(c));
            json += escaped;
        } else {
            json += static_cast<char>(c);
        }
    }
    return json + '"';
}

esp_err_t sendSample(httpd_req_t *request)
{
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    httpd_resp_set_type(request, "application/json");
    if (!hasSample) {
        httpd_resp_set_status(request, "404 Not Found");
        return httpd_resp_sendstr(request, R"({"error":"No sample taken"})");
    }
    String json = R"({"sequence":)" + String(sequence) +
        R"(,"width":)" + String(lastSample.width) + R"(,"height":)" + String(lastSample.height) + R"(,"rgb":[)";
    for (uint8_t i = 0; i < SQUARE_COUNT; ++i) {
        if (i) json += ',';
        const auto &rgb = lastSample.squares[i];
        json += R"({"r":)" + String(rgb.r) + R"(,"g":)" + String(rgb.g) + R"(,"b":)" + String(rgb.b) + "}";
    }
    json += R"(],"results":[)";
    for (uint8_t i = 0; i < SQUARE_COUNT; ++i) {
        if (i) json += ',';
        const auto &result = lastResults.squares[i];
        json += R"({"square":)" + String(i + 1) + R"(,"status":)";
        json += result.status == ResultStatus::Matched ? R"("matched")" : R"("no_match")";
        json += R"(,"text":)" + jsonString(result.text) + "}";
    }
    json += R"(],"mismatch":)";
    json += camera_data_processing::hasMismatch(lastResults) ? "true" : "false";
    json += '}';
    return httpd_resp_send(request, json.c_str(), json.length());
}

esp_err_t takeSample(httpd_req_t *request)
{
    Sample sample;
    if (!camera_data_collection::captureSample(sample)) {
        httpd_resp_set_status(request, "503 Service Unavailable");
        httpd_resp_set_type(request, "application/json");
        httpd_resp_set_hdr(request, "Cache-Control", "no-store");
        return httpd_resp_sendstr(request,
            R"({"error":"Capture or RGB conversion failed; previous readings retained. Try a lower resolution."})");
    }
    const AnalysisResults results = camera_data_processing::processSample(sample);
    // Encoder selection and HTTP sampling share the held results and OLED.
    xSemaphoreTake(resultMutex, portMAX_DELAY);
    lastSample = sample; lastResults = results;
    hasSample = true; ++sequence;
    OLED_display::showResult(selectedSquare, lastResults.squares[selectedSquare]);
    xSemaphoreGive(resultMutex);
    for (uint8_t i = 0; i < SQUARE_COUNT; ++i) {
        Serial.printf("Square %u result: %s\n", i + 1, results.squares[i].text.c_str());
    }
    return sendSample(request);
}
}

void selectRelative(int8_t steps)
{
    if (!resultMutex || !steps) return;
    xSemaphoreTake(resultMutex, portMAX_DELAY);
    int next = static_cast<int>(selectedSquare) + steps;
    if (next < 0) next = 0;
    if (next >= SQUARE_COUNT) next = SQUARE_COUNT - 1;
    if (next != selectedSquare) {
        selectedSquare = static_cast<uint8_t>(next);
        OLED_display::showResult(selectedSquare, lastResults.squares[selectedSquare]);
        Serial.printf("OLED selected square %u / %u\n", selectedSquare + 1, SQUARE_COUNT);
    }
    xSemaphoreGive(resultMutex);
}

esp_err_t begin(httpd_handle_t server)
{
    if (!resultMutex) resultMutex = xSemaphoreCreateMutex();
    if (!resultMutex) return ESP_ERR_NO_MEM;
    // Both routes use the existing control server task. The stream stays separate.
    httpd_uri_t get = {};
    get.uri = "/rgb-sample"; get.method = HTTP_GET; get.handler = sendSample;
    esp_err_t result = httpd_register_uri_handler(server, &get);
    if (result != ESP_OK) return result;
    httpd_uri_t post = {};
    post.uri = "/rgb-sample"; post.method = HTTP_POST; post.handler = takeSample;
    return httpd_register_uri_handler(server, &post);
}
}
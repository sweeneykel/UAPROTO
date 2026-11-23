#include "camera_data_collection.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "esp_heap_caps.h"
#include <algorithm>
namespace camera_data_collection {
bool captureSample(Sample &sample)
{
    // Release any queued frame first so a button press requests a fresh image.
    camera_fb_t *frame = esp_camera_fb_get();
    if (!frame) return false;
    esp_camera_fb_return(frame);
    frame = esp_camera_fb_get();
    if (!frame) return false;
    const int width = frame->width, height = frame->height;
    // Same integer geometry used by web/camera_overlay.html.
    const int side = std::min(width / 5, height * 4 / 5);
    const int gap = width / 40;
    const int left = (width - (SQUARE_COUNT * side + (SQUARE_COUNT - 1) * gap)) / 2;
    const int top = (height - side) / 2;
    if (side <= 0 || left < 0 || top < 0 || frame->format != PIXFORMAT_JPEG) {
        esp_camera_fb_return(frame);
        return false;
    }
    const size_t byteCount = static_cast<size_t>(width) * height * 3;
    uint8_t *pixels = static_cast<uint8_t *>(
        heap_caps_malloc(byteCount, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!pixels) pixels = static_cast<uint8_t *>(malloc(byteCount));
    if (!pixels) {
        esp_camera_fb_return(frame);
        Serial.println("RGB sample failed: insufficient memory. Lower the resolution.");
        return false;
    }
    // Espressif's JPEG-to-RGB888 converter writes B,G,R byte order.
    const bool converted = fmt2rgb888(frame->buf, frame->len, frame->format, pixels);
    esp_camera_fb_return(frame);
    if (!converted) { free(pixels); return false; }

    Sample next;
    const uint32_t count = static_cast<uint32_t>(side) * side;
    for (int i = 0; i < SQUARE_COUNT; ++i) {
        uint64_t red = 0, green = 0, blue = 0;
        const int start = left + i * (side + gap);
        for (int y = top; y < top + side; ++y) {
            for (int x = start; x < start + side; ++x) {
                const uint8_t *pixel = pixels + (static_cast<size_t>(y) * width + x) * 3;
                blue += pixel[0]; green += pixel[1]; red += pixel[2];
            }
        }
        auto &rgb = next.squares[i];
        rgb.r = (red + count / 2) / count;
        rgb.g = (green + count / 2) / count;
        rgb.b = (blue + count / 2) / count;
    }
    free(pixels);
    next.width = width; next.height = height;
    sample = next;

    Serial.printf("RGB sample (%dx%d)\n", width, height);
    for (int i = 0; i < SQUARE_COUNT; ++i) {
        const auto &rgb = next.squares[i];
        Serial.printf("Square %d: R=%u G=%u B=%u\n", i + 1, rgb.r, rgb.g, rgb.b);
    }
    return true;
}

}

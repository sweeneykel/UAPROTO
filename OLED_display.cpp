#include "OLED_display.h"
#include <Wire.h>
#include <U8g2lib.h>

namespace OLED_display {
namespace {
// Preserve the original OLED example's display, full buffer, and 180-degree rotation.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R2, U8X8_PIN_NONE);
bool ready = false;
constexpr uint8_t ADDRESS = 0x3C;
constexpr unsigned int COLUMNS = 21; // 21 * 6 pixels = 126 of 128.
constexpr unsigned int LINES = 3;

void drawText(const String &text)
{
    // Three 6x10 text lines fit below the title/status. Prefer word boundaries;
    // break long words when needed. Indicate overflow rather than hiding it.
    unsigned int offset = 0;
    for (unsigned int line = 0; line < LINES && offset < text.length(); ++line) {
        while (offset < text.length() && text[offset] == ' ') ++offset;
        if (offset >= text.length()) break;
        unsigned int end = offset + COLUMNS;
        if (end > text.length()) end = text.length();
        const int newline = text.indexOf('\n', offset);
        if (newline >= 0 && static_cast<unsigned int>(newline) < end) {
            end = newline;
        } else if (end < text.length() && text[end] != ' ') {
            const int space = text.lastIndexOf(' ', end - 1);
            if (space > static_cast<int>(offset)) end = space;
        }
        String part = text.substring(offset, end);
        if (line == LINES - 1 && end < text.length()) {
            // Last visible line; reserve space for an ellipsis.
            if (part.length() > COLUMNS - 3) part = part.substring(0, COLUMNS - 3);
            part += "...";
        }
        display.drawStr(0, 30 + line * 11, part.c_str());
        offset = end;
        if (offset < text.length() && text[offset] == '\n') ++offset;
    }
}
}

bool begin()
{
    // The sketch's PMU.begin has already started Wire on SDA7/SCL6.
    // Do not create a second PMU instance or change power rails here.
    Wire.beginTransmission(ADDRESS);
    if (Wire.endTransmission() != 0) return false;
    display.setI2CAddress(ADDRESS << 1); // U8g2 uses an 8-bit address.
    display.setBusClock(100000);
    ready = display.begin();
    if (ready) showResult(0, SquareResult{});
    return ready;
}

void showResult(uint8_t squareIndex, const SquareResult &result)
{
    if (!ready || squareIndex >= SQUARE_COUNT) return;
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.setFontRefHeightExtendedText();
    display.setFontPosTop();
    display.setFontDirection(0);
    display.setDrawColor(1);
    char title[22];
    snprintf(title, sizeof(title), "Square %u / %u", squareIndex + 1, SQUARE_COUNT);
    display.drawStr(0, 0, title);
    display.drawHLine(0, 12, 128);
    if (result.status == ResultStatus::NotSampled) {
        display.drawStr(0, 16, "Not sampled");
        drawText("Take RGB sample in the browser.");
    } else {
        display.drawStr(0, 16, result.status == ResultStatus::Matched ? "Matched" : "No match");
        drawText(result.text);
    }
    display.sendBuffer();
}
}
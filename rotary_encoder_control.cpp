#include "rotary_encoder_control.h"
#include "encoder_software_i2c.h"

namespace rotary_encoder_control {
namespace {
constexpr uint8_t STATUS_REGISTER = 0x01;
constexpr uint8_t COUNT_REGISTER = 0x05;
constexpr uint8_t CLICKED_BIT = 1 << 2;
bool ready = false;
int32_t pendingSteps = 0;
uint16_t previousCount = 0;

bool readCount(uint16_t &count)
{
    uint8_t bytes[2];
    if (!encoder_software_i2c::readRegister(COUNT_REGISTER, bytes, 2)) return false;
    // Qwiic Twist count is little endian.
    count = static_cast<uint16_t>(bytes[0]) | (static_cast<uint16_t>(bytes[1]) << 8);
    return true;
}
}

bool begin()
{
    ready = false;
    pendingSteps = 0;
    if (!encoder_software_i2c::begin() || !readCount(previousCount)) return false;
    uint8_t status;
    if (!encoder_software_i2c::readRegister(STATUS_REGISTER, &status, 1)) return false;
    if (!encoder_software_i2c::writeRegister(STATUS_REGISTER, status & ~CLICKED_BIT)) return false;
    ready = true;
    Serial.println("Encoder software I2C: SDA GPIO15, SCL GPIO16, address 0x3F");
    return true;
}

EncoderEvent pollEvent()
{
    if (!ready) return EncoderEvent::None;
    uint16_t count;
    uint8_t status;
    if (!readCount(count) ||
        !encoder_software_i2c::readRegister(STATUS_REGISTER, &status, 1)) {
        return EncoderEvent::None; // Do not fabricate events from failed reads.
    }
    const uint16_t rawDelta = count - previousCount;
    const int32_t delta = rawDelta <= 0x7FFF ? rawDelta : static_cast<int32_t>(rawDelta) - 0x10000;
    previousCount = count;
    pendingSteps += delta;
    if (status & CLICKED_BIT) {
        if (!encoder_software_i2c::writeRegister(STATUS_REGISTER, status & ~CLICKED_BIT)) {
            return EncoderEvent::None;
        }
        return EncoderEvent::SamplePressed;
    }
    if (pendingSteps > 0) { --pendingSteps; return EncoderEvent::Next; }
    if (pendingSteps < 0) { ++pendingSteps; return EncoderEvent::Previous; }
    return EncoderEvent::None;
}

void setIndicator(IndicatorColor color)
{
    (void)color;
    // LED state/timing remains deferred.
}
}
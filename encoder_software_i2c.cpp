#include "encoder_software_i2c.h"
#include "driver/gpio.h"

namespace encoder_software_i2c {
namespace {
constexpr uint32_t HALF_PERIOD_US = 5;
constexpr uint32_t CLOCK_TIMEOUT_US = 2000;
void pause() { delayMicroseconds(HALF_PERIOD_US); }

bool raiseClock()
{
    digitalWrite(SCL_PIN, HIGH); // Open drain: release the line.
    const uint32_t start = micros();
    while (digitalRead(SCL_PIN) == LOW) {
        if (micros() - start >= CLOCK_TIMEOUT_US) return false;
    }
    return true; // Honor clock stretching.
}
void lowerClock() { digitalWrite(SCL_PIN, LOW); }

bool start()
{
    digitalWrite(SDA_PIN, HIGH);
    if (!raiseClock()) return false;
    pause();
    if (digitalRead(SDA_PIN) == LOW) return false;
    digitalWrite(SDA_PIN, LOW);
    pause();
    lowerClock();
    return true;
}
bool stop()
{
    lowerClock();
    digitalWrite(SDA_PIN, LOW);
    pause();
    const bool clockOk = raiseClock();
    pause();
    digitalWrite(SDA_PIN, HIGH);
    pause();
    return clockOk && digitalRead(SDA_PIN) == HIGH;
}
bool writeBit(bool high)
{
    digitalWrite(SDA_PIN, high ? HIGH : LOW);
    pause();
    if (!raiseClock()) return false;
    pause();
    lowerClock();
    return true;
}
bool readBit(bool &high)
{
    digitalWrite(SDA_PIN, HIGH);
    pause();
    if (!raiseClock()) return false;
    high = digitalRead(SDA_PIN) == HIGH;
    pause();
    lowerClock();
    return true;
}
bool writeByte(uint8_t value)
{
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        if (!writeBit(value & mask)) return false;
    }
    bool nack;
    return readBit(nack) && !nack;
}
bool readByte(uint8_t &value, bool last)
{
    value = 0;
    for (int bit = 0; bit < 8; ++bit) {
        bool high;
        if (!readBit(high)) return false;
        value = (value << 1) | high;
    }
    // ACK all but the final byte, which gets a NACK.
    return writeBit(last);
}
}

bool begin()
{
    // Set latches directly before enabling outputs. Arduino digitalWrite only
    // accepts pins already registered as GPIO, so it cannot initialize latches.
    gpio_set_level(static_cast<gpio_num_t>(SDA_PIN), 1);
    gpio_set_level(static_cast<gpio_num_t>(SCL_PIN), 1);
    pinMode(SDA_PIN, OUTPUT_OPEN_DRAIN);
    pinMode(SCL_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(SDA_PIN, HIGH);
    digitalWrite(SCL_PIN, HIGH);
    gpio_pullup_en(static_cast<gpio_num_t>(SDA_PIN));
    gpio_pullup_en(static_cast<gpio_num_t>(SCL_PIN));
    // Qwiic board pull-ups provide normal rise times; these internal pull-ups
    // only help keep idle lines defined. Never drive either line push-pull high.
    if (!raiseClock()) return false;
    // Recover a device left partway through a byte by a previous reset.
    for (int i = 0; i < 9 && digitalRead(SDA_PIN) == LOW; ++i) {
        lowerClock(); pause();
        if (!raiseClock()) return false;
        pause();
    }
    stop();
    const bool acknowledged = start() && writeByte(ADDRESS << 1);
    const bool stopped = stop();
    return acknowledged && stopped;
}

bool readRegister(uint8_t reg, uint8_t *data, size_t length)
{
    if (!data || !length) return false;
    bool ok = start() && writeByte(ADDRESS << 1) && writeByte(reg);
    // Match the SparkFun library's STOP between register selection and read.
    const bool selected = stop();
    if (!ok || !selected) return false;
    ok = start() && writeByte((ADDRESS << 1) | 1);
    for (size_t i = 0; ok && i < length; ++i) {
        ok = readByte(data[i], i + 1 == length);
    }
    const bool stopped = stop();
    return ok && stopped;
}

bool writeRegister(uint8_t reg, uint8_t value)
{
    const bool ok = start() && writeByte(ADDRESS << 1) &&
        writeByte(reg) && writeByte(value);
    const bool stopped = stop();
    return ok && stopped;
}
}
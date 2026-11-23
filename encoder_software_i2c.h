#pragma once
#include <Arduino.h>

// Dedicated GPIO bus; never uses Wire or either hardware I2C controller.
namespace encoder_software_i2c {
constexpr uint8_t SDA_PIN = 15;
constexpr uint8_t SCL_PIN = 16;
constexpr uint8_t ADDRESS = 0x3F;
bool begin();
bool readRegister(uint8_t reg, uint8_t *data, size_t length);
bool writeRegister(uint8_t reg, uint8_t value);
}
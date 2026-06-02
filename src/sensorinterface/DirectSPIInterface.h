#pragma once

#include <SPI.h>
#include <cstdint>

class PinInterface;

namespace SlimeVR {

class DirectSPIInterface {
public:
    explicit DirectSPIInterface(SPIClass& spi = SPI, uint32_t clockFreq = 8000000)
        : m_spi(spi), m_clockFreq(clockFreq) {}

    void beginTransaction(::PinInterface* csPin);
    void endTransaction(::PinInterface* csPin);

    uint8_t readReg(uint8_t reg);
    uint16_t readReg16(uint8_t reg);
    void writeReg(uint8_t reg, uint8_t val);
    void writeReg16(uint8_t reg, uint16_t val);
    void readBytes(uint8_t reg, uint8_t count, uint8_t* dest);
    void writeBytes(uint8_t reg, uint8_t count, uint8_t* src);

private:
    SPIClass& m_spi;
    uint32_t m_clockFreq;
};

} // namespace SlimeVR

#include "DirectSPIInterface.h"
#include <Arduino.h>
#include <PinInterface.h>

namespace SlimeVR {

void DirectSPIInterface::beginTransaction(::PinInterface* csPin) {
    m_spi.beginTransaction(SPISettings(m_clockFreq, MSBFIRST, SPI_MODE0));
    if (csPin) csPin->digitalWrite(LOW);
}

void DirectSPIInterface::endTransaction(::PinInterface* csPin) {
    if (csPin) csPin->digitalWrite(HIGH);
    m_spi.endTransaction();
}

uint8_t DirectSPIInterface::readReg(uint8_t reg) {
    m_spi.transfer(reg | 0x80);
    return m_spi.transfer(0x00);
}

uint16_t DirectSPIInterface::readReg16(uint8_t reg) {
    m_spi.transfer(reg | 0x80);
    uint16_t hi = m_spi.transfer(0x00);
    uint16_t lo = m_spi.transfer(0x00);
    return (hi << 8) | lo;
}

void DirectSPIInterface::writeReg(uint8_t reg, uint8_t val) {
    m_spi.transfer(reg & 0x7F);
    m_spi.transfer(val);
}

void DirectSPIInterface::writeReg16(uint8_t reg, uint16_t val) {
    m_spi.transfer(reg & 0x7F);
    m_spi.transfer(static_cast<uint8_t>(val >> 8));
    m_spi.transfer(static_cast<uint8_t>(val & 0xFF));
}

void DirectSPIInterface::readBytes(uint8_t reg, uint8_t count, uint8_t* dest) {
    m_spi.transfer(reg | 0x80);
    for (uint8_t i = 0; i < count; i++) {
        dest[i] = m_spi.transfer(0x00);
    }
}

void DirectSPIInterface::writeBytes(uint8_t reg, uint8_t count, uint8_t* src) {
    m_spi.transfer(reg & 0x7F);
    for (uint8_t i = 0; i < count; i++) {
        m_spi.transfer(src[i]);
    }
}

} // namespace SlimeVR

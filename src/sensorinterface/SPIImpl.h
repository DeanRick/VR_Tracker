#pragma once

#include "RegisterInterface.h"
#include "DirectSPIInterface.h"
#include <PinInterface.h>
#include <cstdint>

namespace SlimeVR::Sensors {

    class SPIImpl : public RegisterInterface {
    public:
        SPIImpl(SlimeVR::DirectSPIInterface* spi, SlimeVR::PinInterface* csPin)
            : m_spi(spi), m_csPin(csPin) {}

        uint8_t readReg(uint8_t reg) const override {
            m_spi->beginTransaction(m_csPin);
            uint8_t val = m_spi->readReg(reg);
            m_spi->endTransaction(m_csPin);
            return val;
        }

        uint16_t readReg16(uint8_t reg) const override {
            m_spi->beginTransaction(m_csPin);
            uint16_t val = m_spi->readReg16(reg);
            m_spi->endTransaction(m_csPin);
            return val;
        }

        void writeReg(uint8_t reg, uint8_t val) const override {
            m_spi->beginTransaction(m_csPin);
            m_spi->writeReg(reg, val);
            m_spi->endTransaction(m_csPin);
        }

        void writeReg16(uint8_t reg, uint16_t val) const override {
            m_spi->beginTransaction(m_csPin);
            m_spi->writeReg16(reg, val);
            m_spi->endTransaction(m_csPin);
        }

        void readBytes(uint8_t reg, uint8_t count, uint8_t* dest) const override {
            m_spi->beginTransaction(m_csPin);
            m_spi->readBytes(reg, count, dest);
            m_spi->endTransaction(m_csPin);
        }

        void writeBytes(uint8_t reg, uint8_t count, uint8_t* src) const override {
            m_spi->beginTransaction(m_csPin);
            m_spi->writeBytes(reg, count, src);
            m_spi->endTransaction(m_csPin);
        }

    private:
        SlimeVR::DirectSPIInterface* m_spi;
        SlimeVR::PinInterface* m_csPin;
    };

} // namespace SlimeVR::Sensors
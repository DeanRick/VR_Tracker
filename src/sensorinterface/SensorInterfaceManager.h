#pragma once

#include <Wire.h>

// Опережающие объявления типов в корневом пространстве имен SlimeVR
namespace SlimeVR {
    class DirectPinInterface;
    class I2CWireSensorInterface;
    class I2CPCASensorInterface;
    class DirectSPIInterface;
    class PinInterface;

    class SensorInterfaceManager {
    public:
        SensorInterfaceManager();
        ~SensorInterfaceManager();

        SlimeVR::DirectPinInterface& directPinInterface() { return *m_directPin; }
        SlimeVR::I2CWireSensorInterface& i2cWireInterface() { return *m_i2cWire; }
        SlimeVR::I2CPCASensorInterface& pcaWireInterface() { return *m_pcaWire; }
        SlimeVR::DirectSPIInterface& directSPIInterface() { return *m_spi; }

        auto& i2cImpl() { return *m_i2cWire; }
        auto& spiImpl() { return *m_spi; }

    private:
        SlimeVR::DirectPinInterface* m_directPin;
        SlimeVR::I2CWireSensorInterface* m_i2cWire;
        SlimeVR::I2CPCASensorInterface* m_pcaWire;
        SlimeVR::DirectSPIInterface* m_spi;
    };
} // namespace SlimeVR

namespace SlimeVR::Sensors {
    template <typename T, typename... Args>
    class EmptySensorInterface {
    public:
        static constexpr int instance = 0;
    };
}
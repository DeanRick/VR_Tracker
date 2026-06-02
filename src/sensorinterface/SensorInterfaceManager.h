#pragma once

#include <Wire.h>

#include "I2CWireSensorInterface.h"
#include "I2CPCAInterface.h"

namespace SlimeVR {

class SensorInterfaceManager {
public:
    SensorInterfaceManager();
    ~SensorInterfaceManager();

    SlimeVR::I2CWireSensorInterface& i2cWireInterface() { return *m_i2cWire; }
    SlimeVR::I2CPCASensorInterface& pcaWireInterface() { return *m_pcaWire; }
    SlimeVR::I2CWireSensorInterface& i2cImpl() { return *m_i2cWire; }

private:
    SlimeVR::I2CWireSensorInterface* m_i2cWire;
    SlimeVR::I2CPCASensorInterface* m_pcaWire;
};

} // namespace SlimeVR

namespace SlimeVR::Sensors {
    template <typename T, typename... Args>
    class EmptySensorInterface {
    public:
        static constexpr int instance = 0;
    };
}
#pragma once

#include <Wire.h>

#include "I2CWireSensorInterface.h"
#include "I2CPCAInterface.h"
#include "i2cimpl.h"
#include "SPIImpl.h"

namespace SlimeVR {

struct I2CImplFactory {
    SlimeVR::Sensors::RegisterInterface* get(uint8_t addr) {
        return new SlimeVR::Sensors::I2CImpl(addr);
    }
};

struct SPIImplFactory {
    SlimeVR::Sensors::RegisterInterface* get(SlimeVR::DirectSPIInterface* spi, PinInterface* cs) {
        return new SlimeVR::Sensors::SPIImpl(spi, cs);
    }
};

class SensorInterfaceManager {
public:
    SensorInterfaceManager();
    ~SensorInterfaceManager();

    SlimeVR::I2CWireSensorInterface& i2cWireInterface() { return *m_i2cWire; }
    SlimeVR::I2CPCASensorInterface& pcaWireInterface()  { return *m_pcaWire; }

    I2CImplFactory& i2cImpl() { return m_i2cFactory; }
    SPIImplFactory& spiImpl() { return m_spiFactory; }

private:
    SlimeVR::I2CWireSensorInterface* m_i2cWire;
    SlimeVR::I2CPCASensorInterface*  m_pcaWire;
    I2CImplFactory  m_i2cFactory;
    SPIImplFactory  m_spiFactory;
};

} // namespace SlimeVR

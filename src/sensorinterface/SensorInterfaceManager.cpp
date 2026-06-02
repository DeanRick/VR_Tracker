#include "SensorInterfaceManager.h"

#include "DirectPinInterface.h"
#include "I2CWireSensorInterface.h"
#include "I2CPCAInterface.h"
#include "DirectSPIInterface.h"

namespace SlimeVR {

SensorInterfaceManager::SensorInterfaceManager() {
    m_i2cWire = new SlimeVR::I2CWireSensorInterface(Wire);
    m_pcaWire = new SlimeVR::I2CPCASensorInterface(Wire, 0x70, 0);
    m_directPin = new SlimeVR::DirectPinInterface();
    m_spi = new SlimeVR::DirectSPIInterface();
}

SensorInterfaceManager::~SensorInterfaceManager() {
    delete m_i2cWire;
    delete m_pcaWire;
    delete m_directPin;
    delete m_spi;
}

} // namespace SlimeVR
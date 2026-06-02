#include "SensorInterfaceManager.h"

namespace SlimeVR {

SensorInterfaceManager::SensorInterfaceManager() {
    m_i2cWire = new SlimeVR::I2CWireSensorInterface(Wire);
    m_pcaWire = new SlimeVR::I2CPCASensorInterface(Wire, 0x70, 0);
}

SensorInterfaceManager::~SensorInterfaceManager() {
    delete m_i2cWire;
    delete m_pcaWire;
}

} // namespace SlimeVR
#include "I2CPCAInterface.h"

namespace SlimeVR {
    bool I2CPCASensorInterface::init() {
        m_Wire.beginTransmission(m_Address);
        m_Wire.write(1 << m_Channel);
        return m_Wire.endTransmission() == 0;
    }

    void I2CPCASensorInterface::swapIn() {
        m_Wire.beginTransmission(m_Address);
        m_Wire.write(1 << m_Channel);
        m_Wire.endTransmission();
    }
} // namespace SlimeVR

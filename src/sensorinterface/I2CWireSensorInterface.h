#ifndef SLIMEVR_I2CWIRESENSORINTERFACE_H_
#define SLIMEVR_I2CWIRESENSORINTERFACE_H_

#include "SensorInterface.h"
#include <Wire.h>
#include <string>

namespace SlimeVR {
    class I2CWireSensorInterface : public SensorInterface {
    public:
        I2CWireSensorInterface(TwoWire& wire) : m_Wire(wire) {}
        bool init() override { return true; }
        void swapIn() override {}
        std::string toString() const override { return "I2CWireSensorInterface"; }
    private:
        TwoWire& m_Wire;
    };
} // namespace SlimeVR

#endif // SLIMEVR_I2CWIRESENSORINTERFACE_H_
#ifndef SLIMEVR_I2CPCAINTERFACE_H_
#define SLIMEVR_I2CPCAINTERFACE_H_

#include "SensorInterface.h"
#include <Wire.h>
#include <string>

namespace SlimeVR {
    class I2CPCASensorInterface : public SensorInterface {
    public:
        I2CPCASensorInterface(TwoWire& wire, uint8_t address, uint8_t channel)
            : m_Wire(wire), m_Address(address), m_Channel(channel) {}
        bool init() override;
        void swapIn() override;
        std::string toString() const override { return "I2CPCASensorInterface"; }
    private:
        TwoWire& m_Wire;
        uint8_t m_Address;
        uint8_t m_Channel;
    };
} // namespace SlimeVR

#endif // SLIMEVR_I2CPCAINTERFACE_H_

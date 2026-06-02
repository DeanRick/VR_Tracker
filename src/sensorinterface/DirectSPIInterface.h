#pragma once

#include <string>

namespace SlimeVR::Sensors {

class PinInterface;
class SensorInterface {
public:
    virtual ~SensorInterface() = default;
    virtual bool init() = 0;
    virtual void swapIn() = 0;
    virtual std::string toString() const = 0;
};

} // namespace SlimeVR::Sensors
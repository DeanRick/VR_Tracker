#ifndef SLIMEVR_SENSORINTERFACE_H_
#define SLIMEVR_SENSORINTERFACE_H_

#include <string>

namespace SlimeVR {
    class SensorInterface {
    public:
        virtual ~SensorInterface() = default;
        virtual bool init() = 0;
        virtual void swapIn() = 0;
        virtual std::string toString() const = 0;
    };

    class EmptySensorInterface final : public SensorInterface {
    public:
        bool init() override { return true; }
        void swapIn() override {}
        std::string toString() const override { return "EmptyInterface"; }
    };
} // namespace SlimeVR

#endif // SLIMEVR_SENSORINTERFACE_H_
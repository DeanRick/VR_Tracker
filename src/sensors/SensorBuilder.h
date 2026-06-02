/*
    SlimeVR Code is placed under the MIT license
    Copyright (c) 2025 Eiren Rain & SlimeVR Contributors
*/
#pragma once

#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include "SensorManager.h"
#include "sensor.h"
#include "consts.h"

// Опережающие объявления — компилятору не нужно знать их размер здесь
namespace SlimeVR {
    class SensorInterfaceManager;
    class SensorInterface;
    class DirectSPIInterface;
    class PinInterface;
}

namespace SlimeVR::Sensors {
    class RegisterInterface;
}

#ifndef PRIMARY_IMU_ADDRESS_ONE
#define PRIMARY_IMU_ADDRESS_ONE false
#endif

#ifndef SECONDARY_IMU_ADDRESS_TWO
#define SECONDARY_IMU_ADDRESS_TWO true
#endif

namespace SlimeVR::Sensors {

class SensorAuto {};

struct SensorBuilder {
private:
    struct SensorDefinition {
        uint8_t sensorID;
        RegisterInterface& imuInterface;
        float rotation;
        SensorInterface* sensorInterface;
        bool optional;
        PinInterface* intPin;
        int extraParam;
    };

public:
    SensorManager* m_Manager;
    
    explicit SensorBuilder(SensorManager* sensorManager);
    ~SensorBuilder(); // Деструктор для корректной очистки, если менеджер создается внутри

    uint8_t buildAllSensors();
    std::unique_ptr<::Sensor> buildSensorDynamically(SensorTypeID type, SensorDefinition sensorDef);

    // Объявляем шаблоны функций, но их реализацию уносим в .cpp
    template <typename Sensor, typename AccessInterface>
    std::optional<std::pair<SensorTypeID, RegisterInterface*>> checkSensorPresent(
        uint8_t sensorId,
        SensorInterface* sensorInterface,
        AccessInterface accessInterface
    );

    template <typename AccessInterface>
    inline std::optional<std::pair<SensorTypeID, RegisterInterface*>> checkSensorsPresent(uint8_t, SensorInterface*, AccessInterface) {
        return std::nullopt;
    }

    template <typename AccessInterface, typename Sensor, typename... Rest>
    std::optional<std::pair<SensorTypeID, RegisterInterface*>> checkSensorsPresent(
        uint8_t sensorId,
        SensorInterface* sensorInterface,
        AccessInterface accessInterface
    );

    template <typename Sensor, typename AccessInterface>
    RegisterInterface* getRegisterInterface(
        uint8_t sensorId,
        SensorInterface* interface,
        AccessInterface access
    );

    template <typename AccessInterface>
    std::optional<std::pair<SensorTypeID, RegisterInterface*>> findSensorType(
        uint8_t sensorID,
        SensorInterface* sensorInterface,
        AccessInterface accessInterface
    );

    template <typename SensorType, typename AccessInterface>
    bool sensorDescEntry(
        uint8_t sensorID,
        AccessInterface accessInterface,
        float rotation,
        SensorInterface* sensorInterface,
        bool optional = false,
        PinInterface* intPin = nullptr,
        int extraParam = 0
    );

    template <typename ImuType>
    std::unique_ptr<::Sensor> buildSensor(SensorDefinition sensorDef);

private:
    SlimeVR::SensorInterfaceManager* interfaceManager;
};

}  // namespace SlimeVR::Sensors
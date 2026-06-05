#include "SensorBuilder.h"

#include "sensorinterface/SensorInterfaceManager.h"
#include <PinInterface.h>
#include "sensorinterface/DirectPinInterface.h"
#include "sensorinterface/DirectSPIInterface.h"
#include "sensorinterface/RegisterInterface.h"
#include "sensorinterface/SPIImpl.h"
#include "sensorinterface/SensorInterface.h"
#include "sensorinterface/i2cimpl.h"
#include "sensorinterface/I2CPCAInterface.h"
#include "sensorinterface/I2CWireSensorInterface.h"

#include "EmptySensor.h"
#include "ErroneousSensor.h"
#include "bno055sensor.h"
#include "bno080sensor.h"
#include "globals.h"
#include "icm20948sensor.h"
#include "logging/Logger.h"
#include "mpu6050sensor.h"
#include "mpu9250sensor.h"

#include "softfusion/drivers/bmi160.h"
#include "softfusion/drivers/bmi270.h"
#include "softfusion/drivers/icm42688.h"
#include "softfusion/drivers/icm45605.h"
#include "softfusion/drivers/icm45686.h"
#include "softfusion/drivers/lsm6ds3trc.h"
#include "softfusion/drivers/lsm6dso.h"
#include "softfusion/drivers/lsm6dsr.h"
#include "softfusion/drivers/lsm6dsv.h"
#include "softfusion/drivers/mpu6050.h"
#include "softfusion/softfusionsensor.h"
#include "softfusion/SoftfusionCalibration.h"

namespace SlimeVR::Sensors {

// SoftFusion sensor type aliases — referenced by IMU_xxx macros in consts.h
using SoftFusionBMI160    = SoftFusionSensor<SoftFusion::Drivers::BMI160,    SoftfusionCalibrator>;
using SoftFusionBMI270    = SoftFusionSensor<SoftFusion::Drivers::BMI270,    SoftfusionCalibrator>;
using SoftFusionICM42688  = SoftFusionSensor<SoftFusion::Drivers::ICM42688,  SoftfusionCalibrator>;
using SoftFusionICM45686  = SoftFusionSensor<SoftFusion::Drivers::ICM45686,  SoftfusionCalibrator>;
using SoftFusionICM45605  = SoftFusionSensor<SoftFusion::Drivers::ICM45605,  SoftfusionCalibrator>;
using SoftFusionLSM6DS3TRC = SoftFusionSensor<SoftFusion::Drivers::LSM6DS3TRC, SoftfusionCalibrator>;
using SoftFusionLSM6DSO   = SoftFusionSensor<SoftFusion::Drivers::LSM6DSO,   SoftfusionCalibrator>;
using SoftFusionLSM6DSR   = SoftFusionSensor<SoftFusion::Drivers::LSM6DSR,   SoftfusionCalibrator>;
using SoftFusionLSM6DSV   = SoftFusionSensor<SoftFusion::Drivers::LSM6DSV,   SoftfusionCalibrator>;
using SoftFusionMPU6050   = SoftFusionSensor<SoftFusion::Drivers::MPU6050,   SoftfusionCalibrator>;

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

SensorBuilder::SensorBuilder(SensorManager* sensorManager)
    : m_Manager(sensorManager) {
    interfaceManager = new SlimeVR::SensorInterfaceManager();
}

SensorBuilder::~SensorBuilder() {
    delete interfaceManager;
}

// ---------------------------------------------------------------------------
// getRegisterInterface — maps AccessInterface to a concrete RegisterInterface
// ---------------------------------------------------------------------------

template <typename Sensor, typename AccessInterface>
RegisterInterface* SensorBuilder::getRegisterInterface(
    uint8_t /*sensorId*/,
    SensorInterface* interface,
    AccessInterface access
) {
    if constexpr (std::is_base_of_v<::PinInterface, std::remove_pointer_t<AccessInterface>>) {
        // SPI path — CS pin passed as AccessInterface
        return interfaceManager->spiImpl().get(
            reinterpret_cast<SlimeVR::DirectSPIInterface*>(interface),
            access
        );
    } else if constexpr (std::is_same_v<AccessInterface, bool>) {
        // I2C path — bool selects address offset (false=primary, true=secondary)
        const uint8_t addr = Sensor::Address + (access ? 1 : 0);
        return interfaceManager->i2cImpl().get(addr);
    } else if constexpr (std::is_integral_v<AccessInterface>) {
        // I2C path — explicit address supplied
        return interfaceManager->i2cImpl().get(static_cast<uint8_t>(access));
    }
    return &EmptyRegisterInterface::instance;
}

// ---------------------------------------------------------------------------
// checkSensorPresent — probes I2C bus for a specific sensor type
// ---------------------------------------------------------------------------

template <typename Sensor, typename AccessInterface>
std::optional<std::pair<SensorTypeID, RegisterInterface*>>
SensorBuilder::checkSensorPresent(
    uint8_t sensorId,
    SensorInterface* sensorInterface,
    AccessInterface accessInterface
) {
    RegisterInterface* reg =
        getRegisterInterface<Sensor>(sensorId, sensorInterface, accessInterface);

    if (reg == &EmptyRegisterInterface::instance) {
        return std::nullopt;
    }
    if (!reg->hasSensorOnBus()) {
        delete reg;
        return std::nullopt;
    }
    return std::make_pair(Sensor::TypeID, reg);
}

// ---------------------------------------------------------------------------
// findSensorType — auto-detection stub (not used with explicit sensor types)
// ---------------------------------------------------------------------------

template <typename AccessInterface>
std::optional<std::pair<SensorTypeID, RegisterInterface*>>
SensorBuilder::findSensorType(
    uint8_t /*sensorID*/,
    SensorInterface* /*sensorInterface*/,
    AccessInterface /*accessInterface*/
) {
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// buildSensor — constructs a concrete sensor object
// ---------------------------------------------------------------------------

template <typename ImuType>
std::unique_ptr<::Sensor> SensorBuilder::buildSensor(SensorDefinition sensorDef) {
    return std::make_unique<ImuType>(
        sensorDef.sensorID,
        sensorDef.imuInterface,
        sensorDef.rotation,
        sensorDef.sensorInterface,
        sensorDef.intPin,
        sensorDef.extraParam
    );
}

// ---------------------------------------------------------------------------
// sensorDescEntry — detect + build one sensor, add to SensorManager
// ---------------------------------------------------------------------------

template <typename SensorType, typename AccessInterface>
bool SensorBuilder::sensorDescEntry(
    uint8_t sensorID,
    AccessInterface accessInterface,
    float rotation,
    SensorInterface* sensorInterface,
    bool optional,
    PinInterface* intPin,
    int extraParam
) {
    auto found =
        checkSensorPresent<SensorType>(sensorID, sensorInterface, accessInterface);

    if (!found) {
        // Insert placeholder so SensorManager sensorId indices stay contiguous
        m_Manager->m_Sensors.push_back(
            std::make_unique<ErroneousSensor>(sensorID, SensorType::TypeID)
        );
        return false;
    }

    auto [typeID, reg] = *found;
    SensorDefinition def{sensorID, *reg, rotation, sensorInterface, optional, intPin, extraParam};
    m_Manager->m_Sensors.push_back(buildSensor<SensorType>(def));
    return true;
}

// ---------------------------------------------------------------------------
// buildAllSensors — expand SENSOR_DESC_LIST and return active sensor count
// ---------------------------------------------------------------------------

uint8_t SensorBuilder::buildAllSensors() {
    uint8_t sensorId = 0;
    uint8_t activeSensorCount = 0;

    // Make pointer look like a reference so board macros can use dot notation
    auto& interfaceManager = *this->interfaceManager;

#define SENSOR_DESC_ENTRY(type, access, rotation, iface, optional, intPin, extra) \
    if (sensorDescEntry<type>(sensorId, access, rotation, iface, optional, intPin, extra)) \
        ++activeSensorCount;                                                                \
    ++sensorId;

    SENSOR_DESC_LIST

#undef SENSOR_DESC_ENTRY

    return activeSensorCount;
}

} // namespace SlimeVR::Sensors

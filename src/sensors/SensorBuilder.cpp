#include "SensorBuilder.h"

// Подключаем всё тяжелое барахло здесь — в .cpp файле это абсолютно безопасно!
#include "sensorinterface/SensorInterfaceManager.h"
#include "sensorinterface/PinInterface.h"
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

namespace SlimeVR::Sensors {

// Конструктор: создаем менеджер интерфейсов локально, если его нет в SensorManager,
// либо привязываем к существующему. Самый безопасный вариант для кастомных сборок:
SensorBuilder::SensorBuilder(SensorManager* sensorManager) 
    : m_Manager(sensorManager) {
    interfaceManager = new SlimeVR::SensorInterfaceManager();
}

SensorBuilder::~SensorBuilder() {
    delete interfaceManager;
}

// Переносим реализацию шаблона getRegisterInterface в явную специализацию/код .cpp
template <typename Sensor, typename AccessInterface>
RegisterInterface* SensorBuilder::getRegisterInterface(
    uint8_t sensorId,
    SensorInterface* interface,
    AccessInterface access
) {
    if constexpr (std::is_base_of_v<PinInterface, std::remove_pointer_t<AccessInterface>>) {
        return interfaceManager->spiImpl().get(
            static_cast<SlimeVR::DirectSPIInterface*>(interface),
            access
        );
    } else if constexpr (std::is_same_v<AccessInterface, bool>) {
        uint8_t addressIncrement = access ? 1 : 0;
        return interfaceManager->i2cImpl().get(Sensor::Address + addressIncrement);
    } else if constexpr (std::is_integral_v<AccessInterface>) {
        return interfaceManager->i2cImpl().get(access);
    }
    return &EmptyRegisterInterface::instance;
}

// ... Сюда же перенеси тела остальных функций: checkSensorPresent, findSensorType, sensorDescEntry и buildSensor ...
// (Они будут использовать синтаксис SensorBuilder::имя_метода)

} // namespace SlimeVR::Sensors
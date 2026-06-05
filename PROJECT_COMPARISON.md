# Сравнение VR_Tracker vs SlimeVR-Tracker-ESP (upstream)

> Дата: 2026-06-05  
> VR_Tracker: `Z:\Dev_for_fun\Andryshka\VR_Tracker\`  
> Upstream: `Z:\Dev_for_fun\Andryshka\SlimeVR-Tracker-ESP\`

---

## 1. Файлы в upstream, отсутствующие в VR_Tracker

| Файл | Важность | Комментарий |
|------|----------|-------------|
| `src/sensorinterface/MCP23X17PinInterface.cpp/.h` | ⚠️ Умеренная | GPIO-расширитель MCP23x17 — не нужен для BMI160, но если где-то есть ссылка — ошибка |
| `.clang-format`, `.clang-tidy`, `.editorconfig` | Низкая | Инструменты форматирования кода |
| `.github/workflows/actions.yml` | Низкая | CI/CD pipeline |
| `.github/CODEOWNERS`, `FUNDING.yml`, `dependabot.yml` | Низкая | GitHub-метаданные |
| `.gitignore` | Низкая | Не влияет на сборку |

**Итог:** Из критичного — только `MCP23X17PinInterface`. Если в коде на него нет ссылок (у нас убраны) — не проблема.

---

## 2. Файлы только в VR_Tracker (нет в upstream)

Таких файлов нет — все файлы VR_Tracker присутствуют в upstream.

---

## 3. Изменённые файлы (основные, с реальными расхождениями)

### `src/sensors/SensorBuilder.h` — **439 строк различий** 🔴 Критично

| Аспект | VR_Tracker (наш) | Upstream |
|--------|-----------------|----------|
| `using SoftFusion*` алиасы | Перенесены в `.cpp` | **В `.h`** — архитектурно правильно для шаблонов |
| `#include` зависимости | Минимальные + forward declarations | Все include прямо в `.h` (включая `MCP23X17PinInterface.h`) |
| `interfaceManager` поле | `SlimeVR::SensorInterfaceManager*` (указатель) | `SensorInterfaceManager` (значение, без namespace prefix) |
| Методы `buildSensor`, `sensorDescEntry`, `getRegisterInterface`, `findSensorType` | Реализованы в `.cpp` | **Прямо в `.h`** как template inline |
| `SFCALIBRATOR` макрос | Жёстко `SoftfusionCalibrator` | `#define SFCALIBRATOR` — выбор между `RuntimeCalibrator` и `SoftfusionCalibrator` через `USE_RUNTIME_CALIBRATION` |
| `SoftFusionBMI160` алиас | Добавлен нами в `.cpp` | **Отсутствует в upstream** (BMI160 не в основном списке) |

> ⚠️ **Template-функции обязаны быть в `.h`**. Наш подход с `.cpp` работает только если все инстанциации происходят в одном TU.

---

### `src/sensorinterface/SensorInterfaceManager.h/.cpp` — 147/57 строк различий

| Аспект | VR_Tracker | Upstream |
|--------|-----------|----------|
| `I2CImplFactory`, `SPIImplFactory` | Добавлены нами | Присутствуют в upstream |
| `i2cImpl()`, `spiImpl()` методы | Добавлены нами | Присутствуют в upstream |
| `DirectPinInterface`, `DirectSPIInterface` в конструкторе | Убраны | Создаются |
| `MCP23X17PinInterface` | Убран | Создаётся |

---

### `src/sensorinterface/DirectSPIInterface.h/.cpp` — 75/94 строк различий

Полностью переписан нами — исходник содержал класс `SensorInterface` вместо `DirectSPIInterface`.  
Upstream содержит корректную реализацию `DirectSPIInterface`.

---

### `src/sensorinterface/SPIImpl.h` — 182 строки различий

Переписан нами — добавлены реализации `getAddress()`, `hasSensorOnBus()`, `toString()` (были абстрактными).

---

### `src/boards/boards_default.h` — 113 строк различий

| Аспект | VR_Tracker | Upstream |
|--------|-----------|----------|
| `SENSOR_DESC_LIST` | `SENSOR_DESC_ENTRY(IMU_BMI160, PRIMARY_IMU_ADDRESS_ONE, ..., nullptr, 0)` | Нет BMI160, используются другие сенсоры |
| `intPin` параметр | `nullptr` (убран `pinManager.emptyImpl()`) | Используются реальные pin объекты |

---

### `src/sensors/softfusion/drivers/bmi160.h` — 6 строк различий

Добавлены константы `TemperatureSensitivity = 512.0f` и `TemperatureBias = 23.0f` (отсутствовали в upstream).

---

### `platformio.ini` — 262 строки различий

Добавлены:
- `board_build.mcu = esp32c3`
- `board_build.f_cpu = 160000000L`
- ESP32-C3 SuperMini как целевая платформа

---

### `src/defines.h` — 115 строк различий

Добавлены (наши правки):
```cpp
#define WIFI_CREDS_SSID "YOUR_SSID"      // ← заменить реальным!
#define WIFI_CREDS_PASSWD "YOUR_PASSWORD" // ← заменить реальным!
#define IMU IMU_BMI160
#define PRIMARY_IMU_ADDRESS_ONE 0x68
#define PIN_IMU_SDA 8
#define PIN_IMU_SCL 9
#define PIN_IMU_INT 255
#define PIN_BATTERY_LEVEL 255
#define PIN_LED 255
```

---

### `src/network/wifihandler.cpp` — 83 строки различий

Добавлены диагностические логи по всему WiFi state machine.  
**Известная проблема:** retry-логика с `phyModeG` на ESP32 возвращает `false` немедленно → только 1 реальная попытка → сразу SmartConfig. Исправлено отдельным патчем.

---

### `src/consts.h` — 3 строки различий

Добавлено определение `MCU_ESP32_C3` для ESP32-C3.

---

### `src/sensorinterface/RegisterInterface.h` — 3 строки различий

Добавлен виртуальный деструктор `virtual ~RegisterInterface() = default;`

---

### `src/sensors/softfusion/SoftfusionCalibration.h` — 2 строки различий

Добавлен `#include "magneto1.4.h"` для `MagnetoCalibration`.

---

## 4. Итоговая оценка

### Правки, которые корректны ✅
- `defines.h` — WiFi credentials, BMI160 пины
- `platformio.ini` — ESP32-C3 настройки
- `consts.h` — MCU_ESP32_C3
- `bmi160.h` — температурные константы
- `RegisterInterface.h` — виртуальный деструктор
- `SoftfusionCalibration.h` — magneto include
- `wifihandler.cpp` — диагностические логи

### Правки с архитектурными проблемами ⚠️
- `SensorBuilder.h/.cpp` — template-функции в `.cpp` вместо `.h`; алиасы в `.cpp` вместо `.h`
- `SensorInterfaceManager` — `interfaceManager` как указатель вместо значения

### Что рекомендуется сделать дальше
1. Взять `SensorBuilder.h` из upstream, адаптировать (убрать `MCP23X17PinInterface`, добавить `SoftFusionBMI160`)
2. Перенести `interfaceManager` из указателя в значение (как в upstream)
3. Исправить WiFi retry-логику для ESP32 (см. отдельный патч)

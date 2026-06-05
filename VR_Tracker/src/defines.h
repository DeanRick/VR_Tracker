#ifndef SLIMEVR_DEFINES_H_
#define SLIMEVR_DEFINES_H_

#ifndef BOARD
  #define BOARD BOARD_CUSTOM
#endif

// Конфигурация основного IMU
#define IMU IMU_BMI160
#define SECOND_IMU IMU_UNKNOWN
#define PRIMARY_IMU_OPTIONAL false
#define SECONDARY_IMU_OPTIONAL true
#define IMU_ROTATION DEG_0

// Аппаратные пины I2C для твоей платы ESP32-C3 SuperMini
#define PRIMARY_IMU_ADDRESS_ONE 0x68
#define PIN_IMU_SDA 8
#define PIN_IMU_SCL 9
#define I2C_SPEED 400000 // Напрямую решает ошибку в I2CWireSensorInterface.cpp

// Отключаем использование прерываний датчика (опрос пойдет по таймеру)
#define PIN_IMU_INT 255
#define PIN_IMU_INT_2 255

// Полностью глушим мониторинг батареи и светодиод
#define PIN_BATTERY_LEVEL 255
#define PIN_LED 255 
#define LED_INV false

// WiFi credentials — заменить на свои
#define WIFI_CREDS_SSID "YOUR_SSID"
#define WIFI_CREDS_PASSWD "YOUR_PASSWORD"

#define SERIAL_BAUD 115200
#define TRACKER_NAME "SlimeVR C3 BMI160"

#endif // SLIMEVR_DEFINES_H_
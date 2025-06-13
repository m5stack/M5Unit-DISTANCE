/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitUltraSonic I2C/IO
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedDISTANCE.h>
#include <M5Utility.h>

// *********************************************************************
// Choose connection
// *********************************************************************
#if !defined(CONNECT_VIA_I2C) && !defined(CONNECT_VIA_GPIO)
// #define CONNECT_VIA_I2C
// #define CONNECT_VIA_GPIO
#endif

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
#if defined(CONNECT_VIA_I2C)
m5::unit::UnitUltraSonicI2C unit;
constexpr uint32_t interval{150};  // For I2C, the measurement interval is 150 ms or more
#elif defined(CONNECT_VIA_GPIO)
m5::unit::UnitUltraSonicIO unit;
constexpr uint32_t interval{50};  // For GPIO, the measurement interval is 50 ms or more
#else
#error "Choose connection"
#endif

}  // namespace

using namespace m5::unit::rcwl9620;

void setup()
{
    M5.begin();
    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    auto cfg        = unit.config();
    cfg.interval_ms = interval;
    unit.config(cfg);

#if defined(CONNECT_VIA_I2C)
    // PortA as I2C
    M5.Log.printf("Using I2C\n");
    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 400 * 1000U);

    if (!Units.add(unit, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#elif defined(CONNECT_VIA_GPIO)
    // PortB as GPIO if available, PortA if not
    M5.Log.printf("Using GPIO\n");
    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGW("PortB is not available");
        Wire.end();
        pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_a_pin1);
        pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_a_pin2);
    }
    M5_LOGI("getPin :%d,%d", pin_num_gpio_in, pin_num_gpio_out);

    if (!Units.add(unit, pin_num_gpio_in, pin_num_gpio_out) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.setFont(&fonts::AsciiFont8x16);
    lcd.clear(TFT_DARKGREEN);
    lcd.fillRect(8, 8, 8 * 24, 16 * 1, TFT_BLACK);
}

void loop()
{
    M5.update();
    auto touch = M5.Touch.getDetail();

    // Periodic
    Units.update();
    if (unit.updated()) {
        M5.Log.printf("Distance:%f Raw:%x\n", unit.distance(), unit.oldest().raw_distance());

        lcd.fillRect(8, 8, 8 * 24, 16 * 1, TFT_BLACK);
        lcd.setCursor(8, 8 + 16 * 0);
        lcd.printf("Distance:%.2f mm", unit.distance());
    }

    if (M5.BtnA.wasClicked() || touch.wasClicked()) {
        unit.stopPeriodicMeasurement();
        Data d{};
        if (unit.measureSingleshot(d)) {
            M5.Log.printf("Single: %.2f mm\n", d.distance());
        }
        unit.startPeriodicMeasurement(interval);
    }
}

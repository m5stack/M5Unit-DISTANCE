/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitUltraSonicI2C
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedDISTANCE.h>
#include <M5Utility.h>

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitUltraSonicI2C unit;
constexpr uint32_t interval{150};

}  // namespace

using namespace m5::unit::rcwl9620;

void setup()
{
    M5.begin();
    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    auto pin_num_sda = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl = M5.getPin(m5::pin_name_t::port_a_scl);
    M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 100 * 1000U);

    auto cfg        = unit.config();
    cfg.interval_ms = interval;
    unit.config(cfg);

    if (!Units.add(unit, Wire) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
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
        M5_LOGI("\n>Distance:%f Raw:%x", unit.distance(), unit.oldest().raw_distance());

        lcd.fillRect(8, 8, 8 * 24, 16 * 1, TFT_BLACK);
        lcd.setCursor(8, 8 + 16 * 0);
        lcd.printf("Distance:%.2f mm", unit.distance());
    }

    if (M5.BtnA.wasClicked() || touch.wasClicked()) {
        unit.stopPeriodicMeasurement();
        Data d{};
        if (unit.measureSingleshot(d)) {
            M5_LOGI("Single: %.2f mm", d.distance());
        }
        unit.startPeriodicMeasurement(interval);
    }
}

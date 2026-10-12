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
#include <wiring/m5_unit_unified_wiring.hpp>  // Board-aware connection helpers (include last)

// *********************************************************************
// Choose connection
// *********************************************************************
#if !defined(CONNECT_VIA_I2C) && !defined(CONNECT_VIA_GPIO)
// UnitUltraSonicI2C (SKU:U098-B1)
// #define CONNECT_VIA_I2C
// UnitUltraSonicIO  (SKU:U098-B2)
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
    M5.setTouchButtonHeightByRatio(100);
    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    auto cfg        = unit.config();
    cfg.interval_ms = interval;
    unit.config(cfg);

#if defined(CONNECT_VIA_I2C)
    // PortA as I2C (board-aware: NessoN1 -> SoftwareI2C on GROVE, NanoC6/NanoH2 -> Ex_I2C, others -> Wire)
    M5.Log.printf("Using I2C\n");
    if (!m5::unit::wiring::addI2C(Units, unit) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        m5::unit::wiring::failStop();
    }
#elif defined(CONNECT_VIA_GPIO)
    // PortB as GPIO if available, PortA if not
    M5.Log.printf("Using GPIO\n");
    if (!m5::unit::wiring::addGPIO(Units, unit) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        m5::unit::wiring::failStop();
    }
#endif

    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.setFont(&fonts::AsciiFont8x16);
    lcd.fillScreen(TFT_DARKGREEN);
    lcd.fillRect(8, 8, 8 * 24, 16 * 1, TFT_BLACK);
}

void loop()
{
    M5.update();

    // Periodic
    Units.update();
    if (unit.updated()) {
        M5.Log.printf("Distance:%f Raw:%x\n", unit.distance(), unit.oldest().raw_distance());

        lcd.startWrite();
        lcd.fillRect(8, 8, 8 * 24, 16 * 1, TFT_BLACK);
        lcd.setCursor(8, 8 + 16 * 0);
        lcd.printf("Distance:%7.2f mm", unit.distance());
        lcd.endWrite();
    }

    if (M5.BtnA.wasClicked()) {
        unit.stopPeriodicMeasurement();
        Data d{};
        if (unit.measureSingleshot(d)) {
            M5.Log.printf("Single: %.2f mm\n", d.distance());
        }
        unit.startPeriodicMeasurement(interval);
    }
}

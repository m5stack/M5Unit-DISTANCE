/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using UltraSonicI2C and UltraSonicIO
  NOTICE: Core device needs PortA and PortB
*/
#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedDISTANCE.h>
#include <M5Utility.h>

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitUltraSonicI2C unitI2C;
m5::unit::UnitUltraSonicIO unitIO;
m5::unit::UnitRCWL9620* unit[2] = {&unitI2C, &unitIO};

const char* type_table[] = {"I2C", "GPIO"};
}  // namespace

using namespace m5::unit::rcwl9620;

void setup()
{
    M5.begin();

    // The screen shall be in landscape mode
    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    // PortA for I2C, PortB for GPIO
    auto pin_num_sda      = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl      = M5.getPin(m5::pin_name_t::port_a_scl);
    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);
    M5_LOGI("getPin: SDA:%d SCL:%d GPIO:%d %d", pin_num_sda, pin_num_scl, pin_num_gpio_in, pin_num_gpio_out);
    if (pin_num_sda < 0 || pin_num_scl < 0 || pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGE("Not enough pin settings");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
    Wire.end();
    Wire.begin(pin_num_sda, pin_num_scl, 100 * 1000U);

    if (!Units.add(unitI2C, Wire) ||                              // unit[0] I2C
        !Units.add(unitIO, pin_num_gpio_in, pin_num_gpio_out) ||  // unit[1] GPIO
        !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.clear(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    lcd.setFont(&fonts::FreeMonoBold12pt7b);
    lcd.setTextColor(TFT_ORANGE, TFT_BLACK);

    lcd.startWrite();
    lcd.clear(0);
}

void loop()
{
    M5.update();
    Units.update();

    for (uint32_t i = 0; i < m5::stl::size(unit); ++i) {
        m5::unit::UnitRCWL9620* u = unit[i];
        if (u->updated()) {
            M5.Log.printf(">%s_Distance:%f\n>%s_Raw:%u\n", type_table[i], u->distance(), type_table[i],
                          u->oldest().raw_distance());

            lcd.setCursor(8, lcd.height() / 2 * i);
            lcd.printf("%5s:%4.0f mm", type_table[i], u->distance());
        }
    }
}

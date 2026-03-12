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

const char* type_table[]   = {"I2C", "GPIO"};
const uint32_t bar_color[] = {TFT_CYAN, TFT_YELLOW};

bool use_lcd{};  // false if EPD or no display

// Layout (computed in setup)
int32_t panel_h{};    // Height per sensor panel
int32_t margin{};     // Side margin
int32_t bar_x{};      // Bar left X
int32_t bar_w_max{};  // Bar max width
int32_t bar_h{};      // Bar height
int32_t text_y[2]{};  // Text Y per panel
int32_t bar_y[2]{};   // Bar Y per panel

}  // namespace

using namespace m5::unit::rcwl9620;

void setup()
{
    M5.begin();
    M5.setTouchButtonHeightByRatio(100);
    // The screen shall be in landscape mode
    if (lcd.width() > 0 && lcd.height() > 0 && lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    {  // I2C
        auto cfg        = unitI2C.config();
        cfg.interval_ms = 150;
        unitI2C.config(cfg);
    }
    {  // GPIO
        auto cfg        = unitIO.config();
        cfg.interval_ms = 50;
        unitIO.config(cfg);
    }

    // PortA for I2C, PortB for GPIO
    auto pin_num_sda      = M5.getPin(m5::pin_name_t::port_a_sda);
    auto pin_num_scl      = M5.getPin(m5::pin_name_t::port_a_scl);
    auto pin_num_gpio_in  = M5.getPin(m5::pin_name_t::port_b_in);
    auto pin_num_gpio_out = M5.getPin(m5::pin_name_t::port_b_out);

    auto board = M5.getBoard();

    M5_LOGI("getPin: SDA:%d SCL:%d GPIO:%d %d", pin_num_sda, pin_num_scl, pin_num_gpio_in, pin_num_gpio_out);
    if (pin_num_gpio_in < 0 || pin_num_gpio_out < 0) {
        M5_LOGE("PortB not available — DualSensor requires both PortA (I2C) and PortB (GPIO)");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }

    // NessoN1: I2C sensor on QWIIC (port_a) via In_I2C, GPIO sensor on GROVE (port_b)
    //   Wire (I2C_NUM_0) is used by M5Unified In_I2C — use In_I2C directly.
    bool unit_ready{};
    if (board == m5::board_t::board_ArduinoNessoN1) {
        M5_LOGI("Using M5.In_I2C");
        unit_ready = Units.add(unitI2C, M5.In_I2C) &&
                     Units.add(unitIO, pin_num_gpio_in, pin_num_gpio_out) && Units.begin();
    } else {
        M5_LOGI("getPin: SDA:%u SCL:%u", pin_num_sda, pin_num_scl);
        Wire.end();
        Wire.begin(pin_num_sda, pin_num_scl, 100 * 1000U);
        unit_ready = Units.add(unitI2C, Wire) && Units.add(unitIO, pin_num_gpio_in, pin_num_gpio_out) && Units.begin();
    }
    if (!unit_ready) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            m5::utility::delay(10000);
        }
    }
    M5_LOGI("M5UnitUnified has been begun");
    M5_LOGI("%s", Units.debugInfo().c_str());

    // Skip LCD drawing for EPD or no display
    use_lcd = !lcd.isEPD() && lcd.width() > 0 && lcd.height() > 0;

    if (use_lcd) {
        // Compute layout based on screen size
        auto w    = lcd.width();
        auto h    = lcd.height();
        panel_h   = h / 2;
        margin    = w / 40;  // ~2.5% margin
        bar_x     = margin;
        bar_w_max = w - margin * 2;
        bar_h     = panel_h / 4;

        // Choose font size based on screen height
        if (h >= 240) {
            lcd.setFont(&fonts::FreeMonoBold12pt7b);
        } else {
            lcd.setFont(&fonts::Font2);
        }

        for (uint32_t i = 0; i < 2; ++i) {
            auto fh   = lcd.fontHeight();
            text_y[i] = panel_h * i + margin;
            bar_y[i]  = text_y[i] + fh + margin;
        }

        lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        lcd.fillScreen(TFT_BLACK);

        // Draw static labels and separator
        lcd.startWrite();
        for (uint32_t i = 0; i < 2; ++i) {
            lcd.setCursor(margin, text_y[i]);
            lcd.setTextColor(bar_color[i], TFT_BLACK);
            lcd.printf("[%s]", type_table[i]);
        }
        // Separator line
        lcd.drawFastHLine(0, panel_h, w, TFT_DARKGREY);
        lcd.endWrite();
    }
}

void loop()
{
    M5.update();
    Units.update();

    for (uint32_t i = 0; i < m5::stl::size(unit); ++i) {
        m5::unit::UnitRCWL9620* u = unit[i];
        if (u->updated()) {
            float dist = u->distance();
            M5.Log.printf(">%s_Distance:%f\n>%s_Raw:%u\n", type_table[i], dist, type_table[i],
                          u->oldest().raw_distance());

            if (use_lcd) {
                // Bar width proportional to distance (0-4500mm)
                int32_t bar_w = (int32_t)(dist / Data::MAX_DISTANCE * bar_w_max);
                if (bar_w > bar_w_max) {
                    bar_w = bar_w_max;
                }
                if (bar_w < 1) {
                    bar_w = 1;
                }

                lcd.startWrite();

                // Numeric value (right-aligned)
                lcd.setTextColor(TFT_WHITE, TFT_BLACK);
                auto text_x = margin + lcd.textWidth("[GPIO] ");
                lcd.setCursor(text_x, text_y[i]);
                lcd.printf("%7.2f mm", dist);

                // Bar: filled portion + empty portion
                lcd.fillRect(bar_x, bar_y[i], bar_w, bar_h, bar_color[i]);
                lcd.fillRect(bar_x + bar_w, bar_y[i], bar_w_max - bar_w, bar_h, TFT_DARKGREY);

                lcd.endWrite();
            }
        }
    }
}

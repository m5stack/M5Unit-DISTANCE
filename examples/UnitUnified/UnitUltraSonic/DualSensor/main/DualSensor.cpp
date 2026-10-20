/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using UltraSonicI2C and UltraSonicIO
  NOTICE: Core device needs PortA and PortB
*/
#include <cinttypes>

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedDISTANCE.h>
#include <M5Utility.h>
#include <wiring/m5_unit_unified_wiring.hpp>  // Board-aware connection helpers (include last)

namespace {
auto& lcd = M5.Display;
m5::unit::UnitUnified Units;
m5::unit::UnitUltraSonicI2C unitI2C;
m5::unit::UnitUltraSonicIO unitIO;
m5::unit::UnitRCWL9620* unit[2] = {&unitI2C, &unitIO};

const char* type_table[]   = {"I2C", "GPIO"};
const uint16_t bar_color[] = {TFT_CYAN, TFT_YELLOW};

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
    if (lcd.height() > lcd.width()) {
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
    const auto gpio_pins = m5::unit::wiring::gpioPins();
    if (gpio_pins.fallback_a) {
        M5_LOGE("PortB not available — DualSensor requires both PortA (I2C) and PortB (GPIO)");
        m5::unit::wiring::failStop();
    }

    // NessoN1: I2C sensor on QWIIC (PortA), GPIO sensor on GROVE (PortB)
    if (!m5::unit::wiring::addI2C(Units, unitI2C, 0, m5::unit::wiring::NessoPort::PortA) ||
        !m5::unit::wiring::addGPIO(Units, unitIO) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        m5::unit::wiring::failStop();
    }
    M5_LOGI("M5UnitUnified initialized");
    M5_LOGI("%s", Units.debugInfo().c_str());

    // Skip LCD drawing for EPD or no display
    use_lcd = !lcd.isEPD() && lcd.width() > 0 && lcd.height() > 0;

    if (use_lcd) {
        // Compute layout based on screen size
        const auto w = lcd.width();
        const auto h = lcd.height();
        panel_h      = h / 2;
        margin       = w / 40;  // ~2.5% margin
        bar_x        = margin;
        bar_w_max    = w - margin * 2;
        bar_h        = panel_h / 4;

        // Choose font size based on screen height
        if (h >= 240) {
            lcd.setFont(&fonts::FreeMonoBold12pt7b);
        } else {
            lcd.setFont(&fonts::Font2);
        }

        for (uint32_t i = 0; i < 2; ++i) {
            const auto fh = lcd.fontHeight();
            text_y[i]     = panel_h * i + margin;
            bar_y[i]      = text_y[i] + fh + margin;
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
            const float dist{u->distance()};
            M5.Log.printf(">%s_Distance:%f\n>%s_Raw:%" PRIu32 "\n", type_table[i], dist, type_table[i],
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

#if !defined(ARDUINO)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

#if CONFIG_FREERTOS_UNICORE
// Single-core SoCs: run loop() back-to-back, but every ~2 s yield a 5 ms slice so the IDLE task
// runs and feeds the task watchdog (default 5 s).
static inline void feedIdleTaskPeriodically(void)
{
    constexpr uint32_t FEED_INTERVAL_MS{2000};
    constexpr TickType_t FEED_SLEEP_TICKS{pdMS_TO_TICKS(5)};
    static uint32_t s_last_feed_ms{};
    const uint32_t now_ms{static_cast<uint32_t>(esp_timer_get_time() / 1000)};
    if (now_ms - s_last_feed_ms >= FEED_INTERVAL_MS) {
        s_last_feed_ms = now_ms;
        vTaskDelay(FEED_SLEEP_TICKS);
    }
}
#endif

extern "C" void app_main(void)
{
    setup();
    for (;;) {
#if CONFIG_FREERTOS_UNICORE
        feedIdleTaskPeriodically();
#endif
        loop();
    }
}
#endif

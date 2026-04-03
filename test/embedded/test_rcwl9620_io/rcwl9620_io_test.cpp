/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitUltraSonicIO (GPIO variant)
*/
#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <googletest/test_helper.hpp>
#include <unit/unit_UltraSonic.hpp>
#include <cmath>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::rcwl9620;

constexpr uint32_t STORED_SIZE{8};

class TestRCWL9620_IO : public GPIOComponentTestBase<UnitUltraSonicIO> {
protected:
    virtual UnitUltraSonicIO* get_instance() override
    {
        auto ptr         = new m5::unit::UnitUltraSonicIO();
        auto ccfg        = ptr->component_config();
        ccfg.stored_size = STORED_SIZE;
        ptr->component_config(ccfg);
        return ptr;
    }
};

#define RCWL9620_TEST_CLASS TestRCWL9620_IO
#define MIN_INTERVAL        50
#define MEDIAN_TOLERANCE    50  // GPIO: pulseInRX takes up to 50ms per measurement
#include "../rcwl9620_test.inl"

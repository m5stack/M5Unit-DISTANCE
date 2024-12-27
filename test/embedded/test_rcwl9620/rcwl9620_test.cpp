/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitRCWL9620
*/
#include <gtest/gtest.h>
#include <Wire.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <googletest/test_helper.hpp>
#include <unit/unit_RCWL9620.hpp>
#include <cmath>
#include <random>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::rcwl9620;
using m5::unit::types::elapsed_time_t;

const ::testing::Environment* global_fixture = ::testing::AddGlobalTestEnvironment(new GlobalFixture<100000U>());

constexpr uint32_t STORED_SIZE{4};

class TestRCWL9620 : public ComponentTestBase<UnitRCWL9620, bool> {
protected:
    virtual UnitRCWL9620* get_instance() override
    {
        auto ptr         = new m5::unit::UnitRCWL9620();
        auto ccfg        = ptr->component_config();
        ccfg.stored_size = STORED_SIZE;
        ptr->component_config(ccfg);
        return ptr;

        return ptr;
    }
    virtual bool is_using_hal() const override
    {
        return GetParam();
    };
};

// INSTANTIATE_TEST_SUITE_P(ParamValues, TestRCWL9620, ::testing::Values(false, true));
// INSTANTIATE_TEST_SUITE_P(ParamValues, TestRCWL9620, ::testing::Values(true));
INSTANTIATE_TEST_SUITE_P(ParamValues, TestRCWL9620, ::testing::Values(false));

namespace {

}  // namespace

TEST_P(TestRCWL9620, Periodic)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(120));
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

#if 0    
    for (auto&& iir : iir_table) {
        for (auto&& fir : fir_table) {
            if (m5::stl::to_underlying(fir) < 4) {
                continue;
            }

            auto s = m5::utility::formatString("IIR:%u FIR:%u", iir, fir);
            SCOPED_TRACE(s);

            //                EXPECT_TRUE(unit->startPeriodicMeasurement(iir, fir, Gain::Coeff12_5, irs));
            EXPECT_TRUE(unit->startPeriodicMeasurement(iir, fir, Gain::Coeff12_5, IRSensor::Dual));
            EXPECT_TRUE(unit->inPeriodic());

            auto tm      = get_interval(iir, fir);
            auto elapsed = test_periodic(unit.get(), STORED_SIZE, tm ? tm : 1);

            EXPECT_TRUE(unit->stopPeriodicMeasurement());
            EXPECT_FALSE(unit->inPeriodic());

            EXPECT_NE(elapsed, 0);
            EXPECT_GE(elapsed + 2, STORED_SIZE * (tm ? tm : 1));

            M5_LOGI("TM:%u IT:%u e:%ld", tm, unit->interval(), elapsed);

            //
            EXPECT_EQ(unit->available(), STORED_SIZE);
            EXPECT_FALSE(unit->empty());
            EXPECT_TRUE(unit->full());

            uint32_t cnt{STORED_SIZE / 2};
            while (cnt-- && unit->available()) {
                EXPECT_TRUE(std::isfinite(unit->ambientTemperature()));
                EXPECT_FLOAT_EQ(unit->ambientTemperature(), unit->oldest().ambientTemperature());
                EXPECT_TRUE(std::isfinite(unit->objectTemperature1()));
                EXPECT_FLOAT_EQ(unit->objectTemperature1(), unit->oldest().objectTemperature1());
                EXPECT_TRUE(std::isfinite(unit->objectTemperature2()));
                EXPECT_FLOAT_EQ(unit->objectTemperature2(), unit->oldest().objectTemperature2());

                EXPECT_FALSE(unit->empty());
                unit->discard();
            }
            EXPECT_EQ(unit->available(), STORED_SIZE / 2);
            EXPECT_FALSE(unit->empty());
            EXPECT_FALSE(unit->full());

            unit->flush();
            EXPECT_EQ(unit->available(), 0);
            EXPECT_TRUE(unit->empty());
            EXPECT_FALSE(unit->full());

            EXPECT_FALSE(std::isfinite(unit->ambientTemperature()));
            EXPECT_FALSE(std::isfinite(unit->objectTemperature1()));
            EXPECT_FALSE(std::isfinite(unit->objectTemperature2()));
        }
    }
#endif
}

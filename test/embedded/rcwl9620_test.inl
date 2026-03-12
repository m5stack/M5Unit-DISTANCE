/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Shared test implementations for UnitRCWL9620 (I2C and GPIO variants)

  Before including this file, define:
    RCWL9620_TEST_CLASS  — test fixture class name
    MIN_INTERVAL         — minimum measurement interval (ms)
    MEDIAN_TOLERANCE     — tolerance for median interval check (ms)
*/

// clang-format off
#ifndef RCWL9620_TEST_CLASS
#error "Define RCWL9620_TEST_CLASS before including rcwl9620_test.inl"
#endif
#ifndef MIN_INTERVAL
#error "Define MIN_INTERVAL before including rcwl9620_test.inl"
#endif
#ifndef MEDIAN_TOLERANCE
#error "Define MEDIAN_TOLERANCE before including rcwl9620_test.inl"
#endif
// clang-format on

TEST_F(RCWL9620_TEST_CLASS, Periodic)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(MIN_INTERVAL - 1));
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_TRUE(unit->startPeriodicMeasurement(MIN_INTERVAL));
    uint32_t timeout = std::max<uint32_t>(unit->interval(), 500) * (STORED_SIZE + 1);
    auto r           = collect_periodic_measurements(unit.get(), STORED_SIZE, timeout);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_FALSE(r.timed_out);
    EXPECT_EQ(r.update_count, STORED_SIZE);
    EXPECT_LE(r.median(), r.expected_interval + MEDIAN_TOLERANCE);

    //
    EXPECT_EQ(unit->available(), STORED_SIZE);
    EXPECT_FALSE(unit->empty());
    EXPECT_TRUE(unit->full());

    uint32_t cnt{STORED_SIZE / 2};
    while (cnt-- && unit->available()) {
        EXPECT_TRUE(std::isfinite(unit->distance()));
        EXPECT_FLOAT_EQ(unit->distance(), unit->oldest().distance());
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

    EXPECT_FALSE(std::isfinite(unit->distance()));
}

TEST_F(RCWL9620_TEST_CLASS, Singleshot)
{
    SCOPED_TRACE(ustr);

    Data d{};

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->measureSingleshot(d));

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    uint32_t cnt{8};
    while (cnt--) {
        EXPECT_TRUE(unit->measureSingleshot(d));
        EXPECT_TRUE(std::isfinite(d.distance()));
    }
}

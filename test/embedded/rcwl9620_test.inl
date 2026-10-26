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

TEST_F(RCWL9620_TEST_CLASS, BeginAppliesConfig)
{
    SCOPED_TRACE(ustr);

    // begin() cannot start periodic measurement while it is already running
    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    auto cfg           = unit->config();
    cfg.start_periodic = false;
    unit->config(cfg);
    EXPECT_TRUE(unit->begin());
    EXPECT_FALSE(unit->inPeriodic());

    cfg.start_periodic = true;
    cfg.interval_ms    = MIN_INTERVAL + 50;
    unit->config(cfg);
    EXPECT_TRUE(unit->begin());
    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_EQ(unit->interval(), MIN_INTERVAL + 50U);
    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    cfg.interval_ms = MIN_INTERVAL - 1;
    unit->config(cfg);
    EXPECT_FALSE(unit->begin());
    EXPECT_FALSE(unit->inPeriodic());
}

TEST_F(RCWL9620_TEST_CLASS, Periodic)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(MIN_INTERVAL));  // Already running
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());
    EXPECT_FALSE(unit->startPeriodicMeasurement(MIN_INTERVAL - 1));  // Below the minimum interval
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_TRUE(unit->startPeriodicMeasurement(MIN_INTERVAL));
    uint32_t timeout = std::max<uint32_t>(unit->interval(), 500) * (STORED_SIZE + 1);
    auto r           = collect_periodic_measurements(unit.get(), STORED_SIZE, timeout);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_FALSE(unit->inPeriodic());

    EXPECT_FALSE(r.timed_out);
    EXPECT_EQ(r.update_count, STORED_SIZE);
    EXPECT_GE(r.median(), r.expected_interval);
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

TEST_F(RCWL9620_TEST_CLASS, StopAfterInterval)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->stopPeriodicMeasurement());

    // Stopping after the interval has passed must not wait for another interval
    constexpr uint32_t interval{MIN_INTERVAL * 4};
    EXPECT_TRUE(unit->startPeriodicMeasurement(interval));
    m5::utility::delay(interval * 2);

    const auto start_at = m5::utility::millis();
    EXPECT_TRUE(unit->stopPeriodicMeasurement());
    EXPECT_LT(m5::utility::elapsedSince(start_at), interval);
    EXPECT_FALSE(unit->inPeriodic());

    // GPIO measures within update(), so stopping does not wait
    if (unit->adapter()->type() == m5::unit::Adapter::Type::GPIO) {
        EXPECT_TRUE(unit->startPeriodicMeasurement(interval));
        const auto stop_at = m5::utility::millis();
        EXPECT_TRUE(unit->stopPeriodicMeasurement());
        EXPECT_LT(m5::utility::elapsedSince(stop_at), 10U);
        EXPECT_FALSE(unit->inPeriodic());
    }
}

TEST_F(RCWL9620_TEST_CLASS, MeasureAfterMove)
{
    SCOPED_TRACE(ustr);

    // The moved-to unit must measure through its own adapter, not the moved-from unit
    decltype(unit)::element_type moved{std::move(*unit)};

    EXPECT_TRUE(moved.inPeriodic());
    EXPECT_TRUE(moved.stopPeriodicMeasurement());
    EXPECT_FALSE(moved.inPeriodic());

    Data d{};
    uint32_t cnt{8};
    while (cnt--) {
        EXPECT_TRUE(moved.measureSingleshot(d));
        EXPECT_TRUE(std::isfinite(d.distance()));
    }
}

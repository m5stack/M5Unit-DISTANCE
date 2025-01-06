/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_RCWL9620.cpp
  @brief RCWL9620 Unit for M5UnitUnified
*/
#include "unit_RCWL9620.hpp"
#include <M5Utility.hpp>
#include <thread>

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::rcwl9620;
using namespace m5::unit::rcwl9620::command;

namespace {
// Datasheet says
// 向模块写入 0X01 ，模块开始测距；等待 100mS 模块最大测距时间
// Note : Max is assumed to be 50+100 since there is no description of Max.
// A larger value would be considered better?
constexpr uint32_t minimum_interval{150};
}  // namespace

namespace m5 {
namespace unit {

// class UnitRCWL9620
const char UnitRCWL9620::name[] = "UnitRCWL9620";
const types::uid_t UnitRCWL9620::uid{"UnitRCWL9620"_mmh3};
const types::uid_t UnitRCWL9620::attr{0};

bool UnitRCWL9620::begin()
{
    auto ssize = stored_size();
    assert(ssize && "stored_size must be greater than zero");
    if (ssize != _data->capacity()) {
        _data.reset(new m5::container::CircularBuffer<Data>(ssize));
        if (!_data) {
            M5_LIB_LOGE("Failed to allocate");
            return false;
        }
    }

    // May be in the requested state, so data is retrieved and discarded
    Data discard{};
    read_measurement(discard);

    return _cfg.start_periodic ? startPeriodicMeasurement(_cfg.interval_ms) : true;
}

void UnitRCWL9620::update(const bool force)
{
    _updated = false;
    if (inPeriodic()) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            Data d{};
            _updated = read_measurement(d);
            if (_updated) {
                _latest = at;
                _data->push_back(d);
                if (!request_measurement()) {
                    _periodic = false;
                    M5_LIB_LOGE("Periodic measurements have been suspended");
                    return;
                }
            }
        }
    }
}

bool UnitRCWL9620::measureSingleshot(rcwl9620::Data& d)
{
    if (inPeriodic()) {
        M5_LIB_LOGD("Periodic measurements are running");
        return false;
    }

    if (request_measurement()) {
        m5::utility::delay(100);
        return read_measurement(d);
    }
    return false;
}

//
bool UnitRCWL9620::start_periodic_measurement(const uint32_t interval)
{
    if (inPeriodic()) {
        return false;
    }

    if (interval < minimum_interval) {
        M5_LIB_LOGE("Interval must be greater equal %u, %u", minimum_interval, interval);
        return false;
    }

    _periodic = request_measurement();
    if (_periodic) {
        _interval = interval;
        _latest   = m5::utility::millis();
    }
    return _periodic;
}

bool UnitRCWL9620::stop_periodic_measurement()
{
    if (inPeriodic()) {
        // Since the request has already been issued, the value should be retrieved
        auto it  = interval();
        auto dms = it - (m5::utility::millis() - updatedMillis());
        if (dms > it) {
            dms = it;
        }
        m5::utility::delay(dms);

        uint32_t cnt{8};
        Data discard{};
        do {
            if (read_measurement(discard)) {
                _periodic = false;
                return true;
            }
            m5::utility::delay(1);
        } while (cnt--);
    }
    return false;
}

bool UnitRCWL9620::request_measurement()
{
    // Only write command
    return writeRegister(MEASURE_DISTANCE, nullptr, 0);
}

bool UnitRCWL9620::read_measurement(rcwl9620::Data& d)
{
    std::fill(d.raw.begin(), d.raw.end(), 0x00);
    uint32_t cnt{8};
    do {
        if (readWithTransaction(d.raw.data(), d.raw.size()) == m5::hal::error::error_t::OK) {
            return true;
        }
        std::this_thread::yield();
    } while (cnt--);
    return false;
}

}  // namespace unit
}  // namespace m5

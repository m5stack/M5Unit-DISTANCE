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

using namespace m5::utility::mmh3;
using namespace m5::unit::types;
using namespace m5::unit::rcwl9620;
using namespace m5::unit::rcwl9620::command;

namespace {
}  // namespace

namespace m5 {
namespace unit {

// Class that abstracts the interaction between classes and adapters
// For I2C
class InterfaceI2C : public UnitRCWL9620::Interface {
public:
    explicit InterfaceI2C(UnitRCWL9620& u) : UnitRCWL9620::Interface(u)
    {
    }
    virtual ~InterfaceI2C()
    {
    }
    virtual bool read_measurement(Data& d, bool& timeouted) override
    {
        timeouted = false;
        std::fill(d.raw.begin(), d.raw.end(), 0x00);
        uint32_t cnt{4};
        do {
            if (_unit.readWithTransaction(d.raw.data(), d.raw.size()) == m5::hal::error::error_t::OK) {
                _requested = false;
                return true;
            }
            timeouted = true;
        } while (cnt--);
        return false;
    }
    inline virtual bool request_measurement() override
    {
        if (!_requested) {
            _requested = _unit.writeRegister(MEASURE_DISTANCE, nullptr, 0);
        }
        return _requested;
    }
    bool _requested{};
};

// For GPIO
class InterfaceGPIO : public UnitRCWL9620::Interface {
public:
    explicit InterfaceGPIO(UnitRCWL9620& u) : UnitRCWL9620::Interface(u)
    {
        _unit.pinModeRX(gpio::Mode::Input);
        _unit.pinModeTX(gpio::Mode::Output);
        _unit.writeDigitalTX(false);
    }
    virtual ~InterfaceGPIO()
    {
    }
    virtual bool read_measurement(Data& d, bool& timeouted) override
    {
        std::fill(d.raw.begin(), d.raw.end(), 0x00);

        // Request
        _unit.writeDigitalTX(LOW);
        m5::utility::delayMicroseconds(2);
        _unit.writeDigitalTX(HIGH);
        m5::utility::delayMicroseconds(10);
        _unit.writeDigitalTX(LOW);

        // Read
        uint32_t duration{};
        if (!_unit.pulseInRX(duration, HIGH, 50000)) {
            return false;
        }

        const uint32_t distance_mm = static_cast<uint32_t>(duration * 0.343f / 2.0f);
        const uint32_t distance_um = static_cast<uint32_t>(distance_mm * 1000.0f);
        d.raw[0]                   = (distance_um >> 16) & 0xFF;
        d.raw[1]                   = (distance_um >> 8) & 0xFF;
        d.raw[2]                   = distance_um & 0xFF;
        return true;
    }
    virtual bool request_measurement() override
    {
        // Ignored because request and read are not separated by GPIO
        return true;
    }
};

// class UnitRCWL9620
const char UnitRCWL9620::name[] = "UnitRCWL9620";
const types::uid_t UnitRCWL9620::uid{"UnitRCWL9620"_mmh3};
const types::attr_t UnitRCWL9620::attr{attribute::AccessI2C | attribute::AccessGPIO};

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

    // Check adapter type
    auto atype = adapter()->type();
    switch (atype) {
        case Adapter::Type::I2C:
            _interface.reset(new InterfaceI2C(*this));
            break;
        case Adapter::Type::GPIO:
            _interface.reset(new InterfaceGPIO(*this));
            break;
        default:
            break;
    }
    if (!_interface) {
        M5_LIB_LOGE("Invalid adapter %u", atype);
        return false;
    }

    return _cfg.start_periodic ? startPeriodicMeasurement(_cfg.interval_ms) : true;
}

void UnitRCWL9620::update(const bool force)
{
    _updated = false;
    if (inPeriodic()) {
        elapsed_time_t at{m5::utility::millis()};
        if (force || !_latest || at >= _latest + _interval) {
            bool timeouted{};
            Data d{};
            _updated = read_measurement(d, timeouted);
            if (_updated) {
                // Data is invalid after Timeout has occurred
                if (!timeouted) {
                    _data->push_back(d);
                }
                if (!request_measurement()) {
                    _periodic = false;
                    M5_LIB_LOGE("Periodic measurements have been suspended");
                    return;
                }
                _latest = m5::utility::millis();
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

    bool timeouted{};
    if (request_measurement()) {
        m5::utility::delay(100);
        return read_measurement(d, timeouted) && !timeouted;
    }
    return false;
}

//
bool UnitRCWL9620::start_periodic_measurement(const uint32_t interval)
{
    if (inPeriodic()) {
        return false;
    }

    if (interval < minimum_interval()) {
        M5_LIB_LOGE("Interval must be greater equal %u, (%u)", minimum_interval(), interval);
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
        bool timeouted{};
        Data discard{};
        do {
            if (read_measurement(discard, timeouted)) {
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
    //    return writeRegister(MEASURE_DISTANCE, nullptr, 0);
    return _interface->request_measurement();
}

bool UnitRCWL9620::read_measurement(rcwl9620::Data& d, bool& timeouted)
{
    return _interface->read_measurement(d, timeouted);
}

}  // namespace unit
}  // namespace m5

/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_UltraSonic.cpp
  @brief UltraSonic I2C/IO Unit for M5UnitUnified
*/
#include "unit_UltraSonic.hpp"

using namespace m5::utility::mmh3;
using namespace m5::unit;
using namespace m5::unit::types;

namespace m5 {
namespace unit {

// class UnitUltraSonicI2C
const char UnitUltraSonicI2C::name[] = "UnitUltraSonicI2C";
const types::uid_t UnitUltraSonicI2C::uid{"UnitUltraSonicI2C"_mmh3};
const types::attr_t UnitUltraSonicI2C::attr{attribute::AccessI2C};

bool UnitUltraSonicI2C::begin()
{
    if (adapter()->type() != Adapter::Type::I2C) {
        M5_LIB_LOGE("Not I2C connection");
        return false;
    }
    return UnitRCWL9620::begin();
}

// class UnitUltraSonicIO
const char UnitUltraSonicIO::name[] = "UnitUltraSonicIO";
const types::uid_t UnitUltraSonicIO::uid{"UnitUltraSonicIO"_mmh3};
const types::attr_t UnitUltraSonicIO::attr{attribute::AccessGPIO};

bool UnitUltraSonicIO::begin()
{
    const types::uid_t pbhub_uid = "UnitPbHub"_mmh3;

    if (adapter()->type() != Adapter::Type::GPIO) {
        M5_LIB_LOGE("Not GPIO connection");
        if (parent() && parent()->identifier() == pbhub_uid) {
            M5_LIB_LOGE("Not available via PbHub connection");
        }
        return false;
    }
    return UnitRCWL9620::begin();
}

}  // namespace unit
}  // namespace m5

/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_UltraSonic.hpp
  @brief UltraSonic I2C/IO Unit for M5UnitUnified
*/
#ifndef M5_UNIT_DISTANCE_UNIT_ULTRA_SONIC_HPP
#define M5_UNIT_DISTANCE_UNIT_ULTRA_SONIC_HPP

#include "unit_RCWL9620.hpp"

namespace m5 {
namespace unit {

/*!
  @class m5::unit::UnitUltraSonicI2C
  @brief An ultrasonic distance measuring sensor unit for I2C
*/
class UnitUltraSonicI2C : public UnitRCWL9620 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitUltraSonicI2C, 0x57);

public:
    explicit UnitUltraSonicI2C() : UnitRCWL9620(DEFAULT_ADDRESS)
    {
    }
    virtual bool begin() override;
};

/*!
  @class m5::unit::UnitUltraSonicIO
  @brief An ultrasonic distance measuring sensor unit for GPIO
*/
class UnitUltraSonicIO : public UnitRCWL9620 {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitUltraSonicIO, 0x00);

public:
    explicit UnitUltraSonicIO() : UnitRCWL9620(DEFAULT_ADDRESS)
    {
    }
    virtual bool begin() override;

protected:
    inline virtual uint32_t minimum_interval() const
    {
        return 50;
    }
};

}  // namespace unit
}  // namespace m5
#endif

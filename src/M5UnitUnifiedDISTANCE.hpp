/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnifiedDISTANCE.hpp
  @brief Main header of M5UnitDISTANCE using M5UnitUnified

  @mainpage M5Unit-DISTANCE
  Library for UnitDISTANCE using M5UnitUnified.
*/
#ifndef M5_UNIT_UNIFIED_DISTANCE_HPP
#define M5_UNIT_UNIFIED_DISTANCE_HPP

#include "unit/unit_RCWL9620.hpp"

/*!
  @namespace m5
  @brief Top level namespace of M5stack
 */
namespace m5 {

/*!
  @namespace unit
  @brief Unit-related namespace
 */
namespace unit {

using UnitUltraSonicI2C = m5::unit::UnitRCWL9620;

}  // namespace unit
}  // namespace m5
#endif

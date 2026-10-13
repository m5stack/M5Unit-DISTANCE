/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitUltraSonic I2C/IO
*/
// *********************************************************************
// Choose connection
// *********************************************************************
#if !defined(CONNECT_VIA_I2C) && !defined(CONNECT_VIA_GPIO)
// For UnitUltraSonicI2C (U098-B1)
// #define CONNECT_VIA_I2C
// For UnitUltraSonicIO (U098-B2)
// #define CONNECT_VIA_GPIO
#endif
#include "main/PlotToSerial.cpp"

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
// #define CONNECT_VIA_I2C
// #define CONNECT_VIA_GPIO
#endif
#include "main/PlotToSerial.cpp"

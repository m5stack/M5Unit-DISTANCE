/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file unit_RCWL9620.hpp
  @brief RCWL9620 Unit for M5UnitUnified
*/
#ifndef M5_UNIT_THERMO_UNIT_RCWL9620_HPP
#define M5_UNIT_THERMO_UNIT_RCWL9620_HPP

#include <M5UnitComponent.hpp>
#include <m5_utility/container/circular_buffer.hpp>
#include <limits>  // NaN
#include <array>

namespace m5 {
namespace unit {

/*!
  @namespace rcwl9620
  @brief For RCWL9620
 */
namespace rcwl9620 {

/*!
  @struct Data
  @brief Measurement data group
 */
struct Data {
    std::array<uint8_t, 3> raw{};  // Raw data

    static constexpr float MAX_DISTANCE{4500.f};
    static constexpr float MIN_DISTANCE{20.f};

    //! Get distance(Unit 1 mm)
    float distance() const
    {
        float fd = raw_distance() / 1000.f;
        return ((fd >= MIN_DISTANCE) && (fd <= MAX_DISTANCE)) ? fd : std::numeric_limits<float>::quiet_NaN();
    }

    uint32_t raw_distance() const
    {
        return ((uint32_t)raw[0] << 16) | ((uint32_t)raw[1] << 8) | (uint32_t)raw[2];
    }
};

}  // namespace rcwl9620

/*!
  @class UnitRCWL9620
  @brief An ultrasonic distance measuring sensor unit
*/
class UnitRCWL9620 : public Component, public PeriodicMeasurementAdapter<UnitRCWL9620, rcwl9620::Data> {
    M5_UNIT_COMPONENT_HPP_BUILDER(UnitRCWL9620, 0x57);

public:
    /*!
      @struct config_t
      @brief Settings for begin
     */
    struct config_t {
        //! Start periodic measurement on begin?
        bool start_periodic{true};
        //! Interval time if start on begin (ms)
        uint32_t interval_ms{120};
    };

    explicit UnitRCWL9620(const uint8_t addr = DEFAULT_ADDRESS)
        : Component(addr), _data{new m5::container::CircularBuffer<rcwl9620::Data>(1)}
    {
        auto ccfg  = component_config();
        ccfg.clock = 100 * 1000U;
        component_config(ccfg);
    }
    virtual ~UnitRCWL9620()
    {
    }

    virtual bool begin() override;
    virtual void update(const bool force = false) override;

    ///@name Settings for begin
    ///@{
    /*! @brief Gets the configration */
    inline config_t config()
    {
        return _cfg;
    }
    //! @brief Set the configration
    inline void config(const config_t& cfg)
    {
        _cfg = cfg;
    }
    ///@}

    ///@name Measurement data by periodic
    ///@{
    //! @brief Oldest distance (mm)
    float distance() const
    {
        return !empty() ? oldest().distance() : std::numeric_limits<float>::quiet_NaN();
    }
    ///@}

    ///@name Periodic measurement
    ///@{
    /*!
      @brief Start periodic measurement
      @param interval Measurement interval
      @return True if successful
    */
    inline bool startPeriodicMeasurement(const uint32_t interval)
    {
        return PeriodicMeasurementAdapter<UnitRCWL9620, rcwl9620::Data>::startPeriodicMeasurement(interval);
    }
    //! @brief Start periodic measurement using current settings
    inline bool startPeriodicMeasurement()
    {
        return PeriodicMeasurementAdapter<UnitRCWL9620, rcwl9620::Data>::startPeriodicMeasurement();
    }
    /*!
      @brief Stop periodic measurement
      @return True if successful
    */
    inline bool stopPeriodicMeasurement()
    {
        return PeriodicMeasurementAdapter<UnitRCWL9620, rcwl9620::Data>::stopPeriodicMeasurement();
    }
    ///@}

protected:
    bool request_measurement();
    bool read_measurement(rcwl9620::Data& d);

    bool start_periodic_measurement(const uint32_t interval);
    bool start_periodic_measurement();
    bool stop_periodic_measurement();

    M5_UNIT_COMPONENT_PERIODIC_MEASUREMENT_ADAPTER_HPP_BUILDER(UnitRCWL9620, rcwl9620::Data);

private:
    std::unique_ptr<m5::container::CircularBuffer<rcwl9620::Data>> _data{};
    config_t _cfg{};
};

///@cond 0
namespace rcwl9620 {
namespace command {
constexpr uint8_t MEASURE_DISTANCE{0x01};
}  // namespace command
}  // namespace rcwl9620
///@endcond

}  // namespace unit
}  // namespace m5
#endif

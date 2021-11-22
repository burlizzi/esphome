#pragma once

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pid {

struct PIDController {
  float update(float setpoint, float process_value) {
    // e(t) ... error at timestamp t
    // r(t) ... setpoint
    // y(t) ... process value (sensor reading)
    // u(t) ... output value

    float dt = (millis() - this->last_time_)/1000;

    // e(t) := r(t) - y(t)
    error = setpoint - process_value;

    // p(t) := K_p * e(t)
    proportional_term = kp * error;

    // i(t) := K_i * \int_{0}^{t} e(t) dt
    accumulated_integral_ += error * dt * ki;
    // constrain accumulated integral value
    if (!std::isnan(min_integral) && accumulated_integral_ < min_integral)
      accumulated_integral_ = min_integral;
    if (!std::isnan(max_integral) && accumulated_integral_ > max_integral)
      accumulated_integral_ = max_integral;
    if (std::isnan(previous_value_))
      previous_value_= process_value;
      
    integral_term = accumulated_integral_;

    // d(t) := K_d * de(t)/dt
    float derivative = 0.0f;
    if (fabs(process_value-previous_value_)>0.1 || (millis() - this->last_time_)>600000)
    {
      ESP_LOGI("pid","processing %f-%f %d,%d",process_value,previous_value_,millis() , this->last_time_);
      dt=calculate_relative_time_();
      if (dt != 0.0f)
        derivative = (previous_value_ - process_value) / dt;
      previous_value_ = process_value;
      derivative_term = kd * derivative;
    }
    else
      ESP_LOGI("pid","non ancora %f-%f %d,%d",process_value,previous_value_,millis() , this->last_time_);

    //process_value=previous_value_*0.95+process_value*0.05;


    // u(t) := p(t) + i(t) + d(t)
    return proportional_term + integral_term + derivative_term;
  }

  void reset_accumulated_integral() { accumulated_integral_ = 0; }

  /// Proportional gain K_p.
  float kp = 0;
  /// Integral gain K_i.
  float ki = 0;
  /// Differential gain K_d.
  float kd = 0;

  float min_integral = NAN;
  float max_integral = NAN;

  // Store computed values in struct so that values can be monitored through sensors
  float error;
  float proportional_term;
  float integral_term;
  float derivative_term;

 protected:
  float calculate_relative_time_() {
    uint32_t now = millis();
    uint32_t dt = now - this->last_time_;
    if (last_time_ == 0) {
      last_time_ = now;
      return 0.0f;
    }
    last_time_ = now;
    return dt / 1000.0f;
  }

  /// Error from previous update used for derivative term
  float previous_value_ = NAN;
  /// Accumulated integral value
  float accumulated_integral_ = 0;
  uint32_t last_time_ = 0;
};

}  // namespace pid
}  // namespace esphome

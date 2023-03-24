#pragma once
#include "esphome/core/hal.h"
//<<<<<<< HEAD
#include "esphome/core/log.h"
//=======
#include <deque>
#include <cmath>
//>>>>>>> dd8dc1ef1d2d5f99072e61061bd243ae5ec7d6c8

namespace esphome {
namespace pid {

struct PIDController {
/*<<<<<<< HEAD
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


    if (std::isnan(previous_value_))
      previous_value_= process_value;


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
    //else  ESP_LOGI("pid","non ancora %f-%f %d,%d",process_value,previous_value_,millis() , this->last_time_);

    //process_value=previous_value_*0.95+process_value*0.05;

    // i(t) := K_i * \int_{0}^{t} e(t) dt
    accumulated_integral_ += error * dt * ki + derivative_term;
    // constrain accumulated integral value
    if (!std::isnan(min_integral) && accumulated_integral_ < min_integral)
      accumulated_integral_ = min_integral;
    if (!std::isnan(max_integral) && accumulated_integral_ > max_integral)
      accumulated_integral_ = max_integral;
    integral_term = accumulated_integral_;


    // u(t) := p(t) + i(t) + d(t)
    return proportional_term + integral_term + derivative_term;
  }
=======*/
  float update(float setpoint, float process_value);
//>>>>>>> dd8dc1ef1d2d5f99072e61061bd243ae5ec7d6c8

  void reset_accumulated_integral() { accumulated_integral_ = 0; }
  void set_starting_integral_term(float in) { accumulated_integral_ = in; }

  bool in_deadband();

  friend class PIDClimate;

 private:
  /// Proportional gain K_p.
  float kp_ = 0;
  /// Integral gain K_i.
  float ki_ = 0;
  /// Differential gain K_d.
  float kd_ = 0;

  // smooth the derivative value using a weighted average over X samples
  int derivative_samples_ = 8;

  /// smooth the output value using a weighted average over X values
  int output_samples_ = 1;

  float threshold_low_ = 0.0f;
  float threshold_high_ = 0.0f;
  float kp_multiplier_ = 0.0f;
  float ki_multiplier_ = 0.0f;
  float kd_multiplier_ = 0.0f;
  int deadband_output_samples_ = 1;

  float min_integral_ = NAN;
  float max_integral_ = NAN;

  // Store computed values in struct so that values can be monitored through sensors
  float error_;
  float dt_;
  float proportional_term_;
  float integral_term_;
  float derivative_term_;

  void calculate_proportional_term_();
  void calculate_integral_term_();
  void calculate_derivative_term_();
  float weighted_average_(std::deque<float> &list, float new_value, int samples);
  float calculate_relative_time_();

  /// Error from previous update used for derivative term
  float previous_value_ = NAN;
  /// Accumulated integral value
  float accumulated_integral_ = 0;
  uint32_t last_time_ = 0;

  // this is a list of derivative values for smoothing.
  std::deque<float> derivative_list_;

  // this is a list of output values for smoothing.
  std::deque<float> output_list_;

};  // Struct PID Controller
}  // namespace pid
}  // namespace esphome

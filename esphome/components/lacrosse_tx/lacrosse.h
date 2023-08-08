#pragma once

#include <utility>
#include <functional>

#include "esphome/components/remote_receiver/remote_receiver.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/remote_base/rc_switch_protocol.h"

namespace esphome {
namespace lacrosse_tx {
static const char *const TAG = "lacrosse";

class LacrosseSensor : public Component, public remote_base::RemoteReceiverListener {
 public:
  void set_index(std::function<int()> getter) { this->getter_ = getter; }
  void set_index(int index) {
    this->getter_ = [index]() { return index; };
  }
  void set_temperature_sensor(sensor::Sensor *temperature) { this->temperature_ = temperature; }
  void set_humidity_sensor(sensor::Sensor *humidity) { this->humidity_ = humidity; }

 protected:
  // Dummy implement on_receive so implementation is optional for inheritors
  bool on_receive(remote_base::RemoteReceiveData data) override;

  sensor::Sensor *temperature_ = nullptr;
  sensor::Sensor *humidity_ = nullptr;
  std::function<int()> getter_;
};

}  // namespace lacrosse
}  // namespace esphome

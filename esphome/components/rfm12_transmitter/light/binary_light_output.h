#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/remote_base/rc_switch_protocol.h"
#include "esphome/components/rfm12_transmitter/remote_transmitter.h"

namespace esphome {
namespace binary {

class RfmLightOutput : public light::LightOutput {
 public:
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }
  void set_group(const char* code)
  {
    action.set_group(code);
  }
  void set_device(const char*code)
  {
    action.set_device(code);
  }
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
    action.set_parent(transmitter);
  }
  void write_state(light::LightState *state) override {
    bool binary;
    state->current_values_as_binary(&binary);
    action.set_send_times(10);
    action.set_send_wait(0);
    #define PULSE 424
    action.set_protocol(remote_base::RCSwitchBase(PULSE, PULSE*31, PULSE, PULSE*3, PULSE*3, PULSE, false));
    action.set_state(binary);
    action.play(binary);
  }

 protected:
 remote_base::RCSwitchTypeAAction<bool> action;
};

}  // namespace binary
}  // namespace esphome

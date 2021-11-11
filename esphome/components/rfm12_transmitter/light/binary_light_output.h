#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/remote_base/rc_switch_protocol.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/rfm12_transmitter/remote_transmitter.h"
#include "esphome/components/remote_receiver/remote_receiver.h"
#define PULSE 424
namespace esphome {
namespace binary {

static const char *const TAG = "rflight";
class BinaryLightRemoteSensor:public remote_base::RemoteReceiverBinarySensorBase
{
  public:
  void set_group(uint64_t u_group)
  {
    this->group_=u_group;
  }
  void set_device(uint64_t  u_device)
  {
    this->device_=u_device;
  }
  virtual void setup_state(light::LightState *state) 
  {
    state_=state;
    ESP_LOGD(TAG,"registered");
  }

 protected:
  bool matches(remote_base::RemoteReceiveData src) override
  {
    //ESP_LOGD(TAG,"something happened");
    uint8_t nbits{24};
    uint64_t code;
    remote_base::RCSwitchBase::type_a_code(this->group_, this->device_, false, &code, &nbits);
    

    uint64_t decoded_code;
    uint8_t decoded_nbits;


    if (!remote_base::RC_SWITCH_PROTOCOLS[1].decode(src, &decoded_code, &decoded_nbits))
      return false;
    ESP_LOGD(TAG,"decoded %llx|>%llx %d bits",decoded_code,code,decoded_nbits);

    if(!(decoded_nbits == nbits && (decoded_code & this->mask_) == (code & this->mask_)))
      return false;

    ESP_LOGD(TAG,"matches!! %llx %d bits",this->mask_,nbits);
    bool current_state;
    state_->current_values_as_binary(&current_state);
    if ((decoded_code&1) != current_state)
      state_->make_call().set_state(decoded_code&1).perform();
    return true;
  }
  uint64_t mask_{0b111111111111111111110000};
  uint8_t group_;
  uint8_t device_;
  light::LightState *state_;
private:

};



class RfmLightOutput : public light::LightOutput {
 public:

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }
  virtual void setup_state(light::LightState *state) 
  {
    sensor.setup_state(state);
  }
  void set_group(const char* code)
  {
    action.set_group(code);
    sensor.set_group(remote_base::decode_binary_string(code));
  }
  void set_device(const char*code)
  {
    action.set_device(code);
    sensor.set_device(remote_base::decode_binary_string(code));
  }
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
    action.set_parent(transmitter);
  }

  void set_receiver(remote_receiver::RemoteReceiverComponent *receiver) {
     
     receiver->register_listener(&sensor);

  }
  void write_state(light::LightState *state) override {
    bool binary;
    state->current_values_as_binary(&binary);
    action.set_send_times(10);
    action.set_send_wait(0);
    action.set_protocol(remote_base::RC_SWITCH_PROTOCOLS[1]);
    action.set_state(binary);
    action.play(binary);
  }

 protected:
 remote_base::RCSwitchTypeAAction<bool> action;
 BinaryLightRemoteSensor sensor;
};

}  // namespace binary
}  // namespace esphome

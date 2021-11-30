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
  void set_type_a(uint64_t u_group,uint64_t  u_device)
  {
    remote_base::RCSwitchBase::type_a_code(u_group, u_device, true, &code_, &nbits_);
  }
  void set_on_off(uint64_t  on,uint64_t  off)
  {
    code_=on;
    on_mask_=on^off;
    mask_=~on_mask_;
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
    

    uint64_t decoded_code;
    uint8_t decoded_nbits;


    if (!remote_base::RC_SWITCH_PROTOCOLS[1].decode(src, &decoded_code, &decoded_nbits))
      return false;
    //ESP_LOGD(TAG,"decoded %llx %d bits",decoded_code,decoded_nbits);

    if(!(decoded_nbits == nbits_ && (decoded_code & this->mask_) == (code_ & this->mask_)))
      return false;

    bool current_state;

    
    bool new_state=(decoded_code&on_mask_)==(code_&on_mask_);

    ESP_LOGD(TAG,"matches!! %llx->%d",decoded_code,new_state);
    //ESP_LOGD(TAG,"(%llx==%llx) %d bits on_mask=%llx",code_&on_mask_,decoded_code&on_mask_,nbits_,on_mask_);
    state_->current_values_as_binary(&current_state);
    if (new_state != current_state)
    {
        state_->remote_values.set_state(new_state);
        state_->current_values.set_state(new_state);
        state_->publish_state();
    }
      
      //state_->make_call().set_state(new_state).perform();
    return true;
  }
  uint64_t mask_{0xfffff0};
  uint64_t on_mask_{0x1};
  uint8_t nbits_{24};
  uint64_t code_;
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
  void set_type_a(const char* group,const char* device)
  {
    auto aaction=new remote_base::RCSwitchTypeAAction<bool>();
    aaction->set_group(group);
    aaction->set_device(device);
    aaction->set_protocol(remote_base::RC_SWITCH_PROTOCOLS[1]);
    action=aaction;
    sensor.set_type_a(remote_base::decode_binary_string(group),remote_base::decode_binary_string(device));
  }
  void set_on_off(uint64_t  on,uint64_t  off)
  {
    auto aaction=new remote_base::RCSwitchTypeRawAction<bool>();
    aaction->set_on(on);
    aaction->set_off(off);
    aaction->set_nbits(24);
    aaction->set_protocol(remote_base::RC_SWITCH_PROTOCOLS[1]);
    action=aaction;
    sensor.set_on_off(on,off);
  }
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
    action->set_parent(transmitter);
  }

  void set_receiver(remote_receiver::RemoteReceiverComponent *receiver) {
     
     receiver->register_listener(&sensor);

  }
  void write_state(light::LightState *state) override {
    bool binary;
    state->current_values_as_binary(&binary);
    action->set_send_times(10);
    action->set_send_wait(0);
    action->set_state(binary);
    action->play(binary);
  }

 protected:
 remote_base::RemoteTransmitterActionState<bool>* action;
 BinaryLightRemoteSensor sensor;
};

}  // namespace binary
}  // namespace esphome

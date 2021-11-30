#pragma once

#include <utility>

#include "esphome/components/remote_receiver/remote_receiver.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/remote_base/rc_switch_protocol.h"



namespace esphome {
namespace lacrosse {
static const char *const TAG = "lacrosse";

class LacrosseSensor : public Component,public remote_base::RemoteReceiverListener {
 public:
  void set_index(int index){this->index_=index;}
  void set_temperature_sensor(sensor::Sensor* temperature){
    this->temperature_=temperature;
  }
  void set_humidity_sensor(sensor::Sensor* humidity){
    this->humidity_=humidity;
  }
 protected:

  // Dummy implement on_receive so implementation is optional for inheritors
  bool on_receive(remote_base::RemoteReceiveData data) override { 
    if (data.size()!=88){
        ESP_LOGVV(TAG, "size fail %d",data.size());
        return false;
    }
    uint64_t decoded_code=0;
    for (size_t i = 0; i < 44; i++)
    {
      char bit=0;
      if (data.peek_mark(550))
        bit=1;
      else if(data.peek_mark(1400))
        bit=0;
      else return false;

      data.advance();
      if (i!=43 && !data.expect_space(1000))
        return false;
      decoded_code<<=1;
      decoded_code|=bit;
    }
    int type=decoded_code>>32;
    int v1=(decoded_code>>12) & 0xf; 
    int v2=(decoded_code>>16) & 0xf; 
    int v3=(decoded_code>>20) & 0xf; 
    int id=(decoded_code>>25) & 0xff; 
    
    //ESP_LOGD(TAG, "received from %d",id);
    if (id!=index_)
      return false;
    float value=((float)v1)/10+v2+v3*10;
    if (type==0xa0 && temperature_)
    {
      ESP_LOGV(TAG, "temp decode=%llx %d %f",decoded_code,id, value-50);
      temperature_->publish_state(value-50);
    }
    else if (type==0xae && humidity_)
    {
      ESP_LOGV(TAG, "hum decode=%llx %d %f",decoded_code,id, value);
      humidity_->publish_state(value);
    }
    return true; 
  };

  sensor::Sensor* temperature_=nullptr;
  sensor::Sensor* humidity_=nullptr;
  char index_;
};

}  // namespace lacrosse_tx
}  // namespace esphome

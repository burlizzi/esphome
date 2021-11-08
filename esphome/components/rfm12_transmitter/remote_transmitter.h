#pragma once

#include "esphome/core/component.h"
#include "esphome/components/remote_base/remote_base.h"
//#include "esphome/components/spi/spi.h"

namespace esphome {
namespace remote_transmitter {

class RemoteTransmitterComponent : public Component
                                  ,public remote_base::RemoteTransmitterBase
//                                   ,public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING,spi::DATA_RATE_4MHZ>
{
 public:
  explicit RemoteTransmitterComponent(esphome::InternalGPIOPin* pin) : remote_base::RemoteTransmitterBase(pin) {}

  void setup() ;

  void dump_config() ;

  float get_setup_priority() const  { return setup_priority::DATA; }


 protected:

  void send_internal(uint32_t send_times, uint32_t send_wait) ;

  void calculate_on_off_time_(uint32_t carrier_frequency, uint32_t *on_time_period, uint32_t *off_time_period);

  void mark_12( uint32_t usec);

  void space_12(uint32_t usec);
  //remote_base::RemoteTransmitData temp_;

};

}  // namespace remote_transmitter
}  // namespace esphome

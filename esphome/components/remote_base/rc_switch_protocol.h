#pragma once
#include <nvs_flash.h>
#include "esphome/core/component.h"
#include "remote_base.h"
#include <sstream>
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"
extern uint32_t codes_sav[];

namespace esphome {
namespace remote_base {

struct RCSwitchData {
  uint64_t code;
  uint8_t protocol;

  bool operator==(const RCSwitchData &rhs) const { return code == rhs.code && protocol == rhs.protocol; }
};

class RCSwitchBase {
 public:
  RCSwitchBase() = default;
  RCSwitchBase(uint32_t sync_high, uint32_t sync_low, uint32_t zero_high, uint32_t zero_low, uint32_t one_high,
               uint32_t one_low, bool inverted);

  void one(RemoteTransmitData *dst) const;

  void zero(RemoteTransmitData *dst) const;

  void sync(RemoteTransmitData *dst) const;

  void transmit(RemoteTransmitData *dst, uint64_t code, uint8_t len) const;

  bool expect_one(RemoteReceiveData &src) const;

  bool expect_zero(RemoteReceiveData &src) const;

  bool expect_sync(RemoteReceiveData &src) const;

  bool decode(RemoteReceiveData &src, uint64_t *out_data, uint8_t *out_nbits) const;

  optional<RCSwitchData> decode(RemoteReceiveData &src) const;

  static void simple_code_to_tristate(uint16_t code, uint8_t nbits, uint64_t *out_code);

  static void type_a_code(uint8_t switch_group, uint8_t switch_device, bool state, uint64_t *out_code,
                          uint8_t *out_nbits);

  static void type_b_code(uint8_t address_code, uint8_t channel_code, bool state, uint64_t *out_code,
                          uint8_t *out_nbits);

  static void type_c_code(uint8_t family, uint8_t group, uint8_t device, bool state, uint64_t *out_code,
                          uint8_t *out_nbits);

  static void type_d_code(uint8_t group, uint8_t device, bool state, uint64_t *out_code, uint8_t *out_nbits);

  static void type_secplus(uint32_t fixed, uint32_t rolling, uint8_t *out_code);

  static void type_Keeloq(uint32_t fixed, uint32_t rolling, uint8_t *out_code);


 protected:
  uint32_t sync_high_{};
  uint32_t sync_low_{};
  uint32_t zero_high_{};
  uint32_t zero_low_{};
  uint32_t one_high_{};
  uint32_t one_low_{};
  bool inverted_{};
};

extern const RCSwitchBase RC_SWITCH_PROTOCOLS[9];

uint64_t decode_binary_string(const std::string &data);

uint64_t decode_binary_string_mask(const std::string &data);

template<typename... Ts> class RCSwitchRawAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(std::string, code);

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto code = this->code_.value(x...);
    uint64_t the_code = decode_binary_string(code);
    uint8_t nbits = code.size();

    auto proto = this->protocol_.value(x...);
    proto.transmit(dst, the_code, nbits);
  }
};


template<typename... Ts> class RCSwitchSecplusAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(__uint32_t, fixed);
  TEMPLATABLE_VALUE(__uint32_t, rolling);
  void encode(RemoteTransmitData *dst, Ts... x) override {
 
    auto fixed = this->fixed_.value(x...);
    static uint32_t startrolling = this->rolling_.value(x...);
    static auto rtc = global_preferences->make_preference<uint32_t>(fixed,true);
    rtc.load(&startrolling);
    auto rolling=startrolling;
    uint32_t roll=0;
    for (size_t i = 0; i < 31; i++)
    {
        roll|=rolling&(1UL<<31);
        roll>>=1;
        rolling<<=1;
    }
    startrolling+=2;
    rtc.save(&startrolling);
    
    uint8_t code[40];
    RCSwitchBase::type_secplus(fixed, roll, code);
    std::stringstream s;
    for (int32_t item : code) 
      s<<item<<',';
    ESP_LOGD("secplus", "code:%d,%s",startrolling,s.str().c_str());
    dst->space(static_cast<uint32_t>(1500));
    dst->mark(static_cast<uint32_t>(500));
    for (size_t i = 0; i < 20; i++)
    {
      dst->space(static_cast<uint32_t>((4-code[i])*500));
      dst->mark(static_cast<uint32_t>(code[i]*500));
    }
    dst->space(static_cast<uint32_t>(30*2000));
    dst->mark(static_cast<uint32_t>(1500));
    for (size_t i = 20; i < 40; i++)
    {
      dst->space(static_cast<uint32_t>((4-code[i])*500));
      dst->mark(static_cast<uint32_t>(code[i]*500));
    }
    dst->space(static_cast<uint32_t>(30*2000));
    
  }
};



#define KeeLoq_NLF              0x3A5C742EUL
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

class Keeloq
{
  public:
    Keeloq(
		const unsigned long keyHigh,
		const unsigned long keyLow ):
_keyHigh( keyHigh ),
_keyLow( keyLow )
{
}
		
    unsigned long encrypt( const unsigned long data ){
  unsigned long x = data;
  unsigned long r;
  int keyBitNo, index;
  unsigned long keyBitVal,bitVal;

  for ( r = 0; r < 528; r++ )
  {
    keyBitNo = r & 63;
    if(keyBitNo < 32)
      keyBitVal = bitRead(_keyLow,keyBitNo);
    else
      keyBitVal = bitRead(_keyHigh, keyBitNo - 32);
    index = 1 * bitRead(x,1) + 2 * bitRead(x,9) + 4 * bitRead(x,20) + 8 * bitRead(x,26) + 16 * bitRead(x,31);
    bitVal = bitRead(x,0) ^ bitRead(x, 16) ^ bitRead(KeeLoq_NLF,index) ^ keyBitVal;
    x = (x>>1) ^ bitVal<<31;
  }
  return x;
}
    unsigned long decrypt( const unsigned long data ){
  unsigned long x = data;
  unsigned long r;
  int keyBitNo, index;
  unsigned long keyBitVal,bitVal;

  for (r = 0; r < 528; r++)
  {
    keyBitNo = (15-r) & 63;
    if(keyBitNo < 32)
      keyBitVal = bitRead(_keyLow,keyBitNo);
    else
      keyBitVal = bitRead(_keyHigh, keyBitNo - 32);
    index = 1 * bitRead(x,0) + 2 * bitRead(x,8) + 4 * bitRead(x,19) + 8 * bitRead(x,25) + 16 * bitRead(x,30);
    bitVal = bitRead(x,31) ^ bitRead(x, 15) ^ bitRead(KeeLoq_NLF,index) ^ keyBitVal;
    x = (x<<1) ^ bitVal;
  }
  return x;
 }
    
  private:
    unsigned long _keyHigh;
	unsigned long _keyLow;
};


template<typename... Ts> class RCSwitchKeeloqAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(__uint32_t, keyLow);
  TEMPLATABLE_VALUE(__uint32_t, keyHigh);
  TEMPLATABLE_VALUE(__uint16_t, serial);
  void encode(RemoteTransmitData *dst, Ts... x) override {
 
    auto keyLow = this->keyLow_.value(x...);
    auto keyHigh = this->keyHigh_.value(x...);
/*    auto id = this->serial_.value(x...);
    
    Keeloq k1(keyLow, keyHigh);
    auto device_key_msb = k1.decrypt(id | 0x60000000L);
    auto device_key_lsb = k1.decrypt(id | 0x20000000L);
    
    Keeloq k(device_key_msb, device_key_lsb);

    const int disc                 = 0x0100; // 0x0100 for single channel remote
*/
    static uint16_t serial = this->serial_.value(x...);

    static auto rtc = global_preferences->make_preference<uint32_t>(keyLow,true);
    rtc.load(&serial);
    auto rolling=serial;
    serial++;
    rtc.save(&serial);
    
//    unsigned int result = (disc << 16) | rolling;

//    uint64_t enc = k.encrypt(result);
    uint64_t pack = codes_sav[rolling];
    pack <<= 32;
    pack |= keyLow;
    
    ESP_LOGD("keeloq", "code:%llx",pack);

    

    
//    std::stringstream s;

    for (size_t i = 0; i < 12; i++)
    {
      dst->space(432);
      dst->mark(432);
    }
    dst->space(4000-432);

    for(int i=64; i>0; i--)
    {
      if(pack & 0x8000000000000000l)
      {
          dst->mark(848);
          dst->space(432);
      }
      else
      {
          dst->mark(432);
          dst->space(848);
      }
      pack <<= 1;
    }      
    dst->mark(432);
    dst->space(848);
    dst->mark(432);
    dst->space(848);
    dst->space(17000);
  }
};


template<typename... Ts> class RCSwitchTypeRawAction : public RemoteTransmitterActionState<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(uint32_t, on);
  TEMPLATABLE_VALUE(uint32_t, off);
  TEMPLATABLE_VALUE(uint8_t, nbits);
  

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto proto = this->protocol_.value(x...);
    auto state = this->state_.value(x...);
    auto on = this->on_.value(x...);
    auto off = this->off_.value(x...);
    auto nbits = this->nbits_.value(x...);
    proto.transmit(dst, state?on:off, nbits);
  }
};

template<typename... Ts> class RCSwitchTypeAAction : public RemoteTransmitterActionState<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(std::string, group);
  TEMPLATABLE_VALUE(std::string, device);
  

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto group = this->group_.value(x...);
    auto device = this->device_.value(x...);
    auto state = this->state_.value(x...);
    uint8_t u_group = decode_binary_string(group);
    uint8_t u_device = decode_binary_string(device);

    uint64_t code;
    uint8_t nbits;
    RCSwitchBase::type_a_code(u_group, u_device, state, &code, &nbits);

    auto proto = this->protocol_.value(x...);
    proto.transmit(dst, code, nbits);
  }
};

template<typename... Ts> class RCSwitchTypeBAction : public RemoteTransmitterActionState<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(uint8_t, address);
  TEMPLATABLE_VALUE(uint8_t, channel);
  

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto address = this->address_.value(x...);
    auto channel = this->channel_.value(x...);
    auto state = this->state_.value(x...);

    uint64_t code;
    uint8_t nbits;
    RCSwitchBase::type_b_code(address, channel, state, &code, &nbits);

    auto proto = this->protocol_.value(x...);
    proto.transmit(dst, code, nbits);
  }
};

template<typename... Ts> class RCSwitchTypeCAction : public RemoteTransmitterActionState<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(std::string, family);
  TEMPLATABLE_VALUE(uint8_t, group);
  TEMPLATABLE_VALUE(uint8_t, device);
  

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto family = this->family_.value(x...);
    auto group = this->group_.value(x...);
    auto device = this->device_.value(x...);
    auto state = this->state_.value(x...);

    auto u_family = static_cast<uint8_t>(tolower(family[0]) - 'a');

    uint64_t code;
    uint8_t nbits;
    RCSwitchBase::type_c_code(u_family, group, device, state, &code, &nbits);

    auto proto = this->protocol_.value(x...);
    proto.transmit(dst, code, nbits);
  }
};
template<typename... Ts> class RCSwitchTypeDAction : public RemoteTransmitterActionState<Ts...> {
 public:
  TEMPLATABLE_VALUE(RCSwitchBase, protocol);
  TEMPLATABLE_VALUE(std::string, group);
  TEMPLATABLE_VALUE(uint8_t, device);
  

  void encode(RemoteTransmitData *dst, Ts... x) override {
    auto group = this->group_.value(x...);
    auto device = this->device_.value(x...);
    auto state = this->state_.value(x...);

    auto u_group = static_cast<uint8_t>(tolower(group[0]) - 'a');

    uint64_t code;
    uint8_t nbits;
    RCSwitchBase::type_d_code(u_group, device, state, &code, &nbits);

    auto proto = this->protocol_.value(x...);
    proto.transmit(dst, code, nbits);
  }
};

class RCSwitchRawReceiver : public RemoteReceiverBinarySensorBase {
 public:
  void set_protocol(const RCSwitchBase &a_protocol) { this->protocol_ = a_protocol; }
  void set_code(uint64_t code) { this->code_ = code; }
  void set_code(const std::string &code) {
    this->code_ = decode_binary_string(code);
    this->mask_ = decode_binary_string_mask(code);
    this->nbits_ = code.size();
  }
  void set_nbits(uint8_t nbits) { this->nbits_ = nbits; }
  void set_type_a(const std::string &group, const std::string &device, bool state) {
    uint8_t u_group = decode_binary_string(group);
    uint8_t u_device = decode_binary_string(device);
    RCSwitchBase::type_a_code(u_group, u_device, state, &this->code_, &this->nbits_);
  }
  void set_type_b(uint8_t address_code, uint8_t channel_code, bool state) {
    RCSwitchBase::type_b_code(address_code, channel_code, state, &this->code_, &this->nbits_);
  }
  void set_type_c(std::string family, uint8_t group, uint8_t device, bool state) {
    auto u_family = static_cast<uint8_t>(tolower(family[0]) - 'a');
    RCSwitchBase::type_c_code(u_family, group, device, state, &this->code_, &this->nbits_);
  }
  void set_type_d(std::string group, uint8_t device, bool state) {
    auto u_group = static_cast<uint8_t>(tolower(group[0]) - 'a');
    RCSwitchBase::type_d_code(u_group, device, state, &this->code_, &this->nbits_);
  }

 protected:
  bool matches(RemoteReceiveData src) override;

  RCSwitchBase protocol_;
  uint64_t code_;
  uint64_t mask_{0xFFFFFFFFFFFFFFFF};
  uint8_t nbits_;
};

class RCSwitchDumper : public RemoteReceiverDumperBase {
 public:
  bool dump(RemoteReceiveData src) override;
};

using RCSwitchTrigger = RemoteReceiverTrigger<RCSwitchBase, RCSwitchData>;

}  // namespace remote_base
}  // namespace esphome

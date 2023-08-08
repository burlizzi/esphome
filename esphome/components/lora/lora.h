#pragma once
#include "soc/rtc_wdt.h"
#include "esphome/core/controller.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/automation.h"

#include <lorawan.h>

#include <RadioLib.h>

void xxx(const char *format, ...);

namespace esphome {
namespace lora {
class Lora : public Component, public LoRaWANClass, public Controller {
 public:
  bool joined;

  Lora();
  void setup() override;
  void ook(int data, unsigned char len = 24);
  void start();
  void setRX2DR(dataRates_t dr);
  void reset() {
    *(uint32_t *) Session_Data.DevAddr = 0;
    rtc.save(&Session_Data);
  }

  void send(std::string message = "ciao", unsigned char port = 1, bool confirm = false);
  void loop() override;

  Trigger<int, int, std::string> *data_trigger() {
    if (!this->data_trigger_)
      this->data_trigger_ = make_unique<Trigger<int, int, std::string>>();
    return this->data_trigger_.get();
  }

 protected:
  std::unique_ptr<Trigger<int, int, std::string>> data_trigger_{nullptr};
  static const uint32_t RESTORE_STATE_VERSION = 0x2a5a2c13UL;
  ESPPreferenceObject rtc;

#ifdef USE_BINARY_SENSOR
  void on_binary_sensor_update(binary_sensor::BinarySensor *obj, bool state) override {
    send(obj->get_object_id_hash(), state);
  }
#endif
#ifdef USE_COVER
  void on_cover_update(cover::Cover *obj) override;
#endif
#ifdef USE_FAN
  void on_fan_update(fan::FanState *obj) override;
#endif
#ifdef USE_LIGHT
  void on_light_update(light::LightState *obj) override;
#endif
#ifdef USE_SENSOR
  void on_sensor_update(sensor::Sensor *obj, float state) { send(obj->get_object_id_hash(), state); }
#endif
#ifdef USE_SWITCH
  void on_switch_update(switch_::Switch *obj, bool state) override;
#endif
#ifdef USE_TEXT_SENSOR
  void on_text_sensor_update(text_sensor::TextSensor *obj, const std::string &state) override {
    send(obj->get_object_id_hash(), state);
  }
#endif
#ifdef USE_CLIMATE
  void on_climate_update(climate::Climate *obj) override;
#endif
#ifdef USE_NUMBER
  void on_number_update(number::Number *obj, float state) override { /*send(obj->get_object_id_hash(),state );*/
  }
#endif
#ifdef USE_SELECT
  void on_select_update(select::Select *obj, const std::string &state) override {
    send(obj->get_object_id_hash(), state);
  }
#endif
  template<typename T> inline void send(uint32_t hash, const T &data) {
    send(hash, std::string((char *) &data, sizeof(data)));
  }

  inline void send(uint32_t hash, const std::string &data) { send(std::string((char *) &hash, sizeof(hash)) + data); }
};
}  // namespace lora
}  // namespace esphome

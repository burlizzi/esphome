#include "lora.h"

void xxx(const char *format, ...) {  // NOLINT
  va_list arg;
  va_start(arg, format);

  esphome::esp_log_vprintf_(ESPHOME_LOG_LEVEL_ERROR, "lora", 22, format, arg);
  va_end(arg);
}

extern const sRFM_pins RFM_pins;
SX1276 radio = new Module(RFM_pins.CS, RFM_pins.DIO0, RFM_pins.RST, RFM_pins.DIO1);

namespace esphome {
namespace lora {
class Lora : public Component, public LoRaWANClass, public Controller {
 public:
  bool joined;

  Lora() : joined(false) {
    if (!init()) {
      ESP_LOGE("lora", "RFM95 not detected");
    }
  }
  void setup() override {
    setup_controller();
    uint32_t *dev = (uint32_t *) DevEUI;
    rtc = global_preferences->make_preference<sLoRa_Session>(RESTORE_STATE_VERSION ^ dev[0] ^ dev[1], true);
    // start();
  }

  void ook(int data, unsigned char len = 24) {
    int state = radio.beginFSK(433.97);
    if (state == RADIOLIB_ERR_NONE) {
      xxx("beginFSK success!");
    } else {
      xxx("Error=%d", state);
    }

    state = radio.setOutputPower(20.0);
    if (state == RADIOLIB_ERR_NONE) {
      xxx("setOutputPower success!");
    } else {
      xxx("Error=%d", state);
    }
    state = radio.setOOK(true);

    if (state == RADIOLIB_ERR_NONE) {
      xxx("setOOK success!");
    } else {
      xxx("Error=%d", state);
    }

    state = radio.transmitDirect();
    if (state == RADIOLIB_ERR_NONE) {
      xxx("transmitDirect success!");
    } else {
      xxx("Error=%d", state);
    }

    pin_mode(33, OUTPUT);

    for (size_t i = 0; i < 6; i++) {
      delayMicroseconds(12000);
      digital_write(33, 1);
      delayMicroseconds(550);
      digital_write(33, 0);
      delayMicroseconds(1200);
      // std::ostringstream ss;
      for (size_t bit = 0; bit < len; bit++) {
        int x = data >> bit & 1;
        // ss<<x;
        //    xxx("%d",x);
        digital_write(33, 1);
        delayMicroseconds(x ? 1200 : 550);
        digital_write(33, 0);
        delayMicroseconds(x ? 550 : 1200);
      }
      // xxx(ss.str().c_str());
    }
    pin_mode(33, INPUT);

    state = radio.standby();
    if (state == RADIOLIB_ERR_NONE) {
      xxx("success!");
    } else {
      xxx("Error=%d", state);
    }
    RFM_Init();
    setDeviceClass(CLASS_C);

    // Set Data Rate
    setDataRate(drate_common);

    // set channel to random
    setChannel(MULTI);
  }

  void start() {
    //                setTxPower(20,PA_BOOST_PIN);
    auto loaded = rtc.load(&Session_Data);
    ESP_LOGI("lora", "store... load=%d devaddr=%X", loaded, *(uint32_t *) Session_Data.DevAddr);
    if (!loaded || ((*(uint32_t *) Session_Data.DevAddr) == 0)) {
      ESP_LOGI("lora", "Joining...");
      for (int rate = SF9BW125; (rate >= SF12BW125) && !joined; rate--) {
        setDataRate(rate);
        ESP_LOGI("lora", "SF=%d", 12 - rate);
        for (size_t i = CH0; i < CH3 && !joined; i++) {
          setChannel(i);
          ESP_LOGI("lora", "CH=%d", i);
          disableLoopWDT();
          joined = join();
          enableLoopWDT();

          if (!joined) {
            ESP_LOGI("lora", "failed, wait 10 sec");
            for (size_t z = 0; z < 1000; z++) {
              App.feed_wdt();
              delay(10);
              yield();
            }
          }
        }
      }
      if (!joined) {
        ESP_LOGE("lora", "cannot join");
        // mark_failed();
        return;
      }
      rtc.save(&Session_Data);

    } else {
      ESP_LOGI("lora", "using saved session");
      joined = true;
    }
    // setChannel(0);
    setDataRate(SF9BW125);
  }

  void setRX2DR(dataRates_t dr) {
    ESP_LOGI("lora", "OBSOLETE AND DEPRECATED SX2 data rate=%d", 12 - dr);
    // LoRa_Settings.RX2_Datarate_Rx=dr;
  }

  void reset() {
    *(uint32_t *) Session_Data.DevAddr = 0;
    rtc.save(&Session_Data);
  }

  void send(std::string message = "ciao", unsigned char port = 1, bool confirm = false) {
    if (joined) {
      sendUplink((char *) message.c_str(), message.length(), confirm, port);
      ESP_LOGI("lora", "sent on channel %d", LoRa_Settings.Channel_Tx);
      rtc.save(&Session_Data);
    } else {
      ESP_LOGE("lora", "not joined");
    }
  }

  void loop() override {
    char outStr[255];
    update();
    auto recvStatus = readData(outStr);
    if (recvStatus) {
      ESP_LOGI("lora", "====>>(%d) rssi(%d) %s", Message_Rx.Frame_Port, getRssi(), outStr);  // NOLINT
      if (data_trigger_)
        data_trigger_->trigger(Message_Rx.Frame_Port, getRssi(), std::string(outStr, recvStatus));
    }
  }

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
  void on_cover_update(cover::Cover *obj) override { ESP_LOGE("lora", "not implemented"); }  // NOLINT
#endif
#ifdef USE_FAN
  void on_fan_update(fan::FanState *obj) override { ESP_LOGE("lora", "not implemented"); }  // NOLINT
#endif
#ifdef USE_LIGHT
  void on_light_update(light::LightState *obj) override { ESP_LOGE("lora", "not implemented"); }  // NOLINT
#endif
#ifdef USE_SENSOR
  void on_sensor_update(sensor::Sensor *obj, float state) { send(obj->get_object_id_hash(), state); }
#endif
#ifdef USE_SWITCH
  void on_switch_update(switch_::Switch *obj, bool state) override { send(obj->get_object_id_hash(), state); }
#endif
#ifdef USE_TEXT_SENSOR
  void on_text_sensor_update(text_sensor::TextSensor *obj, const std::string &state) override {
    send(obj->get_object_id_hash(), state);
  }
#endif
#ifdef USE_CLIMATE
  void on_climate_update(climate::Climate *obj) override { ESP_LOGE("lora", "not implemented"); }  // NOLINT
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

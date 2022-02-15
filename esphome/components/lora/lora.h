
#include "esphome.h"
#include "esphome/core/controller.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/automation.h"

#include <lorawan.h>

namespace esphome{
    namespace lora{
        class Lora : public Component, public LoRaWANClass, public Controller
        {
            static const uint32_t RESTORE_STATE_VERSION = 0x2a5a2c10UL;

            public:
            void setup() override
            {
                uint32_t* dev=(uint32_t*)DevEUI;
                auto rtc = global_preferences->make_preference<uint32_t>(RESTORE_STATE_VERSION^dev[0]^dev[1]);
                if(!rtc.load(&Session_Data))
                {
                    bool isJoined=false;;
                    ESP_LOGI("lora","Joining...");
                    for (size_t rate = SF7BW125; rate >=SF12BW125 && !isJoined; rate--)
                    {
                        setDataRate(rate);
                        ESP_LOGI("lora","SF=%d",12-rate);
                        for (size_t i = CH0; i < CH3 && !isJoined; i++)
                        {
                        setChannel(i);
                        isJoined = join();
                        if (!isJoined)
                        {
                            delay(10000);

                        }
                        }
                    }
                    if (!isJoined)
                    {
                        ESP_LOGE("lora","cannot join");
                        return;
                    }
                    rtc.save(&Session_Data);
                }
                else
                {
                    ESP_LOGI("lora","using saved session");
                }
                setup_controller();
            }
            void loop() override
            {
                char outStr[255];
                update();
                auto recvStatus = readData(outStr);
                if(recvStatus) {
                    ESP_LOGI("lora","====>>(%d) %s",Message_Rx.Frame_Port, outStr);
                    if(data_trigger_)
                        data_trigger_->trigger(Message_Rx.Frame_Port,std::string(outStr,recvStatus));
                }
            }

            Trigger<int,std::string> *data_trigger() {
                if (!this->data_trigger_)
                this->data_trigger_ = make_unique<Trigger<int,std::string>>();
                return this->data_trigger_.get();
            }
            protected:
            std::unique_ptr<Trigger<int,std::string>> data_trigger_{nullptr};

        };

        #ifdef USE_BINARY_SENSOR
  void on_binary_sensor_update(binary_sensor::BinarySensor *obj, bool state) override;
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
  void on_sensor_update(sensor::Sensor *obj, float state) override;
#endif
#ifdef USE_SWITCH
  void on_switch_update(switch_::Switch *obj, bool state) override;
#endif
#ifdef USE_TEXT_SENSOR
  void on_text_sensor_update(text_sensor::TextSensor *obj, const std::string &state) override;
#endif
#ifdef USE_CLIMATE
  void on_climate_update(climate::Climate *obj) override;
#endif
#ifdef USE_NUMBER
  void on_number_update(number::Number *obj, float state) override;
#endif
#ifdef USE_SELECT
  void on_select_update(select::Select *obj, const std::string &state) override;
#endif

    }
}

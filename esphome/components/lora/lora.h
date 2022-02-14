
#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/automation.h"

#include <lorawan.h>

namespace esphome{
    namespace lora{
        class Lora : public Component, public LoRaWANClass
        {
            static const uint32_t RESTORE_STATE_VERSION = 0x2a5a2c10UL;

            public:
            void setup() override
            {
                uint32_t* dev=(uint32_t*)DevEUI;
                auto rtc = global_preferences->make_preference<uint32_t>(RESTORE_STATE_VERSION^dev[0]^dev[1]);
                if(!rtc.load(&Session_Data))
                {
                    join();
                    rtc.save(&Session_Data);
                }
                else
                {
                    ESP_LOGI("lora","using saved session");
                }
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
    }
}

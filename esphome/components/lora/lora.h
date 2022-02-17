
#include "esphome.h"
#include "esphome/core/controller.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/automation.h"

#include <lorawan.h>



void xxx(const char *format, ...) {  // NOLINT
  va_list arg;
  va_start(arg, format);
  
  esphome::esp_log_vprintf_(ESPHOME_LOG_LEVEL_ERROR, "lora", 22, format, arg);
  va_end(arg);
}



namespace esphome{
    namespace lora{
        class Lora : public Component, public LoRaWANClass, public Controller
        {
            public:
            bool joined;

            Lora():joined(false)
            {
                if(!init()){
                  ESP_LOGE("lora","RFM95 not detected");
                }
            }
            void setup() override
            {
                setup_controller();
                uint32_t* dev=(uint32_t*)DevEUI;
                rtc = global_preferences->make_preference<sLoRa_Session>(RESTORE_STATE_VERSION^dev[0]^dev[1],true);
            }



            void start()
            {

                setTxPower(20,PA_BOOST_PIN);
                auto loaded=rtc.load(&Session_Data);
                ESP_LOGI("lora","store... load=%d devaddr=%d",loaded,*(uint32_t*)Session_Data.DevAddr);
                if(!loaded || ((*(uint32_t*)Session_Data.DevAddr)==0))
                {
                    ESP_LOGI("lora","Joining...");
                    for (size_t rate = SF9BW125; (rate >=SF12BW125) && !joined; rate--)
                    {
                        setDataRate(rate);
                        ESP_LOGI("lora","SF=%d",12-rate);
                        for (size_t i = CH0; i < CH3 && !joined; i++)
                        {

                          setChannel(i);
                          ESP_LOGI("lora","CH=%d",i);
                          disableLoopWDT();
                          joined = join();
                          enableLoopWDT();
                          
                          if (!joined)
                          {
                            ESP_LOGI("lora","failed, wait 10 sec");
                            for (size_t z = 0; z < 10; z++)
                            {
                              App.feed_wdt();
                              delay(1000);
                              yield();
                            }
                          }
                        }
                    }
                    if (!joined)
                    {
                        ESP_LOGE("lora","cannot join");
                        //mark_failed();
                        return;
                    }
                    rtc.save(&Session_Data);
                    
                }
                else
                {
                    ESP_LOGI("lora","using saved session");
                    joined = true;

                }
                setChannel(MULTI);              
                setDataRate(SF9BW125);
            }

            void setRX2DR(dataRates_t dr)
            {
                ESP_LOGI("lora","OBSOLETE AND DEPRECATED SX2 data rate=%d",12-dr);
                //LoRa_Settings.RX2_Datarate_Rx=dr;
            }

            void reset()
            {
              *(uint32_t*)Session_Data.DevAddr=0;
              rtc.save(&Session_Data);
            }
            void send(String message="ciao")
            {
              if (joined)
              {
                sendUplink((char*)message.c_str(), message.length(), 0, 1);
                ESP_LOGI("lora","sent on channel %d",LoRa_Settings.Channel_Tx);
                rtc.save(&Session_Data);
              }
              else
              {
                ESP_LOGE("lora","not joined");
              }
            }


            void loop() override
            {
                char outStr[255];
                update();
                auto recvStatus = readData(outStr);
                if(recvStatus) {
                    ESP_LOGI("lora","====>>(%d) rssi(%d) %s",Message_Rx.Frame_Port, getRssi(), outStr);
                    if(data_trigger_)
                        data_trigger_->trigger(Message_Rx.Frame_Port,getRssi(),std::string(outStr,recvStatus));
                }
            }

            Trigger<int,int,std::string> *data_trigger() {
                if (!this->data_trigger_)
                this->data_trigger_ = make_unique<Trigger<int,int,std::string>>();
                return this->data_trigger_.get();
            }
            protected:
            std::unique_ptr<Trigger<int,int,std::string>> data_trigger_{nullptr};
            static const uint32_t RESTORE_STATE_VERSION = 0x2a5a2c13UL;
            ESPPreferenceObject rtc;



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
  void on_switch_update(switch_::Switch *obj, bool state) override {ESP_LOGE("lora","not implemented");}
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

        };


    }
}

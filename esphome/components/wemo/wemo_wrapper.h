#ifndef WemoWrapper_h
#define WemoWrapper_h

#include "esphome/core/defines.h"
#include <lwip/igmp.h>
#include "esphome/core/entity_base.h"
#include "esphome/components/web_server_base/web_server_base.h"

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <WiFi.h>
#endif
#ifdef USE_ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_state.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif



namespace esphome {
namespace wemo {

enum WemoType{        WEMO_SWITCH,  WEMO_LIGHT,     WEMO_HEATER,    WEMO_BRIDGE,    };
const String uuids[]={"Socket",     "Lightswitch",  "Humidifier",   "Bridge",       };





class WemoWrapper:public AsyncWebHandler
{
private:
    esphome::EntityBase* device_;
    WemoType type_;
public:
#ifdef USE_SWITCH
    WemoWrapper(switch_::Switch* device):device_(device),type_(WEMO_SWITCH){};
#endif
#ifdef USE_LIGHT
    WemoWrapper(light::LightState* device):device_(device),type_(WEMO_LIGHT){};
#endif
#ifdef USE_CLIMATE
    WemoWrapper(climate::Climate* device):device_(device),type_(WEMO_HEATER){};
#endif
    void respondToSearch(IPAddress& senderIP, unsigned int senderPort);
    String persistent_uuid(){return uuids[0] + "-1_0-" + serial() + "-80";}
    String serial();
    String state();
    void on();
    void off();
    const char* name(){return device_->get_name().c_str();}
    void set_port(uint16_t port){server_.set_port(port);}

    void init()
    {
        server_.add_handler(this);
        server_.init();
    }
    bool canHandle(AsyncWebServerRequest *request) override { return true; }
    void handleRequest(AsyncWebServerRequest *request) override;
    void handleBody(AsyncWebServerRequest *req,uint8_t *data, size_t len, size_t index, size_t total) override;
    web_server_base::WebServerBase server_;
};
}
}
#endif
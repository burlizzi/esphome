#pragma once

#include "esphome/core/defines.h"
#include <lwip/igmp.h>
#include "esphome/core/entity_base.h"
#include "esphome/components/socket/socket.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/wifi/wifi_component.h"

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

enum WemoType {
#ifdef USE_SWITCH
  WEMO_SWITCH,
#endif
#ifdef USE_LIGHT
  WEMO_LIGHT,
#endif
#ifdef USE_CLIMATE
  WEMO_HEATER,
#endif
  WEMO_BRIDGE,
};
const String uuids[] = {
#ifdef USE_SWITCH
    "Socket",
#endif
#ifdef USE_LIGHT
    "Lightswitch",
#endif
#ifdef USE_CLIMATE
    "Humidifier",
#endif
    "Bridge",
};

class WemoWrapper : public AsyncWebHandler {
 private:
  esphome::EntityBase *wemodevice;
  WemoType wemotype;
public:
#ifdef USE_SWITCH
    WemoWrapper(switch_::Switch* device):wemodevice(device),wemotype(WEMO_SWITCH){};
#endif
#ifdef USE_LIGHT
    WemoWrapper(light::LightState* device):wemodevice(device),wemotype(WEMO_LIGHT){};
#endif
#ifdef USE_CLIMATE
    WemoWrapper(climate::Climate* device):wemodevice(device),wemotype(WEMO_HEATER){};
#endif
    void respondToSearch(in_addr senderIP, unsigned int senderPort);
    String persistent_uuid(){return uuids[0] + "-1_0-" + serial() + "-80";}
    String serial();
    String state();
    void on();
    void off();
    const char* name(){return wemodevice->get_name().c_str();}
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

}  // namespace wemo
}  // namespace esphome

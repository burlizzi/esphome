#ifndef WemoManager_h
#define WemoManager_h

#include <Arduino.h>
#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <WiFi.h>
#endif
#ifdef USE_ESP8266
#include <ESP8266WiFi.h>
#endif

#include "esphome/core/log.h"
#include "wemo_wrapper.h"

#include <Udp.h>
#include "esphome/core/entity_base.h"
#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_LIGHT
#include "esphome/components/light/light_state.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
#include <memory>
#include <set>
#include <map>
namespace esphome {
namespace wemo {
class WemoManager:public esphome::Component,public AsyncWebHandler {
private:
        std::unique_ptr<UDP> udp_;
        web_server_base::WebServerBase * web_;
        std::vector<WemoWrapper> devices;
public:
        WemoManager(web_server_base::WebServerBase * web):web_(web){}
        void setup() override;
        void loop() override;
        float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
        bool web_callback(AsyncWebServerRequest * req,WemoWrapper device);
        static bool handleEventservice(AsyncWebServerRequest * req);
        bool handleUpnpControl(WemoWrapper dev, AsyncWebServerRequest * req,__uint8_t * data,size_t len);
        void add_device(WemoWrapper dev);
        void add_all_devices();
        bool canHandle(AsyncWebServerRequest *request) override { return true; }
        void handleRequest(AsyncWebServerRequest *request) override;
        void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

        bool isRequestHandlerTrivial() override { return true; }
};
}
}
#endif
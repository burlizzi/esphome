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

#include <Udp.h>
#include "esphome/core/entity_base.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/climate/climate.h"
#include <memory>
#include <set>
#include <map>
namespace esphome {

typedef std::function<void(void)> CallbackFunction;


class WebServer
{
    public:
    void send(int code,const char* mime,const char* data)
    {

    }

};

class WemoSwitch {
private:
        std::unique_ptr<UDP> udp_;
        WebServer *server = NULL;
        String serial;
        String persistent_uuid;
        String device_name;
        unsigned int localPort;
        String deviceStatus;
        CallbackFunction onCallback;
        CallbackFunction offCallback;

        void startWebServer();
        void handleEventservice();
        void handleUpnpControl();
        void handleRoot();
        void handleSetupXml();
public:
        WemoSwitch();
        WemoSwitch(String alexaInvokeName, unsigned int port, CallbackFunction onCallback, CallbackFunction offCallback);
        ~WemoSwitch();
        String getAlexaInvokeName();
        void serverLoop();
        void respondToSearch(IPAddress& senderIP, unsigned int senderPort);
};


class WemoManager:public esphome::Component {
private:
        std::unique_ptr<UDP> udp_;
public:
        void setup() override;
        void loop() override;
        void add_device(switch_::Switch* device){add_device(device,"Socket");}
        void add_device(light::LightState* device){add_device(device,"Lightswitch");}
        void add_device(climate::Climate* device){add_device(device,"Heater");}
        void add_device(esphome::EntityBase* device,std::string type);
        void add_all_devices();
};
}
#endif
#pragma once

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

enum WemoType {
  WEMO_SWITCH,
  WEMO_LIGHT,
  WEMO_HEATER,
  WEMO_BRIDGE,
};
const String uuids[] = {
    "Socket",
    "Lightswitch",
    "Humidifier",
    "Bridge",
};

class WemoWrapper : public AsyncWebHandler {
 private:
  esphome::EntityBase *wemodevice;
  WemoType wemotype;
};

}  // namespace wemo
}  // namespace esphome

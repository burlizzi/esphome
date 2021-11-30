#include "wemo_wrapper.h"
#include <lwip/igmp.h>

#include "esphome/core/log.h"





namespace esphome {
namespace wemo {

static const char *const TAG = "wemo";



String WemoWrapper::state()
{
    switch(type_)
    {
    case WEMO_BRIDGE:
    {
        auto br=static_cast<switch_::Switch*>(device_);
        ESP_LOGD(TAG,"bridge? %p",br);
        return br->state?"1":"0";

    }
#ifdef USE_SWITCH
    case WEMO_SWITCH:
    {
        auto sw=static_cast<switch_::Switch*>(device_);
        ESP_LOGD(TAG,"switch? %p",sw);
        return sw->state?"1":"0";

    }
#endif
#ifdef USE_LIGHT
    case WEMO_LIGHT:
        auto sw=static_cast<Li::light_state*>(device_);
        ESP_LOGD(TAG,"switch? %p",sw);
        return sw->state?"1":"0";
        break;
#endif
#ifdef USE_CLIMATE
    case WEMO_HEATER:
        auto sw=static_cast<switch_::Switch*>(device_);
        ESP_LOGD(TAG,"switch? %p",sw);
        return sw->state?"1":"0";
        break;
#endif
    }
    return "0";
}

void WemoWrapper::on()
{
#ifdef USE_SWITCH
        auto sw=static_cast<switch_::Switch*>(device_);
        sw->turn_on();
#endif
#ifdef USE_LIGHT
#endif
#ifdef USE_CLIMATE
#endif
}

void WemoWrapper::off()
{
#ifdef USE_SWITCH
        auto sw=static_cast<switch_::Switch*>(device_);
        sw->turn_off();
#endif
#ifdef USE_LIGHT
#endif
#ifdef USE_CLIMATE
#endif
}

String WemoWrapper::serial()
{
    uint32_t uniqueSwitchId = device_->get_object_id_hash();
    char uuid[64];
    sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
          (uint16_t) ((uniqueSwitchId >> 16) & 0xff),
          (uint16_t) ((uniqueSwitchId >>  8) & 0xff),
          (uint16_t)   uniqueSwitchId        & 0xff);
    return uuid;
}

void WemoWrapper::respondToSearch(IPAddress& senderIP, unsigned int senderPort)
{
    IPAddress localIP = WiFi.localIP();


    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
    WiFiUDP UDP;
    String response =
        "HTTP/1.1 200 OK\r\n"
        "CACHE-CONTROL: max-age=86400\r\n"
        "DATE: Sat, 26 Nov 2016 04:56:29 GMT\r\n"
        "EXT:\r\n"
        "LOCATION: http://" + String(s) + ":80/" + serial() + ".xml\r\n"
        "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
        "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
        "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
        "ST: urn:Belkin:device:**\r\n"
        "USN: uuid:" + persistent_uuid() + "::urn:Belkin:device:**\r\n"
        "X-User-Agent: redsonic\r\n\r\n";

    ESP_LOGD(TAG,"answering UDP\n");
    UDP.beginPacket(senderIP, senderPort);
    UDP.print(response.c_str());
    UDP.endPacket();

 
}


}
}
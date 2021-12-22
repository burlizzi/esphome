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
        auto light=static_cast<light::LightState*>(device_);
        ESP_LOGD(TAG,"light %p",light);
        bool state;
        light->current_values_as_binary(&state);
        return state?"1":"0";
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

    switch(type_)
    {
#ifdef USE_SWITCH
    case WEMO_SWITCH:
    {
        auto sw=static_cast<switch_::Switch*>(device_);
        ESP_LOGD(TAG,"switch? %p",sw);
        sw->turn_on();
        break;
    }
#endif
#ifdef USE_LIGHT
    case WEMO_LIGHT:
        auto light=static_cast<light::LightState*>(device_);
        ESP_LOGD(TAG,"light %p",light);
        bool state;
        light->make_call().set_state(true).perform();
        break;
#endif
    }

#ifdef USE_CLIMATE
#endif
}

void WemoWrapper::off()
{
    switch(type_)
    {
#ifdef USE_SWITCH
    case WEMO_SWITCH:
    {
        auto sw=static_cast<switch_::Switch*>(device_);
        ESP_LOGD(TAG,"switch? %p",sw);
        sw->turn_off();
        break;
    }
#endif
#ifdef USE_LIGHT
    case WEMO_LIGHT:
        auto light=static_cast<light::LightState*>(device_);
        ESP_LOGD(TAG,"light %p",light);
        bool state;
        light->make_call().set_state(false).perform();
        break;
#endif
    }

#ifdef USE_CLIMATE
#endif
}

String WemoWrapper::serial()
{
    uint32_t uniqueSwitchId = device_->get_object_id_hash();
    char uuid[64];
    sprintf_P(uuid, PSTR("38323636-4558-cafe-9188-cda0e6%02x%02x%02x"),
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
        "LOCATION: http://" + String(s) + ":"+server_.get_port()+"/" + serial() + ".xml\r\n"
        "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
        "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
        "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
        "ST: urn:Belkin:device:**\r\n"
        "USN: uuid:" + persistent_uuid() + "::urn:Belkin:device:**\r\n"
        "X-User-Agent: redsonic\r\n\r\n";
    

    ESP_LOGD(TAG,"answering UDP :%s \n",response.c_str());
    UDP.beginPacket(senderIP, senderPort);
    UDP.print(response.c_str());
    UDP.endPacket();

 
}

void WemoWrapper::handleBody(AsyncWebServerRequest *req,uint8_t *data, size_t len, size_t index, size_t total)
{
    ESP_LOGD(TAG,"body: %s \n",req->url().c_str());
    if (req->url()=="/upnp/control/basicevent1")
    {

  std::string request;
  request.assign((const char*)data,len);

  ESP_LOGD(TAG,"body=%s\n",request.c_str());


  String response_xml = "";
  
  if(
      (request.find("<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">") != std::string::npos)
      and (request.find("<BinaryState>1</BinaryState>") != std::string::npos)
    ) {
      ESP_LOGD(TAG,"Got Turn on request");
      on();
      response_xml =  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                        "<s:Body>"
                          "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
                            "<BinaryState>1</BinaryState>"
                          "</u:SetBinaryStateResponse>"
                        "</s:Body>"
                      "</s:Envelope>\r\n"
                      "\r\n";
  }

  if(
      (request.find("<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">") != std::string::npos)
      and (request.find("<BinaryState>0</BinaryState>") != std::string::npos)
    ) {
      ESP_LOGD(TAG,"Got Turn off request");
      //offCallback();
      off();
      response_xml =  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                        "<s:Body>"
                          "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
                            "<BinaryState>0</BinaryState>"
                          "</u:SetBinaryStateResponse>"
                        "</s:Body>"
                      "</s:Envelope>\r\n"
                      "\r\n";
  }

  if(request.find("<u:GetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">") != std::string::npos) {
      ESP_LOGD(TAG,"Got GetStatus request");
      response_xml =  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                        "<s:Body>"
                          "<u:GetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
                            "<BinaryState>" + state() + "</BinaryState>"
                          "</u:GetBinaryStateResponse>"
                        "</s:Body>"
                      "</s:Envelope>\r\n"
                      "\r\n";
  }


  AsyncResponseStream *stream = req->beginResponseStream("text/plain");
  stream->print(response_xml.c_str());
  req->send(stream);

          
    }
}


void WemoWrapper::handleRequest(AsyncWebServerRequest *req)
{
    ESP_LOGW(TAG,"req: %s\n",req->url().c_str());
    if (req->url()=="/eventservice.xml")
    {
        ESP_LOGE(TAG,"CAZZO: %s\n",req->url().c_str());
        ESP_LOGE(TAG," ########## Responding to eventservice.xml ... ########\n");
        AsyncResponseStream *stream = req->beginResponseStream("text/xml");
        
        stream->print(F("<scpd xmlns=\"urn:Belkin:service-1-0\">"
                "<actionList>"
                "<action>"
                    "<name>SetBinaryState</name>"
                    "<argumentList>"
                    "<argument>"
                        "<retval/>"
                        "<name>BinaryState</name>"
                        "<relatedStateVariable>BinaryState</relatedStateVariable>"
                        "<direction>in</direction>"
                    "</argument>"
                    "</argumentList>"
                "</action>"
                "</actionList>"
                    "<serviceStateTable>"
                    "<stateVariable sendEvents=\"yes\">"
                        "<name>BinaryState</name>"
                        "<dataType>Boolean</dataType>"
                        "<defaultValue>0</defaultValue>"
                    "</stateVariable>"
                    "<stateVariable sendEvents=\"yes\">"
                        "<name>level</name>"
                        "<dataType>string</dataType>"
                        "<defaultValue>0</defaultValue>"
                    "</stateVariable>"
                    "</serviceStateTable>"
                "</scpd>\r\n"
                "\r\n"));

            req->send(stream);
       return;
    }

    ESP_LOGD(TAG," ########## Responding to setup.xml ... ########\n");

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  AsyncResponseStream *stream = req->beginResponseStream("text/xml");



  
  stream->print(F("<?xml version=\"1.0\"?>"
         "<root xmlns=\"urn:Belkin:device-1-0\">"
           "<specVersion>"
           "<major>1</major>"
           "<minor>0</minor>"
           "</specVersion>"
           "<device>"
             "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
             "<friendlyName>"));
  stream->print(name());
  stream->print(F("</friendlyName>"
             "<manufacturer>Belkin International Inc.</manufacturer>"
             "<modelName>Emulated Socket</modelName>"
             "<modelNumber>3.1415</modelNumber>"
             "<manufacturerURL>http://www.belkin.com</manufacturerURL>"
             "<modelDescription>Belkin Plugin Socket 1.0</modelDescription>"
             "<modelURL>http://www.belkin.com/plugin/</modelURL>"
             "<UDN>uuid:"));
  stream->print(persistent_uuid().c_str());
             
  stream->print(F("</UDN>"
             "<serialNumber>"));
  stream->print(serial());
  stream->print(F("</serialNumber>"
             "<binaryState>0</binaryState>"
             "<serviceList>"
               "<service>"
                 "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                 "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                 "<controlURL>/upnp/control/basicevent1</controlURL>"
                 "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                 "<SCPDURL>/eventservice.xml</SCPDURL>"
               "</service>"
             "</serviceList>"
           "</device>"
         "</root>\r\n"
         "\r\n"));
/**/
    req->send(stream);

}

}
}
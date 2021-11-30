#include "wemo.h"
#include "wemo_wrapper.h"
#include <lwip/igmp.h>
#include "esphome/core/helpers.h"
#include <vector>
#include "esphome/core/log.h"


namespace esphome {
namespace wemo {

static ip4_addr_t ipMulti{static_cast<uint32_t>(IPAddress(239, 255, 255, 250))};
static const unsigned int portMulti = 1900;
static const char *const TAG = "wemo";


void WemoManager::setup(){
  ESP_LOGD(TAG,"Begin multicast ..\n");
  udp_ = make_unique<WiFiUDP>();
  ESP_LOGD(TAG,"created\n");
  if (!udp_->begin(portMulti)) {
    ESP_LOGD(TAG,"Connection failed\n");
    mark_failed();
    return;
  }
  ESP_LOGD(TAG,"begin\n");


  if (igmp_joingroup(IP4_ADDR_ANY4, &ipMulti)){
    ESP_LOGD(TAG,"Multicast join failed\n");
    mark_failed();
    return;
  }
  ESP_LOGD(TAG,"igmp_joingroup\n");
  this->web_->add_handler(this);

}


        
void WemoManager::add_all_devices() {
//  for(auto& entity : App.)
}



bool WemoManager::handleUpnpControl(WemoWrapper sw,AsyncWebServerRequest * req,__uint8_t * data,size_t len){
  ESP_LOGD(TAG,"upnp control: %s len=%d body=%p\n",req->url().c_str(),len,data);

  std::string request;
  request.assign((const char*)data,len);
  Serial.print("request:");
  Serial.println(request.c_str());

  Serial.println("Responding to Control request");

  String response_xml = "";
  
  if(
      (request.find("<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">") != std::string::npos)
      and (request.find("<BinaryState>1</BinaryState>") != std::string::npos)
    ) {
      Serial.println("Got Turn on request");
      sw.on();
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
      Serial.println("Got Turn off request");
      //offCallback();
      sw.off();
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
      Serial.println("Got GetStatus request");
      response_xml =  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
                        "<s:Body>"
                          "<u:GetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">"
                            "<BinaryState>" + sw.state() + "</BinaryState>"
                          "</u:GetBinaryStateResponse>"
                        "</s:Body>"
                      "</s:Envelope>\r\n"
                      "\r\n";
  }


  AsyncResponseStream *stream = req->beginResponseStream("text/plain");
  stream->print(response_xml.c_str());
  req->send(stream);

  
  Serial.print("Sending :");
  Serial.println(response_xml);
  return true;
}


bool WemoManager::web_callback(AsyncWebServerRequest * req,WemoWrapper device){
  
 ESP_LOGD(TAG," ########## Responding to setup.xml ... ########\n");

  IPAddress localIP = WiFi.localIP();
  char s[16];
  sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

  AsyncResponseStream *stream = req->beginResponseStream("text/xml");









stream->print(F("<?xml version=\"1.0\"?>\n<root xmlns=\"urn:Belkin:device-1-0\">\n \
          <specVersion>\n    <major>1</major>\n    <minor>0</minor>\n  </specVersion>\n\
           <device>\n<deviceType>urn:Belkin:device:bridge:1</deviceType>\n<friendlyName>WeMo\
          Link</friendlyName>\n    <manufacturer>Belkin International Inc.</manufacturer>\n\
             <manufacturerURL>http://www.belkin.com</manufacturerURL>\n    <modelDescription>Belkin\
          WeMo Wi-Fi to ZigBee Bridge</modelDescription>\n    <modelName>Bridge</modelName>\n\
             <modelNumber>1.0</modelNumber>\n    <modelURL>http://www.belkin.com/plugin/</modelURL>\n\
        <serialNumber>SERIALNUMBER</serialNumber>\n<UDN>uuid:Bridge-1_0-SERIALNUMBER</UDN>\n\
             <UPC>123456789</UPC>\n<macAddress>001122334455</macAddress>\n<firmwareVersion>WeMo_WW_2.00.11057.PVT-OWRT-Link</firmwareVersion>\n\
        <iconVersion>0|49153</iconVersion>\n<binaryState>0</binaryState>\n    <iconList>\n\
               <icon>\n        <mimetype>jpg</mimetype>\n        <width>100</width>\n\
                 <height>100</height>\n        <depth>100</depth>\n         <url>icon.jpg</url>\n\
               </icon>\n    </iconList>\n    <serviceList>\n      <service>\n    \
             <serviceType>urn:Belkin:service:WiFiSetup:1</serviceType>\n        <serviceId>urn:Belkin:serviceId:WiFiSetup1</serviceId>\n\
                 <controlURL>/upnp/control/WiFiSetup1</controlURL>\n        <eventSubURL>/upnp/event/WiFiSetup1</eventSubURL>\n\
                 <SCPDURL>/setupservice.xml</SCPDURL>\n      </service>\n      <service>\n\
                 <serviceType>urn:Belkin:service:timesync:1</serviceType>\n      \
           <serviceId>urn:Belkin:serviceId:timesync1</serviceId>\n        <controlURL>/upnp/control/timesync1</controlURL>\n\
                 <eventSubURL>/upnp/event/timesync1</eventSubURL>\n        <SCPDURL>/timesyncservice.xml</SCPDURL>\n\
               </service>\n      <service>\n        <serviceType>urn:Belkin:service:basicevent:1</serviceType>\n\
                 <serviceId>urn:Belkin:serviceId:basicevent1</serviceId>\n       \
          <controlURL>/upnp/control/basicevent1</controlURL>\n        <eventSubURL>/upnp/event/basicevent1</eventSubURL>\n\
                 <SCPDURL>/eventservice.xml</SCPDURL>\n      </service>\n      <service>\n\
                 <serviceType>urn:Belkin:service:firmwareupdate:1</serviceType>\n\
                 <serviceId>urn:Belkin:serviceId:firmwareupdate1</serviceId>\n   \
              <controlURL>/upnp/control/firmwareupdate1</controlURL>\n        <eventSubURL>/upnp/event/firmwareupdate1</eventSubURL>\n\
                 <SCPDURL>/firmwareupdate.xml</SCPDURL>\n      </service>\n      <service>\n\
                 <serviceType>urn:Belkin:service:rules:1</serviceType>\n        <serviceId>urn:Belkin:serviceId:rules1</serviceId>\n\
                 <controlURL>/upnp/control/rules1</controlURL>\n        <eventSubURL>/upnp/event/rules1</eventSubURL>\n\
                 <SCPDURL>/bridgerules.xml</SCPDURL>\n      </service>\n\n      <service>\n\
                 <serviceType>urn:Belkin:service:metainfo:1</serviceType>\n      \
           <serviceId>urn:Belkin:serviceId:metainfo1</serviceId>\n        <controlURL>/upnp/control/metainfo1</controlURL>\n\
                 <eventSubURL>/upnp/event/metainfo1</eventSubURL>\n        <SCPDURL>/metainfoservice.xml</SCPDURL>\n\
               </service>\n\n      <service>\n        <serviceType>urn:Belkin:service:remoteaccess:1</serviceType>\n\
                 <serviceId>urn:Belkin:serviceId:remoteaccess1</serviceId>\n     \
            <controlURL>/upnp/control/remoteaccess1</controlURL>\n        <eventSubURL>/upnp/event/remoteaccess1</eventSubURL>\n\
                 <SCPDURL>/remoteaccess.xml</SCPDURL>\n      </service>\n\n      <service>\n\
                 <serviceType>urn:Belkin:service:deviceinfo:1</serviceType>\n    \
             <serviceId>urn:Belkin:serviceId:deviceinfo1</serviceId>\n        <controlURL>/upnp/control/deviceinfo1</controlURL>\n\
                 <eventSubURL>/upnp/event/deviceinfo1</eventSubURL>\n        <SCPDURL>/deviceinfoservice.xml</SCPDURL>\n\
               </service>\n\n      <service>\n        <serviceType>urn:Belkin:service:bridge:1</serviceType>\n\
                 <serviceId>urn:Belkin:serviceId:bridge1</serviceId>\n        <controlURL>/upnp/control/bridge1</controlURL>\n\
                 <eventSubURL>/upnp/event/bridge1</eventSubURL>\n        <SCPDURL>/bridgeservice.xml</SCPDURL>\n\
               </service>\n\n      <service>\n        <serviceType>urn:Belkin:service:manufacture:1</serviceType>\n\
                 <serviceId>urn:Belkin:serviceId:manufacture1</serviceId>\n      \
           <controlURL>/upnp/control/manufacture1</controlURL>\n        <eventSubURL>/upnp/event/manufacture1</eventSubURL>\n\
                 <SCPDURL>/manufacture.xml</SCPDURL>\n      </service>\n\n    </serviceList>\n\
            <presentationURL>/pluginpres.html</presentationURL>\n</device>\n</root>\n"));










  /*
  stream->print(F("<?xml version=\"1.0\"?>"
         "<root xmlns=\"urn:Belkin:device-1-0\">"
           "<specVersion>"
           "<major>1</major>"
           "<minor>0</minor>"
           "</specVersion>"
           "<device>"
             "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
             "<friendlyName>"));
  stream->print(device.name());
  stream->print(F("</friendlyName>"
             "<manufacturer>Belkin International Inc.</manufacturer>"
             "<modelName>Emulated Socket</modelName>"
             "<modelNumber>3.1415</modelNumber>"
             "<manufacturerURL>http://www.belkin.com</manufacturerURL>"
             "<modelDescription>Belkin Plugin Socket 1.0</modelDescription>"
             "<modelURL>http://www.belkin.com/plugin/</modelURL>"
             "<UDN>uuid:"));
  stream->print(device.persistent_uuid().c_str());
             
  stream->print(F("</UDN>"
             "<serialNumber>"));
  stream->print(device.serial());
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
*/
    req->send(stream);
    return true;
}





bool WemoManager::handleEventservice(AsyncWebServerRequest * req){
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
    return true;
}
/*void WemoManager::set_web_server(web_server::WebServer *web){
  this->web_=web;
  web->addCustomHandler("/eventservice.xml",handleEventservice);
  web->addCustomHandler("/upnp/control/basicevent1",[this](AsyncWebServerRequest * req){return handleUpnpControl(req);});
  
  for (auto device:devices)
    web->addCustomHandler("/"+device.serial()+ ".xml",[this,device](AsyncWebServerRequest * req){return web_callback(req,device);});
}*/



void WemoManager::add_device(WemoWrapper device) {
  ESP_LOGD(TAG,device.name());
  devices.push_back(device);

  //this->web_->addCustomHandler("/"+device.serial()+ ".xml",[this,device](AsyncWebServerRequest * req){return web_callback(req,device);});

  ESP_LOGD(TAG," index : %d",devices.size());
}

void WemoManager::handleBody(AsyncWebServerRequest *req,uint8_t *data, size_t len, size_t index, size_t total)
{
    ESP_LOGD(TAG,"body: %s \n",req->url().c_str());
    if (req->url()=="/upnp/control/basicevent1")
    {
        handleUpnpControl(devices.front(),req,data,len);
    }
    
      
}
void WemoManager::handleRequest(AsyncWebServerRequest *req)
{
    ESP_LOGD(TAG,"req: %s\n",req->url().c_str());
    for(auto& sw:devices) {
      if (req->url()=="/"+sw.serial()+".xml")
      {
          web_callback(req,sw);
      }
    }
    if (req->url()=="/eventservice.xml")
      handleEventservice(req);
    

}
void WemoManager::loop(){
  int packetSize = udp_->parsePacket();
  char packetBuffer[packetSize];
  if (packetSize > 0)
  {
    IPAddress senderIP = udp_->remoteIP();
    unsigned int senderPort = udp_->remotePort();

    // read the packet into the buffer
    udp_->read(packetBuffer, packetSize);

    // check if this is a M-SEARCH for WeMo device
    String request = String((char *)packetBuffer);
     //ESP_LOGD(TAG,"----------\n");
     //Serial.println(request);
     //ESP_LOGD(TAG,"-----------\n");
    if(request.indexOf("M-SEARCH") >= 0) {
        if((request.indexOf("urn:Belkin:device:**") > 0) || (request.indexOf("ssdp:all") > 0) || (request.indexOf("upnp:rootdevice") > 0)) {
          ESP_LOGD(TAG,"Got UDP Belkin Request..\n");

          // int arrSize = sizeof(switchs) / sizeof(Switch);
          for(auto& sw:devices) {
             sw.respondToSearch(senderIP, senderPort);
          }
        }
    }
  }

  // for(int i = 0; i < numOfSwitchs; i++) {
  //     WemoSwitch &swit = *switches[i];

  //     if (&swit != NULL) {
  //       swit.serverLoop();
  //     }
  // }
}
}
}

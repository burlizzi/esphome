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
  //this->web_->add_handler(this);

  for(auto& sw:devices) {
      sw.init();
  }
        server_.set_port(80);
        server_.add_handler(this);
        server_.init();

}



bool WemoManager::handleUpnpControl(WemoWrapper sw,AsyncWebServerRequest * req,__uint8_t * data,size_t len){
  ESP_LOGD(TAG,"upnp control: %s len=%d body=%p\n",req->url().c_str(),len,data);

  std::string request;
  request.assign((const char*)data,len);

  ESP_LOGD(TAG,"body=%s\n",request.c_str());


  String response_xml = "";
  
  if(
      (request.find("<u:SetBinaryState xmlns:u=\"urn:Belkin:service:basicevent:1\">") != std::string::npos)
      and (request.find("<BinaryState>1</BinaryState>") != std::string::npos)
    ) {
      ESP_LOGD(TAG,"Got Turn on request");
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
      ESP_LOGD(TAG,"Got Turn off request");
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






void WemoManager::add_device(WemoWrapper device,uint16_t port) {
  ESP_LOGD(TAG,device.name());
  device.set_port(port);
  devices.push_back(device);

  //this->web_->addCustomHandler("/"+device.serial()+ ".xml",[this,device](AsyncWebServerRequest * req){return web_callback(req,device);});

  ESP_LOGD(TAG," index : %d",devices.size());
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

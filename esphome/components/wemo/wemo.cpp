#include "wemo.h"
#include <lwip/igmp.h>
#include "esphome/core/helpers.h"

namespace esphome {



ip4_addr_t ipMulti{static_cast<uint32_t>(IPAddress(239, 255, 255, 250))};
const unsigned int portMulti = 1900;

char packetBuffer[512];

#define MAX_SWITCHES 14
esphome::EntityBase* switches[MAX_SWITCHES] = {};
int numOfSwitchs = 0;

static const char *const TAG = "wemo";





void WemoManager::setup(){
  ESP_LOGD(TAG,"Begin multicast ..\n");
  udp_ = make_unique<WiFiUDP>();

  if (!udp_->begin(portMulti)) {
    ESP_LOGD(TAG,"Connection failed\n");
    mark_failed();
    return;
  }


  if (!igmp_joingroup(IP4_ADDR_ANY4, &ipMulti)){
    ESP_LOGD(TAG,"Multicast join failed\n");
    mark_failed();
    return;
  }

}


        
void WemoManager::add_all_devices() {
//  for(auto& entity : App.)
}

void WemoManager::add_device(esphome::EntityBase* device,std::string type) {
  ESP_LOGD(TAG,"Adding switch : ");
  ESP_LOGD(TAG,device->get_name().c_str());
  ESP_LOGD(TAG," index : ");
  Serial.println(numOfSwitchs);

  switches[numOfSwitchs] = device;
  numOfSwitchs++;
}

void WemoManager::loop(){

  int packetSize = udp_->parsePacket();
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

          for(int n = 0; n < numOfSwitchs; n++) {
              esphome::EntityBase &sw = *switches[n];

              if (&sw != NULL) {
                //sw.respondToSearch(senderIP, senderPort);
              }
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
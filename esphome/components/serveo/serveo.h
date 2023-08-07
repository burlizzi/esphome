#include "soc/rtc_wdt.h"
#include "esphome.h"
#include "esphome/core/controller.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"

#include <libssh/libssh.h>
#include "libssh_esp32.h"
#include "WiFi.h"

namespace esphome {
namespace serveo {
class Serveo : public Component {
 public:
  bool joined;
  const char *privkey;
  const char *host;
  int port;
  ssh_session session;
  ssh_channel channel;
  
  char buffer[2560];
  Serveo() : joined(false), session(NULL), channel(NULL) {}
  

  int authenticate_console(ssh_session session) {
    int rc;
    int method;
    char *banner;

    // Try to authenticate
    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR) {
      return rc;
    }

    method = ssh_userauth_list(session, NULL);
    while (rc != SSH_AUTH_SUCCESS) {
      ssh_key key = NULL;

      ssh_pki_import_privkey_base64(privkey, NULL, NULL, NULL, &key);
      rc = ssh_userauth_try_publickey(session, NULL, key);

      ssh_key_free(key);
      // Try to authenticate with keyboard interactive";
      if (method & SSH_AUTH_METHOD_INTERACTIVE) {
        rc = ssh_userauth_kbdint(session, NULL, NULL);
        if (rc == SSH_AUTH_ERROR) {
          return rc;
        } else if (rc == SSH_AUTH_SUCCESS) {
          break;
        }
      }
    }

    return rc;
  }

  void setup() override {}
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  void connect()  {
    disableLoopWDT();
    libssh_begin();
    session = ssh_new();
    if (session == NULL)
      ESP_LOGI("px", "NULL session");

    if (ssh_options_set(session, SSH_OPTIONS_HOST, "serveo.net") < 0) {
      ESP_LOGD("ssh", "SSH_OPTIONS_HOST");
      ssh_disconnect(session);
      ssh_free(session);
      return;
    }

    if (ssh_connect(session)) {
      ESP_LOGD("ssh", "cannot connect");
      ssh_disconnect(session);
      ssh_free(session);
      return;
    }
    ESP_LOGD("ssh", "autenticate\n\n");
    if (authenticate_console(session))
      ESP_LOGD("ssh", "cannot autenticate\n\n");

    ESP_LOGD("ssh", "ssh_channel_listen_forward");
    // ssh_set_log_level(SSH_LOG_DEBUG);
    auto rc = ssh_channel_listen_forward(session, host, 80, NULL);
    if (rc != SSH_OK) {
      ESP_LOGD("ssh", "Error opening remote port: %s\n", ssh_get_error(session));
      return;
    }
    // ssh_set_log_level(SSH_LOG_NONE);
  
    ESP_LOGD("ssh", "connected to host: %s.serveo.net", host);
    
    // uint32_t* dev=(uint32_t*)DevEUI;
    // rtc = global_preferences->make_preference<sLoRa_Session>(RESTORE_STATE_VERSION^dev[0]^dev[1],true);
    // start();
  }

  void start() {}

  void reset() {
    //              *(uint32_t*)Session_Data.DevAddr=0;
    // rtc.save(&Session_Data);
  }

  void setKey(const char *key) { this->privkey = key; }
  void setHost(const char *host) { this->host = host; }
  void setPort(int port) { this->port = port; }

  void loop() override {
    if (session==NULL)
      connect();
    if (ssh_get_status(session) == SSH_CLOSED)
    {
      ESP_LOGD("ssh","restart connection");
      connect();
    }
    if (ssh_channel_is_closed(channel))
      channel = ssh_channel_accept_forward(session, 0, &port);
    if (channel == NULL){
      return;
    }

    if (!ssh_channel_is_eof(channel)){
      ESP_LOGD("ssh","read\n");
      
      auto nbytes = ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 0,0);
      ESP_LOGD("ssh","read %d\n",nbytes);
      if (nbytes <= 0)
      {
        return;
      }
      WiFiClient client;
       if (!client.connect("127.0.0.1", 80)) {
        Serial.println("Connection failed.");
        
        return;
      }
      Serial.println("Connected.");
      client.write(buffer,nbytes);
        //Serial.println(buffer);
      int maxloops = 0;
      while (!client.available() && maxloops < 1000)
      {
        maxloops++;
        delay(1); //delay 1 msec
      }  
      while (client.available() > 0)
      {

        //read back one line from the server
        nbytes = client.read((unsigned char*)buffer, sizeof(buffer));
        //Serial.println(buffer);
         ESP_LOGD("ssh","ssh_channel_write\n");
        auto nwritten = ssh_channel_write(channel, buffer, nbytes);
        if (nwritten != nbytes)
        {
          ESP_LOGD("ssh", "Error sending answer: %s\n",
                  ssh_get_error(session));
          ssh_channel_send_eof(channel);
          ssh_channel_free(channel);
          return;
        }
      }
      client.stop();
      ESP_LOGD("ssh","Sent answer\n");/**/
      channel=NULL;
    }
  }

  Trigger<int, int, std::string> *data_trigger() {
    if (!this->data_trigger_)
      this->data_trigger_ = make_unique<Trigger<int, int, std::string>>();
    return this->data_trigger_.get();
  }

 protected:
  std::unique_ptr<Trigger<int, int, std::string>> data_trigger_{nullptr};
  static const uint32_t RESTORE_STATE_VERSION = 0x2a5a2c13UL;
  ESPPreferenceObject rtc;
};
}  // namespace serveo
}  // namespace esphome

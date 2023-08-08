#pragma once
#include "soc/rtc_wdt.h"
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
  void connect();
  void start() {}
  void reset() {}

  void setKey(const char *key) { this->privkey = key; }
  void setHost(const char *host) { this->host = host; }
  void setPort(int port) { this->port = port; }

  void loop() override;
}

Trigger<int, int, std::string>
    *data_trigger() {
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

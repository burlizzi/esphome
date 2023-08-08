#include "serveo.h"

namespace esphome {
namespace serveo {
void Serveo::connect() {
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

void Serveo::loop() {
  if (session == NULL)
    connect();
  if (ssh_get_status(session) == SSH_CLOSED) {
    ESP_LOGD("ssh", "restart connection");
    connect();
  }
  if (ssh_channel_is_closed(channel))
    channel = ssh_channel_accept_forward(session, 0, &port);
  if (channel == NULL) {
    return;
  }

  if (!ssh_channel_is_eof(channel)) {
    ESP_LOGD("ssh", "read\n");

    auto nbytes = ssh_channel_read_timeout(channel, buffer, sizeof(buffer), 0, 0);
    ESP_LOGD("ssh", "read %d\n", nbytes);
    if (nbytes <= 0) {
      return;
    }
    WiFiClient client;
    if (!client.connect("127.0.0.1", 80)) {
      Serial.println("Connection failed.");

      return;
    }
    Serial.println("Connected.");
    client.write(buffer, nbytes);
    // Serial.println(buffer);
    int maxloops = 0;
    while (!client.available() && maxloops < 1000) {
      maxloops++;
      delay(1);  // delay 1 msec
    }
    while (client.available() > 0) {
      // read back one line from the server
      nbytes = client.read((unsigned char *) buffer, sizeof(buffer));
      // Serial.println(buffer);
      ESP_LOGD("ssh", "ssh_channel_write\n");
      auto nwritten = ssh_channel_write(channel, buffer, nbytes);
      if (nwritten != nbytes) {
        ESP_LOGD("ssh", "Error sending answer: %s\n", ssh_get_error(session));
        ssh_channel_send_eof(channel);
        ssh_channel_free(channel);
        return;
      }
    }
    client.stop();
    ESP_LOGD("ssh", "Sent answer\n"); /**/
    channel = NULL;
  }
}

}  // namespace serveo
}  // namespace esphome

#include "lacrosse.h"

namespace esphome {
namespace lacrosse_tx {
static const char *const TAG = "lacrosse";

bool LacrosseSensor::on_receive(remote_base::RemoteReceiveData data) {
  if (data.size() != 88) {
    ESP_LOGVV(TAG, "size fail %d", data.size());
    return false;
  }
  uint64_t decoded_code = 0;
  for (size_t i = 0; i < 44; i++) {
    char bit = 0;
    if (data.peek_mark(550))
      bit = 1;
    else if (data.peek_mark(1400))
      bit = 0;
    else
      return false;

    data.advance();
    if (i != 43 && !data.expect_space(1000))
      return false;
    decoded_code <<= 1;
    decoded_code |= bit;
  }
  int type = decoded_code >> 32;
  int v1 = (decoded_code >> 12) & 0xf;
  int v2 = (decoded_code >> 16) & 0xf;
  int v3 = (decoded_code >> 20) & 0xf;
  int id = (decoded_code >> 25) & 0xff;

  if (v2 != ((decoded_code >> 4) & 0xf))
    return false;
  ;
  if (v3 != ((decoded_code >> 8) & 0xf))
    return false;
  ;

  // ESP_LOGD(TAG, "received from %d",id);
  if (id != getter_())
    return false;
  float value = ((float) v1) / 10 + v2 + v3 * 10;
  if (type == 0xa0 && temperature_) {
    ESP_LOGV(TAG, "temp decode=%llx %d %f", decoded_code, id, value - 50);
    temperature_->publish_state(value - 50);
  } else if (type == 0xae && humidity_) {
    ESP_LOGV(TAG, "hum decode=%llx %d %f", decoded_code, id, value);
    humidity_->publish_state(value);
  }
  return true;
};
}  // namespace lacrosse_tx
}  // namespace esphome

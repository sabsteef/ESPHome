#include "racon_gateway.h"
#include "esphome/core/log.h"
#include "esphome/components/mqtt/mqtt_client.h"
#include <sstream>
#include <iomanip>

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";
static const char send_string[] = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";
static const char *mqtt_topic_parsed_data = "esp32/parsed_data";

std::vector<int> byte_to_bit(uint8_t x) {
  std::vector<int> bits(8);
  for (int i = 0; i < 8; ++i) {
    bits[7 - i] = (x >> i) & 1;
  }
  return bits;
}

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  this->set_interval("send_and_read_data", 10000, [this]() {
    this->send_and_read_data();
  });
}

std::string RaconGateway::parse_data(const std::string &data) {
  std::ostringstream oss;
  oss << "{";

  for (const auto& field : this->datamap) {
    if (field.offset >= data.size()) continue;

    int raw_value = static_cast<uint8_t>(data[field.offset]);
    float transformed_value = field.transform(raw_value);

    oss << "\"" << field.name << "\": " << std::fixed << std::setprecision(2) << transformed_value << ",";
  }

  std::string result = oss.str();
  result.pop_back();
  result += "}";

  return result;
}

void RaconGateway::send_and_read_data() {
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  std::string data;
  while (this->available()) {
    data += static_cast<char>(this->read());
  }

  if (!data.empty()) {
    std::string parsed_data = parse_data(data);
    ESP_LOGI(TAG, "Parsed Data: %s", parsed_data.c_str());

    if (mqtt::global_mqtt_client != nullptr) {
      ESP_LOGI(TAG, "Publishing parsed data to MQTT topic: %s", mqtt_topic_parsed_data);
      mqtt::global_mqtt_client->publish(mqtt_topic_parsed_data, parsed_data.c_str());
    } else {
      ESP_LOGW(TAG, "MQTT client not available, data not published");
    }
  }
}

}  // namespace racon_gateway
}  // namespace esphome

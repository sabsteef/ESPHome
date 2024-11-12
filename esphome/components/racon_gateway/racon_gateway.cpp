#include "racon_gateway.h"
#include "esphome/core/log.h"
#include "esphome/components/mqtt/mqtt_client.h"  // Include MQTT client

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";
static const char send_string[] = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";
static const char *mqtt_topic = "esp32/parsed_data";  // Set your desired MQTT topic

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  // Schedule a repeating task every 10 seconds (10000 ms)
  this->set_interval("send_and_read_data", 10000, [this]() {
    this->send_and_read_data();
  });
}

void RaconGateway::send_and_read_data() {
  // Write the specified string to UART
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  // Read incoming data from UART
  std::string data;
  while (this->available()) {
    data += static_cast<char>(this->read());
  }

  // Parse and log data if available
  if (!data.empty()) {
    std::string parsed_data = parse_data(data);
    ESP_LOGI(TAG, "Parsed Data: %s", parsed_data.c_str());

    // Publish parsed data to MQTT
    if (mqtt::global_mqtt_client != nullptr) {
      ESP_LOGI(TAG, "Publishing parsed data to MQTT topic: %s", mqtt_topic);
      mqtt::global_mqtt_client->publish(mqtt_topic, parsed_data.c_str());
    } else {
      ESP_LOGW(TAG, "MQTT client not available, data not published");
    }
  }
}

std::string RaconGateway::parse_data(const std::string &data) {
  // Example data parsing logic; modify this to match your needs
  std::string result = "parsed_data:";
  for (unsigned char c : data) {
    char hex[4];
    snprintf(hex, sizeof(hex), " %02X", c);
    result += hex;
  }
  return result;
}

}  // namespace racon_gateway
}  // namespace esphome

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

// Convert byte to bit array
std::vector<int> byte_to_bit(uint8_t x) {
  std::vector<int> bits(8);
  for (int i = 0; i < 8; ++i) {
    bits[7 - i] = (x >> i) & 1;
  }
  return bits;
}

// Initialize the datamap
void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  // Define fields based on your Python datamap
  this->datamap = {
    {5, [](int x) { return x * 0.01f; }, "aanvoer_temp"},
    {7, [](int x) { return x * 0.01f; }, "retour_temp"},
    {9, [](int x) { return x * 0.01f; }, "zonneboiler_temp"},
    {11, [](int x) { return x * 0.01f; }, "buiten_temp"},
    {13, [](int x) { return x * 0.01f; }, "boiler_temp"},
    {15, [](int x) { return static_cast<float>(x); }, "dunno1"},
    // Add additional fields here, following the same pattern
  };

  // Schedule a repeating task every 10 seconds
  this->set_interval("send_and_read_data", 10000, [this]() {
    this->send_and_read_data();
  });
}

// Parse the data according to datamap and convert to JSON string
std::string RaconGateway::parse_data(const std::string &data) {
  std::ostringstream oss;
  oss << "{";

  for (const auto& field : this->datamap) {
    // Ensure we are within data bounds
    if (field.offset >= data.size()) continue;

    int raw_value = static_cast<uint8_t>(data[field.offset]);
    float transformed_value = field.transform(raw_value);

    // Append to JSON string
    oss << "\"" << field.name << "\": " << std::fixed << std::setprecision(2) << transformed_value << ",";
  }

  std::string result = oss.str();
  result.pop_back();  // Remove trailing comma
  result += "}";

  return result;
}

// Read data from UART, parse it, and publish to MQTT
void RaconGateway::send_and_read_data() {
  // Write the specified string to UART
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  // Read incoming data from UART
  std::string data;
  while (this->available()) {
    data += static_cast<char>(this->read());
  }

  // Parse and publish if data is available
  if (!data.empty()) {
    std::string parsed_data = parse_data(data);
    ESP_LOGI(TAG, "Parsed Data: %s", parsed_data.c_str());

    // Publish parsed data to MQTT
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

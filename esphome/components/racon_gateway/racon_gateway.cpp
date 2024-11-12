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

// Define a struct for the parsed data according to the fmt string
struct ParsedData {
    int8_t start_byte;
    int16_t field1;
    int8_t field2;
    int8_t field3;
    int16_t field4;
    std::array<int16_t, 13> array_field;
    int8_t field5;
    int16_t field6;
    std::array<int8_t, 7> field7_array;
    std::array<uint8_t, 4> field8_array;
    std::array<int8_t, 11> field9_array;
    int16_t field10;
    int16_t field11;
    std::array<int8_t, 9> field12_array;
    int16_t field13;
    int8_t stop_byte;
};

// Parse the binary data into the ParsedData structure
ParsedData parse_binary_data(const std::vector<uint8_t>& data) {
    ParsedData parsed;
    int offset = 0;

    parsed.start_byte = data[offset++];
    parsed.field1 = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    parsed.field2 = data[offset++];
    parsed.field3 = data[offset++];
    parsed.field4 = data[offset] | (data[offset + 1] << 8);
    offset += 2;

    for (int i = 0; i < 13; ++i) {
        parsed.array_field[i] = data[offset] | (data[offset + 1] << 8);
        offset += 2;
    }

    parsed.field5 = data[offset++];
    parsed.field6 = data[offset] | (data[offset + 1] << 8);
    offset += 2;

    for (int i = 0; i < 7; ++i) {
        parsed.field7_array[i] = data[offset++];
    }

    for (int i = 0; i < 4; ++i) {
        parsed.field8_array[i] = data[offset++];
    }

    for (int i = 0; i < 11; ++i) {
        parsed.field9_array[i] = data[offset++];
    }

    parsed.field10 = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    parsed.field11 = data[offset] | (data[offset + 1] << 8);
    offset += 2;

    for (int i = 0; i < 9; ++i) {
        parsed.field12_array[i] = data[offset++];
    }

    parsed.field13 = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    parsed.stop_byte = data[offset++];

    return parsed;
}

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  // Schedule recurring task to send and read data every 10 seconds
  this->set_interval("send_and_read_data", 10000, [this]() {
    this->send_and_read_data();
  });
}

void RaconGateway::send_and_read_data() {
  // Write the specified string to UART
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  // Read incoming data from UART
  std::vector<uint8_t> data;
  while (this->available()) {
    data.push_back(this->read());
  }

  // Parse and publish if data is available and matches the expected length
  if (data.size() >= sizeof(ParsedData)) {
    ParsedData parsed = parse_binary_data(data);

    // Convert parsed data to JSON format for MQTT
    std::ostringstream oss;
    oss << "{";
    oss << "\"start_byte\": " << +parsed.start_byte << ",";
    oss << "\"field1\": " << parsed.field1 << ",";
    oss << "\"field2\": " << +parsed.field2 << ",";
    oss << "\"field3\": " << +parsed.field3 << ",";
    oss << "\"field4\": " << parsed.field4 << ",";
    // Additional fields go here based on parsed struct...
    oss << "\"stop_byte\": " << +parsed.stop_byte;
    oss << "}";

    std::string payload = oss.str();
    ESP_LOGI(TAG, "Parsed Data JSON: %s", payload.c_str());

    // Publish parsed data to MQTT
    if (mqtt::global_mqtt_client != nullptr) {
      ESP_LOGI(TAG, "Publishing parsed data to MQTT topic: %s", mqtt_topic_parsed_data);
      mqtt::global_mqtt_client->publish(mqtt_topic_parsed_data, payload.c_str());
    } else {
      ESP_LOGW(TAG, "MQTT client not available, data not published");
    }
  } else {
    ESP_LOGW(TAG, "Received data length is less than expected, data not parsed!");
  }
}

}  // namespace racon_gateway
}  // namespace esphome

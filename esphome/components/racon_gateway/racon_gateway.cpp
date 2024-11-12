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

// Hulpfunctie om een byte om te zetten naar een bit-array
std::vector<int> byte_to_bit(uint8_t x) {
  std::vector<int> bits(8);
  for (int i = 0; i < 8; ++i) {
    bits[7 - i] = (x >> i) & 1;
  }
  return bits;
}

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  // Schedule een herhalende taak om elke 10 seconden data te lezen en verwerken
  this->set_interval("send_and_read_data", 10000, [this]() {
    this->send_and_read_data();
  });
}

// Functie om data van de UART te lezen, te parsen en te publiceren naar MQTT
void RaconGateway::send_and_read_data() {
  // Stuur de voorgedefinieerde string naar UART
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);

  // Lees de binnenkomende data van UART
  std::string data;
  while (this->available()) {
    data += static_cast<char>(this->read());
  }

  // Parse de data en publiceer als deze beschikbaar is
  if (!data.empty()) {
    std::string parsed_data = parse_data(data);
    ESP_LOGI(TAG, "Parsed Data: %s", parsed_data.c_str());

    // Publiceer de geparste data naar MQTT
    if (mqtt::global_mqtt_client != nullptr) {
      ESP_LOGI(TAG, "Publishing parsed data to MQTT topic: %s", mqtt_topic_parsed_data);
      mqtt::global_mqtt_client->publish(mqtt_topic_parsed_data, parsed_data.c_str());
    } else {
      ESP_LOGW(TAG, "MQTT client not available, data not published");
    }
  }
}

// Functie om ruwe binaire data te parsen volgens de fmt-specificatie (implementatie van parse_binary_data)
ParsedData RaconGateway::parse_binary_data(const std::vector<uint8_t>& data) {
  ParsedData parsed_data;
  // Vul hier de parsed_data struct in volgens de benodigde specificatie
  // Bijvoorbeeld:
  // parsed_data.start_byte = data[0];
  // parsed_data.field1 = (data[1] << 8) | data[2]; // etc.
  return parsed_data;
}

}  // namespace racon_gateway
}  // namespace esphome

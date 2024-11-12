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

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");

  this->datamap = {
    {0, [](int x) { return x * 0.01f; }, "aanvoer_temp"},              // Aanvoer temp.
    {2, [](int x) { return x * 0.01f; }, "retour_temp"},               // Retour temp.
    {4, [](int x) { return x * 0.01f; }, "zonneboiler_temp"},          // Zonneboiler temp.
    {6, [](int x) { return x * 0.01f; }, "buiten_temp"},               // Buiten temp.
    {8, [](int x) { return x * 0.01f; }, "boiler_temp"},               // Boiler temp.
    {10, [](int x) { return static_cast<float>(x); }, "dunno1"},       // Placeholder voor onbekende data.
    {12, [](int x) { return x * 0.01f; }, "automaat_temp"},            // Automaat temp.
    {14, [](int x) { return x * 0.01f; }, "ruimte_temp"},              // Ruimte temp.
    {16, [](int x) { return x * 0.01f; }, "cv_setpoint"},              // CV Setpunt.
    {18, [](int x) { return x * 0.01f; }, "ww_setpoint"},              // WW Setpunt.
    {20, [](int x) { return x * 0.01f; }, "ruimte_setpoint"},          // Ruimte setpunt.
    {22, [](int x) { return static_cast<float>(x); }, "ventilator_setpoint"}, // Ventilator setpoint.
    {24, [](int x) { return static_cast<float>(x); }, "ventilator_toeren"},   // Ventilator toeren.
    {26, [](int x) { return x * 0.1f; }, "ionisatie_stroom"},          // Ionisatie stroom.
    {27, [](int x) { return x * 0.01f; }, "intern_setpoint"},          // Intern setpunt.
    {29, [](int x) { return static_cast<float>(x); }, "beschikbaar_vermogen"}, // Beschikbaar vermogen.
    {30, [](int x) { return static_cast<float>(x); }, "pomp"},         // Pomp.
    {31, [](int x) { return static_cast<float>(x); }, "dunno2"},       // Placeholder voor onbekende data.
    {32, [](int x) { return static_cast<float>(x); }, "gewenst_vermogen"},    // Gewenst vermogen.
    {33, [](int x) { return static_cast<float>(x); }, "geleverd_vermogen"},   // Geleverd vermogen.
    {34, [](int x) { return static_cast<float>(x); }, "dunno3"},       // Placeholder voor onbekende data.
    {35, [](int x) { return static_cast<float>(x); }, "dunno4"},       // Placeholder voor onbekende data.

    // Bitmanipulatie voor veld 36
    {36, [](int x) { return static_cast<float>(byte_to_bit(x ^ 0b00010000)[0]); }, "ww_warmtevraag"},  // WW warmtevraag (bitveld).
    {37, [](int x) { return static_cast<float>(byte_to_bit(x ^ 0b00000011)[0]); }, "ww_vrijgave"},     // WW vrijgave (bitveld).

    // Bitmap velden
    {38, [](int x) { return static_cast<float>(x); }, "bitmap3"},      // Bitmap veld.
    {39, [](int x) { return static_cast<float>(x); }, "bitmap4"},      // Bitmap veld.
    {40, [](int x) { return static_cast<float>(x); }, "status"},       // Status veld.
    {41, [](int x) { return static_cast<float>(x); }, "vergrendeling_e"}, // Vergrendeling E.
    {42, [](int x) { return static_cast<float>(x); }, "blokkering_b"}, // Blokkering B.
    {43, [](int x) { return static_cast<float>(x); }, "substatus"},    // Sub-status veld.
    {44, [](int x) { return static_cast<float>(x); }, "dunno5"},       // Placeholder voor onbekende data.
    {45, [](int x) { return static_cast<float>(x); }, "dunno6"},       // Placeholder voor onbekende data.
    {46, [](int x) { return static_cast<float>(x); }, "dunno7"},       // Placeholder voor onbekende data.
    {47, [](int x) { return static_cast<float>(x); }, "dunno8"},       // Placeholder voor onbekende data.
    {48, [](int x) { return static_cast<float>(x); }, "dunno9"},       // Placeholder voor onbekende data.
    {49, [](int x) { return x * 0.1f; }, "waterdruk"},                 // Waterdruk.
    {50, [](int x) { return static_cast<float>(x); }, "bitmap5"},      // Bitmap veld.
    {51, [](int x) { return x * 0.01f; }, "regel_temp"},               // Regel temp.
    {53, [](int x) { return x * 0.01f; }, "tapdebiet"},                // Tapdebiet.
    {60, [](int x) { return static_cast<float>(x); }, "ch_setpoint_hmi"}, // ch_setpoint_hmi.
    {62, [](int x) { return static_cast<float>(x); }, "service_mode"}, // Service mode.

    // Placeholder velden voor ontbrekende data aan het eind
    {63, [](int x) { return static_cast<float>(x); }, "dunno19"},
    {64, [](int x) { return static_cast<float>(x); }, "dunno29"},
    {65, [](int x) { return static_cast<float>(x); }, "dunno39"},
    {66, [](int x) { return static_cast<float>(x); }, "dunno49"},
    {67, [](int x) { return static_cast<float>(x); }, "dunno59"},
    {68, [](int x) { return static_cast<float>(x); }, "crc"},          // CRC veld.
    {69, [](int x) { return static_cast<float>(x); }, "stop"}          // Stop veld.
  };

  // Schedule een herhalende taak om elke 10 seconden data te lezen en verwerken
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

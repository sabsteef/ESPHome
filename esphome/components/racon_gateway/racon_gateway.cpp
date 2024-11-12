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

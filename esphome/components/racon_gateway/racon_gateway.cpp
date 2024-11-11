#include "racon_gateway.h"
#include "esphome/core/log.h"
#include <WiFi.h>          // Include WiFi support
#include <WiFiUdp.h>       // Include WiFi UDP support
#include <string>          // Standard string support

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Setting up Racon Gateway component...");
  udp.begin(this->udp_port);  // Initialize UDP with the specified port
}

void RaconGateway::loop() {
  static const char send_string[] = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";

  // Send the string over serial
  this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);
  delay(1000);

  // Read incoming serial data
  std::string data;
  while (this->available()) {
    data += static_cast<char>(this->read());
  }

  // Parse the data as required
  if (data.length() > 0) {
    // Dummy parsing function
    std::string parsed_data = parse_data(data);

    // Send parsed data via UDP
    udp.beginPacket(IPAddress(10, 3, 1, 127), this->udp_port);
    udp.write(reinterpret_cast<const uint8_t*>(parsed_data.c_str()), parsed_data.length());
    udp.endPacket();
  }
  delay(10000);  // 10-second delay similar to your Python code
}

// Example parsing function for data
std::string RaconGateway::parse_data(const std::string &data) {
  // Example data parsing logic
  return "parsed_data:" + data;  // For illustration only
}

}  // namespace racon_gateway
}  // namespace esphome

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include <WiFiUdp.h>
#include <vector>
#include <string>
#include <functional>

namespace esphome {
namespace racon_gateway {

// Helper to convert byte to bit array
std::vector<int> byte_to_bit(uint8_t x);

// Define a data field structure
struct DataField {
  int offset;
  std::function<float(int)> transform;
  const char* name;
};

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  explicit RaconGateway(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  void setup() override;
  void send_and_read_data();
  std::string parse_data(const std::string &data);

 private:
  std::vector<DataField> datamap;
};

}  // namespace racon_gateway
}  // namespace esphome

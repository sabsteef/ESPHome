#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <string>
#include <vector>
#include <functional>

namespace esphome {
namespace racon_gateway {

// Struct to define a data field in the datamap
struct DataField {
  int offset;  // Byte offset in the data array
  std::function<float(int)> transform;  // Transformation function for the raw data
  std::string name;  // Name of the data field
};

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  RaconGateway(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  void setup() override;
  void send_and_read_data();
  std::string parse_data(const std::string &data);

 private:
  std::vector<DataField> datamap;  // datamap to hold the data structure
};

}  // namespace racon_gateway
}  // namespace esphome

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"

namespace esphome {
namespace racon_gateway {

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  explicit RaconGateway(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}
  void setup() override;
  void loop() override;

 private:
  std::string parse_data(const std::string &data);  // Parse data from serial
};

}  // namespace racon_gateway
}  // namespace esphome

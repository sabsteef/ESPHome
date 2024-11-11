#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace racon_gateway {

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  RaconGateway(UARTComponent *parent) : UARTDevice(parent) {}
  void setup() override;
  void loop() override;

 private:
  WiFiUDP udp;
  String parse_data(const String &data);
};

}  // namespace racon_gateway
}  // namespace esphome

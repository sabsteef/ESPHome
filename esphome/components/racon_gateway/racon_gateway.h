#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace racon_gateway {

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
};

}  // namespace racon_gateway
}  // namespace esphome

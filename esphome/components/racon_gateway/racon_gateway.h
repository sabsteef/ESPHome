#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"  // Include text_sensor header
#include "esphome/core/log.h"

namespace esphome {
namespace racon_gateway {

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  explicit RaconGateway(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  void setup() override;
  void send_and_read_data();
  
  // Define a text sensor to hold the parsed data
  text_sensor::TextSensor *parsed_data_sensor = new text_sensor::TextSensor();

 private:
  std::string parse_data(const std::string &data);
};

}  // namespace racon_gateway
}  // namespace esphome

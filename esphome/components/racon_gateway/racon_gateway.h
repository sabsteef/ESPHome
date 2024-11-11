#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <WiFiUdp.h>           // Include WiFi UDP support
#include "esphome/core/log.h"   // Include ESPHome logging support

namespace esphome {
namespace racon_gateway {

class RaconGateway : public Component, public uart::UARTDevice {
 public:
   // Constructor with a UARTComponent pointer as a parameter
   RaconGateway(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

   // Setup and loop functions
   void setup() override;
   void loop() override;

   // Setter for UDP port
   void set_udp_port(int port) { this->udp_port = port; }

 private:
   int udp_port;
   WiFiUDP udp;  // WiFi UDP instance
   std::string parse_data(const std::string &data);  // Use std::string instead of Arduino String
};

}  // namespace racon_gateway
}  // namespace esphome

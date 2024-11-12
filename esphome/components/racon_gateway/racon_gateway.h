#pragma once

#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include <vector>
#include <string>
#include <array>
#include <functional>

namespace esphome {
namespace racon_gateway {

// Definieer de ParsedData struct volgens de fmt-specificatie
struct ParsedData {
    int8_t start_byte;
    int16_t field1;
    int8_t field2;
    int8_t field3;
    int16_t field4;
    std::array<int16_t, 13> array_field;
    int8_t field5;
    int16_t field6;
    std::array<int8_t, 7> field7_array;
    std::array<uint8_t, 4> field8_array;
    std::array<int8_t, 11> field9_array;
    int16_t field10;
    int16_t field11;
    std::array<int8_t, 9> field12_array;
    int16_t field13;
    int8_t stop_byte;
};

class RaconGateway : public Component, public uart::UARTDevice {
 public:
  RaconGateway(UARTComponent *parent) : UARTDevice(parent) {}

  // Voegt de setup functie toe voor initialisatie
  void setup() override;

  // Functie om data te verzenden en te lezen
  void send_and_read_data();

 private:
  // Functie om binnenkomende data te parsen en om te zetten naar ParsedData struct
  ParsedData parse_binary_data(const std::vector<uint8_t>& data);

  // Hulpmethode om de data te converteren naar een JSON string voor MQTT-publicatie
  std::string parse_data(const std::string &data);
};

}  // namespace racon_gateway
}  // namespace esphome

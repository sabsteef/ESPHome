#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include <WiFiUdp.h>

class RaconGateway : public Component, public UARTDevice {
 public:
  RaconGateway(UARTComponent *parent) : UARTDevice(parent) {}
  void setup() override {
    udp.begin(8125);  // UDP port
  }

  void loop() override {
    static const char send_string[] = "\x02\xFE\x01\x05\x08\x02\x01\x69\xAB\x03";

    // Send the string over serial
    this->write_array(reinterpret_cast<const uint8_t *>(send_string), sizeof(send_string) - 1);
    delay(1000);

    // Read incoming serial data
    String data;
    while (this->available()) {
      data += static_cast<char>(this->read());
    }

    // Parse the data as required
    if (data.length() > 0) {
      // Dummy parsing function
      String parsed_data = parse_data(data);

      // Send parsed data via UDP
      udp.beginPacket(IPAddress(10, 3, 1, 127), 8125);
      udp.write(parsed_data.c_str());
      udp.endPacket();
    }
    delay(10000);  // 10-second delay similar to your Python code
  }

 private:
  WiFiUDP udp;

  String parse_data(const String &data) {
    // Example data parsing logic
    // Replace this with your real parsing function based on `datamap`
    return "parsed_data:" + data;  // For illustration only
  }
};
